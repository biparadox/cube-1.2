FLAGS= -g -o
CFLAGS= -g -c -Wall -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= memfunc.o 

main: ../lib/libmem.a  test_main
test_main: test_main.c ../lib/libmem.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lmem  
../lib/libmem.a: $(STRUCTOBJ)
	ar rs libmem.a $(STRUCTOBJ)
	mv    libmem.a ../lib
memfunc.o:  memfunc.c ../include/memory.h
	$(CC) $(CFLAGS) memfunc.c
clean:
	rm -f test_main *~ *.a *.o ../lib/libmem.a
