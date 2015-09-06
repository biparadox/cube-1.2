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
 * Author: Hu jun (algorist@bjut.edu.cn)
 */

#include <stdlib.h>
#include <errno.h>
#include "../include/alloc.h"
#include "buddy.h"

static buddy_t c_buddy;
const int c_order=20;

int Cmeminit()
{
	return buddy_init(&c_buddy,c_order);
}
void *  Calloc(int size)
{
	return bmalloc(size,&c_buddy);
}
void *  Calloc0(int size)
{
	return bmalloc0(size,&c_buddy);
}
void Cfree(void * pointer)
{
	 bfree(pointer,&c_buddy);
}
void Cfree0(void * pointer)
{
	 bfree0(pointer,&c_buddy);
}

void Cmemdestroy()
{
	buddy_destroy(&c_buddy);
}
int Cgetfreecount()
{
	return total_free(&c_buddy);
}
