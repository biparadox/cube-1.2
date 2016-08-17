/**
 * Copyright [2015] Tianfu Ma (matianfu@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * File: main.c
 *
 * Created on: Jun 5, 2015
 * Author: Tianfu Ma (matianfu@gmail.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "alloc_init.h"
/*
void test001() {
  void * p1, * p2, * p3;
  int freecount;
  int ret;
  ret = Galloc0(&p1,250);
  ret = Galloc0(&p2,98);
  ret = Galloc0(&p3,77);

  freecount=Ggetfreecount();
  
  printf("free count %d!\n",freecount);

  Free0(p2);
  Free0(p3);
  Free0(p1);
}
void test003() {
  void * p1, * p2, * p3;
  int freecount;
  p1 = Talloc(25);
  p2 = Talloc(9);
  p3 = Talloc(7);

  freecount=Tgetfreecount();
  
  printf("free count %d!\n",freecount);

  Free(p2);
  Free(p3);
  Free(p1);
}
*/
int main() {

  int i;
  unsigned char * alloc_buffer;
  alloc_buffer=malloc(4096*(256+1));
  if((int)alloc_buffer%4096==0)
  	alloc_init(alloc_buffer,256);
  else
	alloc_init(alloc_buffer+4096-(int)alloc_buffer%4096,256);
  UINT32 addr1;
  UINT32 addr2;

  addr1=bmalloc(123);	

  Memset(get_cube_pointer(addr1),'A',123);

  addr2=bmalloc(15);	
  Memset(get_cube_pointer(addr1),'B',15);
  bfree(addr1);
  bfree(addr2);  

  addr1=bmalloc(23);	

  addr1=get_page();
  addr2=get_page();
  free_page(addr2);
  UINT32 addr3;
  addr2=get_page();
  addr3=get_page();	
  free_page(addr1);
  free_page(addr3);

  for(i=0;i<100;i++)
  {
     addr1=salloc(78+i);
  }

	
/*
  test001();
  test003();
  Tclear();
*/
  free(alloc_buffer);
  return 0;
}
