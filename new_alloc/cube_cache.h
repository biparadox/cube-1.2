/**
 *
 * File: cube_static.h
 *
 */

#ifndef CUBE_CACHE_H_
#define CUBE_CACHE_H_


/******************************************************************************
 *
 * Definitions
 *
 ******************************************************************************/
#define PAGE_SIZE       4096
#define PAGE_ORDER      12
#define INDEX_OFFSET    256

struct cache_index
{
	UINT16 cache_size;
	UINT16 index_size;
	UINT16 page_num;
	UINT16 free_size;
	UINT16 index_page;
	UINT16 first_page;
	UINT16 curr_page;
}__attribute__((packed));


struct cache_page_index
{
	BYTE empty_slot;
	BYTE index_size;	
	BYTE index[0];
} __attribute__((packed));

UINT32 cache_init(UINT32 addr);

UINT32 calloc(int size);

#endif /* CACHE_H_ */
