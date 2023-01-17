struct cache{
    int set;
    int tag;
    int line;
    int counter;
    cache();
    cache(int _set,int _tag,int _line, int _counter);
};