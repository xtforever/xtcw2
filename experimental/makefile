VPATH=../utils
#CC=gcc -I. -I../utils -g -O0 -DMLS_DEBUG -Wall
CC=gcc -Wall -I. -I../utils  -O3
T=svar-test
D=mls.o svar.o
ALL: clean ${T}
${T}: ${D}
clean:
	-${RM} *.o ${T}
