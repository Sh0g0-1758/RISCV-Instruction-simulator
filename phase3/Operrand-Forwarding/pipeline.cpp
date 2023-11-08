// include statements
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <bitset>
#include "../../Tools/Color/Color.hpp"
#include "../../Memory/memory.hpp"
#include "../../Registers/registers.hpp"

// namespaces
using namespace std;

// colored output
const Color::Modifier red(Color::FG_RED);
const Color::Modifier def(Color::FG_DEFAULT);
const Color::Modifier green(Color::FG_GREEN);
const Color::Modifier blue(Color::FG_BLUE);
const Color::Modifier cyan(Color::FG_CYAN);

// instruction execution
Memory virtual_memory;
Registers virtual_register;

// ===================================================================================
// ======================     Intermediate Pipeline classes     ======================
// ===================================================================================

class IFID
{
public:
    int DPC = 0;
    std::string IR = "";
    bool is_empty = true;

    void change_DPC(int val)
    {
        DPC = val;
    }
    void change_IR(std::string val)
    {
        IR = val;
    }
    void change_is_empty(bool val)
    {
        is_empty = val;
    }
};

class IDEX
{
public:
    int J_DPC = 0;
    int IMM = 0;
    string CW = "";
    int RS1 = 0;
    int RS2 = 0;
    string FUNC = "";
    int RDI = 0;
    bool is_empty = true;

    void change_J_DPC(int val)
    {
        J_DPC = val;
    }
    void change_IMM(int val)
    {
        IMM = val;
    }
    void change_CW(string val)
    {
        CW = val;
    }
    void change_RS1(int val)
    {
        RS1 = val;
    }
    void change_RS2(int val)
    {
        RS2 = val;
    }
    void change_FUNC(std::string val)
    {
        FUNC = val;
    }
    void change_RDI(int val)
    {
        RDI = val;
    }
    void change_is_empty(bool val)
    {
        is_empty = val;
    }
};

class EXMO
{
public:
    string CW = "";
    int ALUOUT = 0;
    int RS2VAL = 0;
    int RDI = 0;
    bool is_empty = true;
    void change_CW(std::string val)
    {
        CW = val;
    }
    void change_ALUOUT(int val)
    {
        ALUOUT = val;
    }
    void change_RS2VAL(int val)
    {
        RS2VAL = val;
    }
    void change_RDI(int val)
    {
        RDI = val;
    }
    void change_is_empty(bool val)
    {
        is_empty = val;
    }
};

class MOWB
{
public:
    string CW = "";
    int LDOUT = 0;
    int ALUOUT = 0;
    int RDI = 0;
    bool is_empty = true;
    void change_RDI(int val)
    {
        RDI = val;
    }
    void change_ALUOUT(int val)
    {
        ALUOUT = val;
    }
    void change_CW(std::string val)
    {
        CW = val;
    }
    void change_LDOUT(int val)
    {
        LDOUT = val;
    }
    void change_is_empty(bool val)
    {
        is_empty = val;
    }
};

class OF
{
public:
    int data = 0;
    string source_opcode = "";
    int register_check = 0;
    bool is_empty = true;
    void change_data(int val)
    {
        data = val;
    }
    void change_source_opcode(string val)
    {
        source_opcode = val;
    }
    void change_is_empty(bool val)
    {
        is_empty = val;
    }
    void change_register_check(int val)
    {
        register_check = val;
    }
};

// ===================================================================================
// ======================        Single Cycle Functions         ======================
// ===================================================================================

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
string ControlUnit(string opcode)
{
    string ControlWord = "";
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
    return ControlWord;
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
        if ((std::string(1, imm1[0])) == "1")
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
        if ((std::string(1, imm1[0])) == "1")
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

// ===================================================================================
// ======================         Pipeline stages               ======================
// ===================================================================================

void FirstStage(IFID &ifid, string instruction, int program_counter)
{
    ifid.change_IR(instruction);
    ifid.change_DPC((program_counter + 1));
    ifid.change_is_empty(false);
}

void SecondStage(IDEX &idex, int program_counter, IFID ifid, OF of)
{
    if (ifid.is_empty)
    {
        return;
    }
    idex.change_CW(ControlUnit(ifid.IR.substr(25)));
    idex.change_IMM(find_immediate_value(ifid.IR.substr(0, 12), ifid.IR.substr(20, 5), ifid.IR.substr(25)));
    bitset<32> bitsetrs1(ifid.IR.substr(12, 5));
    idex.change_RS1(static_cast<int>(bitsetrs1.to_ulong()));
    bitset<32> bitsetrs2(ifid.IR.substr(7, 5));
    idex.change_RS2(static_cast<int>(bitsetrs2.to_ulong()));
    bitset<32> bitsetrd(ifid.IR.substr(20, 5));
    idex.change_RDI(static_cast<int>(bitsetrd.to_ulong()));
    idex.change_J_DPC((idex.IMM / 4) + program_counter);
    idex.change_FUNC(ifid.IR.substr(17, 3) + ifid.IR[1]);

    if ((std::string(1, idex.CW[0])) == "1")
    {
        int temp_RS1 = idex.RS1;
        int temp_RS2 = idex.RS2;
        idex.change_RS1(virtual_register.getRegisterValue(idex.RS1));
        idex.change_RS2(virtual_register.getRegisterValue(idex.RS2));
        if (!of.is_empty)
        {
            if (of.source_opcode == "exmo")
            {
                if (idex.CW == "101000100")
                {
                    if (of.register_check == temp_RS2)
                    {
                        idex.change_RS2(of.data);
                    }
                }
                else
                {
                    if (of.register_check == temp_RS1)
                    {
                        idex.change_RS1(of.data);
                    }
                }
            }
            else if (of.source_opcode == "mowb")
            {
                if (idex.CW == "101000100")
                {
                    if (of.register_check == temp_RS2)
                    {
                        idex.change_RS2(of.data);
                    }
                }
                else
                {
                    if (of.register_check == temp_RS1)
                    {
                        idex.change_RS1(of.data);
                    }
                }
            }
        }
    }
    idex.change_is_empty(false);
}

void ThirdStage(EXMO &exmo, IDEX idex, OF &of, bool &program_counter_valid, int &program_counter, int size)
{
    if (idex.is_empty)
    {
        return;
    }
    int FirstValue = idex.RS1;
    int SecondValue = idex.RS2;
    exmo.change_RS2VAL(SecondValue);
    if ((std::string(1, idex.CW[1])) == "1")
    {

        SecondValue = idex.IMM;
    }
    string ALU_SELECT = alu_control(idex.CW.substr(2, 2), idex.FUNC.substr(0, idex.FUNC.length() - 1), idex.FUNC[idex.FUNC.length() - 1]);
    int ALURESULT = do_alu_operation(ALU_SELECT, FirstValue, SecondValue);
    bool ALU_ZERO_FLAG = FirstValue == SecondValue;
    exmo.change_ALUOUT(ALURESULT);
    exmo.change_RDI(idex.RDI);
    if (((std::string(1, idex.CW[8])) == "1") && ALU_ZERO_FLAG)
    {
        program_counter = idex.J_DPC - 1;
        program_counter_valid = false;
    }
    if (program_counter < 0 || program_counter >= size)
    {
        cout << red << "ERROR : Invalid Branch/Jump Address" << def << endl;
        return;
    }
    exmo.change_CW(idex.CW);
    exmo.change_is_empty(false);
    if (idex.CW == "111100100" || idex.CW == "101000100")
    {
        of.change_data(ALURESULT);
        of.change_source_opcode("exmo");
        of.change_is_empty(false);
        of.change_register_check(exmo.RDI);
    }
}

void FourthStage(MOWB &mowb, EXMO exmo, OF &of, bool &program_counter_valid)
{
    if (exmo.is_empty)
    {
        return;
    }
    if ((std::string(1, exmo.CW[4])) == "1")
    {
        virtual_memory.addValue(exmo.ALUOUT, exmo.RS2VAL);
    }
    if ((std::string(1, exmo.CW[5])) == "1")
    {
        mowb.change_LDOUT(virtual_memory.getValue(exmo.ALUOUT));
    }
    mowb.change_CW(exmo.CW);
    mowb.change_ALUOUT(exmo.ALUOUT);
    mowb.change_RDI(exmo.RDI);
    if (((std::string(1, exmo.CW[8])) == "1"))
    {
        program_counter_valid = true;
    }
    mowb.change_is_empty(false);
    if ((std::string(1, mowb.CW[6])) == "1")
    {
        if ((std::string(1, mowb.CW[7])) == "1")
        {
            if (exmo.CW != "110010000")
            {
                of.change_data(mowb.LDOUT);
                of.change_source_opcode("mowb");
                of.change_is_empty(false);
                of.change_register_check(mowb.RDI);
            }
        }
        else
        {
            if (exmo.CW != "110010000")
            {
                of.change_data(mowb.ALUOUT);
                of.change_source_opcode("mowb");
                of.change_is_empty(false);
                of.change_register_check(mowb.RDI);
            }
        }
    }
}

void FifthStage(MOWB mowb)
{
    if (mowb.is_empty)
    {
        return;
    }
    if ((std::string(1, mowb.CW[6])) == "1")
    {
        if ((std::string(1, mowb.CW[7])) == "1")
        {
            virtual_register.setRegisterValue(mowb.RDI, mowb.LDOUT);
        }
        else
        {
            virtual_register.setRegisterValue(mowb.RDI, mowb.ALUOUT);
        }
    }
}
// ===================================================================================
// ======================               Main                    ======================
// ===================================================================================

int main()
{
    IFID ifid;
    IDEX idex;
    EXMO exmo;
    MOWB mowb;
    OF of;

    string filePath = "../../phase1/part3/machine_encoding.txt";

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
    int extra_instructions = 1;
    bool program_counter_valid = true;

    for (int program_counter = 0; program_counter < instructionVector.size() + 4; program_counter++)
    {
        // if (!program_counter_valid)
        // {
        //     ifid = IFID();
        //     idex = IDEX();
        //     program_counter_valid = true;
        // }
        // else
        {
            if (program_counter < instructionVector.size())
            {
                // if (ifid.stall)
                // {
                //     program_counter--;
                // }
                FifthStage(mowb);
                FourthStage(mowb, exmo, of, program_counter_valid);
                ThirdStage(exmo, idex, of, program_counter_valid, program_counter, instructionVector.size());
                SecondStage(idex, program_counter, ifid, of);
                FirstStage(ifid, instructionVector[program_counter], program_counter);
            }
            else
            {
                // if (ifid.stall)
                // {
                //     program_counter--;
                //     extra_instructions--;
                // }
                FifthStage(mowb);
                FourthStage(mowb, exmo, of, program_counter_valid);
                int temp1 = program_counter - extra_instructions;
                ThirdStage(exmo, idex, of, program_counter_valid, temp1, instructionVector.size());
                program_counter = temp1 + extra_instructions;
                SecondStage(idex, program_counter - extra_instructions, ifid, of);
                FirstStage(ifid, instructionVector[program_counter - extra_instructions], program_counter - extra_instructions);
                extra_instructions++;
            }
        }
        cout << "program_counter : " << program_counter << endl;
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