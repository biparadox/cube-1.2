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
#include "../include/alloc.h"

void test001() {
  void * p1, * p2, * p3;
  int freecount;
  int ret;
  ret = Galloc(&p1,250);
  ret = Galloc(&p2,98);
  ret = Galloc(&p3,77);

  freecount=Ggetfreecount();
  
  printf("free count %d!\n",freecount);

  Gfree(p2);
  Gfree(p3);
  Gfree(p1);
}
void test003() {
  void * p1, * p2, * p3;
  int freecount;
  p1 = Talloc(25);
  p2 = Talloc(9);
  p3 = Talloc(7);

  freecount=Tgetfreecount();
  
  printf("free count %d!\n",freecount);

  Tfree(p2);
  Tfree(p3);
  Tfree(p1);
}

int main() {

  Gmeminit();
  Tmeminit();
  test001();
  test003();
  Tclear();
  Tmemdestroy();
  Gmemdestroy();
  return 0;
}
