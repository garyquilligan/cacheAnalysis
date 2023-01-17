#include <iostream>
#include <vector>
#include "cache.h"
using namespace std;

cache::cache(){
    set = 0;
    tag = -1;
    line = 0;
    counter = 0;
}

cache::cache(int _set,int _tag,int _line, int _counter){
    set = _set;
    tag = _tag;
    line = _line;
    counter = _counter;
}