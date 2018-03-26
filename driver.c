#include <stdio.h>

int sum_array_s(int a[],int n);
int find_max_s(int a[],int n);
int fib_itr_s(int n);
int fib_rec_s(int n);
int find_str_s(char *s,char *sub);
void arm_state_init(struct arm_state *as, unsigned int *func, 
		unsigned int arg0, unsigned int arg1,
		unsigned int arg2, unsigned int arg3);
void arm_state_print(struct arm_state *as);
unsigned int armemu(struct arm_state *state);


int * initarray(){
    static int temp[1000];
    for(int i = 0; i < 1000; i++){
	temp[i] = i;
    }
    return temp;
}

void stringtester(char *s, char *sub){
    struct arm_state state;
    int r;
    r = find_str_s(s,sub);
    printf("(assembly) str in position : %d\n",r);
    arm_state_init(&state, (unsigned int *) find_str_s, (unsigned int) s, (unsigned int) sub, 0,0);
}

void stest(){
    printf("_-_-_-_Lets STRING together some test_-_-_-_-\n\n");
    printf("looking for string aabbcc in aaabbccdefgh \n\n");
    
    char *b = "aaabbccdefgh";
    char *c = "aabbcc";
    stringtester(b,c);

    printf("\nlooking for string accccc in aaabbccdefgh \n\n");
    
    b = "aaabbccdefgh";
    c = "accccc";
    stringtester(b,c);
    
    printf("\nlooking for string efgh in aaabbccdefgh \n\n");
    
    b = "aaabbccdefgh";
    c = "efgh";
    stringtester(b,c);

    printf("\nlooking for string aagg in aaaaaa \n\n");
    
    b = "aaaaaa";
    c = "aagg";
    stringtester(b,c);

    printf("\nlooking for string abcdefh in abcdefgh \n\n");
    
    b = "abcdefgh";
    c = "abcdefh";
    stringtester(b,c);
    
    printf("\nlooking for string cde in abscdegas \n\n");
    b = "abscdegas";
    c = "cde";
    stringtester(b,c);
}

void arraysum(int *a,int size){
    int r;
    r = sum_array_s(a,size);
    printf("sum array assembly : %d\n",r);
}

void arraymax(int *a,int size){
    int r;
    r = find_max_s(a,size);
    printf("find max assembly : %d\n", r);
}

void arraysumtest(){
    int a[5] = {100,200,300,400,500};
    int r;
    printf("__________________________________\n");
    printf("_-_-_-_-Here are SUM tests_-_-_-_-\n");
    printf("__________________________________\n");
    printf("input array = {100,200,300,400,500} \n\n");
    arraysum(a,5);

    int * a2 = initarray();
    printf("input array = {0...999} \n\n");
    arraysum(a2,1000);

    int a3[10] = {-111,-222,-333,-444,-555,-666,-777,-888,-999,-1010};
    printf("input array = {-111,-222,-333,-444,-555,-666,-777,-888,-999,-1010} \n\n");
    arraysum(a3,10);

    int a4[6]= {0,0,0,0,0,1};
    printf("input array = {0,0,0,0,0,1}\n\n");
    arraysum(a4,6);
}

void arraymaxtest(){
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

void fibtester(){
    int r1;
    int r2;
    printf("________________________________________\n");
    printf("i cant fibonnaci something for this test\n");
    printf("________________________________________\n");
    printf("fibonnaci for 20 numbers \n\n");
    printf("itr_s    itr_c\n");
    for(int i = 1; i < 20; i++){
    r1 = fib_itr_s(i);
    printf("%d       \n",r1);
    }
    
    printf("\n\nfibonnaci for 20 numbers \n\n");
    printf("rec_s    rec_c\n");
    for(int i = 1; i < 20; i++){
    r1 = fib_rec_s(i);
    printf("%d       \n",r1);
    }
}

int main(int argc, char **argv)
{
    arraysumtest();
    arraymaxtest();
    fibtester();
    stest();
}
