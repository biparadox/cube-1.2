/*************************************************
*       Hige security Linux Operating System Project
*
*	File description: 	Definition of data describe struct header file
*	File name:		struct_deal.h
*	date:    	2008-05-09
*	Author:    	Hu jun
*************************************************/
#ifndef  STRUCT_ATTR_H
#define  STRUCT_ATTR_H

static inline int _isdefineelem(int type)
{
	switch(type)
	{
		case OS210_TYPE_DEFINE:
		case OS210_TYPE_DEFSTR:
		case OS210_TYPE_DEFSTRARRAY:
		case OS210_TYPE_DEFUUIDARRAY:
		case OS210_TYPE_DEFNAMELIST:
			return 1;
		default:
			return 0;
	}
}

static inline int _isarrayelem(int type)
{
	switch(type)
	{
		case OS210_TYPE_UUIDARRAY:
		case OS210_TYPE_DEFUUIDARRAY:
		case OS210_TYPE_DEFNAMELIST:
		case OS210_TYPE_BINARRAY:
		case OS210_TYPE_DEFSTRARRAY:
			return 1;
		default:
			return 0;
	}
}

static inline int _isvalidvalue(int type)
{
	switch(type)
	{
		case OS210_TYPE_STRING:
		case OS210_TYPE_ESTRING:
		case OS210_TYPE_INT:
		case OS210_TYPE_UCHAR:
		case OS210_TYPE_USHORT:
		case OS210_TYPE_LONGLONG:
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
		case OS210_TYPE_ESTRING:
		case OS210_TYPE_DEFINE:
		case OS210_TYPE_DEFSTR:
		case OS210_TYPE_DEFSTRARRAY:
		case OS210_TYPE_UUIDARRAY:
		case OS210_TYPE_DEFUUIDARRAY:
		case OS210_TYPE_DEFNAMELIST:
			return 1;
		default:
			return 0;
	}
}

static inline int _elem_get_defvalue(void * elem)
{
	struct elem_template * curr_elem=elem;
	struct elem_template * temp_elem=curr_elem->def;
	if(temp_elem==NULL)
		return -EINVAL;
	return (int)temp_elem->def & 0x00000FFF;
}

#endif
