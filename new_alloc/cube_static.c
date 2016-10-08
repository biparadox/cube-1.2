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
#include "cube_static.h"

static struct static_sys * static_struct;
extern struct page_index *pages;


UINT16  static_init ()
{
	UINT32 ret;
	UINT16 first_page;

	ret=get_firstpagemem_bottom(sizeof(struct static_sys));
	if(ret>0x80000000)
		return 	ret;
	static_struct = (struct static_sys *)get_cube_pointer(ret);

	Memset(static_struct,0,sizeof(*static_struct));
	first_page=get_page();
	if(first_page==0)
		return 0;
	
	static_struct->pages_num++;
	static_struct->first_page=first_page;
	static_struct->curr_page=first_page;

	return (UINT16)ret;
}

UINT32 salloc(int size)
{
	UINT32 addr;
	UINT16 page;
	
	if(size>PAGE_SIZE)
		return 0;
	if(PAGE_SIZE-static_struct->curr_offset >size)	
	{
		addr=static_struct->curr_page*PAGE_SIZE+static_struct->curr_offset;
		static_struct->curr_offset+=size;
		static_struct->total_size+=size;
		return addr;
	}		
		
	page=get_page();
	if(page==0)
		return 0;
	pages[page].priv_page=static_struct->curr_page;
	pages[static_struct->curr_page].next_page=page;
	static_struct->curr_page=page;
	static_struct->pages_num++;
	addr=page*PAGE_SIZE;
	static_struct->curr_offset=size;
	return addr;
}
