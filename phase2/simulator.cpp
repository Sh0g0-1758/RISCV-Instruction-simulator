#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <bitset>
#include "../Tools/Color/Color.hpp"
#include "../Memory/memory.hpp"
#include "../Registers/registers.hpp"
using namespace std;

const Color::Modifier red(Color::FG_RED);
const Color::Modifier def(Color::FG_DEFAULT);
const Color::Modifier green(Color::FG_GREEN);
const Color::Modifier blue(Color::FG_BLUE);
const Color::Modifier cyan(Color::FG_CYAN);

Memory virtual_memory;
Registers virtual_register;
string ControlWord;
bool ALU_ZERO_FLAG;
int ALURESULT;
int LDRESULT;
int BRANCH_PROGRAM_COUNTER;
/*

The control word consist of :
● RegRead
● ALUSrc
● ALUOp:
● MemWrite
● MemRead
● RegWrite
● Mem2Reg
● Branch and Jump

For Load Type Instructions : 1 1 00 0 1 1 1 0
For Store type Instructions : 1 1 00 1 0 0 0 0
For R-Type Instructions : 1 0 10 0 0 1 0 0
For Branch Type Instructions : 1 0 01 0 0 0 0 1
For I Type Instructions : 1 1 11 0 0 1 0 0
To do : Implement Jump type instructions :
For Jump (JAL) Type Instruction : 1 1

*/
void ControlUnit(string opcode)
{
    ControlWord = "";
    if (opcode == "0000011")
    { // opcode for load type instructions
        ControlWord = "110001110";
    }
    else if (opcode == "0100011")
    { // opcode for store instructions
        ControlWord = "110010000";
    }
    else if (opcode == "0110011")
    { // opcode for R-Type Instructions
        ControlWord = "101000100";
    }
    else if (opcode == "1100011")
    { // opcode for branch type instruction
        ControlWord = "100100001";
    }
    else if (opcode == "0010011")
    { // opcode for I type instructions
        ControlWord = "111100100";
    }
}

int find_immediate_value(string imm1, string imm2, string opcode)
{
    if (opcode == "0110011")
    { // R-Type Instruction
        return 0;
    }
    else if (opcode == "0000011")
    { // Load Type Instruction
        bitset<32> load(imm1);
        return static_cast<int>(load.to_ulong());
    }
    else if (opcode == "0100011")
    { // Store Type Instructions
        bitset<32> store(imm1.substr(0, 7) + imm2);
        return static_cast<int>(store.to_ulong());
    }
    else if (opcode == "1100011")
    { // Branch Type Instructions
        bool negative_flag = false;
        if (imm1[0] == '1')
        {
            negative_flag = true;
        }
        bitset<32> jump(imm2[4] + imm1.substr(1, 6) + imm2.substr(0, 4));
        int temp_number = static_cast<int>(jump.to_ulong());

        if (negative_flag)
        {
            return (0 - temp_number);
        }
        else
        {
            return temp_number;
        }
    }
    else if (opcode == "0010011")
    { // For I Type Instructions
        bool negative_flag = false;
        if (imm1[0] == '1')
        {
            negative_flag = true;
        }
        bitset<32> jump(imm1.substr(1));
        int temp_number = static_cast<int>(jump.to_ulong());
        if (negative_flag)
        {
            return (0 - temp_number);
        }
        else
        {
            return temp_number;
        }
    }
    return 0;
}

string alu_control(string ALUOP, string funct3, char funct7)
{
    if (ALUOP == "00")
    {
        return "0010";
    }
    else if (ALUOP == "01")
    {
        return "0110";
    }
    else if (ALUOP == "10")
    {
        if (funct7 == '0')
        {
            if (funct3 == "000")
            {
                return "0010";
            }
            else if (funct3 == "111")
            {
                return "0000";
            }
            else if (funct3 == "110")
            {
                return "0001";
            }
        }
        else if (funct7 == '1')
        {
            if (funct3 == "000")
            {
                return "0110";
            }
        }
    }
    else if (ALUOP == "11")
    {
        if (funct3 == "000")
        {
            return "0010";
        }
        else if (funct3 == "110")
        {
            return "0001";
        }
        else if (funct3 == "111")
        {
            return "0000";
        }
    }
    return "";
}

int do_alu_operation(string alu_select, int val1, int val2)
{
    if (alu_select == "0010")
    {
        return (val1 + val2);
    }
    else if (alu_select == "0110")
    {
        return (val1 - val2);
    }
    else if (alu_select == "0000")
    {
        return (val1 and val2);
    }
    else if (alu_select == "0001")
    {
        return (val1 or val2);
    }
    return 0;
}

int main()
{

    string filePath = "../phase1/part3/machine_encoding.txt";

    ifstream inputFile(filePath);

    if (!inputFile.is_open())
    {
        cerr << red << "Error: Could not open the file." << def << endl;
        return 1;
    }

    vector<string> instructionVector;

    while (inputFile)
    {
        char buffer[33];

        inputFile.read(buffer, 32);
        buffer[inputFile.gcount()] = '\0';
        instructionVector.push_back(buffer);
    }

    inputFile.close();
    instructionVector.pop_back();
    for (int program_counter = 0; program_counter < instructionVector.size(); program_counter++)
    {
        ControlUnit(instructionVector[program_counter].substr(25));
        if (ControlWord == "")
        {
            cout << red << "Error : INVALID INSTRUCTION" << def << endl;
            return 0;
        }
        int immediate_value = find_immediate_value(instructionVector[program_counter].substr(0, 12), instructionVector[program_counter].substr(20, 5), instructionVector[program_counter].substr(25));
        bitset<32> bitsetrs1(instructionVector[program_counter].substr(12, 5));
        int rs1 = static_cast<int>(bitsetrs1.to_ulong());
        bitset<32> bitsetrs2(instructionVector[program_counter].substr(7, 5));
        int rs2 = static_cast<int>(bitsetrs2.to_ulong());
        bitset<32> bitsetrd(instructionVector[program_counter].substr(20, 5));
        int rd = static_cast<int>(bitsetrd.to_ulong());
        int FirstValue;
        int SecondValue;
        if (ControlWord[0] == '1')
        {
            FirstValue = virtual_register.getRegisterValue(rs1);
        }
        if (ControlWord[1] == '0')
        {
            SecondValue = virtual_register.getRegisterValue(rs2);
        }
        else
        {
            SecondValue = immediate_value;
        }
        string ALU_SELECT = alu_control(ControlWord.substr(2, 2), instructionVector[program_counter].substr(17, 3), instructionVector[program_counter][1]);
        ALURESULT = do_alu_operation(ALU_SELECT, FirstValue, SecondValue);
        ALU_ZERO_FLAG = FirstValue == SecondValue;
        if (ControlWord[4] == '1')
        {
            virtual_memory.addValue(ALURESULT, virtual_register.getRegisterValue(rs2));
        }
        if (ControlWord[5] == '1')
        {
            LDRESULT = virtual_memory.getValue(ALURESULT);
        }
        BRANCH_PROGRAM_COUNTER = ((immediate_value) / 4) + program_counter;
        if (ControlWord[8] == '1' && ALU_ZERO_FLAG)
        {
            program_counter = BRANCH_PROGRAM_COUNTER - 1;
        }
        if (program_counter < 0 || program_counter >= instructionVector.size())
        {
            cout << red << "ERROR : Invalid Branch/Jump Address" << def << endl;
        }
        if (ControlWord[6] == '1')
        {
            if (ControlWord[7] == '1')
            {
                virtual_register.setRegisterValue(rd, LDRESULT);
            }
            else
            {
                virtual_register.setRegisterValue(rd, ALURESULT);
            }
        }
    }
    cout << green << "SUCCESS : SIMULATOR RAN SUCCESSFULLY" << def << endl;
    cout << endl;
    cout << red << "MEMORY : " << def << endl;
    cout << endl;
    cout << blue << "ADDRESS"
         << " --- "
         << "DECIMAL VALUE" << def << endl;
    virtual_memory.PrintMemory();
    cout << endl;
    cout << red << "REGISTERS : " << def << endl;
    cout << endl;
    virtual_register.PrintRegisters();
    return 0;
}
