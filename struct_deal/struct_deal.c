#ifdef KERNEL_MODE

#include <linux/string.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>

#else

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<time.h>
//#include "../include/kernel_comp.h"
//#include "../include/list.h"

#endif

#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/struct_deal.h"
#include "struct_ops.h"

const int deftag=0x00FFF000;
static VALUE2POINTER InitFuncList [] =
{
	{OS210_TYPE_STRING,&string_convert_ops},
	{OS210_TYPE_ESTRING,&estring_convert_ops},
	{OS210_TYPE_BINDATA,&bindata_convert_ops},
	{OS210_TYPE_DEFINE,&define_convert_ops},
	{OS210_TYPE_ENDDATA,NULL},
};

void ** struct_deal_ops;
void * ops_list[OS210_TYPE_ENDDATA];

static inline int struct_register_ops(int value,void * pointer)
{
	if((value<0) || (value>=OS210_TYPE_ENDDATA))
		return -EINVAL;
	struct_deal_ops=&ops_list;
	struct_deal_ops[value]=pointer;
	return 0; 
}


int struct_deal_init()
{
	int count;
	int ret;
	int i;
	for(i=0;i<OS210_TYPE_ENDDATA;i++)
	{
		struct_deal_ops[i]=NULL;
	}
	
	for(i=0;InitFuncList[i].value!=OS210_TYPE_ENDDATA;i++)
	{
		ret=struct_register_ops(InitFuncList[i].value,
			InitFuncList[i].pointer);
		if(ret<0)
			return ret;
	}
	return 0;		
}

enum struct_template_state
{
	STRUCT_TEMP_INIT,
	STRUCT_TEMP_START,
	STRUCT_TEMP_UPDATE,
	STRUCT_TEMP_ERROR,
};


typedef struct struct_template_node
{
	void * parent;
	int offset;
	int size;
	int elem_no;
	void * struct_desc;
	struct elem_template * elem_list;
	int temp_var;
}STRUCT_NODE;

static inline int _count_struct_num(struct struct_elem_attr * struct_desc)
{
	int i=0;
	while(struct_desc[i].name!=NULL)
	{
		if(struct_desc[i].type==OS210_TYPE_ENDDATA)
			break;
		i++;
	}
	return i;
}

const int max_name_len=64;

static inline STRUCT_NODE * _get_root_node(STRUCT_NODE * start_node)
{
	while( start_node->parent != NULL)
		start_node=start_node->parent;
	return start_node;
}

static inline char * _get_next_name(char * name,char * buffer)
{
	int i;
	for(i=0;i<max_name_len+1;i++)
	{
		if(name[i]==0)
		{
			buffer[i]=0;
			return NULL;
		}
		if(name[i]=='.')
		{
			buffer[i]=0;
			return name+i+1;
		}
		buffer[i]=name[i];
	}
	buffer[0]=0;
	return NULL;
}

static inline struct elem_template * _get_elem_from_struct (STRUCT_NODE * node, char * name)
{
	int i;
	struct struct_elem_attr * curr_desc=node->struct_desc;
	for(i=0;i<node->elem_no;i++)
	{
		if(strcmp(curr_desc[i].name,name)==0)
			return &(node->elem_list[i]);
	}	
	return NULL;
}

void * _get_elem_by_name(void * start_node, char * name)
{
	
	char buffer[65];
	STRUCT_NODE * node = start_node;
	
	struct elem_template *  curr_elem;
	
	if(name[0]=='/')
	{
		node=_get_root_node(node);
		name = name+1;
	}
	do
	{
		name=_get_next_name(name,buffer);
		if(buffer[0]==0)
			return NULL;
		curr_elem=_get_elem_from_struct(node,buffer);
		if(name==NULL)	
			return curr_elem;
		if(curr_elem->elem_desc->type != OS210_TYPE_ORGCHAIN)
			return NULL;
		node=curr_elem->ref;
	}while(node != NULL);
	return NULL;
}

static inline int _isdefineelem(int type)
{
	switch(type)
	{
		case OS210_TYPE_DEFINE:
		case OS210_TYPE_DEFSTR:
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


void * create_struct_template(struct struct_elem_attr * struct_desc)
{
	STRUCT_NODE * root_node;
	STRUCT_NODE * curr_node;
	STRUCT_NODE * temp_node;
	struct elem_template * curr_elem;
	int offset=0;
	int i;
	struct struct_elem_attr * curr_desc=struct_desc;
	struct struct_elem_attr * elem_desc;
	ELEM_OPS * elem_ops;
	int ret;

	root_node=(STRUCT_NODE *)Calloc0(sizeof(STRUCT_NODE));
	curr_node=root_node;
	root_node->struct_desc=struct_desc;
	offset=0;

	do {
	// first step: count elem no and alloc all the struct node;
		if(curr_node->elem_list==NULL)
		{
			curr_desc=curr_node->struct_desc;
			curr_node->elem_no=_count_struct_num(curr_desc);
			if(curr_node->elem_no<=0)
				return NULL;
			ret=Galloc0(&(curr_node->elem_list),sizeof(struct elem_template )*curr_node->elem_no);
			if(ret<0)
				return NULL;
			curr_node->offset=offset;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		elem_desc=&curr_desc[curr_node->temp_var];
			
		if((elem_desc->type<0)||(elem_desc->type>OS210_TYPE_ENDDATA))
			return NULL;
				
		if(elem_desc->name==NULL)
		{
			curr_node->size=offset-curr_node->offset;
			curr_node=curr_node->parent;
			if(curr_node==NULL)
				break;
			curr_desc=curr_node->struct_desc;
			continue;
		}
			// process the child struct
		if(elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			ret=Galloc0(&(curr_elem->ref),sizeof(STRUCT_NODE));
			if(ret<0)
				return NULL;
			temp_node=(STRUCT_NODE *)curr_elem->ref;
			temp_node->parent=curr_node;
			temp_node->struct_desc=elem_desc->ref;
			curr_node->temp_var++;
			curr_node=temp_node;
			curr_elem->elem_desc=elem_desc;
			curr_elem->offset=offset;
			curr_node->offset=offset;
			continue;	
		}
		if(elem_desc->type==OS210_TYPE_ENDDATA)
		{
			curr_node->size=offset-curr_node->offset;
			curr_node=curr_node->parent;
			if(curr_node==NULL)
				break;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		// process the defined elem
		if(_isdefineelem(elem_desc->type))
		{
			struct elem_template * temp_elem;
			temp_elem  = _get_elem_by_name(curr_node,elem_desc->ref);
			if(temp_elem==NULL)
				return NULL;
			if(temp_elem->elem_desc==NULL)
				return NULL;
			if(!_isvalidvalue(temp_elem->elem_desc->type))
				return NULL;
			temp_elem->ref = (void *)deftag;

			curr_elem->ref=temp_elem;
			elem_ops=struct_deal_ops[elem_desc->type];
			if(elem_ops==NULL)
				return NULL;
			curr_elem->elem_desc=elem_desc;
			curr_elem->offset=offset;
			curr_elem->size=elem_desc->size;
			offset+=curr_elem->size;
		}
		else
		{
			// get this elem's ops
			elem_ops=struct_deal_ops[elem_desc->type];
			if(elem_ops==NULL)
				return NULL;
			curr_elem->elem_desc=elem_desc;
			curr_elem->ref=curr_elem->elem_desc->ref;
			curr_elem->offset=offset;
			if(elem_ops->elem_size==NULL)
			{
				curr_elem->size=elem_desc->size;
				offset+=curr_elem->size;
			}
			else
			{
				curr_elem->size=elem_ops->elem_size(curr_elem->elem_desc);
				offset+=curr_elem->size;
			}
		}
		curr_node->temp_var++;
	}while(1);

	return root_node;

}

void free_struct_template(void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;

	do{
		if(curr_node->temp_var == curr_node->elem_no)
		{
			temp_node=curr_node;
			curr_node=curr_node->parent;
			if(curr_node==NULL)
				break;
			if(temp_node->elem_list!=NULL)
				Gfree(temp_node->elem_list);
			if(temp_node!=root_node)
				Gfree(temp_node);
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			continue;
		}
		curr_node->temp_var++;
	}while(1);
	Cfree(root_node);
}

int struct_2_blob(void * addr, void * blob, void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int addr_offset=0;
	int blob_offset=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int ret;

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			temp_node=curr_node;
			curr_node=curr_node->parent;
			if(curr_node==NULL)
				break;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_desc[curr_node->temp_var].type];
		if(elem_ops==NULL)
			return -EINVAL;
		addr_offset=curr_elem->offset;
		if(elem_ops->elem_2_blob==NULL)
		{
			memcpy(blob+blob_offset,addr+addr_offset,curr_elem->size);
			blob_offset+=curr_elem->size;
		}
		else
		{
			ret=elem_ops->elem_2_blob(addr+addr_offset,blob+blob_offset,curr_elem);
			if(ret<0)
				return ret;
			blob_offset+=ret;
		}
		curr_node->temp_var++;
	}while(1);
	return blob_offset;
}
int blob_2_struct(void * blob, void * addr, void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int addr_offset=0;
	int blob_offset=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int ret;

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			temp_node=curr_node;
			curr_node=curr_node->parent;
			if(curr_node==NULL)
				break;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_desc[curr_node->temp_var].type];
		if(elem_ops==NULL)
			return -EINVAL;
		addr_offset=curr_elem->offset;
		if(elem_ops->elem_2_blob==NULL)
		{
			memcpy(addr+addr_offset,blob+blob_offset,curr_elem->size);
			blob_offset+=curr_elem->size;
		}
		else
		{
			ret=elem_ops->blob_2_elem(addr+addr_offset,blob+blob_offset,curr_elem);
			if(ret<0)
				return ret;
			blob_offset+=ret;
		}
		curr_node->temp_var++;
	}while(1);
	return blob_offset;
}

int struct_read_elem(char * name,void * addr, void * elem_data,void * struct_template)
{
	STRUCT_NODE * curr_node=struct_template;
	ELEM_OPS * elem_ops;
	struct struct_elem_attr * curr_desc;
	int ret;
	struct elem_template * curr_elem= _get_elem_by_name(curr_node,name);
	if(curr_elem==NULL)
		return -EINVAL;
	curr_desc=curr_elem->elem_desc;
	elem_ops=struct_deal_ops[curr_desc->type];
	if(elem_ops==NULL)
		return -EINVAL;
	if(elem_ops->elem_2_blob==NULL)
	{
		ret=curr_elem->size;
		memcpy(elem_data,addr+curr_elem->offset,curr_elem->size);
	}
	else
	{
		ret=elem_ops->elem_2_blob(addr+curr_elem->offset,elem_data,curr_elem);
	}
	return ret;
}

int _struct_get_elem_value(char * name, void * addr,void * struct_template)
{
	char elem_data[64];
	STRUCT_NODE * curr_node=struct_template;
	ELEM_OPS * elem_ops;
	struct struct_elem_attr * curr_desc;
	int ret;
	struct elem_template * curr_elem= _get_elem_by_name(curr_node,name);
	if(curr_elem==NULL)
		return -EINVAL;
	curr_desc=curr_elem->elem_desc;
	elem_ops=struct_deal_ops[curr_desc->type];
	if(elem_ops==NULL)
		return -EINVAL;
	if(elem_ops->get_value==NULL)
	{
		return -EINVAL;
	}
	else
	{
		ret=elem_ops->get_value(addr+curr_elem->offset,curr_elem);
	}
	return ret;
}
