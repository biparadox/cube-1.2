/*************************************************
*       Hige security Linux Operating System Project
*
*	File description: 	Definition of data describe struct header file
*	File name:		struct_deal.h
*	date:    	2008-05-09
*	Author:    	Hu jun
*************************************************/
#ifndef  _CUBE_STRUCT_ATTR_H
#define  _CUBE_STRUCT_ATTR_H
#include "../include/struct_deal.h"

enum elem_attr_flag
{
	ELEM_ATTR_POINTER=0x01,
	ELEM_ATTR_DEFINE=0x02,
	ELEM_ATTR_ARRAY=0x04,
	ELEM_ATTR_ENUM=0x10,
	ELEM_ATTR_FLAG=0x20,
	ELEM_ATTR_SUBSET=0x40,
	ELEM_ATTR_VALUE=0x80,
};

void * _elem_get_addr(void * elem,void * addr);
int _elem_get_offset(void * elem);
int _elem_get_defvalue(void * elem,void * addr);
int _elem_set_defvalue(void * elem,void * addr,int value);

#endif
