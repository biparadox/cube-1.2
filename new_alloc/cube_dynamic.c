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

extern struct page_index *pages;

UINT16  dynamic_init ()
{
	UINT32 ret;
	UINT16 first_page;

//	ret=get_firstpagemem_bottom(sizeof(struct cache_sys));
//	if(ret>0x80000000)
//		return 	ret;
	return (UINT16)ret;
}

UINT32 Dalloc(int size)
{

	UINT32 index_addr;
	UINT32 page_index_addr;
	UINT32 addr;
	return addr;
}
