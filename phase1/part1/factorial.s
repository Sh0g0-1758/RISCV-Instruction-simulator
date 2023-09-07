.data
number: .word 5

.text

main:

lw a0,number
li a1,1
li a2,1
li a3,0

loop:

mul a2,a2,a1
beq a1,a0,done
addi a1,a1,1
j loop

done:
sw a2,0(a3)
