#ifndef REGISTERS_H
#define REGISTERS_H

#include <cstdint>
#include <array>

class Registers
{
public:
    Registers();

    void setRegisterValue(int index, uint32_t value);

    void setRegisterDirtyBit(int index, uint32_t value);

    uint32_t getRegisterValue(int index);

    uint32_t getRegisterDirtyBit(int index);

    void PrintRegisters();

private:
    std::array<uint32_t, 32> registerArray;
    std::array<uint32_t, 32> registerDirtyBit;
};

#endif
