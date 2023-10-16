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