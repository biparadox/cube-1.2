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
 * File: buddy.c
 *
 * Created on: Jun 5, 2015
 * Author: Tianfu Ma (matianfu@gmail.com)
 */

#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/memory.h"
#include "buddy.h"

static unsigned char alloc_buffer[4096*(1+1+4+1+16+1+256)];
static unsigned char * start_addr ;
static unsigned char * empty_addr ;
static int empty_pages=1+4+1+16+1+256;

const int g_order=20;
const int t_order=14;
const int c_order=16;

int mem_init( )
{
	start_addr = alloc_buffer + PAGE_SIZE-(((int)alloc_buffer)&0x0fff);
	empty_addr = start_addr;
	T_mem_struct=buddy_init(t_order);
	C_mem_struct=buddy_init(c_order);
	G_mem_struct=buddy_init(g_order);
	return 0;
}
void * buddy_init(int order) {

	buddy_t * buddy;
	int pagenum;
	if(order>MAX_ORDER)
		return NULL;
  	if(order<MIN_ORDER)
		return NULL;
	buddy=(buddy_t *)empty_addr;
	buddy->order=order;
	buddy->poolsize=BLOCKSIZE(order);
	pagenum=buddy->poolsize/PAGE_SIZE;	
	if(pagenum>=empty_pages)
		return NULL;
	empty_pages-=pagenum+1;

	buddy->pool=empty_addr+PAGE_SIZE;
	buddy->freelist= buddy->pool-sizeof(void *)*(order+2); 
	if(buddy->freelist==NULL)
		return NULL;
  	Memset(buddy->freelist,0,sizeof(void *)*(order+2)+buddy->poolsize);
	empty_addr+=(pagenum+1)*PAGE_SIZE;
  	buddy->freelist[order] = buddy->pool;
	return buddy;
}

void buddy_destroy(buddy_t * buddy) {
	int pagenum;
	buddy_clear(buddy);
	if(buddy->pool+buddy->poolsize!=empty_addr)
		return;
	pagenum=buddy->poolsize/PAGE_SIZE;	
	empty_addr=buddy->pool-PAGE_SIZE;
	empty_pages+=pagenum+1;
	return;
}

int Palloc(void ** pointer,int size)
{
	if(Tisinmem(pointer))
	{
		*pointer=Talloc(size);
		if(pointer==NULL)
			return -ENOMEM;
		return 0;
	}
	return Galloc(pointer,size);
}

int Palloc0(void ** pointer,int size)
{
	if(Tisinmem(pointer))
	{
		*pointer=Talloc(size);
		if(pointer==NULL)
			return -ENOMEM;
		return 0;
	}
	return Galloc0(pointer,size);
}

int Free(void * pointer)
{
	if(ispointerinbuddy(pointer,T_mem_struct))
	{
		bfree(pointer,T_mem_struct);
		return 0;
	}
	if(ispointerinbuddy(pointer,C_mem_struct))
	{
		bfree(pointer,C_mem_struct);
		return 0;
	}
	if(ispointerinbuddy(pointer,G_mem_struct))
	{
		bfree(pointer,G_mem_struct);
		return 0;
	}
	return -EINVAL;
}
int Free0(void * pointer)
{
	if(ispointerinbuddy(pointer,T_mem_struct))
	{
		bfree0(pointer,T_mem_struct);
		return 0;
	}
	if(ispointerinbuddy(pointer,C_mem_struct))
	{
		bfree0(pointer,C_mem_struct);
		return 0;
	}
	if(ispointerinbuddy(pointer,G_mem_struct))
	{
		bfree0(pointer,G_mem_struct);
		return 0;
	}
	return -EINVAL;
}
