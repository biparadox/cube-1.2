FLAGS= -g -o
CFLAGS= -g -c -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= routine.o channel.o routine_init.o routine_switch.o routine_manage.o

main: ../lib/libroutine.a  test_main
test_main: test_main.c ../lib/libroutine.a  test_subroutine.o
	$(CC) $(FLAGS) test_main test_main.c test_subroutine.o -L../lib -lroutine -lmessage -lmemdb -lbasefunc -lcrypto_func -lstruct -ljson -lalloc -lstring 
../lib/libroutine.a: $(STRUCTOBJ)
	ar rs libroutine.a $(STRUCTOBJ)
	mv    libroutine.a ../lib
routine.o:  routine.c ../include/routine.h 
	$(CC) $(CFLAGS) routine.c
routine_init.o:  routine_init.c ../include/routine.h routine_internal.h 
	$(CC) $(CFLAGS) routine_init.c
routine_switch.o:  routine_switch.c ../include/routine.h routine_internal.h 
	$(CC) $(CFLAGS) routine_switch.c
routine_manage.o:  routine_manage.c ../include/routine.h routine_internal.h 
	$(CC) $(CFLAGS) routine_manage.c
test_subroutine.o:  test_subroutine.c routine_internal.h
	$(CC) $(CFLAGS) test_subroutine.c
channel.o:	channel.c ../include/channel.h
	$(CC) $(CFLAGS) channel.c
clean:
	rm -f *~ *.a *.o test_main
