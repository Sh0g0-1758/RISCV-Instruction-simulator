.data
number: .word 69 # storing 69 in the memory

.text

main:
lui x1,0 # adding some comments, these will not affect the result XD
jal x0,4
bge x1,x2,48
jal x0,main
lw x2,0(x0)

label:
lui x2,0
jal x0,label