VPATH=../utils
#CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address -fno-omit-frame-pointer
CC=gcc-10 -Wall -I. -I../utils  -O3 -march=znver2 
#CC=gcc -Wall -I. -I../utils  -O3 -pg -march=znver2

T=svar-test
D=mls.o svar.o
ALL: clean ${T} svar2-test
${T}: ${D}
svar2-test: mls.o svar2.o 
svar4-test: mls.o svar2.o 

clean:
	-${RM} *.o ${T}
