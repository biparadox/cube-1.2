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

struct cache_sys
{
	UINT16 pages_num;  // cache pages num
	BYTE   index_num;  // cache index pages's num
	UINT16 index_offset; // index page offset
	UINT32 total_size;  // total cache pages
}__attribute__((packed));

struct cache_index
{
	UINT16 cache_size;  // this cache's size
	UINT16 index_size;  // this cache's index size
	UINT16 page_num;    // cache page num
	UINT16 index_slot;  // cache index slot
	UINT16 index_page;  // cache index page
	UINT16 first_page;  // cache's first page 
	UINT16 curr_page;   // cache's index page
}__attribute__((packed));  


struct cache_page_index
{
	BYTE empty_slot;   // how many slot this cache page has
	BYTE index_size;   // this page's index size	
	BYTE index[0];     // this page's index bit array
} __attribute__((packed));

#endif /* CACHE_H_ */
