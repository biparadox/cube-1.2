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

static buddy_t g_buddy;
const int g_order=24;

int Gmeminit()
{
	return buddy_init(&g_buddy,g_order);
}
int Galloc(void ** pointer,int size)
{
	*pointer = bmalloc(size,&g_buddy);
	if(pointer==NULL)
		return -ENOMEM;
	return size;
}
void Gfree(void * pointer)
{
	 bfree(pointer,&g_buddy);
}

void Gmemdestroy()
{
	buddy_destroy(&g_buddy);
}
int Ggetfreecount()
{
	return total_free(&g_buddy);
}
