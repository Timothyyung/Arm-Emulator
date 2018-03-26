.global find_max_s
.func find_max_s
/*
* r0 stores the array
* r1 stores the length of the array
*/

find_max_s:
    mov r2,#0		    //r2 stores the incrementing variable
    ldr r3,[r0,r2,lsl #2]   //r3 store the max number, defaulted to the first element
loop:
    cmp r2,r1		    //compare r2 with r1(size of array)
    beq loop_end	    //ends the loop is r2 == r1
    ldr r12,[r0,r2,lsl #2]  //puts the next element in the array to r12
    cmp r12,r3		    //compares r12 with r3
    bge checkmax	    //checks to see is r3 is greater than r12 if greater branch
    add r2,r2,#1	    //if not go to next itr	
    b loop		    
checkmax:
    mov r3,r12		    //if greater move r3 to r12
    add r2,r2,#1	    //incrment
    b loop		    // go back the loop
loop_end:
    mov r0,r3		    // when ended mov r3 to the r0 register
    bx lr		    // return
