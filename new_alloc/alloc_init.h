/**
 * Copyright [2016] Hu jun (algorist@bjut.edu.cn)
 *
 * File: alloc_init.h
 *
 * Created on: Jun 9, 2016
 * Author: Hu jun (algorist@bjut.edu.cn)
 */

#ifndef ALLOC_INIT_H_
#define ALLOC_INIT_H_

#include "../include/data_type.h"
/******************************************************************************
 *
 * Definitions
 *
 ******************************************************************************/
#define MAX_ORDER       20   // 2 ** 26 == 64M bytes
#define MIN_ORDER       4   // 2 ** 4 == 16 bytes
#define PAGE_SIZE       4096
#define PAGE_ORDER      12

enum page_type
{
	EMPTY_PAGE=0x00,
	FIRST_PAGE=0x01,
	PAGE_TABLE,
	STATIC_PAGE,
	SLAB_POINTER,
	SLAB_PAGE,
	DYNAMIC_POINTER,
	DYNAMIC_PAGE,
	TEMPALLOC_PAGE,
	WHOLE_PAGE,
	
};

struct page_index
{
	BYTE  type;
	BYTE  index;
	UINT16 state;
	UINT16 priv_page;
	UINT16 next_page;
}__attribute__((packed));
/*
struct free_page_index
{
	BYTE  type;
	BYTE  index;
	UINT16 state;
	UINT16 priv_page;
	UINT16 next_page;
}__attribute__((packed));

struct static_page_index
{
	BYTE  type;
	BYTE  index;
	UINT16 state;
	UINT16 priv_page;
	UINT16 next_page;
}__attribute__((packed));
*/

struct alloc_total_struct
{
	UINT32 total_size;
	UINT32 pagetable_size;
	UINT32 static_size;
	UINT32 occupy_size;
	UINT16 empty_pages;
	UINT16 temp_pages;
}__attribute__((packed));

struct alloc_segment_address
{
	UINT16 temp_area;
	UINT16 page_table;
	UINT16 static_area;
	UINT16 slab_area;
	UINT16 dynamic_area;
	UINT16 page_lists; 
}__attribute__((packed));


struct page_head
{
	UINT16 occupy_space;
        UINT16 next_page;
	UINT16 priv_page;
	UINT16 Reserved;
}__attribute__((packed));

struct pagetable_sys
{
	UINT32 size;
	UINT32 start;
	UINT32 end;
}__attribute__((packed));

struct static_sys
{
	UINT32 size;
	UINT32 page_table_start;
	UINT32 page_table_end;
}__attribute__((packed));

struct temp_mem_sys
{
	UINT32 order;
	UINT32 size;
	UINT32 freelist;
	UINT32 pool;
}__attribute__((packed));



void * salloc(int size);
int Free(void * addr);
int _salloc_mem_size( );

/*
typedef struct buddy {
  int order;
  int poolsize;
  void ** freelist;  // one more slot for first block in pool
  BYTE * pool;
} buddy_t;

extern void * T_mem_struct;
extern void * G_mem_struct;
extern void * C_mem_struct;

#define BLOCKSIZE(i)    (1 << (i))

// the address of the buddy of a block from freelists[i]. 

static inline int offset(void * b,buddy_t * buddy) 
{return (BYTE *)b-buddy->pool; };
static inline void * buddyof(void * b, int i,buddy_t * buddy)
{
	int off=offset(b,buddy)^(1<<i);
	return (void *)(off+buddy->pool);
};

// the address of the buddy of a block from freelists[i]. 

// not used yet, for higher order memory alignment
#define ROUND4(x)       ((x % 4) ? (x / 4 + 1) * 4 : x)


void * bmalloc(int size, buddy_t * buddy);
void * bmalloc0(int size, buddy_t * buddy);
void bfree(void * block,buddy_t * buddy);
void bfree0(void * block,buddy_t * buddy);


void * buddy_init(int order);
void buddy_clear(buddy_t * buddy);
void buddy_reset(buddy_t * buddy);
void buddy_destroy(buddy_t * buddy);

void print_buddy(buddy_t * buddy);
int total_free(buddy_t * buddy);
int ispointerinbuddy(void * pointer,buddy_t * buddy);
*/
#endif 
