FLAGS= -g -o
CFLAGS= -g -c -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= memdb.o

main: ../lib/libmemdb.a  test_main
test_main: test_main.c ../lib/libstruct.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lmemdb -lbasefunc -lcrypto_func -lstruct -lalloc -lstring 
../lib/libmemdb.a: $(STRUCTOBJ)
	ar rs libmemdb.a $(STRUCTOBJ)
	mv    libmemdb.a ../lib
memdb.o:  memdb.c ../include/memdb.h
	$(CC) $(CFLAGS) memdb.c
clean:
	rm -f *~ *.a *.o test_main
