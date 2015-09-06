FLAGS= -g -o
CFLAGS= -g -c -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= struct_ops.o struct_deal.o

main: ../lib/libstruct.a  test_main
test_main: test_main.c ../lib/libstruct.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lstruct -lalloc  
../lib/libstruct.a: $(STRUCTOBJ)
	ar rs libstruct.a $(STRUCTOBJ)
	mv    libstruct.a ../lib
struct_deal.o:  struct_deal.c ../include/struct_deal.h
	$(CC) $(CFLAGS) struct_deal.c
struct_ops.o:  struct_ops.c struct_ops.h
	$(CC) $(CFLAGS) struct_ops.c
clean:
	rm -f *~ *.a *.o 
