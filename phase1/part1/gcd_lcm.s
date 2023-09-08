.data
num1: .word 12
num2: .word 8

.text

main:

lui x23,0x10000
lw a0,0(x23) # load num1 into x1
lui x24,0x10000
ori x24,x24,4
lw a1,0(x24) # load num2 into x2
addi a4,a0,0 # a4 = a0 + 0
addi a5,a1,0 # a5 = a1 + 0

# using The Euclidean algorithm to find the gcd of two numbers
gcd_loop:

beq a1,x0,20 # if a1 == 0, branch to gcd_done
addi a3,a1,0 # a3 = a1
rem a1,a0,a1 # a1 = a0 % a1
addi a0,a3,0 # a0 = a3
jal x0,-16 # jump to gcd_loop

# now simply using lcm = (a*b) / gcd
gcd_done:

sw a0,0(x0) # storing a0 at memory address 0x00000000
mul a3,a4,a5 # a3 = a4 * a5
div a1,a3,a0 # a1 = a3 / a0
jal x0,4

lcm_done:

sw a1,1(x0) # storing a1 at memory address 0x00000001
