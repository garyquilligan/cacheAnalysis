#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <sstream>
#include <istream>
#include <string>
#include <iomanip>
#include "cache.h"
#include "tagSet.h"
using namespace std;

void createVector(vector<cache>& c,int numLines,int nWay);
void lruinsert(vector<cache>& c, int tag, int set, int& hitCount, int& counter, int nWay, int& missCount);
void fifoinsert(vector<cache>& c, int tag, int set, int& hitCount, int& counter, int nWay, int& missCount);
string decimalToBinary(long long dec);
long long binaryToDecimal(string bin);


int main(){
   vector<cache> cacheVect;
   
   int cacheSize = 0;
   int lineSize = 0;
   int nWay = 0;
   int hitCount = 0;
   int missCount = 0;
   int counter = 0;
   int lruOrFifo;
   cout << "How many bytes in the cache?";
   cin >> cacheSize;
   cout << "How many bytes in a block/line?";
   cin >> lineSize;
   cout << "How many lines in a set?";
   cin >> nWay;
   cout << "0 for LRU, 1 for FIFO: ";
   cin >> lruOrFifo;
   int numLines = cacheSize / lineSize;
   int numSets = numLines/nWay;
   createVector(cacheVect,numLines,nWay);
   string line;
   vector<string> hexStrings;
   ifstream myFile("../traceFiles/read03.trace");
   if(myFile.is_open()){
        while(getline(myFile,line)){
            string load;
            string hex;
            string bytes;
            stringstream parse(line);
            parse >> load >> hex >> bytes;
            hexStrings.push_back(hex);
        }
        myFile.close();
   }

   vector<long long> numbers;
   //hexStrings.push_back("end");
   
   for(int i =0; i < hexStrings.size();i++){
        hexStrings.at(i) = hexStrings.at(i).erase(0,2);
        if(i == hexStrings.size()-1){
            hexStrings.at(i).erase(8,1);
        }
        long long hexRep = stoll(hexStrings.at(i),nullptr,16);
        numbers.push_back(hexRep);
   }
   vector<string> binRep;
   
   for(int i = 0; i < numbers.size(); i++){
        binRep.push_back(decimalToBinary(numbers.at(i)));
   }

   vector<tagSet> t;

   if(numSets == 1){ //fully associative get tags
    for(int i = 0; i < binRep.size(); i++){
        string tag;
        for(int j = 0; j < binRep.at(i).size(); j++){
            if(j < (binRep.at(i).size()-log2(lineSize))){
                tag.push_back(binRep.at(i).at(j));
            }
        }
        long long tagINT = binaryToDecimal(tag);
        tagSet tt(tagINT,0);
        tag.clear();
        t.push_back(tt);
    }
   }

   else{
     for(int i = 0; i < binRep.size(); i++){//direct mapped/set associative get tags and sets
        string tag;
        string set;
        for(int j = 0; j < binRep.at(i).size(); j++){
            if(j < (binRep.at(i).size()-(log2(lineSize)+log2(numSets)))){
                tag.push_back(binRep.at(i).at(j));
            }
            if(j >= (binRep.at(i).size()-(log2(lineSize)+log2(numSets))) && j < (binRep.at(i).size()-log2(lineSize))){
                set.push_back(binRep.at(i).at(j));
            }
        }
            long long tagINT = binaryToDecimal(tag);
            long long setINT = binaryToDecimal(set);
            tagSet tt(tagINT,setINT);
            tag.clear();
            set.clear();
            t.push_back(tt);
     }
   }

   if(lruOrFifo == 0){
    for(int i = 0; i < t.size(); i++){
        lruinsert(cacheVect,t.at(i).tag,t.at(i).set,hitCount,counter,nWay,missCount);
    }
   }
   else if(lruOrFifo == 1){
    for(int i = 0; i < t.size(); i++){
        fifoinsert(cacheVect,t.at(i).tag,t.at(i).set,hitCount,counter,nWay,missCount);
    }
   }

  cout << "Hit count: "<< hitCount << endl;
  cout << "Miss count: " << missCount << endl;
  double hitRate = double(hitCount)/(hitCount+missCount);
  cout << "Hit rate: " << setprecision(5) << hitRate << endl;

   return 0; 
}

void createVector(vector<cache>& c,int numLines,int nWay){
    int setCount = 0;
    int countUpToSet = 0;
    for(int i = 0; i < numLines; i++){
        cache curr(setCount,-1,i,0);
        c.push_back(curr);
        countUpToSet++;
        if(countUpToSet == nWay){
            setCount++;
            countUpToSet = 0;
        }
    }
}

string decimalToBinary(long long dec){
    double temp = log2(dec);
    int power = temp;
    string bin;
    for(int i = power; i >= 0; i--){
        if(dec - pow(2,i) >= 0){
            dec = dec - pow(2,i);
            bin.push_back('1');
        }
        else{
            bin.push_back('0');
        }
    }
    return bin;
}

long long binaryToDecimal(string bin){
    int sum = 0;
    if(bin == ""){
        return sum;
    }

    for(int i = 0; i < bin.length(); i++){
        if(bin.at(i)== '1'){
            sum = sum + pow(2,bin.length()-1-i);
        }
    }
    return sum;
}

void lruinsert(vector<cache>& c, int tag, int set, int& hitCount, int& counter, int nWay, int& missCount){
    int loc = set*nWay;
    bool found = false;
    bool empty = false;
    for(int i = loc; i < (loc+nWay); i++){ //see if tag exists in set already
            if(c.at(i).tag == tag){
                hitCount++;
                counter++;
                found = true;
                c.at(i).counter = counter;
            }
    }
    
    

    if(!found){
        for(int i = loc; i < (loc+nWay); i++){ //if there is an empty space in the set put the tag there and mark a compulsory miss
            if(c.at(i).tag == -1){
                missCount++;
                counter++;
                c.at(i).tag = tag;
                c.at(i).counter = counter;
                empty = true;
            }
        }
    }

    int min = loc;

    if(!found && !empty){
        for(int i = loc; i < (loc+nWay); i++){ //if there is an empty space in the set put the tag there and mark a compulsory miss
            if(c.at(i).counter < c.at(min).counter){
                min = i;
            }
        }
        counter++;
        missCount++;
        c.at(min).tag = tag;
        c.at(min).counter = counter;
    }

}

void fifoinsert(vector<cache>& c, int tag, int set, int& hitCount, int& counter, int nWay, int& missCount){
    int loc = set*nWay;
    bool found = false;
    bool empty = false;
    for(int i = loc; i < (loc+nWay); i++){ //see if tag exists in set already
            if(c.at(i).tag == tag){
                hitCount++;
                found = true;
            }
    }
    
    

    if(!found){
        for(int i = loc; i < (loc+nWay); i++){ //if there is an empty space in the set put the tag there and mark a compulsory miss
            if(c.at(i).tag == -1){
                missCount++;
                counter++;
                c.at(i).tag = tag;
                c.at(i).counter = counter;
                empty = true;
            }
        }
    }

    int min = loc;

    if(!found && !empty){
        for(int i = loc; i < (loc+nWay); i++){ //if there is an empty space in the set put the tag there and mark a compulsory miss
            if(c.at(i).counter < c.at(min).counter){
                min = i;
            }
        }
        counter++;
        missCount++;
        c.at(min).tag = tag;
        c.at(min).counter = counter;
    }

}





