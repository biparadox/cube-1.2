FLAGS= -g -o
CFLAGS= -g -c -Wall -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= alloc_init.o buddy.o talloc.o galloc.o calloc.o

main: ../lib/liballoc.a  test_main
test_main: test_main.c ../lib/liballoc.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lalloc  -lstring 
../lib/liballoc.a: $(STRUCTOBJ)
	ar rs liballoc.a $(STRUCTOBJ)
	mv    liballoc.a ../lib
buddy.o:  buddy.c buddy.h ../include/alloc.h
	$(CC) $(CFLAGS) buddy.c
galloc.o:  galloc.c ../include/alloc.h
	$(CC) $(CFLAGS) galloc.c
talloc.o:  talloc.c ../include/alloc.h
	$(CC) $(CFLAGS) talloc.c
calloc.o:  calloc.c ../include/alloc.h
	$(CC) $(CFLAGS) calloc.c
alloc_init.o:  alloc_init.c ../include/alloc.h
	$(CC) $(CFLAGS) alloc_init.c
clean:
	rm -f test_main *~ *.a *.o ../lib/liballoc.a
