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
 * Author: Hu jun (algorist@bjut.edu.cn)
 */

#include "../include/errno.h"
#include "../include/data_type.h"
//#include "../include/alloc.h"
#include "../include/string.h"
#include "alloc_init.h"
#include "buddy.h"

//static unsigned char alloc_buffer[4096*(1+1+4+1+16+1+256)];
static BYTE * first_page;
static struct alloc_total_struct * root_struct;
static struct alloc_segment_address * root_address;
static struct page_index * pages;
//static struct page_head * root_head;
const UINT32 temp_page_order = 4;

void * get_cube_pointer(UINT32 addr)
{
	if(addr<0)
		return NULL;
	if(addr>root_struct->total_size)
		return NULL;
	return 	(void *)first_page+addr;
}

UINT32 get_cube_addr(void * pointer)
{
	UINT32 addr;
	addr=pointer-(void *)first_page;
	if(addr<0)
		return -EINVAL;
	if(addr>root_struct->total_size)
		return -EINVAL;
	return 	addr;
}

UINT32 get_cube_data(UINT32 addr)
{
	UINT32 * pointer =get_cube_pointer(addr);
	return *pointer;
}

int alloc_init(void * start_addr,int page_num)
{
	int ret;
	int temp_size;
	int offset=0;
	int page_offset=0;
	int i,j;
	if((int)start_addr % PAGE_SIZE!=0)
		return -EINVAL;
	if(page_num>=32768)
		return -E2BIG;
	if(page_num<8)
		return -ENOMEM;

	first_page=start_addr;
	page_offset=1;	

	// let three root_object  get their address
	root_struct=(struct alloc_total_struct *)first_page;
	offset=sizeof(*root_struct);
	root_address=(struct alloc_segment_address *)(first_page+sizeof(*root_struct));
//	root_head=(struct page_head *)(root_address+sizeof(root_address));

	offset+=sizeof(*root_address);

	// add temp mem struct	

	struct  temp_mem_sys * temp_mem_struct = (struct temp_mem_sys *)(first_page+offset);
	temp_mem_struct->order=temp_page_order+PAGE_ORDER;
	j=temp_page_order;
	i=1;
	while(j>0)
	{
		i*=2;
		j--;
	}
	
	temp_mem_struct->size=PAGE_SIZE*i;

	page_offset+=i;

	// compute the page index
	root_struct->total_size=PAGE_SIZE*256;
	temp_size=sizeof(struct page_index)*page_num;
	
	root_struct->pagetable_size=(temp_size-1)/PAGE_SIZE+1;

	root_address->page_table=offset;

	struct pagetable_sys * page_table = (struct pagetable_sys *)(first_page+offset);

	offset+=sizeof(*page_table);

	page_table->size=temp_size;
	page_table->start=(UINT32)(page_offset*PAGE_SIZE);
	page_table->end=page_table->start+temp_size;


	pages=get_cube_pointer(page_table->start);

	Memset(pages,0,page_table->size);
	
	pages[0].type=FIRST_PAGE;
	
	

	for(i=0;i<root_struct->pagetable_size;i++)
		pages[page_offset+i].type=PAGE_TABLE;
	page_offset+=root_struct->pagetable_size;

	buddy_struct_init(temp_page_order+PAGE_ORDER,PAGE_SIZE);

//	page_table		

/*
	start_addr = alloc_buffer + PAGE_SIZE-(((int)alloc_buffer)&0x0fff);
	empty_addr = start_addr;
	T_mem_struct=buddy_init(t_order);
	C_mem_struct=buddy_init(c_order);
	G_mem_struct=buddy_init(g_order);
*/
	return 0;
}
/*
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
*/
