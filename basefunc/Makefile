FLAGS= -g -o
CFLAGS= -g -c -Wall -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= basefunc.o 

main: ../lib/libbasefunc.a  test_main
test_main: test_main.c ../lib/libbasefunc.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lbasefunc -lalloc -lstring 
../lib/libbasefunc.a: $(STRUCTOBJ)
	ar rs libbasefunc.a $(STRUCTOBJ)
	mv    libbasefunc.a ../lib
basefunc.o:  basefunc.c ../include/basefunc.h
	$(CC) $(CFLAGS) basefunc.c
clean:
	rm -f test_main *~ *.a *.o ../lib/libbasefunc.a
