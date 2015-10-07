/*************************************************
*       Hige security Linux Operating System Project
*
*	File description: 	Definition of data describe struct header file
*	File name:		struct_deal.h
*	date:    	2008-05-09
*	Author:    	Hu jun
*************************************************/
#ifndef  STRUCT_OPS_H
#define  STRUCT_OPS_H
ELEM_OPS string_convert_ops;
ELEM_OPS estring_convert_ops;
ELEM_OPS bindata_convert_ops;
ELEM_OPS define_convert_ops;
ELEM_OPS uuid_convert_ops;
ELEM_OPS enum_convert_ops;
ELEM_OPS flag_convert_ops;


typedef struct tagvalue2pointer
{
	int value;
	void * pointer;
}VALUE2POINTER;

#define DEFINE_TAG  0x00FFF000
extern void ** struct_deal_ops;
struct elem_template
{
	struct struct_elem_attr * elem_desc;
	int offset;
	int size;
	void * ref;	
};
#endif
