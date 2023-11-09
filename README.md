# RISCV-Instruction-simulator

This project is part of a Computer Architecture course which I have Undertaken at IIT-Roorkee. The Project seems very interesting to me and I will try my best to add further improvements to the project. I have tried to document the code as much as possible. You can find the references I used to make this project in the References folder. 

For more details, you can check out the project details pdf at References. 

# Working

While Explaining the working of different parts of the project, the following test code will be taken : 

```asm
.data
number: .word 43

.text

main:
addi a1,a1,7
sw a1,34(x0)

loop:
addi a2,a2,7
beq a1,a2,shogo

skip:
addi x4,x4,9

shogo:
addi x5,x5,69
lw x9,34(x0)
```

## Assembler

The Assembler Takes benefit of the patterns present in various instruction encodings. It creates tokens of the code and parses the tokens while analyzing the patterns present in them. 
This thus acts as a general instruction assembler. 

To add more instructions in the assembler, simply add instructions along with the relevant data in the instructions.txt file like so : 

```asm
bge 3 6 i12{11-11,9-4} rs2{5} rs1{5} funct3{101} i12{3-0,10-10} op{1100011}
add 3 6 funct7{0000000} rs2{5} rs1{5} funct3{000} rd{5} op{0110011}
sub 3 6 funct7{0100000} rs2{5} rs1{5} funct3{000} rd{5} op{0110011}
and 3 6 funct7{0000000} rs2{5} rs1{5} funct3{111} rd{5} op{0110011}
or 3 6 funct7{0000000} rs2{5} rs1{5} funct3{110} rd{5} op{0110011}
beq 3 6 i12{11-11,9-4} rs2{5} rs1{5} funct3{000} i12{3-0,10-10} op{1100011}
lw 3 5 i12{11-0} rs1{5} funct3{010} rd{5} op{0000011}
sw 3 6 i12{11-5} rs2{5} rs1{5} funct3{010} i12{4-0} op{0100011}
addi 3 5 i12{11-0} rs1{5} funct3{000} rd{5} op{0010011}
ori 3 5 i12{11-0} rs1{5} funct3{110} rd{5} op{0010011}
andi 3 5 i12{11-0} rs1{5} funct3{111} rd{5} op{0010011}
```

Now to run the assembler, Simply clone the repo and run 

```sh
cd phase1/part3
chmod +x run.sh
./run.sh assembler
```

To generarte specific encodings, change the test.s file and then run the above code. 

If everything runs properly, you should see something like this : 

![Screenshot from 2023-11-10 00-18-23](https://github.com/Sh0g0-1758/RISCV-Instruction-simulator/assets/114918019/e4aa8ca0-b8ca-458f-9e7d-fde563d951fe)

## Single Cycle Processor

The Single Cycle Processor gets the machine encodings from the assembler and then iterates through them sequentially. To generate the control word, it uses the following Logic : 

```
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
```

Apart from that it has functions , namely find_immediate_value, alu_control, do_alu_operation which does what the name intends. 

To run the simulator, clone this repo and run 

```sh
cd phase2
chmod +x run.sh
./run.sh simulator
```

If everything runs correctly, you should get the following output : 

![Screenshot from 2023-11-10 00-24-58](https://github.com/Sh0g0-1758/RISCV-Instruction-simulator/assets/114918019/8c720b7c-4af7-425c-b9f9-180c64001751)

## 5 - Stage PipeLine

In this Repo, you can find the following Implementations of 5-Stage PipeLine : 

1) Illusion of Pipeline By reversing the stages with stall Logic
2) Implementation using Multi-Threading (Under Development)
3) PipeLine with Operand-Forwarding,Support-For-Branch Instructions,Harvard Architecture and Reverse Execution

The Final PipeLine is the third one and it works by defining 4 intermediate pipeline registers : 

```
ifid
idex
exmo
mowb
```

While the Illusion of PipeLine is made possible by the following code snippet : 

```cpp
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
```

As seen above, I have divided the Execution into 5 stages with each stage having some dependency on some register. 

To run the simulator, clone this repository and run the following commands : 

```sh
cd phase3
chmod +x run.sh
./run.sh pipeline
```

If everything runs correctly , you should see something like this : 

![Screenshot from 2023-11-10 00-36-51](https://github.com/Sh0g0-1758/RISCV-Instruction-simulator/assets/114918019/66a572a3-bc69-48eb-934b-49fe7b99ec33)

## Cache

Here I have implemented Write-Back Allocate Cache with LRU Replacement Policy. 

The specs of the cache are as follows : 

```
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
```

To run the cache, clone the repo and run : 

```sh
cd phase4
cd Cache
chmod +x run.sh
./run.sh cache
```

If everything runs correctly, you should see several main memory blocks and several Writes and Reads from the cache Like so : 

![Screenshot from 2023-11-10 00-41-21](https://github.com/Sh0g0-1758/RISCV-Instruction-simulator/assets/114918019/36e1d83d-c551-4c55-9cf2-6dad32b3dcd9)

## Cache Integrated Simulator 

In the Final Part of this Project, I integrated the cache with the 5 stage Pipeline and its working snippets are as follows : 

![Screenshot from 2023-11-10 00-42-31](https://github.com/Sh0g0-1758/RISCV-Instruction-simulator/assets/114918019/48c1393b-c494-4b95-8083-46fbf755d318)

![Screenshot from 2023-11-10 00-45-06](https://github.com/Sh0g0-1758/RISCV-Instruction-simulator/assets/114918019/d9027254-61eb-4d4e-a415-d96aa55f2dfa)

# GUI 

The GUI is under active Development and will most likely be made using ImGui or Electronjs

# Contributor

Made with ❤️ By : <a href="https://twitter.com/ShogLoFi">shogo</a>

