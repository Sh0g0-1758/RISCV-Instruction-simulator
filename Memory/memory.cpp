#include "memory.hpp"
#include <iostream>
#include "../Tools/Color/Color.hpp"

const Color::Modifier red(Color::FG_RED);
const Color::Modifier def(Color::FG_DEFAULT);
const Color::Modifier green(Color::FG_GREEN);
const Color::Modifier blue(Color::FG_BLUE);
const Color::Modifier cyan(Color::FG_CYAN);

Memory::Memory() {}

void Memory::addValue(int key, int value)
{
    memoryMap[key] = value;
}

int Memory::getValue(int key)
{
    return memoryMap[key];
}

void Memory::PrintMemory()
{
    for (const auto &pair : memoryMap)
    {
        std::cout << cyan << pair.first << " --- " << pair.second << def << std::endl;
    }
}