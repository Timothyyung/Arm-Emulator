PROGS = armemu

OBJS_ARMEMU = add_s.o fib_itr_s.o fib_rec_s.o  sum_array_s.o find_max_s.o find_str_s.o
 

CFLAGS = -g

%.o : %.s
	as -o $@ $<

%.o : %.c
	gcc -c ${CFLAGS} -o $@ $<

all : ${PROGS}


armemu : armemu.c ${OBJS_ARMEMU}
	gcc -o armemu armemu.c ${OBJS_ARMEMU}

clean :
	rm -rf ${PROGS} ${OBJS_ARMEMU}


