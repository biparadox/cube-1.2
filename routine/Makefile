FLAGS= -g -o
CFLAGS= -g -c -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= routine.o routine_init.o routine_switch.o routine_manage.o routine_dispatch.o routine_channel.o

main: ../lib/libroutine.a  test_main
test_main: test_main.c ../lib/libroutine.a  test_subroutine.o
	$(CC) $(FLAGS) test_main test_main.c test_subroutine.o -L../lib -lroutine -ldispatch -lchannel -lmessage -lmemdb -lchannel -lbasefunc -lcrypto_func -lstruct -ljson -lalloc -lstring -lpthread
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
routine_dispatch.o:  routine_dispatch.c ../include/routine.h routine_internal.h 
	$(CC) $(CFLAGS) routine_dispatch.c
routine_channel.o:  routine_channel.c ../include/routine.h routine_internal.h 
	$(CC) $(CFLAGS) routine_channel.c
test_subroutine.o:  test_subroutine.c routine_internal.h
	$(CC) $(CFLAGS) test_subroutine.c
clean:
	rm -f *~ *.a *.o test_main
