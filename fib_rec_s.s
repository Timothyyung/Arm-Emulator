.global fib_rec_s
.func fib_rec_s

fib_rec_s:
    sub sp,sp,#16	//create space on the stack
    str lr,[sp]		//push the link register on the stack
    str r4,[sp,#4]	//push r4 to the stack
    str r5,[sp,#8]	//push r5 to the stack
    mov r4,r0		//moves the contents of r0 to r4
    cmp r0,#1		//compares r0 with #1
    ble returno		//branches if r0 is less than 1
    sub r0,r4,#1	//creates n-1
    bl fib_rec_s	//call the recursive function for fib(n-1)
    mov r5,r0		//moves the content of r0 to r5
    sub r0,r4,#2	//creates n-2
    bl fib_rec_s	//call the recursive function for fib(n-2)
    add r0,r5,r0	//adds up fib(n-1) and fib(n-2)
    ldr r4,[sp,#4]	//pops r4
    ldr r5,[sp,#8]	//pops r5
    ldr lr,[sp]		//pops lr
    add sp,sp,#16	//adds back to the stack
    bx lr		//returns
returno:
    mov r0,r4		//puts content of r4 (either 0 or 1) to r0
    ldr r4,[sp,#4]	//pops r4
    ldr r5,[sp,#8]	//pops r5
    ldr lr,[sp]		//pops link register
    add sp,sp,#16	//adds back to the stack
    bx lr		//returns
