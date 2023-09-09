.data
number: .word 43

.text

main:
addi x1,x1,7
sw x1,34(x0)

loop:
addi x2,x2,7
beq x1,x2,shogo

skip:
addi x4,x4,9

shogo:
addi x5,x5,69
lw x9,34(x0)