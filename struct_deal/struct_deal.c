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
#include "struct_internal.h"
#include "../include/string.h"

static VALUE2POINTER InitFuncList [] =
{
	{OS210_TYPE_STRING,&string_convert_ops},
	{OS210_TYPE_ESTRING,&estring_convert_ops},
	{OS210_TYPE_BINDATA,&bindata_convert_ops},
	{OS210_TYPE_DEFINE,&define_convert_ops},
	{OS210_TYPE_UUID,&uuid_convert_ops},
	{OS210_TYPE_INT,&int_convert_ops},
	{OS210_TYPE_ENUM,&enum_convert_ops},
	{OS210_TYPE_FLAG,&flag_convert_ops},
	{OS210_TYPE_ENDDATA,NULL},
};

void ** struct_deal_ops;
static void * ops_list[OS210_TYPE_ENDDATA];

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
	struct_deal_ops=&ops_list;
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
	int flag;
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

const int max_name_len=DIGEST_SIZE;

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


static inline int _ispointerelem(int type)
{
	switch(type)
	{
		case OS210_TYPE_ESTRING:
		case OS210_TYPE_DEFINE:
		case OS210_TYPE_DEFSTR:
		case OS210_TYPE_DEFSTRARRAY:
			return 1;
		default:
			return 0;
	}
}

int get_fixed_elemsize(int type)
{

	switch(type)
	{
		case OS210_TYPE_STRING:
			return -1;
		case OS210_TYPE_UUID:
			return DIGEST_SIZE;
		case OS210_TYPE_ENUM:
		case OS210_TYPE_FLAG:
		case OS210_TYPE_TIME:
			return sizeof(int);
		case OS210_TYPE_BINDATA: 
		case OS210_TYPE_HEXDATA:	 
			return -1;
		case OS210_TYPE_ESTRING:
		case OS210_TYPE_JSONSTRING:
		case OS210_TYPE_DEFINE:
		case OS210_TYPE_DEFSTR:	
		case OS210_TYPE_DEFSTRARRAY:
			return sizeof(char *);
		case OS210_TYPE_BINARRAY:
		case OS210_TYPE_BITMAP:	 
			return -1;
		case OS210_TYPE_INT:
			return sizeof(int);
		case OS210_TYPE_UCHAR:    
			return sizeof(char);
		case OS210_TYPE_USHORT:   
			return sizeof(short);
		case OS210_TYPE_LONGLONG: 
		case TPM_TYPE_UINT64:
			return sizeof(long long);
		case TPM_TYPE_UINT32:
			return sizeof(int);
		case TPM_TYPE_UINT16:
			return 2;
		case OS210_TYPE_NODATA:
			return 0;
        	case OS210_TYPE_CHOICE:
		case OS210_TYPE_ENDDATA:
			return 0;
		case OS210_TYPE_ORGCHAIN:
			return -1;
		default:
			break;
	}
	return -1;	
}

static inline int _getelemjsontype(int type)
{
	switch(type)
	{
		case OS210_TYPE_STRING:
		case OS210_TYPE_UUID:
		case OS210_TYPE_ENUM:
		case OS210_TYPE_FLAG:
		case OS210_TYPE_TIME:
		case OS210_TYPE_BINDATA: 
		case OS210_TYPE_HEXDATA:	 
		case OS210_TYPE_ESTRING:
		case OS210_TYPE_JSONSTRING:
		case OS210_TYPE_DEFINE:
		case OS210_TYPE_DEFSTR:	
			return JSON_ELEM_STRING;
		case OS210_TYPE_DEFSTRARRAY:
		case OS210_TYPE_BINARRAY:
		case OS210_TYPE_BITMAP:	 
			return JSON_ELEM_ARRAY;
		case OS210_TYPE_INT:
		case OS210_TYPE_UCHAR:    
		case OS210_TYPE_USHORT:   
		case OS210_TYPE_LONGLONG: 
		case TPM_TYPE_UINT64:
		case TPM_TYPE_UINT32:
		case TPM_TYPE_UINT16:
			return JSON_ELEM_NUM;
		case OS210_TYPE_NODATA:
        	case OS210_TYPE_CHOICE:
		case OS210_TYPE_ENDDATA:
			return 0;
		case OS210_TYPE_ORGCHAIN:
			return JSON_ELEM_MAP;
		
		default:
			break;
	}
	return -EINVAL;
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
			ret=Palloc0(&(curr_node->elem_list),sizeof(struct elem_template )*curr_node->elem_no);
			if(ret<0)
				return NULL;
			for(i=0;i<curr_node->elem_no;i++)
			{
				curr_node->elem_list[i].father=curr_node;
			}
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
			ret=Palloc0(&(curr_elem->ref),sizeof(STRUCT_NODE));
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
			temp_elem->ref = (void *)DEFINE_TAG;

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
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			if(temp_node->elem_list!=NULL)
				Free(temp_node->elem_list);
			if(temp_node!=root_node)
				Free(temp_node);
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
	Free(root_node);
}

struct elem_template * _get_last_elem(void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	struct elem_template * last_elem;
	STRUCT_NODE * temp_node;

	last_elem=&root_node->elem_list[root_node->elem_no-1];
	while(last_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
	{
		last_elem=&root_node->elem_list[root_node->elem_no-1];
	}
	return last_elem;
}

int struct_size(void * struct_template)
{
	struct elem_template * last_elem=_get_last_elem(struct_template);
	ELEM_OPS * elem_ops;
	elem_ops=struct_deal_ops[last_elem->elem_desc->type];
	if((elem_ops==NULL)|| (elem_ops->elem_size==NULL))
		return last_elem->offset+last_elem->elem_desc->size;
	return last_elem->offset+elem_ops->elem_size(last_elem);
		
} 

int struct_free_alloc(void * addr,void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int ret;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			continue;
		}
		// get this elem's ops
		if(_ispointerelem(curr_elem->elem_desc->type))
		{
			Free(*(void **)(addr+curr_elem->offset)); 
		}
		curr_node->temp_var++;
	}while(1);
	return 0;
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
	int def_value;
	int ret;

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
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
		if(elem_ops->get_bin_value==NULL)
		{
			memcpy(blob+blob_offset,addr+addr_offset,curr_elem->size);
			blob_offset+=curr_elem->size;
		}
		else
		{
			ret=elem_ops->get_bin_value(addr+addr_offset,blob+blob_offset,curr_elem);
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
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
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
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;
		// pre_fretch the define value
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = elem_ops->get_int_value(addr+curr_elem->offset,curr_elem);
			if((define_value<0)||define_value>=1024)
				return define_value;
			curr_elem->ref=(int)curr_elem->ref&DEFINE_TAG+define_value;			
		}
		addr_offset=curr_elem->offset;
		if(elem_ops->get_bin_value==NULL)
		{
			memcpy(addr+addr_offset,blob+blob_offset,curr_elem->size);
			blob_offset+=curr_elem->size;
		}
		else
		{
			ret=elem_ops->set_bin_value(addr+addr_offset,blob+blob_offset,curr_elem);
			if(ret<0)
				return ret;
			blob_offset+=ret;
		}
		curr_node->temp_var++;
	}while(1);
	return blob_offset;
}

int struct_2_text(void * addr, char * text, void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int str_offset=0;
	int text_len=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int ret;

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
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
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;
		// pre_fretch the define value
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = elem_ops->get_int_value(addr+curr_elem->offset,curr_elem);
			if((define_value<0)||define_value>=1024)
				return define_value;
			curr_elem->ref=((int)curr_elem->ref&DEFINE_TAG)+define_value;			
		}
		if(elem_ops->get_text_value==NULL)
		{
			if(_ispointerelem(curr_elem->elem_desc->type))
			{
				text_len=strlen(*(char **)(addr+curr_elem->offset))+1;
				if((text_len<0)||(text_len>1024))
					return -EINVAL;
				strncpy(text+str_offset,*(char **)(addr+curr_elem->offset),text_len);
			}
			else
			{
				text_len=strnlen(addr+curr_elem->offset,curr_elem->size)+1;
				if(text_len>curr_elem->size)
					text_len=curr_elem->size;
				strncpy(text+str_offset,addr+curr_elem->offset,text_len);
			}
			str_offset+=text_len;
		}
		else
		{
			text_len=elem_ops->get_text_value(addr+curr_elem->offset,text+str_offset,curr_elem);
			if(text_len<0)
				return text_len;
			str_offset+=text_len;
		}
		curr_node->temp_var++;
	}while(1);
	return str_offset;
}

int text_2_struct(char * text, void * addr, void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int text_len=0;
	int str_offset=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int ret;

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
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
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;
		// pre_fretch the define value
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = get_string_value(text+str_offset,curr_elem);
			if((define_value<0)||define_value>=1024)
				return define_value;
			curr_elem->ref=(int)curr_elem->ref&DEFINE_TAG+define_value;			
		}
		if(elem_ops->set_text_value==NULL)
		{
			if(_ispointerelem(curr_elem->elem_desc->type))
			{
				text_len=strlen(text+str_offset)+1;
				if((text_len<0)||(text_len>1024))
					return -EINVAL;
				ret=Palloc(addr+curr_elem->offset,text_len);
				if(ret<0)
					return ret;
				strncpy(*(char **)(addr+curr_elem->offset),text+str_offset,text_len);
			}
			else
			{
				text_len=strnlen(text+str_offset,curr_elem->size)+1;
				if(text_len>curr_elem->size)
					text_len=curr_elem->size;
				strncpy(addr+curr_elem->offset,text+str_offset,text_len);
			}
			str_offset+=text_len;
		}
		else
		{
			text_len=elem_ops->set_text_value(text,addr+curr_elem->offset,curr_elem);
			if(text_len<0)
				return text_len;
			str_offset+=text_len;
		}
		curr_node->temp_var++;
	}while(1);
	return str_offset;
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
	if(elem_ops->get_bin_value==NULL)
	{
		ret=curr_elem->size;
		memcpy(elem_data,addr+curr_elem->offset,curr_elem->size);
	}
	else
	{
		ret=elem_ops->get_bin_value(addr+curr_elem->offset,elem_data,curr_elem);
	}
	return ret;
}

int struct_read_elem_text(char * name,void * addr, char * text,void * struct_template)
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
	if(elem_ops->get_text_value==NULL)
	{
		ret=curr_elem->size;
		memcpy(text,addr+curr_elem->offset,curr_elem->size);
	}
	else
	{
		ret=elem_ops->get_text_value(addr+curr_elem->offset,text,curr_elem);
	}
	return ret;
}
int struct_write_elem(char * name,void * addr, void * elem_data,void * struct_template)
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
	if(elem_ops->set_bin_value==NULL)
	{
		ret=curr_elem->size;
		memcpy(addr+curr_elem->offset,elem_data,curr_elem->size);
	}
	else
	{
		ret=elem_ops->set_bin_value(addr+curr_elem->offset,elem_data,curr_elem);
	}
	return ret;
}

int struct_write_elem_text(char * name,void * addr, char * text,void * struct_template)
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
	if(elem_ops->set_text_value==NULL)
	{
		ret=curr_elem->size;
		memcpy(addr+curr_elem->offset,text,curr_elem->size);
	}
	else
	{
		ret=elem_ops->set_text_value(addr+curr_elem->offset,text,curr_elem);
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
	if(elem_ops->get_int_value==NULL)
	{
		return -EINVAL;
	}
	else
	{
		ret=elem_ops->get_int_value(addr+curr_elem->offset,curr_elem);
	}
	return ret;
}

int _getjsonstr(char * json_str,char * text,int text_len,int json_type)
{
	int str_offset=0;
	int str_len=0;
	int i=0;
	switch(json_type)
	{
		case JSON_ELEM_NUM:
			str_len=strnlen(text,text_len);
			memcpy(json_str,text,str_len);
			str_offset=str_len;
			break;
		case JSON_ELEM_STRING:
			str_len=strnlen(text,text_len);
			*json_str='\"';
			memcpy(json_str+1,text,str_len);
			*(json_str+str_len+1)='\"';
			str_offset=str_len+2;
			break;
		case JSON_ELEM_ARRAY:
			*json_str='[';
			*(json_str+1)='\"';
			str_offset+=2;
			for(i=0;i<text_len;i++)
			{
				if(text[i]==0)
				{	
					*(json_str+str_offset)='\"';
					*(json_str+str_offset+1)=',';
					*(json_str+str_offset+2)='\"';
					str_offset+=3;	
				}
				else
				{
					*(json_str+str_offset)=*(text+i);
					str_offset++;
				}
			}
			if(*(text+i-1)!=0)
			{
				*(json_str+str_offset)='\"';
				*(json_str+str_offset+1)=',';
				str_offset+=2;	
			}
			else
			{
				*(json_str+str_offset-1)=']';
			}
			break;
	
		default:
			return -EINVAL;

	}
	return str_offset;	
}

int struct_2_json(void * addr, char * json_str, void * struct_template)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int str_offset;
	int text_len=0;
	int str_len=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int ret;
	char buf[1024];

	curr_desc=root_node->struct_desc;
	*json_str='{';
	str_offset=1;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			*(json_str+str_offset)='}';
			str_offset++;	
			*(json_str+str_offset)=',';
			str_offset++;	
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			curr_desc=curr_node->struct_desc;

			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		// print this elem's name
		text_len=strnlen(curr_elem->elem_desc->name,64);
		*(json_str+str_offset)='\"';
		memcpy(json_str+str_offset+1,curr_elem->elem_desc->name,text_len);
		str_offset+=text_len+1;
		*(json_str+str_offset)='\"';
		*(json_str+str_offset+1)=':';
		str_offset+=2;
		// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_desc=curr_node->struct_desc;
	
			*(json_str+str_offset)='{';
			str_offset+=1;

			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;
		// pre_fretch the define value
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = elem_ops->get_int_value(addr+curr_elem->offset,curr_elem);
			if((define_value<0)||define_value>=1024)
				return define_value;
			curr_elem->ref=((int)curr_elem->ref&DEFINE_TAG)+define_value;			
		}
		if(elem_ops->get_text_value==NULL)
		{
			if(_ispointerelem(curr_elem->elem_desc->type))
			{
				text_len=strlen(*(char **)(addr+curr_elem->offset));
				if((text_len<0)||(text_len>1024))
					return -EINVAL;
				strncpy(buf,*(char **)(addr+curr_elem->offset),text_len);
			}
			else
			{
				text_len=strnlen(addr+curr_elem->offset,curr_elem->size)+1;
				if(text_len>curr_elem->size)
					text_len=curr_elem->size;
				strncpy(buf,addr+curr_elem->offset,text_len);
			}
		}
		else
		{
			text_len=elem_ops->get_text_value(addr+curr_elem->offset,buf,curr_elem);
			if(text_len<0)
				return text_len;
		}
		str_len = _getjsonstr(json_str+str_offset,buf,text_len,_getelemjsontype(curr_elem->elem_desc->type));
		*(json_str+str_offset+str_len)=',';
		str_offset+=str_len+1;
		curr_node->temp_var++;
	}while(1);
	*(json_str+str_offset-1)=0;
	return str_offset;
}

int json_2_struct(void * root,void * addr,void * struct_template)
{
	JSON_NODE * root_json_node = (JSON_NODE *)root;
	JSON_NODE * curr_json_node = (JSON_NODE *)root;
	JSON_NODE * temp_json_node = (JSON_NODE *)root;

	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;

	// json node init
	if(json_get_type(root_json_node) != JSON_ELEM_MAP)
		return -EINVAL;
//	curr_json_node=get_first_json_child(root_json_node);

	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int text_len=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int ret;

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			curr_json_node=get_json_father(curr_json_node);
			curr_desc=curr_node->struct_desc;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		curr_node->temp_var++;
		// find this elem's node
		temp_json_node=find_json_elem(curr_elem->elem_desc->name,curr_json_node);
		// if there is no json node for this elem,use default value
		if(temp_json_node == NULL)
		{
			continue;
		}
		
		// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			// search if there has a json node)
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_desc=curr_node->struct_desc;
			curr_json_node=temp_json_node;
			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;
		// pre_fretch the define value
		char * value_str = json_get_valuestr(temp_json_node);
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = get_string_value(value_str,curr_elem);
			if((define_value<0)||define_value>=1024)
				return define_value;
			curr_elem->ref=(int)curr_elem->ref&DEFINE_TAG+define_value;			
		}
		if(elem_ops->set_text_value==NULL)
		{
			if(_ispointerelem(curr_elem->elem_desc->type))
			{
				text_len=strlen(value_str)+1;
				if((text_len<0)||(text_len>1024))
					return -EINVAL;
				ret=Palloc(addr+curr_elem->offset,text_len);
				if(ret<0)
					return ret;
				strncpy(*(char **)(addr+curr_elem->offset),value_str,text_len);
			}
			else
			{
				text_len=strnlen(value_str,curr_elem->size)+1;
				if(text_len>curr_elem->size)
					text_len=curr_elem->size;
				strncpy(addr+curr_elem->offset,value_str,text_len);
			}
		}
		else
		{
			text_len=elem_ops->set_text_value(addr+curr_elem->offset,value_str,curr_elem);
			if(text_len<0)
				return text_len;
		}
	}while(1);
	return 1;
}

int struct_set_allflag(void * struct_template,int flag)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;

	do{
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_node->flag |=flag;
			continue;
		}
		else
		{
			curr_elem->flag |=flag;
		}
		curr_node->temp_var++;
		
	}while(1);
	root_node->flag |=flag;
	return 0;
}

int struct_clear_allflag(void * struct_template,int flag)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;

	do{
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_node->flag &=~flag;
			continue;
		}
		else
		{
			curr_elem->flag &=~flag;
		}
		curr_node->temp_var++;
		
	}while(1);
	root_node->flag !=flag;
	return 0;
}

int struct_set_flag(void * struct_template,int flag, char * namelist)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * father_node=NULL;
	struct elem_template * curr_elem;
	int ret;
	char name[DIGEST_SIZE+1];
	int offset=0;
	int i;
	int count=0;

	while((i=Getfiledfromstr(name,namelist+offset,',',DIGEST_SIZE+1))>0)
	{
		offset+=i;
		curr_elem = _get_elem_by_name(root_node,name);
		if(curr_elem==NULL)
			return -EINVAL;
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			ret=struct_set_allflag(curr_elem->ref,flag);
		}
		else
		{
			curr_elem->flag |= flag;
		}
		count++;
		if(namelist[offset]==',')
			offset++;
		father_node=curr_elem->father;
		do{
			father_node->flag |= flag;
			if(father_node==root_node)
				break;
			father_node=father_node->parent;
		}while(1);

	}
	return count;
}

int struct_clear_flag(void * struct_template,int flag, char * namelist)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * father_node;
	STRUCT_NODE * temp_node;
	struct elem_template * curr_elem;
	struct elem_template * temp_elem;
	int ret;
	char name[DIGEST_SIZE+1];
	int offset=0;
	int i;
	int count=0;

	while((i=Getfiledfromstr(name,namelist+offset,',',DIGEST_SIZE+1))>0)
	{
		offset+=i;
		curr_elem = _get_elem_by_name(root_node,name);
		if(curr_elem==NULL)
			return -EINVAL;
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			ret=struct_set_allflag(curr_elem->ref,flag);
		}
		else
		{
			curr_elem->flag &= ~flag;
		}
		count++;
		if(namelist[offset]==',')
			offset++;
		father_node=curr_elem->father;
		do {
			for(i=0;i<father_node->elem_no;i++)
			{	
				temp_elem=&father_node->elem_list[i];
				if(temp_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
				{
					temp_node=temp_elem->ref;
					if(temp_node->flag &flag)
						break;
				}
				else
				{
					if(temp_elem->flag &flag)
						break;

				}
			}
			if(i<father_node->elem_no)
				break;
			father_node->flag &=~flag;
			if(father_node==root_node)
				break;
			father_node=father_node->parent;
		}while(1);
	}
	return count;
	
}

int struct_get_flag(void * struct_template,char * name)
{
	STRUCT_NODE * root_node=struct_template;
	struct elem_template * curr_elem;
	int ret;

	curr_elem = _get_elem_by_name(root_node,name);
	if(curr_elem==NULL)
		return -1;
	return curr_elem->flag;
}


int struct_2_part_blob(void * addr,void * blob, void * struct_template,int flag)
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
	int def_value;
	int ret;

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			temp_node=curr_elem->ref;
			if(!(temp_node->flag &flag))
			{
				curr_node->temp_var++;
				continue;
			}
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		if(!(curr_elem->flag & flag))
		{
			curr_node->temp_var++;
			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_desc[curr_node->temp_var].type];
		if(elem_ops==NULL)
			return -EINVAL;
		addr_offset=curr_elem->offset;
		if(elem_ops->get_bin_value==NULL)
		{
			if(curr_elem->flag&flag)
			{
				memcpy(blob+blob_offset,addr+addr_offset,curr_elem->size);
				blob_offset+=curr_elem->size;
			}
		}
		else
		{
			if(curr_elem->flag&flag)
			{
				ret=elem_ops->get_bin_value(addr+addr_offset,blob+blob_offset,curr_elem);
				if(ret<0)
					return ret;
				blob_offset+=ret;
			}
		}
		curr_node->temp_var++;
	}while(1);
	return blob_offset;
	
}

int part_blob_2_struct(void * blob,void * addr,void * struct_template,int flag)
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
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		// throughout the node tree: into the sub_struct
		if(!(curr_elem->flag & flag))
		{
			curr_node->temp_var++;
			continue;
		}
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			temp_node=curr_elem->ref;
			if(!(temp_node->flag &flag))
			{
				curr_node->temp_var++;
				continue;
			}
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_desc=curr_node->struct_desc;
			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;
		// pre_fretch the define value
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = elem_ops->get_int_value(addr+curr_elem->offset,curr_elem);
			if((define_value<0)||define_value>=1024)
				return define_value;
			curr_elem->ref=(int)curr_elem->ref&DEFINE_TAG+define_value;			
		}
		addr_offset=curr_elem->offset;
		if(elem_ops->get_bin_value==NULL)
		{
			if(curr_elem->flag&flag)
			{
				memcpy(addr+addr_offset,blob+blob_offset,curr_elem->size);
				blob_offset+=curr_elem->size;
			}
		}
		else
		{
			if(curr_elem->flag&flag)
			{
				ret=elem_ops->set_bin_value(addr+addr_offset,blob+blob_offset,curr_elem);
				if(ret<0)
					return ret;
				blob_offset+=ret;
			}
		}
		curr_node->temp_var++;
	}while(1);
	return blob_offset;

}

int struct_2_part_json(void * addr,char * json_str,void *struct_template,int flag)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;
	int str_offset;
	int text_len=0;
	int str_len=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int ret;
	char buf[1024];

	curr_desc=root_node->struct_desc;
	*json_str='{';
	str_offset=1;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			*(json_str+str_offset)='}';
			str_offset++;	
			*(json_str+str_offset)=',';
			str_offset++;	
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			curr_desc=curr_node->struct_desc;

			continue;
		}
		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		// print this elem's name
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			temp_node=curr_elem->ref;
			if(!(temp_node->flag &flag))
			{
				curr_node->temp_var++;
				continue;
			}
		}
		else if(!(curr_elem->flag & flag))
		{
			curr_node->temp_var++;
			continue;
		}
		text_len=strnlen(curr_elem->elem_desc->name,64);
		*(json_str+str_offset)='\"';
		memcpy(json_str+str_offset+1,curr_elem->elem_desc->name,text_len);
		str_offset+=text_len+1;
		*(json_str+str_offset)='\"';
		*(json_str+str_offset+1)=':';
		str_offset+=2;
		// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			curr_desc=curr_node->struct_desc;
	
			*(json_str+str_offset)='{';
			str_offset+=1;

			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;
		// pre_fretch the define value
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = elem_ops->get_int_value(addr+curr_elem->offset,curr_elem);
			if((define_value<0)||define_value>=1024)
				return define_value;
			curr_elem->ref=((int)curr_elem->ref&DEFINE_TAG)+define_value;			
		}
		if(elem_ops->get_text_value==NULL)
		{
			if(curr_elem->flag & flag)
			{
				if(_ispointerelem(curr_elem->elem_desc->type))
				{
					text_len=strlen(*(char **)(addr+curr_elem->offset));
					if((text_len<0)||(text_len>1024))
						return -EINVAL;
					strncpy(buf,*(char **)(addr+curr_elem->offset),text_len);
				}
				else
				{
					text_len=strnlen(addr+curr_elem->offset,curr_elem->size)+1;
					if(text_len>curr_elem->size)
						text_len=curr_elem->size;
					strncpy(buf,addr+curr_elem->offset,text_len);
				}
			}	
		}
		else
		{
			if(curr_elem->flag & flag)
			{
				text_len=elem_ops->get_text_value(addr+curr_elem->offset,buf,curr_elem);
				if(text_len<0)
					return text_len;
			}
		}
		str_len = _getjsonstr(json_str+str_offset,buf,text_len,_getelemjsontype(curr_elem->elem_desc->type));
		*(json_str+str_offset+str_len)=',';
		str_offset+=str_len+1;
		curr_node->temp_var++;
	}while(1);
	*(json_str+str_offset-1)=0;
	return str_offset;
	
}
