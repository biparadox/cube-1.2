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
#include "cube_cache.h"

static struct cache_sys * cache_struct;
extern struct page_index *pages;

UINT16  cache_init ()
{
	UINT32 ret;
	UINT16 first_page;

	ret=get_firstpagemem_bottom(sizeof(struct cache_sys));
	if(ret>0x80000000)
		return 	ret;
	cache_struct = (struct cache_sys *)get_cube_pointer(ret);

	Memset(cache_struct,0,sizeof(*cache_struct));

	cache_struct->pages_num=0;
	cache_struct->index_num=0;
	cache_struct->total_size=0;
	cache_struct->index_offset=get_firstpagemem_upper(0);

	return (UINT16)ret;
}

UINT32 cache_find_index(int size)
{
	struct cache_index * cache_index;
	UINT32 index_addr=cache_struct->index_offset;
	int cache_size=((size-1)/4+1)*4;
	int i;
	for(i=1;i<=cache_struct->index_num;i++)
	{
		index_addr-=sizeof(*cache_index);
		cache_index=(struct cache_index *)get_cube_pointer(index_addr);
		if(cache_index->cache_size==size)
			return index_addr;		
	}
	return 0;
}

UINT32 cache_add_page(UINT32 index_addr,UINT32 page_index_addr)
{
	struct cache_index * cache_index;
	struct cache_page_index * page_index;
	cache_index=(struct cache_index *)(get_cube_pointer(index_addr));

	if(cache_index->first_page==0)
	{
		cache_index->first_page=get_page();
		if(cache_index->first_page==0)
			return 0;
		cache_index->curr_page=cache_index->first_page;
	}
	else
	{
		cache_index->curr_page=get_page();
		if(cache_index->curr_page==0)
			return 0;
	}

	pages[cache_index->curr_page].type=CACHE_PAGE;
	// page index's offset
	pages[cache_index->curr_page].state=0;
	pages[cache_index->first_page].index=PAGE_SIZE/cache_index->cache_size;

	page_index=(struct cache_page_index *)(get_cube_pointer(page_index_addr));
	page_index->empty_slot=PAGE_SIZE/cache_index->cache_size;
	page_index->index_size=cache_index->index_size;
	Memset(page_index->index,0,page_index->index_size);
	
	return cache_index->curr_page;
}

UINT32 cache_add_index(int size)
{
	struct cache_index * cache_index;
	struct cache_page_index * page_index;

	if((size<12) || (size>512))
		return 0;
	
	cache_struct->index_num++;
	UINT32 index_addr=get_firstpagemem_upper(sizeof(*cache_index));
	int cache_size=((size-1)/4+1)*4;

	cache_index=get_cube_pointer(index_addr);

	Memset(cache_index,0,sizeof(*cache_index));
	cache_index->cache_size=cache_size;
	cache_index->index_size=sizeof(page_index)+(PAGE_SIZE/cache_size-1)/8+1;
	cache_index->page_num=1;
//	cache_index->free_size=PAGE_SIZE;
	cache_index->index_page=get_page();
	if(cache_index->index_page==0)
		return 0;
	pages[cache_index->index_page].type=CACHE_PAGE_INDEX;

	cache_add_page(index_addr,PAGE_SIZE*cache_index->index_page);	
	return index_addr;
}



UINT32 Calloc(int size)
{

	UINT32 index_addr;
	UINT32 page_index_addr;
	struct cache_index * cache_index;	
	struct cache_page_index * cache_page_index;	
	UINT32 curr_page;
	UINT16 slot_site;
	UINT32 addr;
	UINT16 page;


	index_addr=cache_find_index(size);
	if(index_addr==0)
	{
		index_addr=cache_add_index(size);
		if(index_addr==0)
			return 0;
	}

	cache_index=(struct cache_index *)get_cube_pointer(index_addr);

	curr_page=cache_index->curr_page;
	page_index_addr=PAGE_SIZE*cache_index->index_page+pages[curr_page].state;
	cache_page_index=(struct cache_page_index *)get_cube_pointer(page_index_addr);

	if(cache_page_index->empty_slot==0)
	{
		curr_page=cache_add_page(index_addr,page_index_addr);
		if(curr_page==0)
			return 0;
		page_index_addr=PAGE_SIZE*cache_index->index_page+pages[curr_page].state;
		cache_page_index=(struct cache_page_index *)get_cube_pointer(page_index_addr);
	}


	cache_page_index->empty_slot--;
//		cache_index->free_size-=cache_index->cache_size;
	slot_site=Getlowestbit(cache_page_index->index,cache_page_index->index_size,0);			
	bitmap_set(cache_page_index->index,slot_site-1);
	addr=curr_page*PAGE_SIZE+(slot_site-1)*cache_index->cache_size;		

	return addr;
}
