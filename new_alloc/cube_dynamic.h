/**
 *
 * File: cube_static.h
 *
 */

#ifndef CUBE_DYNAMIC_H_
#define CUBE_DYNAMIC_H_

#define CUBE_DMEM_EMPTY 0x00
#define CUBE_DMEM_ARRAY 0x01

/******************************************************************************
 *
 * Definitions
 *
 ******************************************************************************/

struct dmem_sys
{
	UINT32 total_size;  // total dmem size
	UINT16 pages_num;  // dmem pages num
	UINT16 first_page;  //first dmem page
	UINT16 curr_page; // current dmem page
	UINT16 curr_offset; // current dmem page
	UINT32 half_stone;  // 
	UINT32 quad_stone;  // 
}__attribute__((packed));

struct dmem_object_head
{
	BYTE object_type;
	int ref_no;
	UINT32 ref_addr[];
}__attribute__((packed));

struct dmem_head
{
	UINT32 pointer_addr;  // this dmem pointer's addr, we should change the value in this addr when we realloc it
	UINT16 dmem_attr;  // the lowerest 12 bits are dmem's size, the higerest 4 bits are the flag of dmem
}__attribute__((packed));  


struct dmem_page_index
{
	BYTE empty_size;   // how many slot this cache page has
	BYTE index_size;   // this page's index size	
	BYTE index[0];     // this page's index bit array
} __attribute__((packed));

#define DMEM_FLAG_MASK 0xf000
#define DMEM_SIZE_MASK 0x0fff

#define DMEM_GET_SIZE(pointer)    (((struct dmem_head *)((BYTE *)pointer - sizeof(struct dmem_head)))->dmem_attr & DMEM_SIZE_MASK) 
#define DMEM_GET_FLAG(pointer)    ((((struct dmem_head *)((BYTE *)pointer - sizeof(struct dmem_head)))->dmem_attr & DMEM_FLAG_MASK)>>12) 

#endif /* CUBE_DYNAMIC_H_ */
