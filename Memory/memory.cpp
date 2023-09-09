#include "memory.hpp"

Memory::Memory() {}

void Memory::addValue(int key, int value) {
    memoryMap[key] = value;
}

int Memory::getValue(int key) {
    return memoryMap[key];
}
