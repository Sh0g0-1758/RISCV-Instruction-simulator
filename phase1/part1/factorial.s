.data
number: .word 5

.text

main:

lw a0,number # load number from memory into a0
li a1,1 # load 1 into a1
li a2,1 # load 1 in a2
li a3,0 # load 0 in a3

loop:

mul a2,a2,a1 # a2 = a2 * a1
beq a1,a0,done # if a1 == a0, branch to done
addi a1,a1,1 # a1 = a1 + 1
j loop # branch to loop

done:
sw a2,0(a3) # store result at memory location 0x00000000
