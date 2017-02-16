/*
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
#include "cube_freepage.h"

static struct free_mem_sys * free_pages;
extern struct page_index * pages;

UINT16  freepage_init()
{
	UINT32 ret;
	int i;

	ret=get_firstpagemem_bottom(sizeof(struct free_mem_sys));
	if(ret>0x80000000)
		return 	ret;
	free_pages = (struct free_mem_sys *)get_cube_pointer(ret);

	free_pages->first_page=get_fixed_pages(0);
	pages[free_pages->first_page].priv_page=0;
	
	for(i=free_pages->first_page;i<get_pages_num();i++)
	{
		pages[i].next_page=i+1;
		pages[i+1].priv_page=i;
	}
	pages[i].next_page=0;
	free_pages->pages_num=get_pages_num()-free_pages->first_page;

	return ret;
}

UINT16 get_page()
{
	UINT16 page;
	if(free_pages->pages_num==0)
		return 0;
	free_pages->pages_num--;
	page=free_pages->first_page;
	
	if(pages[page].next_page==0)
	{
		free_pages->first_page=0;
		pages[page].next_page=0;
	}
	else
	{
		free_pages->first_page=pages[page].next_page;
		pages[page].next_page=0;
		pages[free_pages->first_page].priv_page=0;	
	}
	return page;		
}

UINT32 free_page(UINT16 page)
{
	pages[page].priv_page=0;
	pages[page].next_page=free_pages->first_page;
	pages[free_pages->first_page].priv_page=page;
	free_pages->first_page=page;
	free_pages->pages_num++;
		
	return 0;
}
