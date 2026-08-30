VPATH=../utils
CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address -fno-omit-frame-pointer
#CC=gcc-10 -Wall -I. -I../utils  -O3 -march=znver2 
#CC=gcc -Wall -I. -I../utils  -O3 -pg -march=znver2

T=wrap-test
D=mls.o text-wrap.o
ALL: clean ${T} 
${T}: ${D}

clean:
	-${RM} *.o ${T}
