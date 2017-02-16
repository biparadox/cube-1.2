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
#include "../include/alloc.h"
#include "../include/string.h"
#include "alloc_init.h"

//static unsigned char alloc_buffer[4096*(1+1+4+1+16+1+256)];
static BYTE * first_page;
static struct alloc_total_struct * root_struct;
struct alloc_segment_address * root_address;
struct page_index * pages;
//static struct page_head * root_head;
const UINT32 temp_page_order = 2;

UINT32 root_struct_init(void * addr,UINT32 page_num)
{
	if((int)addr % PAGE_SIZE!=0)
		return -EINVAL;
	if(page_num>=32768)
		return -E2BIG;
	if(page_num<8)
		return -ENOMEM;
	first_page=addr;

	root_struct=(struct alloc_total_struct *)first_page;
	Memset(root_struct,0,sizeof(*root_struct));
	
	root_struct->bottom=sizeof(*root_struct);
	root_struct->upper=PAGE_SIZE;
	root_struct->total_size=PAGE_SIZE*page_num;
	root_struct->page_num=page_num;
	root_struct->fixed_pages=1;
	root_struct->empty_pages=page_num-1;
	return 0;
}

UINT32 get_firstpagemem_bottom(UINT32 size)
{
	UINT32 addr;
	if(size>PAGE_SIZE)
		return -E2BIG;
	if(root_struct->bottom+size>root_struct->upper)
		return -ENOMEM;
	addr=root_struct->bottom;
	root_struct->bottom+=size;
	return addr;
}

UINT32 get_firstpagemem_upper(UINT32 size)
{
	UINT32 addr;
	if(size>PAGE_SIZE)
		return -E2BIG;
	if(root_struct->upper-size<root_struct->bottom)
		return -ENOMEM;
	addr=root_struct->upper-size;
	root_struct->upper=addr;
	return addr;
}

void * get_cube_pointer(UINT32 addr)
{
	if(addr<0)
		return NULL;
	if(addr>root_struct->total_size)
		return NULL;
	return 	(void *)(first_page+addr);
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

UINT16 get_fixed_pages(UINT16 page_num)
{
	UINT16 page_offset=root_struct->fixed_pages;
	if(page_num>0)
		root_struct->fixed_pages+=page_num;
	return page_offset;
}

int alloc_init(void * start_addr,int page_num)
{
	UINT32 ret;
	int temp_size;
	int i,j;

	// init the root_struct
	ret=root_struct_init(start_addr,page_num);
	if(ret>0x80000000)
		return 	ret;

	// alloc first_page's mem for root_address struct
	ret=get_firstpagemem_bottom(sizeof(*root_address));	

	if(ret>0x80000000)
		return 	ret;

	root_address=get_cube_pointer(ret);

	Memset(root_address,0,sizeof(*root_address));


	// add temp mem struct	
	ret=temp_memory_init(temp_page_order+PAGE_ORDER);	

	// compute the page index

	root_struct->total_size=PAGE_SIZE*256;
	root_struct->page_num=page_num;
	temp_size=sizeof(struct page_index)*page_num;
	
	root_struct->pagetable_size=(temp_size-1)/PAGE_SIZE+1;

	root_address->page_table=get_firstpagemem_bottom(sizeof(struct pagetable_sys));

	struct pagetable_sys * page_table = (struct pagetable_sys *)get_cube_pointer(root_address->page_table);

	page_table->size=temp_size;
	page_table->start=(UINT32)(root_struct->fixed_pages*PAGE_SIZE);
	page_table->end=page_table->start+temp_size;


	pages=get_cube_pointer(page_table->start);

	Memset(pages,0,page_table->size);
	
	pages[0].type=FIRST_PAGE;

	for(i=1;i<root_struct->fixed_pages;i++)
	{
		pages[i].type=TEMP_PAGE;
	}
	
	for(i=0;i<root_struct->pagetable_size;i++)
		pages[root_struct->fixed_pages+i].type=PAGE_TABLE;
	root_struct->fixed_pages+=root_struct->pagetable_size;

	// build the free pages list

	ret=freepage_init();
	if(ret>0x80000000)
		return 	ret;
	root_address->free_area=ret;

	// build the static mem struct 
	ret=static_init();
	if(ret>0x80000000)
		return 	ret;
	root_address->static_area=ret;

	// build the cache mem struct
	ret=cache_init();
	if(ret>0x80000000)
		return 	ret;
	root_address->cache_area=ret;
//	offset+=cache_init(offset); 


/*
	start_addr = alloc_buffer + PAGE_SIZE-(((int)alloc_buffer)&0x0fff);
	empty_addr = start_addr;
	T_mem_struct=buddy_init(t_order);
	C_mem_struct=buddy_init(c_order);
	G_mem_struct=buddy_init(g_order);
*/
	return 0;
}

UINT32 page_get_addr(UINT16 page)
{
	return (UINT32)page*PAGE_SIZE;	
}
UINT16 addr_get_page(UINT32 addr)
{
	return addr/PAGE_SIZE;	
}

UINT16 get_pages_num()
{
	return root_struct->page_num;
}
