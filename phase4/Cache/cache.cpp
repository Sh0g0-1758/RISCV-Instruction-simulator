// Include Statements
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <functional>
#include <thread>
#include <chrono>
#include <utility>
#include "../../Tools/Color/Color.hpp"
#include <iomanip>

// namespaces
using namespace std;

// colored output
const Color::Modifier red(Color::FG_RED);
const Color::Modifier def(Color::FG_DEFAULT);
const Color::Modifier green(Color::FG_GREEN);
const Color::Modifier blue(Color::FG_BLUE);
const Color::Modifier cyan(Color::FG_CYAN);

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
    SetAssociativeCache(int numSets, int numWays) : numSets(numSets), numWays(numWays)
    {
        cache.resize(numSets, std::vector<CacheBlock>(numWays));
        lruOrder.resize(numSets, std::deque<int>(numWays, -1));
    }

    void LoadMemory(const std::vector<std::array<int, 64>> &data)
    {
        memory = data;
    }

    void CPU_Read_Request(int index, int tag, int blockOffset)
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
                    CPUResponse(block.data[blockOffset]);
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
                                block.state = VALID;
                                block.data = it.second.second.second;
                                CPUResponse(block.data[blockOffset]);
                                UpdateLRUOrder(order, tag);
                                return;
                            }
                            else
                            {
                                block.state = VALID;
                                block.data = it.second.second.second;
                                CPUResponse(block.data[blockOffset]);
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
        CPU_Read_Request(index, tag, blockOffset);
    }

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

    void CPUResponse(int data)
    {
        std::cout << green << "CPU Response: " << data << def << std::endl;
    }

    void Memory_Read_Request(int index, int tag)
    {
        int row = index * 16 + tag;
        std::array<int, 64> data = memory[row];
        auto start_time = std::chrono::high_resolution_clock::now();
        Miss_Status_Holding_Register.push_back({start_time, {index, {tag, data}}});
    }

    void UpdateLRUOrder(std::deque<int> &order, int tag)
    {
        order.erase(std::remove(order.begin(), order.end(), -1), order.end());
        order.erase(std::remove(order.begin(), order.end(), tag), order.end());
        order.push_front(tag);
    }

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

int main()
{
    SetAssociativeCache cache(4, 2);

    // Simulate memory contents
    std::vector<std::array<int, 64>> memoryContents(64);

    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            memoryContents[i][j] = i * 64 + j;
        }
    }

    std::cout << cyan << "Printing Memory Contents: " << def << endl;

    for (int i = 0; i < 64; i++)
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

    cache.LoadMemory(memoryContents);

    // Simulate CPU requests

    // Batch 1
    cache.CPU_Read_Request(0, 10, 10);
    cache.CPU_Read_Request(1, 7, 5);
    cache.CPU_Read_Request(2, 6, 9);
    cache.CPU_Read_Request(3, 5, 5);

    // Batch 2
    cache.CPU_Read_Request(0, 4, 17);
    cache.CPU_Read_Request(1, 3, 5);
    cache.CPU_Read_Request(2, 2, 9);
    cache.CPU_Read_Request(3, 1, 5);

    // Batch 3
    cache.CPU_Read_Request(0, 0, 17);
    cache.CPU_Read_Request(1, 1, 5);
    cache.CPU_Read_Request(2, 7, 9);
    cache.CPU_Read_Request(3, 3, 5);

    // Batch 4
    cache.CPUWriteRequest(0, 0, 17, 100);
    cache.CPUWriteRequest(1, 1, 5, 200);
    cache.CPUWriteRequest(2, 7, 9, 300);
    cache.CPUWriteRequest(3, 3, 5, 400);

    // Batch 5
    cache.CPUWriteRequest(0, 9, 17, 500);
    cache.CPUWriteRequest(1, 8, 5, 600);
    cache.CPUWriteRequest(2, 5, 9, 700);
    cache.CPUWriteRequest(3, 4, 5, 800);

    // Batch 6
    cache.CPU_Read_Request(0, 9, 17);

    cout << cyan << "Successfully Executed Cache Simulation" << def << endl;
    return 0;
}