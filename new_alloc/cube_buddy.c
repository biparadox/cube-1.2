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
#include "cube_buddy.h"

static UINT32 * listarray;
static buddy_t * buddy_struct;
static struct temp_mem_sys * temp_mem_struct;
extern struct page_index *pages;
extern struct alloc_segment_address * root_address;

UINT32  buddy_struct_init (int order, UINT32 addr);

UINT32 temp_memory_init(int order)
{
	UINT32 ret;
	ret=get_firstpagemem_bottom(sizeof(struct temp_mem_sys));
	if(ret>0x80000000)
		return ret;
	root_address->temp_area=(UINT16)ret;
	temp_mem_struct=get_cube_pointer(ret);
	ret=buddy_struct_init(order,PAGE_SIZE);
	return ret;
}

UINT32  buddy_struct_init (int order, UINT32 addr)
{
	int buddy_manager_size;
	int buddy_manager_offset;
	int i;

	if((order<PAGE_ORDER) || (order >=MAX_ORDER))
	if(addr%PAGE_SIZE!=0)
		return -EINVAL;
	// get buddy_manager's site	
	buddy_manager_size= sizeof(buddy_struct)+order*sizeof(UINT32)+sizeof(UINT32)*2;
	
	buddy_manager_offset=get_firstpagemem_upper(buddy_manager_size);
	buddy_struct=(buddy_t *)get_cube_pointer(buddy_manager_offset);

	// empty the buddy_struct
	Memset(buddy_struct,0,buddy_manager_offset);

	int buddy_page_num=1<<(order-PAGE_ORDER);

	get_fixed_pages(buddy_page_num);
	// fill the buddy_struct
	buddy_struct->order=order;
	buddy_struct->poolsize=PAGE_SIZE<<(order-PAGE_ORDER);
	buddy_struct->freelist=buddy_manager_offset+sizeof(buddy_struct);
	buddy_struct->free_size=buddy_struct->poolsize;
	buddy_struct->pool=addr;

	// empty the buddy's pool
	Memset(get_cube_pointer(buddy_struct->pool),0,buddy_struct->poolsize);
 
	// init the buddy_struct's freelist       
	listarray = (UINT32 *)get_cube_pointer(buddy_struct->freelist);
	listarray[order]=buddy_struct->pool;

	// return buddy_struct's site
	return buddy_manager_offset;
}

void buddy_clear() 
{

	Memset(listarray,0,sizeof(UINT32)*(buddy_struct->order+1));
	listarray[buddy_struct->order]=buddy_struct->pool;
	buddy_struct->free_size=buddy_struct->poolsize;
	return;
}

UINT32 bmalloc(int size) {

	int i, order;
	UINT32 block;
	UINT32 buddymem;
	int alloc_size;

  // calculate minimal order for this size
	i = MIN_ORDER;
	while (BLOCKSIZE(i) < size + 1) // one more byte for storing order
		i++;

	order = i;
	alloc_size=BLOCKSIZE(order);

  // level up until non-null list found
	for (;; i++) {
  		if (i > buddy_struct->order)
			return NULL;
    		if (listarray[i]!=0)
      			break;
  	}

  // remove the block out of list
 	block = listarray[i];
 	listarray[i] = get_cube_data(listarray[i]);

  // split until i == order
 	while (i-- > order) {
		buddymem = buddyof(block, i,buddy_struct);
    		listarray[i] = buddymem;
		*(UINT32 *)get_cube_pointer(buddymem)=0;
	}

  // store order in previous byte
 	*((BYTE*) (get_cube_pointer(block - 1))) = order;
	buddy_struct->free_size-=alloc_size;
	return block;
}

UINT32 bmalloc0(int size) 
{
	UINT32 addr=bmalloc(size);
	if(addr!=0)
	{
		Memset(get_cube_pointer(addr),0,size);
	}
	return addr;
}

void bfree(UINT32 block) {

	int i;
	UINT32 buddymem;
	UINT32 p;
	int alloc_size;

  // fetch order in previous byte
	i = *((BYTE*) (get_cube_pointer(block) - 1));
	alloc_size=BLOCKSIZE(i);

	for (;i<=buddy_struct->order; i++) {
    // calculate buddy
		buddymem = buddyof(block, i,buddy_struct);
		p = get_cube_addr(&listarray[i]);

    // find buddy in list
		while ((get_cube_data(p) != NULL) && (get_cube_data(p) != buddymem))
			p = get_cube_data(p);

    // not found, insert into list
  		if (get_cube_data(p) != buddymem) {
  			*((UINT32 *)get_cube_pointer(block)) = listarray[i];
      			listarray[i] = block;
			buddy_struct->free_size+=alloc_size;
      			return;
		}
    // found, merged block starts from the lower one
    		block = (block < buddymem) ? block : buddymem;
    // remove buddy out of list
    		*((UINT32 *)get_cube_pointer(p)) = get_cube_data(get_cube_data(p));
  	}
	buddy_struct->free_size+=alloc_size;
	return;
}

/*
void bfree0(void * pointer,buddy_t * buddy) 
{
	int i;
	i = *((BYTE*) (pointer - 1));
	if(pointer!=NULL)
	{
		Memset(pointer,0,BLOCKSIZE(i));
	}
	bfree(pointer,buddy);
}
*/
/*
 * The following functions are for simple tests.
 */
/*
static int count_blocks(int i,buddy_t * buddy) {

  int count = 0;
  void * * p = &(buddy->freelist[i]);

  while (*p != NULL) {
    count++;
    p = (void **) *p;
  }
  return count;
}

int total_free(buddy_t * buddy) {

  int i, bytecount = 0;

  for (i = MIN_ORDER; i <= buddy->order; i++) {
    bytecount += count_blocks(i,buddy) * BLOCKSIZE(i);
  }
  return bytecount;
}

int ispointerinbuddy(void * pointer,buddy_t * buddy)
{
	int offset=pointer-(void *)buddy->pool;
	if((offset>0) && (offset<buddy->poolsize))
		return 1;
	return 0;
}
*/
/*
static void print_list(int i,buddy_t * buddy) {

  printf("freelist[%d]: \n", i);

  void **p = &buddy->freelist[i];
  while (*p != NULL) {
    printf("    0x%08lx, 0x%08lx\n", (uintptr_t) *p, (uintptr_t) *p - (uintptr_t) buddy->pool);
    p = (void **) *p;
  }
}
*/
