#include <string.h>
#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/struct_deal.h"
#include "../include/basefunc.h"
#include "../include/memdb.h"
#include "../include/valuelist.h"
#include "../include/list.h"
#include "../include/attrlist.h"
#include "../struct_deal/struct_ops.h"
#include "memdb_internal.h"

const char * nulstring="NULL";

int enumtype_get_text_value(void * addr,char * text,void * elem_template)
{

	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int enum_value;
	NAME2VALUE * enum_list;
	int len;
	int offset=0;
	int i;

	if(elemenumlist==NULL)
		return -EINVAL;
	if(elemenumlist->elemlist==NULL)
		return -EINVAL;

	enum_list=elemenumlist->elemlist;

	enum_value=*(int *)addr;
	if(enum_value<0)
		return -EINVAL;

	for(i=0;enum_list[i].name!=NULL;i++)
	{
		if(enum_list[i].value==enum_value)
		{
			len=Strlen(enum_list[i].name);
			Memcpy(text+offset,enum_list[i].name,len+1);
			offset+=len+1;
			return offset;
		}
	}
	if(enum_value==0)
	{
		len=Strlen(nulstring);
		Memcpy(text+offset,nulstring,len+1);
		offset+=len+1;
		return offset;
	}

	return -EINVAL;
}

int enumtype_set_text_value(void * addr,void * text,void * elem_template){
	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int enum_value;
	NAME2VALUE * enum_list;
	int len;
	int offset=0;
	int i;

	if(elemenumlist==NULL)
		return -EINVAL;
	if(elemenumlist->elemlist==NULL)
		return -EINVAL;

	enum_list=elemenumlist->elemlist;

	if(!strcmp(nulstring,text))
	{
		enum_value=0;
		*(int *)addr=enum_value;
		return 0;
	}
	else
	{
		for(i=0;enum_list[i].name!=NULL;i++)
		{
			if(!strcmp(enum_list[i].name,text))
			{
				enum_value=enum_list[i].value;
				*(int *)addr=enum_value;
				return enum_value;
			}
		}
	}
	return -EINVAL;
}

int recordtype_get_text_value(void * addr,char * text,void * elem_template)
{

	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int enum_value;
	NAME2VALUE * enum_list;
	int len;
	int offset=0;
	int i;

	if(typeenumlist==NULL)
		return -EINVAL;
	if(typeenumlist->elemlist==NULL)
		return -EINVAL;

	enum_list=typeenumlist->elemlist;

	enum_value=*(int *)addr;
	if(enum_value<0)
		return -EINVAL;

	for(i=0;enum_list[i].name!=NULL;i++)
	{
		if(enum_list[i].value==enum_value)
		{
			len=Strlen(enum_list[i].name);
			Memcpy(text+offset,enum_list[i].name,len+1);
			offset+=len+1;
			return offset;
		}
	}
	if(enum_value==0)
	{
		len=Strlen(nulstring);
		Memcpy(text+offset,nulstring,len+1);
		offset+=len+1;
		return offset;
	}

	return -EINVAL;
}

int recordtype_set_text_value(void * addr,void * text,void * elem_template){
	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int enum_value;
	NAME2VALUE * enum_list;
	int len;
	int offset=0;
	int i;

	if(typeenumlist==NULL)
		return -EINVAL;
	if(typeenumlist->elemlist==NULL)
		return -EINVAL;

	enum_list=typeenumlist->elemlist;

	if(!strcmp(nulstring,text))
	{
		enum_value=0;
		*(int *)addr=enum_value;
		return 0;
	}
	else
	{
		for(i=0;enum_list[i].name!=NULL;i++)
		{
			if(!strcmp(enum_list[i].name,text))
			{
				enum_value=enum_list[i].value;
				*(int *)addr=enum_value;
				return enum_value;
			}
		}
	}
	return -EINVAL;
}

int subtype_get_text_value(void * addr,char * text,void * elem){
	struct elem_template * elem_attr=elem;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int enum_value;
	NAME2VALUE * enum_list;
	int len;
	int offset=0;
	int i;
	int defvalue;

	enum_value=*(int *)addr;
	if(enum_value<0)
		return -EINVAL;
	if(enum_value==0)
	{
		len=Strlen(nulstring);
		Memcpy(text+offset,nulstring,len+1);
		offset+=len+1;
		return offset;
	}


	defvalue=_elem_get_defvalue(elem,addr);
	if(defvalue<0)
		return -EINVAL;

	
	enum_list=memdb_get_subtypelist(defvalue);

	if(enum_list==NULL)
	{
		len=Itoa(enum_value,text);
		return len;
	}
	
	for(i=0;enum_list[i].name!=NULL;i++)
	{
		if(enum_list[i].value==enum_value)
		{
			len=Strlen(enum_list[i].name);
			Memcpy(text+offset,enum_list[i].name,len+1);
			offset+=len+1;
			return offset;
		}
	}
	return -EINVAL;
}

int subtype_set_text_value(void * addr,void * text,void * elem){
	struct elem_template * elem_attr=elem;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int enum_value;
	NAME2VALUE * enum_list;
	int len;
	int offset=0;
	int i;
	int defvalue;

	if(!strcmp(nulstring,text))
	{
		enum_value=0;
		*(int *)addr=enum_value;
		return 0;
	}

	defvalue=_elem_get_defvalue(elem,addr);
	if(defvalue<0)
		return -EINVAL;

	enum_list=memdb_get_subtypelist(defvalue);
	if(enum_list==NULL)
	{
		enum_value=Atoi(text,DIGEST_SIZE);
		if(enum_value<0)
			return enum_value;
		*(int *)addr=enum_value;
		return enum_value;
		
	}
	else
	{
		for(i=0;enum_list[i].name!=NULL;i++)
		{
			if(!strcmp(enum_list[i].name,text))
			{
				enum_value=enum_list[i].value;
				*(int *)addr=enum_value;
				return enum_value;
			}
		}
	}
	return -EINVAL;
}

ELEM_OPS enumtype_convert_ops =
{
	.get_text_value = enumtype_get_text_value,
	.set_text_value = enumtype_set_text_value,
};

ELEM_OPS recordtype_convert_ops =
{
	.get_text_value = recordtype_get_text_value,
	.set_text_value = recordtype_set_text_value,
};
ELEM_OPS subtype_convert_ops =
{
	.get_text_value = subtype_get_text_value,
	.set_text_value = subtype_set_text_value,
};
