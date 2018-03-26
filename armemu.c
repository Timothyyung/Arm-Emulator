#include <stdbool.h>
#include <stdio.h>

#define NREGS 16
#define STACK_SIZE 1024
#define SP 13
#define LR 14
#define PC 15

int add_s(int a, int b);
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

void arm_state_print(struct arm_state *as)
{
    int i;

    for (i = 0; i < NREGS; i++) {
        printf("reg[%d] = %x\n", i, as->regs[i]);
    }
    printf("cpsr = %X\n", as->cpsr);
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
	printf("char char \n\n");
	state->regs[mi.rd] = *((char *) (state->regs[mi.rn] + index));
    }
    printf("%d value to store\n",state->regs[mi.rd]);
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

void armemu_beqne(struct arm_state *state,int bitno, int cbit)
{
    unsigned int im24;
    unsigned int iw = *((unsigned int *) state->regs[PC]);
    im24 = im24_gen(iw);
    
    if((state->cpsr >> bitno & 1) == cbit)
    {
	state->regs[PC] = state->regs[PC] + 8;
	state->regs[PC] = state->regs[PC] + (im24 << 2);
    }else
	state->regs[PC] = state->regs[PC] + 4;
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

void armemu_one(struct arm_state *state)
{
    unsigned int iw;  
    iw = *((unsigned int *) state->regs[PC]);
    if (is_bx_inst(iw)) {
	printf("bx instruction");
        armemu_bx(state);
    } else if (is_add_inst(iw)) {
	printf("add instruction\n");
        armemu_add(state);
    } else if (is_mov_inst(iw)) {
	printf("mov instruction\n");
	armemu_mov(state);
    } else if (is_sub_inst(iw)) {
	printf("sub instruction\n");
	armemu_sub(state);
    } else if (is_cmp_inst(iw)) {
	printf("cmp instruction\n");
	armemu_cmp(state);
    } else if (is_b_inst(iw)) {
	printf("b instuction\n");
	armemu_b(state);
    } else if (is_beq_inst(iw)) {
	printf("beq isntuction\n");
	armemu_beqne(state,30,1);
    } else if (is_bne_inst(iw)) {
	printf("bne instruction\n");
	armemu_beqne(state,30,0);
    } else if (is_bge_inst(iw)) {
	printf("bge instruction \n");
	armemu_beqne(state,31,0);
    } else if (is_ble_inst(iw)){
	printf("ble instruction \n");
	armemu_beqne(state,31,1);
    } else if (is_ldr_inst(iw)) {
	printf("ldr instruction\n");
	armemu_ldr(state);
    } else if (is_str_inst(iw)){
	printf("str instruction\n");
	armemu_str(state);
    } else {
	printf("BAD INSTRUCTION %x\n\n\n", iw);
	arm_state_print(state);
	state->regs[PC] = 0;
    }
}
	
unsigned int armemu(struct arm_state *state)
{
    while (state->regs[PC] != 0){
        armemu_one(state);
    }
    return state->regs[0];
}
                      
int main(int argc, char **argv)
{
    struct arm_state state;
    unsigned int r;
    char *b = "grrrab";
    char *c = "rrab";
    int a[5] = {1,2,10,4,5};
    arm_state_init(&state, (unsigned int *) find_str_s, (unsigned int ) b,(unsigned int ) c,3,4);
    arm_state_print(&state);
    r = armemu(&state);
    printf("r = %d\n", r); 
    return 0;
}
