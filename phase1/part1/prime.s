.data
number: .word 42 # storing the number to check in memory

.text

main:

lw x2,number # Load the number from memory
li x1,1 # store 1 in x1
ble x2,x1,not_prime # Check if number is less than or equal to 1
li x3,2 # store 2 in x3

prime_loop:

mul x4,x3,x3 # x4 = x3*x3
bge x4,x2,prime # if x4 >= x2 , branch to prime
rem x5,x2,x3 # x5 = x2%x3
beqz x5,not_prime # if x5 == 0, branch to not_prime
addi x3,x3,1 # x3 = x3 + 1
j prime_loop # jump to prime_loop

prime:

li x1,1 # store 1 in x1
j done

not_prime:

li x1,0 # store 0 in x0
j done

done:
