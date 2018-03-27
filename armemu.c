#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

int fib_itr_s(int a);
int fib_rec_s(int a);
int sum_array_s(int a[], int n);
int find_max_s(int a[], int n);
int find_str_s(char *s,char *sub);

struct arm_state {
    unsigned int regs[NREGS];
    unsigned int cpsr;
    unsigned char stack[STACK_SIZE];
};

struct instruction{
    unsigned int rd;
    unsigned int rn;
    unsigned int rm;
    unsigned int imm;
    unsigned int rot;
    unsigned int s;
};

struct mem_instruction{
    unsigned int rd;
    unsigned int rn;
    unsigned int u;
    unsigned int b;
};

struct analysis{
    unsigned int ci;
    unsigned int mi;
    unsigned int bi;
    unsigned int nb;
};

void arm_state_init(struct arm_state *as, unsigned int *func,
                    unsigned int arg0, unsigned int arg1,
                    unsigned int arg2, unsigned int arg3)
{
    int i;
    /* zero out all arm state */
    for (i = 0; i < NREGS; i++) {
        as->regs[i] = 0;
    }

    as->cpsr = 0;

    for (i = 0; i < STACK_SIZE; i++) {
        as->stack[i] = 0;
    }

    as->regs[PC] = (unsigned int) func;
    as->regs[SP] = (unsigned int) &as->stack[STACK_SIZE];
    as->regs[LR] = 0;

    as->regs[0] = arg0;
    as->regs[1] = arg1;
    as->regs[2] = arg2;
    as->regs[3] = arg3;
}

void analysis_init(struct analysis *ana)
{
    ana->ci = 0;
    ana->mi = 0;
    ana->bi = 0;
    ana->nb = 0;
}

void arm_state_print(struct arm_state *as)
{
    int i;

    for (i = 0; i < NREGS; i++) {
        printf("reg[%d] = %d\n", i, as->regs[i]);
    }
    printf("cpsr = %X\n", as->cpsr);
}

void analysis_print(struct analysis *ana)
{
    printf("_________A_N_A_L_Y_S_I_S_________\n");
    printf("Computation instructions:     %d\n",ana->ci);
    printf("Memory instructions:          %d\n",ana->mi);
    printf("Branch instructions taken:    %d\n",ana->bi);
    printf("Branch instructions not taken:%d\n",ana->nb);
    printf("_________________________________\n\n");
}

unsigned int maskgen(unsigned int size)
{
    unsigned int mask;
    for(int i = 0; i < size; i++){
	mask = mask & 1;
	if(i + 1 < size)
	    mask = mask << 1;
    }
    return mask;
}

unsigned int rotation(unsigned int rot, unsigned int imml){
    unsigned long int shift;
    unsigned long int temp;
    rot = rot * 2;
    shift = 32 - rot;
    temp = (maskgen(rot) & imml) << shift;
    return (temp | (imml >> rot));
}

bool check_inst(unsigned int iw, unsigned int code)
{
 
    unsigned int op;
    unsigned int opcode;

    op = (iw >> 26) & 0b11;
    opcode = (iw >> 21) & 0b1111;

    return (op == 0) && (opcode == code);   
}

void read_inst(unsigned int iw,struct instruction *inst )
{
    inst->imm = (iw >> 25) &0b1;
    inst->rd = (iw >> 12) & 0xF;
    inst->rn = (iw >> 16) & 0xF;
    inst->rm = iw & 0xFF;
    inst->rot = (iw >> 8) & 0xF;
}

bool is_add_inst(unsigned int iw)
{
    return check_inst(iw,0b0100);
}

void armemu_add(struct arm_state *state)
{
    unsigned int iw;
    struct instruction inst;

    iw = *((unsigned int *) state->regs[PC]);
    read_inst(iw,&inst);
    if(inst.imm == 0)
    {
	state->regs[inst.rd] = state->regs[inst.rn] + state->regs[inst.rm];
    }else
	state->regs[inst.rd] = state->regs[inst.rn] + rotation(inst.rot,inst.rm);
    if (inst.rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}

bool is_sub_inst(unsigned int iw)
{
    return check_inst(iw, 0b0010);
}

void armemu_sub(struct arm_state *state)
{
    unsigned int iw;
    struct instruction inst;
    
    iw = *((unsigned int *) state->regs[PC]);
    read_inst(iw,&inst);

    if(inst.imm == 0)
    {
	state->regs[inst.rd] = state->regs[inst.rn] - state->regs[inst.rm];
    }else
	state->regs[inst.rd] = state->regs[inst.rn] - rotation(inst.rot,inst.rm);
    if (inst.rd != PC) {
        state->regs[PC] = state->regs[PC] + 4;
    }
}

bool is_cmp_inst(unsigned int iw)
{
    check_inst(iw,0b1010);
}

unsigned int set_flags(unsigned int value)
{
    unsigned int n,z;
    z = 0;
    n = 0; 
    if(value == 0)
	z = 1 << 30;
    if((value >> 31) & 1 == 1)
	n = 1 << 31;
    return z|n;
}

void armemu_cmp(struct arm_state *state)
{
    unsigned int iw;
    unsigned int value;
    struct instruction inst;
    iw = *((unsigned int *) state->regs[PC]);
    read_inst(iw,&inst);

    if(inst.imm == 0)
	value = state->regs[inst.rn] - state->regs[inst.rm];
    else
	value = state->regs[inst.rn] - rotation(inst.rot,inst.rm);
    
    state->cpsr = set_flags(value);

    state->regs[PC] = state->regs[PC] + 4;

}

bool is_mov_inst(unsigned int iw)
{
    return check_inst(iw,0b1101) || check_inst(iw,0b1111);
}
		    
void armemu_mov(struct arm_state *state)
{
    unsigned int iw;
    struct instruction inst;
    iw = *((unsigned int *) state->regs[PC]);
    read_inst(iw,&inst);
    
    if (inst.imm == 0)
	state->regs[inst.rd] = state->regs[inst.rm];
    else
	state->regs[inst.rd] = rotation(inst.rot,inst.rm);
    if( inst.rd != PC) {
	state->regs[PC] = state->regs[PC] + 4;
    }
}

bool is_ldr_inst(unsigned int iw)
{
    return (iw >> 26 & 0b1 == 0b1) && (iw >> 20 & 0b1 == 0b1);
}

bool is_str_inst(unsigned int iw)
{
    return (iw >> 26 & 0b1 == 0b1) && !(iw >> 20 & 0b1 == 0);
}

void read_mem_instruction(unsigned int iw, struct mem_instruction * inst)
{
    inst->rd = iw >> 12 & 0xF;
    inst->rn = iw >> 16 & 0xF;
    inst->u = iw >> 23 & 1;
    inst->b = iw >> 22 & 1;
}

unsigned int get_index(unsigned int iw, struct arm_state * state)
{
    if( (iw >> 25 & 0b1) == 0b0)
    {
	unsigned int imm = iw & 0xFFF;
	return imm;
    }
    unsigned int rm = iw & 0xF;
    unsigned int shift = (iw >> 7) & 0xF;
    return state->regs[rm] << shift;
}

void armemu_str(struct arm_state * state)
{
    unsigned int iw = *((unsigned int *) state->regs[PC]);
    unsigned int index = get_index(iw,state);
    unsigned int *sindex;
    unsigned int *target;
    struct mem_instruction mi;
    read_mem_instruction(iw,&mi);
    if(mi.u == 1)
    {
	target =  (state->regs[mi.rn] + index);
    }else{
	target =  (state->regs[mi.rn] - index);
    }
    *target = state->regs[mi.rd];
    state->regs[PC] = state->regs[PC] + 4;
}

void armemu_ldr(struct arm_state * state)
{
    unsigned int iw = *((unsigned int *) state->regs[PC]);
    unsigned int index = get_index(iw,state);
    struct mem_instruction mi;
    read_mem_instruction(iw,&mi);
    if(mi.u == 0)
    {
	index = index * -1;
    }
    if(mi.b == 0)
	state->regs[mi.rd] = *((unsigned int *) (state->regs[mi.rn] + index));
    else
    {
	state->regs[mi.rd] = *((char *) (state->regs[mi.rn] + index));
    }
    state->regs[PC] = state->regs[PC] + 4;    
}

unsigned int im24_gen(unsigned int iw)
{
    unsigned int im24;
    im24 = iw &0xFFFFFF;

    if(im24 >> 23 & 1 == 1)
    {
	im24 = im24 | 0xFF000000;
    }
    
    return im24;
}

bool is_b_inst(unsigned int iw)
{
    return (((iw>>26) & 0b10) == 0b10) && (( iw>>28 & 0b1110) == 0b1110);
}

void armemu_b(struct arm_state *state)
{
    unsigned int im24;
    unsigned int iw = *((unsigned int *) state->regs[PC]);
    im24 = im24_gen(iw);
    if( iw >> 24 & 0b1 == 1)
    {
	state->regs[LR] = state->regs[PC] + 4;
    }
    state->regs[PC] = state->regs[PC] + 8;
    state->regs[PC] = state->regs[PC] + (im24 << 2);
}

bool is_bne_inst(unsigned int iw)
{
    
    return ((iw>>26 & 0b11) == 0b10) && ((iw>>28 & 0xf) == 0x1); 
}

bool is_beq_inst(unsigned int iw)
{
    return (((iw >> 26) & 0b11) == 0b10) && ((iw>>28 & 0xf) == 0); 
}

bool is_bge_inst(unsigned int iw)
{
    return ((iw >> 26 & 0b11) == 0b10) && ((iw>>28 & 0xf) == 0b1010);
}

bool is_ble_inst(unsigned int iw)
{
    return ((iw >>26 & 0b11) == 0b10) && ((iw>>28 & 0xf) == 0b1101);
}

int armemu_beqne(struct arm_state *state,int bitno, int cbit)
{
    unsigned int im24;
    unsigned int iw = *((unsigned int *) state->regs[PC]);
    im24 = im24_gen(iw);
    
    if((state->cpsr >> bitno & 1) == cbit)
    {
	state->regs[PC] = state->regs[PC] + 8;
	state->regs[PC] = state->regs[PC] + (im24 << 2);
	return 1;
    }else
    {
	state->regs[PC] = state->regs[PC] + 4;
	return 0;
    }
}

bool is_bx_inst(unsigned int iw)
{
    unsigned int bx_code;

    bx_code = (iw >> 4) & 0x00FFFFFF;

    return (bx_code == 0b000100101111111111110001);
}

void armemu_bx(struct arm_state *state)
{
    unsigned int iw;
    unsigned int rn;

    iw = *((unsigned int *) state->regs[PC]);
    rn = iw & 0b1111;

    state->regs[PC] = state->regs[rn];
}

void armemu_one(struct arm_state *state, struct analysis *ana)
{
    unsigned int iw;
    int b = -1;
    iw = *((unsigned int *) state->regs[PC]);
    if (is_bx_inst(iw)) {
        armemu_bx(state);
    } else if (is_add_inst(iw)) {
        armemu_add(state);
	ana->ci = ana->ci + 1; 
    } else if (is_mov_inst(iw)) {
	armemu_mov(state);
	ana->ci = ana->ci + 1;
    } else if (is_sub_inst(iw)) {
	armemu_sub(state);
	ana->ci = ana->ci + 1;
    } else if (is_cmp_inst(iw)) {
	armemu_cmp(state);
	ana->ci = ana->ci + 1;
    } else if (is_b_inst(iw)) {
	armemu_b(state);
	ana->bi = ana->bi + 1;
    } else if (is_beq_inst(iw)) {
	b = armemu_beqne(state,30,1);
    } else if (is_bne_inst(iw)) {
	b = armemu_beqne(state,30,0);
    } else if (is_bge_inst(iw)) {
	b = armemu_beqne(state,31,0);
    } else if (is_ble_inst(iw)){
	b = armemu_beqne(state,31,1);
    } else if (is_ldr_inst(iw)) {
	armemu_ldr(state);
	ana->mi = ana->mi + 1;
    } else if (is_str_inst(iw)){
	armemu_str(state);
	ana->mi = ana->mi + 1;
    } else {
	printf("BAD INSTRUCTION %x\n\n\n", iw);
	arm_state_print(state);
	state->regs[PC] = 0;
    }
    if(b == 1)
    {
	ana->bi = ana->bi + 1;
    }else if( b == 0)
    {
	ana->nb = ana->nb + 1;
    }

}
	
unsigned int armemu(struct arm_state *state,struct analysis *ana)
{
    analysis_init(ana);
    while (state->regs[PC] != 0){
        armemu_one(state,ana);
    }
    return state->regs[0];
}

void string_stest(char *s, char *sub)
{
    unsigned int r;
    struct arm_state state;
    struct analysis ana;
    arm_state_init(&state,(unsigned int *) find_str_s,(unsigned int) s, (unsigned int) sub, 0 ,0);
    r= find_str_s(s,sub);
    printf("assembly str in pos : %d\n\n", r);
    
    r=  (armemu(&state, &ana));
    printf("emulated assembly str in pos : %d \n", r);   
    analysis_print(&ana);
}


void string_tester()
{
    printf("Testing Strings\n\n");
    
    printf("String = aaabbbccc \nSubString = bbb\n");
    char *a = "aaabbbccc";
    char *b = "bbb";
    string_stest(a,b);

    printf("String = abcabccd\nSubString = ccd\n");
    a = "abcabccd";
    b = "ccd";
    string_stest(a,b);

    printf("String = aaaaa\nSubString = bbb\n");
    a = "aaaaa";
    b= "bbb";
    string_stest(a,b);
}

void fib_rec_test()
{
    unsigned int r1;
    unsigned int r2;
    struct arm_state state;
    struct analysis ana;
    printf("emulated assembly :  assembly\n");
    for(int i = 1; i <= 20; i++)
    {
	arm_state_init(&state,(unsigned int *) fib_rec_s,i, 0, 0 ,0);
	r1 = armemu(&state, &ana);
	r2 = fib_rec_s(i);
	printf( "%d_______%d\n",r1,r2);
    }
    analysis_print(&ana);
}

void fib_itr_test()
{
    unsigned int r1;
    unsigned int r2;
    struct arm_state state;
    struct analysis ana;
    printf("emulated assembly : assembly\n");
    for(int i = 1; i <= 20; i++)
    {
	arm_state_init(&state,(unsigned int *) fib_itr_s,i, 0, 0 ,0);
	r1 = armemu(&state, &ana);
	r2 = fib_itr_s(i);
	printf( "%d________%d\n",r1,r2);
    }
    analysis_print(&ana);
}

int * initarray(){
    static int temp[1000];
    for (int i = 0; i< 1000; i++){
	temp[i] = i;
    }
    return temp;
}

void arraymax (int *a, int size)
{
    unsigned int r;
    struct arm_state state;
    struct analysis ana;
    r = find_max_s(a,size);
    printf("find max assembly : %d\n",r);
   
    arm_state_init(&state,(unsigned int *) find_max_s,(unsigned int) a, size, 0 ,0);
    r=  (armemu(&state, &ana));
    printf("find max emulated assembly : %d\n", r);
    analysis_print(&ana);
}

void arraymaxtest()
{
    int a[5] = {100,200,300,400,500};
    int r;
    printf("____________________________________________\n");
    printf("did we MAXimum the amount of test cases (no)\n");
    printf("____________________________________________\n");
    printf("input array = {100,200,300,400,500} \n\n");
    arraymax(a,5);
  
    int * a2 = initarray();
    printf("input array = {0...999} \n\n");
    arraymax(a2,1000); 
    
    int a3[7] = {0,0,0,0,0,0,0};
    printf("input array = {0,0,0,0,0,0,0} \n\n");
    arraymax(a3,7);

    int a4[6]= {-1,-10,-100,-1000,-10000,-100000};
    printf("input array = {-1,-10,-100,-1000,-10000,-1000000}\n\n");
    arraymax(a4,6);	 
}

void arraysum(int *a, int size)
{
    unsigned int r;
    struct arm_state state;
    struct analysis ana;
    arm_state_init(&state,(unsigned int *) sum_array_s,(unsigned int) a, size, 0 ,0);

    r = sum_array_s(a,size);
    printf("find sum assembly : %d\n",r);

    r=  (armemu(&state,&ana));
    printf("find sum emulated assembly : %d\n", r);

    analysis_print(&ana);

}

void arraysumtest()
{
    int a[5] = {100,200,300,400,500};
    int r;
    printf("____________________________________________\n");
    printf("              Sum array test\n");
    printf("____________________________________________\n");
    printf("input array = {100,200,300,400,500} \n\n");
    arraysum(a,5);
  
    int * a2 = initarray();
    printf("input array = {0...999} \n\n");
    arraysum(a2,1000); 
    
    int a3[7] = {0,0,0,0,0,0,0};
    printf("input array = {0,0,0,0,0,0,0} \n\n");
    arraysum(a3,7);

    int a4[6]= {-1,-10,-100,-1000,-10000,-100000};
    printf("input array = {-1,-10,-100,-1000,-10000,-1000000}\n\n");
    arraysum(a4,6);	 
}

void function_test()
{
    arraysumtest();
    arraymaxtest();
    string_tester();
    fib_rec_test();
    fib_itr_test();
}

void sum_array_time_test()
{
    int a[5] = {100,200,300,400,500};
    struct arm_state state;
    struct analysis ana;

    unsigned int time = clock();
    for(int i = 0; i < 50; i++)
    {
	arm_state_init(&state,(unsigned int *) sum_array_s,(unsigned int) a, 5, 0 ,0);
	(armemu(&state,&ana));
    }
    time = clock() - time;

    unsigned int time2 = clock();
    for(int i = 0; i < 50; i++)
    {
	sum_array_s(a,5);	
    }
    time2 = clock() - time2;

    double ratio = (double) (time / time2);
    printf("sum array timing test\n");
    printf("Time for emu: %d\nTime for native: %d\n ",time, time2);
    printf("Ratio = %f\n\n",ratio);
}

void max_array_time_test()
{
    int a[5] = {100,200,300,400,500};
    struct arm_state state;
    struct analysis ana;

    unsigned int time = clock();
    for(int i = 0; i < 50; i++)
    {
	arm_state_init(&state,(unsigned int *) find_max_s,(unsigned int) a, 5, 0 ,0);
	(armemu(&state,&ana));
    }
    time = clock() - time;

    unsigned int time2 = clock();
    for(int i = 0; i < 50; i++)
    {
	find_max_s(a,5);	
    }
    time2 = clock() - time2;
    double ratio = (double) (time / time2);
    printf("max array time test \n");
    printf("Time for emu: %d\nTime for native: %d\n ",time, time2);
    printf("Ratio = %f\n\n",ratio);
}

void fib_itr_time_test()
{
    struct arm_state state;
    struct analysis ana;
    unsigned int time = clock();
    for(int i = 0; i < 50; i++)
    {
	arm_state_init(&state,(unsigned int *) fib_itr_s,10, 0, 0 ,0);
	(armemu(&state,&ana));
    }
    time = clock() - time;

    unsigned int time2 = clock();
    for(int i = 0; i < 50; i++)
    {
	fib_itr_s(10);	
    }
    time2 = clock() - time2;
    double ratio = (double) (time / time2);
    printf("fib itr timing test \n");
    printf("Time for emu: %d\nTime for native: %d\n ",time, time2);
    printf("Ratio = %f\n\n",ratio);
}

void fib_rec_time_test()
{
    struct arm_state state;
    struct analysis ana;
    unsigned int time = clock();
    for(int i = 0; i < 50; i++)
    {
	arm_state_init(&state,(unsigned int *) fib_rec_s,10, 0, 0 ,0);
	(armemu(&state,&ana));
    }
    time = clock() - time;

    unsigned int time2 = clock();
    for(int i = 0; i < 50; i++)
    {
	fib_rec_s(10);	
    }
    time2 = clock() - time2;
    double ratio = (double) (time / time2);
    printf("fib rec timing test\n");
    printf("Time for emu: %d\nTime for native: %d\n ",time, time2);
    printf("Ratio = %f\n\n",ratio);
}

void find_str_time_test()
{
    char *a = "aaabbbccc";
    char *b = "bbb";
    struct arm_state state;
    struct analysis ana;
    unsigned int time = clock();
    for(int i = 0; i < 50; i++)
    {
	arm_state_init(&state,(unsigned int *) find_str_s,a, b, 0 ,0);
	(armemu(&state,&ana));
    }
    time = clock() - time;

    unsigned int time2 = clock();
    for(int i = 0; i < 50; i++)
    {
	find_str_s(a,b);	
    }
    time2 = clock() - time2;
    double ratio = (double) (time / time2);
    printf("find str timing test\n");
    printf("Time for emu: %d\nTime for native: %d\n ",time, time2);
    printf("Ratio = %f\n\n",ratio);
}

void time_test()
{
    printf("-_-_-_Timing tests_-_-_-\n\n");
    sum_array_time_test();
    max_array_time_test();
    fib_itr_time_test();
    fib_rec_time_test();
    find_str_time_test();
}

int main(int argc, char **argv)
{
    function_test();
    time_test();
    return 0;
}
