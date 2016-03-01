FLAGS= -g -o
CFLAGS= -g -c -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= routine.o routine_init.o

main: ../lib/libroutine.a  test_main
test_main: test_main.c ../lib/libroutine.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lroutine -lmessage -lmemdb -lbasefunc -lcrypto_func -lstruct -ljson -lalloc -lstring 
../lib/libroutine.a: $(STRUCTOBJ)
	ar rs libroutine.a $(STRUCTOBJ)
	mv    libroutine.a ../lib
routine.o:  routine.c ../include/routine.h 
	$(CC) $(CFLAGS) routine.c
routine_init.o:  routine_init.c ../include/routine.h routine_internal.h 
	$(CC) $(CFLAGS) routine_init.c
clean:
	rm -f *~ *.a *.o test_main
