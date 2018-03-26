.global add_s
.func add_s

add_s:
    mov r4, #20
    mov r5, #10
    mov r6, #30
    sub sp, sp, #16
    str r4,[sp]
    str r5,[sp, #4]
    str r6,[sp, #8]
    mov r4, #100
    ldr r4,[sp]
    ldr r5,[sp, #4]
    ldr r6,[sp, #8]
    add r0, r4,r5
    add r0, r0 ,r6
    bx lr
brook:
    mov r0, #10
    bx lr
