#ifndef MEMORYHEADER_H
#define MEMORYHEADER_H

#include <map>

class Memory {
public:
    Memory(); 

    void addValue(int key, int value);

    int getValue(int key);

private:
    std::map<int, int> memoryMap; 
};

#endif