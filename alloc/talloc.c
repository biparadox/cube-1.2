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

#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "buddy.h"

//static buddy_t t_buddy;
//const int t_order=12;
/*
int Tmeminit()
{
	return buddy_init(&t_buddy,t_order);
}
*/
void * Talloc(int size)
{
	return (void *)bmalloc(size,T_mem_struct);
}
void Treset()
{
	buddy_reset(T_mem_struct);
}
void Tclear()
{
	buddy_clear(T_mem_struct);
}

void Tmemdestroy()
{
	buddy_destroy(T_mem_struct);
}
int Tgetfreecount()
{
	return total_free(T_mem_struct);
}
int Tisinmem(void * pointer)
{
	return ispointerinbuddy(pointer,T_mem_struct);
}
