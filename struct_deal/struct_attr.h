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

static inline int _isdefineelem(int type)
{
	switch(type)
	{
		case CUBE_TYPE_DEFINE:
		case CUBE_TYPE_DEFSTR:
		case CUBE_TYPE_DEFSTRARRAY:
		case CUBE_TYPE_DEFUUIDARRAY:
		case CUBE_TYPE_DEFNAMELIST:
			return 1;
		default:
			return 0;
	}
}

static inline int _isarrayelem(int type)
{
	switch(type)
	{
		case CUBE_TYPE_UUIDARRAY:
		case CUBE_TYPE_DEFUUIDARRAY:
		case CUBE_TYPE_DEFNAMELIST:
		case CUBE_TYPE_BINARRAY:
		case CUBE_TYPE_DEFSTRARRAY:
		case CUBE_TYPE_ARRAY:
			return 1;
		default:
			return 0;
	}
}

static inline int _isvalidvalue(int type)
{
	switch(type)
	{
		case CUBE_TYPE_STRING:
		case CUBE_TYPE_ESTRING:
		case CUBE_TYPE_INT:
		case CUBE_TYPE_UCHAR:
		case CUBE_TYPE_USHORT:
		case CUBE_TYPE_LONGLONG:
		case TPM_TYPE_UINT64:
		case TPM_TYPE_UINT32:
		case TPM_TYPE_UINT16:
			return 1;
		default:
			return 0;
	}
}


static inline int _ispointerelem(int type)
{
	switch(type)
	{
		case CUBE_TYPE_ESTRING:
		case CUBE_TYPE_DEFINE:
		case CUBE_TYPE_DEFSTR:
		case CUBE_TYPE_DEFSTRARRAY:
		case CUBE_TYPE_UUIDARRAY:
		case CUBE_TYPE_DEFUUIDARRAY:
		case CUBE_TYPE_DEFNAMELIST:
			return 1;
		default:
			return 0;
	}
}

static inline int _issubsetelem(int type)
{
	switch(type)
	{
		case CUBE_TYPE_SUBSTRUCT:
		case CUBE_TYPE_ARRAY:
			return 1;
		default:
			return 0;
	}
}

static inline int _is_elem_in_subset(void * elem,void * subset)
{
	struct elem_template * curr_elem=elem;
	struct elem_template * curr_subset=subset;
	
	if(_issubsetelem(curr_subset->elem_desc->type))
	{
		while(curr_elem->father!=NULL)
		{
			if(curr_elem->father==subset)
				return 1;
			curr_elem=curr_elem->father;
		}
	}
	return 0;
}


static inline int _elem_get_offset(void * elem)
{
	int offset=0;
	struct elem_template * curr_elem=elem;

	while(curr_elem->elem_desc!=NULL)
	{
		if(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY)
			offset+=curr_elem->offset +
				curr_elem->size * curr_elem->index;
		else
			offset+=curr_elem->offset;
		
		curr_elem=curr_elem->father;
	}
	return offset;	
	
}

static inline int _elem_get_defvalue(void * elem,void * addr)
{
	struct elem_template * curr_elem=elem;
	struct elem_template * temp_elem=curr_elem->def;
	struct elem_template * temp_subset;
	ELEM_OPS * elem_ops;
	int define_value;
	int offset;
	if(temp_elem==NULL)
		return -EINVAL;
	elem_ops=struct_deal_ops[temp_elem->elem_desc->type];
	if(elem_ops==NULL)
		return NULL;

	if(elem_ops->get_int_value == NULL)
		return -EINVAL;

	// now compute the define elem's offset

	// if define elem is the first layer child
	if(temp_elem->father->elem_desc!=NULL)
	{
		if(!_is_elem_in_subset(curr_elem,temp_elem->father))
			return -EINVAL;	
	}

	offset=_elem_get_offset(temp_elem);
	// if define elem is an elem in curr_elem's father subset
	define_value=elem_ops->get_int_value(addr+offset,curr_elem);
	if((define_value<0) || (define_value >=1024))
		return -EINVAL;
	return define_value;
}

#endif
