.global fib_itr_s
.func fib_itr_s

fib_itr_s:
    mov r1,#1
    mov r2,#0
    mov r3,#1
    mov r4, #5
loop:
    cmp r1,r0
    beq loop_end
    mov r4,r3
    add r3,r2,r3
    mov r2,r4
    add r1,r1,#1
    b loop
loop_end:
    mov r0,r3
    bx lr

    
