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
#include <deque>
#include <functional>
#include <utility>
#include "../../Tools/Color/Color.hpp"
// #include "../../Memory/memory.hpp"
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
// Memory virtual_memory;
Registers virtual_register;

// ===================================================================================
// ======================                 Cache                 ======================
// ===================================================================================

/*

Implementation of Write-Back Allocate Cache Policy

Main Memory --> 64 X 64 array of integers
Cache Memory --> 4 sets, 2 ways, 64 blocks per cache line
Every Set will have to map 16 blocks of main memory, so we will have 16 tags per set

So if we ask for the data of :

TAG ---- INDEX ---- BLOCK OFFSET
9   ----   2   ----     10

Then in the main memory, this will map to -->

2*16 + 9 = 41st row of the main memory
and in that row, the 10th element.

This Simulation uses LRU replacement policy.

To make it more realsitic, each call to the main Memory makes the program stall by 1 second.

*/

enum CacheState
{
    INVALID,
    VALID,
    MODIFIED,
    MISS_PENDING
};

struct CacheBlock
{
    int tag = -1;
    CacheState state;
    std::array<int, 64> data;
};

class SetAssociativeCache
{
public:
    // constructor
    SetAssociativeCache(int numSets, int numWays) : numSets(numSets), numWays(numWays)
    {
        cache.resize(numSets, std::vector<CacheBlock>(numWays));
        lruOrder.resize(numSets, std::deque<int>(numWays, -1));
    }

    // Function to load memory in the cache

    void LoadMemory(const std::vector<std::array<int, 64>> &data)
    {
        memory = data;
    }

    // Send a data read request to the CPU

    int CPU_Read_Request(int index, int tag, int blockOffset)
    {
        std::vector<CacheBlock> &set = cache[index % numSets];
        std::deque<int> &order = lruOrder[index % numSets];

        for (int i = 0; i < numWays; i++)
        {
            CacheBlock &block = set[i];
            if (block.tag == tag)
            {
                if (block.state == VALID || block.state == MODIFIED)
                {
                    UpdateLRUOrder(order, tag);
                    return (block.data[blockOffset]);
                }
                else if (block.state == MISS_PENDING)
                {
                    for (auto it : Miss_Status_Holding_Register)
                    {
                        auto end_time = std::chrono::high_resolution_clock::now();
                        if (it.second.first == index && it.second.second.first == tag)
                        {
                            auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - it.first).count();
                            if (elapsed_time < 1)
                            {
                                cout << red << "Waiting for Response from Main Memory ..." << def << endl;
                                while (elapsed_time < 1)
                                {
                                    end_time = std::chrono::high_resolution_clock::now();
                                    elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - it.first).count();
                                }
                                cout << green << "Response from Main Memory Received" << def << endl;
                                block.state = VALID;
                                block.data = it.second.second.second;
                                UpdateLRUOrder(order, tag);
                                return (block.data[blockOffset]);
                            }
                            else
                            {
                                block.state = VALID;
                                block.data = it.second.second.second;
                                UpdateLRUOrder(order, tag);
                                return (block.data[blockOffset]);
                            }
                        }
                    }
                }
            }
        }

        int wayToAllocate = AllocateBlock(index, tag);
        set[wayToAllocate].tag = tag;
        set[wayToAllocate].state = MISS_PENDING;

        Memory_Read_Request(index, tag);
        CPU_Read_Request(index, tag, blockOffset);
        return 0;
    }

    // Send a data Write Request to the CPU

    void CPUWriteRequest(int index, int tag, int blockOffset, int newData)
    {
        std::vector<CacheBlock> &set = cache[index % numSets];
        std::deque<int> &order = lruOrder[index % numSets];

        for (int i = 0; i < numWays; i++)
        {
            CacheBlock &block = set[i];
            if (block.tag == tag)
            {
                if (block.state == VALID || block.state == MODIFIED)
                {
                    block.data[blockOffset] = newData;
                    block.state = MODIFIED;
                    UpdateLRUOrder(order, tag);
                    return;
                }
                else if (block.state == MISS_PENDING)
                {
                    for (auto it : Miss_Status_Holding_Register)
                    {
                        auto end_time = std::chrono::high_resolution_clock::now();
                        if (it.second.first == index && it.second.second.first == tag)
                        {
                            auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - it.first).count();
                            if (elapsed_time < 1)
                            {
                                cout << red << "Waiting for Response from Main Memory ..." << def << endl;
                                while (elapsed_time < 1)
                                {
                                    end_time = std::chrono::high_resolution_clock::now();
                                    elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - it.first).count();
                                }
                                cout << green << "Response from Main Memory Received" << def << endl;
                                block.data[blockOffset] = newData;
                                block.state = MODIFIED;
                                UpdateLRUOrder(order, tag);
                                return;
                            }
                            else
                            {
                                block.data[blockOffset] = newData;
                                block.state = MODIFIED;
                                UpdateLRUOrder(order, tag);
                                return;
                            }
                        }
                    }
                }
            }
        }

        int wayToAllocate = AllocateBlock(index, tag);
        set[wayToAllocate].tag = tag;
        set[wayToAllocate].state = MISS_PENDING;

        Memory_Read_Request(index, tag);
        CPUWriteRequest(index, tag, blockOffset, newData);
    }

    // Send a data read request to the memory
    // And add the data when it comes back into the Miss Status Holding Register

    void Memory_Read_Request(int index, int tag)
    {
        int row = index * 16 + tag;
        std::array<int, 64> data = memory[row];
        auto start_time = std::chrono::high_resolution_clock::now();
        Miss_Status_Holding_Register.push_back({start_time, {index, {tag, data}}});
    }

    // Update the LRU order, special care to be taken care of the 0 that is added by default
    // when initialising the deque

    void UpdateLRUOrder(std::deque<int> &order, int tag)
    {
        order.erase(std::remove(order.begin(), order.end(), -1), order.end());
        order.erase(std::remove(order.begin(), order.end(), tag), order.end());
        order.push_front(tag);
    }

    // Allocate a block in the cache and also take care of the eviction of the block

    int AllocateBlock(int index, int tag)
    {
        std::vector<CacheBlock> &set = cache[index % numSets];
        std::deque<int> &order = lruOrder[index % numSets];
        int wayToAllocate = -1;
        bool remove = false;
        for (int i = 0; i < numWays; i++)
        {
            if (set[i].state == INVALID)
            {
                wayToAllocate = i;
                remove = false;
                break;
            }
            if (set[i].state == VALID && order.back() == set[i].tag)
            {
                wayToAllocate = i;
                remove = true;
            }
        }

        if (remove)
        {
            order.pop_back();
            if (set[wayToAllocate].state == MODIFIED)
            {
                memory[16 * index + set[wayToAllocate].tag] = set[wayToAllocate].data;
                set[wayToAllocate] = CacheBlock{-1, INVALID, std::array<int, 64>()};
            }
        }
        return wayToAllocate;
    }

    std::vector<std::vector<CacheBlock>> ReturnCache()
    {
        return cache;
    }

private:
    int numSets;
    int numWays;
    std::vector<std::vector<CacheBlock>> cache;
    std::vector<std::deque<int>> lruOrder;
    std::vector<std::array<int, 64>> memory;
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using InnerPair = std::pair<int, std::pair<int, std::array<int, 64>>>;
    using OuterPair = std::pair<TimePoint, InnerPair>;

    std::vector<OuterPair> Miss_Status_Holding_Register;
};

// global cache variable
SetAssociativeCache cache(4, 2);

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
    int data_exmo = 0;
    int data_mowb = 0;
    int register_exmo = 0;
    int register_mowb = 0;
    bool is_empty_exmo = true;
    bool is_empty_mowb = true;
    void change_data_exmo(int val)
    {
        data_exmo = val;
    }
    void change_data_mowb(int val)
    {
        data_mowb = val;
    }
    void change_register_exmo(int val)
    {
        register_exmo = val;
    }
    void change_register_mowb(int val)
    {
        register_mowb = val;
    }
    void change_is_empty_exmo(bool val)
    {
        is_empty_exmo = val;
    }
    void change_is_empty_mowb(bool val)
    {
        is_empty_mowb = val;
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
    ifid.change_DPC(program_counter);
    ifid.change_is_empty(false);
}

void SecondStage(IDEX &idex, IFID ifid, OF of)
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
    idex.change_J_DPC((idex.IMM / 4) + ifid.DPC);
    idex.change_FUNC(ifid.IR.substr(17, 3) + ifid.IR[1]);

    if ((std::string(1, idex.CW[0])) == "1")
    {
        int temp_RS1 = idex.RS1;
        int temp_RS2 = idex.RS2;
        idex.change_RS1(virtual_register.getRegisterValue(idex.RS1));
        idex.change_RS2(virtual_register.getRegisterValue(idex.RS2));
        if (!of.is_empty_exmo)
        {
            if (idex.CW == "101000100" || idex.CW == "110010000" || idex.CW == "100100001")
            {
                if (of.register_exmo == temp_RS2)
                {
                    idex.change_RS2(of.data_exmo);
                }
            }
            if (of.register_exmo == temp_RS1)
            {
                idex.change_RS1(of.data_exmo);
            }
        }
        if (!of.is_empty_mowb)
        {
            if (idex.CW == "101000100" || idex.CW == "110010000" || idex.CW == "100100001")
            {
                if (of.register_mowb == temp_RS2)
                {
                    idex.change_RS2(of.data_mowb);
                }
            }
            if (of.register_mowb == temp_RS1)
            {
                idex.change_RS1(of.data_mowb);
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
        of.change_data_exmo(ALURESULT);
        of.change_is_empty_exmo(false);
        of.change_register_exmo(exmo.RDI);
    }
}

void FourthStage(MOWB &mowb, EXMO exmo, OF &of)
{
    if (exmo.is_empty)
    {
        return;
    }
    if ((std::string(1, exmo.CW[4])) == "1")
    {
        cache.CPUWriteRequest(0, 0, exmo.ALUOUT, exmo.RS2VAL);
        // virtual_memory.addValue(exmo.ALUOUT, exmo.RS2VAL);
    }
    if ((std::string(1, exmo.CW[5])) == "1")
    {
        mowb.change_LDOUT(cache.CPU_Read_Request(0, 0, exmo.ALUOUT));
        // mowb.change_LDOUT(virtual_memory.getValue(exmo.ALUOUT));
    }
    mowb.change_CW(exmo.CW);
    mowb.change_ALUOUT(exmo.ALUOUT);
    mowb.change_RDI(exmo.RDI);
    mowb.change_is_empty(false);
    if ((std::string(1, mowb.CW[6])) == "1")
    {
        if ((std::string(1, mowb.CW[7])) == "1")
        {
            of.change_data_mowb(mowb.LDOUT);
            of.change_is_empty_mowb(false);
            of.change_register_mowb(mowb.RDI);
        }
        else
        {
            of.change_data_mowb(mowb.ALUOUT);
            of.change_is_empty_mowb(false);
            of.change_register_mowb(mowb.RDI);
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

    // Simulate memory contents
    std::vector<std::array<int, 64>> memoryContents(64);

    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            memoryContents[i][j] = 0;
        }
    }

    cache.LoadMemory(memoryContents);

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
        if (!program_counter_valid)
        {
            ifid = IFID();
            idex = IDEX();
            program_counter_valid = true;
        }
        {
            if (program_counter < instructionVector.size())
            {
                FifthStage(mowb);
                FourthStage(mowb, exmo, of);
                ThirdStage(exmo, idex, of, program_counter_valid, program_counter, instructionVector.size());
                SecondStage(idex, ifid, of);
                FirstStage(ifid, instructionVector[program_counter], program_counter);
            }
            else
            {
                FifthStage(mowb);
                FourthStage(mowb, exmo, of);
                int temp1 = program_counter - extra_instructions;
                ThirdStage(exmo, idex, of, program_counter_valid, temp1, instructionVector.size());
                program_counter = temp1 + extra_instructions;
                SecondStage(idex, ifid, of);
                FirstStage(ifid, instructionVector[program_counter - extra_instructions], program_counter - extra_instructions);
                extra_instructions++;
            }
        }
    }
    cout << green << "SUCCESS : SIMULATOR RAN SUCCESSFULLY" << def << endl;
    cout << endl;
    cout << red << "MAIN MEMORY : " << def << endl;
    cout << endl;
    for (int i = 0; i < 1; i++)
    {
        std::cout << cyan << "======================"
                  << "Memory Block : " << i << "======================" << def << endl;
        for (int j = 0; j < 64; j++)
        {
            cout << cyan << "| " << def;
            cout << red << "Address : (" << j << ")" << def << " -> ";
            std ::cout << blue << "Data : (" << memoryContents[i][j] << ")"
                       << " " << def;
        }
        std ::cout << endl;
    }
    cout << endl;
    cout << red << "CACHE : " << def << endl;
    cout << endl;
    for (int i = 0; i < 4; i++)
    {
        std::cout << cyan << "======================"
                  << "Cache Set : " << i << "======================" << def << endl;
        cout << endl;
        for (int j = 0; j < 2; j++)
        {
            cout << cyan << "| " << def;
            cout << red << "Tag : (" << cache.ReturnCache()[i][j].tag << ")" << def << " -> ";
            int state = cache.ReturnCache()[i][j].state;

            enum CacheState
            {
                INVALID,
                VALID,
                MODIFIED,
                MISS_PENDING
            };
            if (state == 0)
            {
                cout << blue << "State : ("
                     << "INVALID"
                     << ")" << def << " -> ";
            }
            else if (state == 1)
            {
                cout << blue << "State : ("
                     << "VALID"
                     << ")" << def << " -> ";
            }
            else if (state == 2)
            {
                cout << blue << "State : ("
                     << "MODIFIED"
                     << ")" << def << " -> ";
            }
            else if (state == 3)
            {
                cout << blue << "State : ("
                     << "MISS_PENDING"
                     << ")" << def << " -> ";
            }
            cout << green << "Data : (" << def;
            for (int k = 0; k < 64; k++)
            {
                cout << cache.ReturnCache()[i][j].data[k] << " ";
            }
            cout << green << ")" << def;
        }
        std ::cout << endl;
        std::cout << endl;
    }
    cout << endl;
    cout << red << "REGISTERS : " << def << endl;
    cout << endl;
    virtual_register.PrintRegisters();
    return 0;
}