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
#include "../include/string.h"
#include "alloc_init.h"
#include "cube_dynamic.h"

int dmem_upper_defaultfresh(void * pointer);
int dmem_lower_defaultfresh(void * pointer);

int (*dmem_upper_fresh[16])(void * );
int (*dmem_lower_fresh[16])(void * );

static struct dmem_sys * dmem_struct;
extern struct page_index *pages;

UINT16  dynamic_init ()
{
	UINT32 ret;
	UINT16 first_page;

	ret=get_firstpagemem_bottom(sizeof(struct dmem_sys));
	if(ret>0x80000000)
		return 	ret;
	dmem_struct = (struct dmem_sys *)get_cube_pointer(ret);
	Memset(dmem_struct,0,sizeof(*dmem_struct));
	first_page=get_page();
	if(first_page==0)
		return 0;
	dmem_struct->total_size=PAGE_SIZE;	
	dmem_struct->pages_num++;	
	dmem_struct->first_page=first_page;
	dmem_struct->curr_page=first_page;

	return (UINT16)ret;
}

UINT32 Dalloc(int size,UINT32 parent)
{

	UINT32 addr;
	UINT16 page;
	UINT16 oldpage;

	struct dmem_object_head * object_head;
	struct dmem_head * elem_head;

	if(parent==NULL)
		size+=sizeof(struct dmem_object_head);
	else	
		size+=sizeof(struct dmem_head);
	
	if(size>PAGE_SIZE)
		return 0;

	if(PAGE_SIZE-dmem_struct->curr_offset <size)
	{
		page=get_page();
		if(page==0)
			return 0;
		oldpage=dmem_struct->curr_page;
		pages[page].priv_page=oldpage;
		pages[page].next_page=pages[oldpage].next_page;
		pages[oldpage].next_page=page;
		dmem_struct->curr_page=page;
		dmem_struct->pages_num++;
		dmem_struct->curr_offset=0;
	}

	dmem_struct->curr_offset+=size;

	dmem_struct->curr_offset+=size;
	dmem_struct->total_size+=size;
	pages[page].state+=size;

		// set dmem_head's value
	addr=dmem_struct->curr_page*PAGE_SIZE+dmem_struct->curr_offset;
	if(parent==NULL)
	{
		object_head=(struct dmem_object_head *)get_cube_pointer(addr);
		Memset(object_head,0,sizeof(*object_head));
		addr+=sizeof(*object_head); 
		DMEM_SET_SIZE(get_cube_pointer(addr),size-sizeof(*object_head));
		DMEM_SET_FLAG(get_cube_pointer(addr),CUBE_DMEM_OBJECT);
		pages[page].state += size;
		dmem_struct->total_size += size;		
	}
	else
	{
		elem_head=(struct dmem_head *)get_cube_pointer(addr);	
		Memset(elem_head,0,sizeof(*object_head));
		elem_head->pointer_addr=NULL;
		addr+=sizeof(*elem_head); 
		DMEM_SET_SIZE(get_cube_pointer(addr),size-sizeof(*elem_head));
		DMEM_SET_FLAG(get_cube_pointer(addr),CUBE_DMEM_ELEM);
		pages[page].state += size;
		dmem_struct->total_size += size;		
	}	
	return addr;
}

UINT32 Dfree(UINT32 addr)
{
	UINT16 size;
	BYTE flag;
	UINT16 page;

	struct dmem_object_head * object_head;
	struct dmem_head * elem_head;

	size=DMEM_GET_SIZE(addr);
	flag=DMEM_GET_FLAG(addr);
	page=addr_get_page(addr);
	switch(flag)
	{
		case CUBE_DMEM_EMPTY:
		case CUBE_DMEM_ZOMBIE:
			return 0;
		case CUBE_DMEM_ELEM:
			size+=sizeof(*elem_head);
			elem_head=(struct dmem_elem_head *)((BYTE *)addr -sizeof(*elem_head));
			DMEM_SET_FLAG(get_cube_pointer(addr),CUBE_DMEM_ZOMBIE);
			elem_head->pointer_addr=NULL;
			pages[page].state-=size;
			dmem_struct->total_size-=size;		
			break;
		case CUBE_DMEM_OBJECT:
			size+=sizeof(*object_head);
			object_head=(struct dmem_object_head *)((BYTE *)addr -sizeof(*object_head));
			DMEM_SET_FLAG(get_cube_pointer(addr),CUBE_DMEM_ZOMBIE);
			object_head->pointer_addr=NULL;
			pages[page].state-=size;
			dmem_struct->total_size-=size;		
			break;
	}
	return size;	
}
