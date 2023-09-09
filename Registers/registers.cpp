#include "registers.hpp"

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

uint32_t Registers::getRegisterValue(int index)
{
    return registerArray[index];
}
