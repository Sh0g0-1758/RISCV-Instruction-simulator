.data
num1: .word 12
num2: .word 8

.text

main:

lw a0,num1 # load num1 from memory
lw a1,num2 # load num2 from memory
mv a4,a0 # a4 = a0
mv a5,a1 # a5 = a1

# using The Euclidean algorithm to find the gcd of two numbers
gcd_loop:

beqz a1,gcd_done # if a1 == 0, branch to gcd_done
mv a3,a1 # a3 = a1
rem a1,a0,a1 # a1 = a0 % a1
mv a0,a3 # a0 = a3
j gcd_loop # jump to gcd_loop

# now simply using lcm = (a*b) / gcd
gcd_done:

sw a0,0(x0) # storing a0 at memory address 0x00000000
mul a3,a4,a5 # a3 = a4 * a5
div a1,a3,a0 # a1 = a3 / a0
j lcm_done

lcm_done:

sw a1,1(x0) # storing a1 at memory address 0x00000001
