.global find_str_s
.func find_str_s

find_str_s:
    mov r2, #0
    mov r3, #100
    sub sp,sp,#16
    str r4,[sp]
    str r5,[sp, #4]
    str r6,[sp, #8]
    str r7,[sp, #12]
    mov r6, r1
    mov r12, #0
loop:
    mov r7, #0
    ldrb r4, [r0,r2]
    cmp r4, #0		
    beq end
    mov r12, r2
loops:
    ldrb r4, [r0, r12]
    ldrb r5, [r1, r7]
    cmp r5, #0
    beq end
    cmp r4,r5
    bne break
    mov r3,r2
    add r12, r12, #1
    add r7, r7, #1
    b loops
break:
    mov r3, #100
    add r2, r2, #1	// i += 1
    b loop
end:
    ldr r4,[sp]
    ldr r5,[sp,#4]
    ldr r6,[sp,#8]
    ldr r7,[sp,#12]
    add sp,sp,#16
    mov r0, r3
    bx lr
