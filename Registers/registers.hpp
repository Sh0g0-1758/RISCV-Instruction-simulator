#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstdint>
#include <array>

class Registers
{
public:
    Registers();

    void setRegisterValue(int index, uint32_t value);

    uint32_t getRegisterValue(int index);

    void PrintRegisters();

private:
    std::array<uint32_t, 32> registerArray;
};

#endif
