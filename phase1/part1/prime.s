.data
number: .word 67 # storing the number to check in memory

.text

main:

lui x23, 0x10000 # Load upper 20 bits of 0x10000000
lw x2,0(x23) # Load the number from memory
lui x1,0 # Load the upper 20 bits of the immediate value (0)
addi x1, x1, 1 # Add the lower 12 bits of the immediate value (1)
bge x1,x2,48 # Check if number is less than or equal to 1
lui x3,0 # Load the upper 20 bits of the immediate value (0)
addi x3,x3,2 # Add the lower 12 bits of the immediate value (1)

prime_loop:

mul x4,x3,x3 # x4 = x3*x3
bge x4,x2,20 # if x4 >= x2 , branch to prime
rem x5,x2,x3 # x5 = x2%x3
beq x5,x0,24 # if x5 == 0, branch to not_prime
addi x3,x3,1 # x3 = x3 + 1
jal x0, -20 # jump to prime_loop

prime:

andi x1,x1,0 # make all lower 12 bits of x1 equal to 0
addi x1,x1,1 # store 1 in x1
jal x0, 12 # jump pc to offset 12

not_prime:

andi x1,x1,0 # store 0 in x1
jal x0, 4 # jump pc to offset 4

done: