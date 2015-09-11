#ifndef  CUBE_ALLOC_H
#define  CUBE_ALLOC_H

int mem_init();
void * Calloc(int size);
void * Calloc0(int size);
int Cgetfreecount();

int Galloc(void ** pointer,int size);
int Galloc0(void ** pointer,int size);
int Ggetfreecount();

void * Talloc(int size);
void Treset();
void Tclear();
int  Tisinmem();
int Tgetfreecount();

int Free(void * pointer);
int Free0(void * pointer);
#endif
