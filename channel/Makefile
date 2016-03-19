FLAGS= -g -o
CFLAGS= -g -c -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= channel_buf.o  channel.o

main: ../lib/libchannel.a  test_main
test_main: test_main.c ../lib/libchannel.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lchannel -lbasefunc -lcrypto_func -lstruct -ljson -lalloc -lstring 
../lib/libchannel.a: $(STRUCTOBJ)
	ar rs libchannel.a $(STRUCTOBJ)
	mv    libchannel.a ../lib
channel_buf.o:  channel_buf.c ../include/channel.h 
	$(CC) $(CFLAGS) channel_buf.c
channel.o:  channel.c ../include/channel.h 
	$(CC) $(CFLAGS) channel.c
clean:
	rm -f *~ *.a *.o test_main
