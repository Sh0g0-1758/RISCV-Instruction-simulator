#include "registers.hpp"
#include <iostream>
#include "../Tools/Color/Color.hpp"

const Color::Modifier red(Color::FG_RED);
const Color::Modifier def(Color::FG_DEFAULT);
const Color::Modifier green(Color::FG_GREEN);
const Color::Modifier blue(Color::FG_BLUE);
const Color::Modifier cyan(Color::FG_CYAN);

Registers::Registers()
{
    registerArray[0] = 0;
}

void Registers::setRegisterValue(int index, uint32_t value)
{
    if (index >= 0 && index < 32)
    {
        registerArray[index] = value;
    }
}

void Registers::setRegisterDirtyBit(int index, uint32_t value)
{
    if (index >= 0 && index < 32)
    {
        registerDirtyBit[index] = value;
    }
}

uint32_t Registers::getRegisterValue(int index)
{
    if (index >= 0 && index < 32) {
        return registerArray[index];
    } else {
        return 0;
    }
}

uint32_t Registers::getRegisterDirtyBit(int index)
{
    if (index >= 0 && index < 32) {
        return registerDirtyBit[index];
    } else {
        return 0;
    }
}

void Registers::PrintRegisters()
{
    for (size_t i = 0; i < registerArray.size(); ++i)
    {
        std::cout << cyan << "Value at Register " << i << " : " << registerArray[i] << def << std::endl;
    }
}