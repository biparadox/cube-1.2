#ifndef  CUBE_ALLOC_H
#define  CUBE_ALLOC_H

int Cmeminit();
void * Calloc(int size);
void * Calloc0(int size);
void Cfree(void * pointer);
void Cmemdestroy();
int Cgetfreecount();

int Gmeminit();
int Galloc(void ** pointer,int size);
int Galloc0(void ** pointer,int size);
void Gfree(void * pointer);
void Gmemdestroy();
int Ggetfreecount();

int Tmeminit();
void * Talloc(int size);
void Tfree(void * pointer);
void Treset();
void Tclear();
int  Tisinmem();
void Tmemdestroy();
int Tgetfreecount();

#endif
