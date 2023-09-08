.data
number: .word 5

.text

main:

lui x23, 0x10000 # Load upper 20 bits of 0x10000000
lw a0,0(x23) # Load the number from memory
andi a1,a1,0 # make all bits of a1 == 0
addi a1,a1,1 # store 1 in a1
andi a2,a2,0 # make all bits of a2 == 0
addi a2,a2,1 # store 1 in a2
andi a3,a3,0 # load 0 in a3

loop:

mul a2,a2,a1 # a2 = a2 * a1
beq a1,a0,12 # if a1 == a0, branch to done
addi a1,a1,1 # a1 = a1 + 1
jal x0, -12 # branch to loop

done:
sw a2,0(a3) # store result at memory location 0x00000000
