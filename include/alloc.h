#ifndef  CUBE_ALLOC_H
#define  CUBE_ALLOC_H

int Gmeminit();
int Tmeminit();
int Galloc(void ** pointer,int size);
void Gfree(void * pointer);
void * Talloc(int size);
void Tfree(void * pointer);
void Treset();
void Tclear();
int  Tisinmem();
void Gmemdestroy();
void Tmemdestroy();
int Ggetfreecount();
int Tgetfreecount();

#endif
