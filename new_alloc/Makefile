FLAGS= -g -o
CFLAGS= -g -c -Wall -DUSER_MODE 
CC=gcc
LD=ld
CCP=g++
STRUCTOBJ= alloc_init.o cube_buddy.o cube_static.o cube_cache.o cube_freepage.o cube_dynamic.o

main: ../lib/liballoc.a  test_main
test_main: test_main.c ../lib/liballoc.a 
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lalloc  -lstring 
../lib/liballoc.a: $(STRUCTOBJ)
	ar rs liballoc.a $(STRUCTOBJ)
	mv    liballoc.a ../lib
cube_buddy.o:  cube_buddy.c  cube_buddy.h alloc_init.h ../include/alloc.h
	$(CC) $(CFLAGS) cube_buddy.c
cube_static.o:  cube_static.c  cube_static.h alloc_init.h ../include/alloc.h
	$(CC) $(CFLAGS) cube_static.c
cube_cache.o:  cube_cache.c  cube_cache.h alloc_init.h ../include/alloc.h
	$(CC) $(CFLAGS) cube_cache.c
cube_freepage.o:  cube_freepage.c  cube_freepage.h alloc_init.h ../include/alloc.h
	$(CC) $(CFLAGS) cube_freepage.c
cube_dynamic.o:  cube_dynamic.c  cube_dynamic.h alloc_init.h ../include/alloc.h
	$(CC) $(CFLAGS) cube_dynamic.c
alloc_init.o:  alloc_init.c ../include/alloc.h
	$(CC) $(CFLAGS) alloc_init.c
clean:
	rm -f test_main *~ *.a *.o ../lib/liballoc.a
