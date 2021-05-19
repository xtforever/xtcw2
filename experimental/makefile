VPATH=../utils
CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address -fno-omit-frame-pointer
#CC=gcc -Wall -I. -I../utils  -O3
#CC=gcc -Wall -I. -I../utils  -O3 -pg

T=svar-test
D=mls.o svar.o
ALL: clean ${T} 
${T}: ${D}
clean:
	-${RM} *.o ${T}
