VPATH=../utils
CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address -fno-omit-frame-pointer
#CC=gcc -Wall -I. -I../utils  -O3 -march=znver2 
#CC=gcc -Wall -I. -I../utils  -O3 -pg -march=znver2
CC=gcc -Wall -I. -I../utils  -O3  -fomit-frame-pointer

T=var5 hash_test
D=mls.o ctx.o m_tool.o
ALL: ${T}

ADDT: clean ${T} svar2-test svar-util-test
${T}: ${D}
svar2-test: mls.o svar2.o 
svar4-test: mls.o svar2.o 
svar-util-test: svar-util.o svar3.o mls.o

clean:
	-${RM} *.o ${T}
