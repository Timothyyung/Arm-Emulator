.global sum_array_s
.func sum_array_s
/*
* r0 is where the array is stored
* r1 is where the array len is stored
*/

sum_array_s: 
    mov r2,#0     //r2 is the incrementing variable
    mov r3,#0	  //r3 is where the sum is stored
loop:
    cmp r2,r1     //compare r2 to r1 to see weither or not to terminate loop
    beq loop_end  //is not equal terminate loop
    ldr r12,[r0,r2,lsl #2]  // iterates through the array and store the next elem
    add r3, r3, r12         // adds next element to r3
    add r2, r2, #1	    // increments incrementing varaiable
    b loop		    // loops back
loop_end:
    mov r0, r3              // moves sum to r0 (return register)
    bx lr                   // returns the sum
