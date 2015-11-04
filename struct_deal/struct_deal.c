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
#include "struct_attr.h"
#include "../include/string.h"
#include "../include/json.h"

static VALUE2POINTER InitFuncList [] =
{
	{OS210_TYPE_STRING,&string_convert_ops},
	{OS210_TYPE_ESTRING,&estring_convert_ops},
	{OS210_TYPE_BINDATA,&bindata_convert_ops},
	{OS210_TYPE_DEFINE,&define_convert_ops},
	{OS210_TYPE_UUID,&uuid_convert_ops},
	{OS210_TYPE_UUIDARRAY,&uuidarray_convert_ops},
	{OS210_TYPE_DEFUUIDARRAY,&defuuidarray_convert_ops},
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
		case OS210_TYPE_BOOL:
			return sizeof(int);
		case OS210_TYPE_BINDATA: 
		case OS210_TYPE_HEXDATA:	 
			return -1;
		case OS210_TYPE_ESTRING:
		case OS210_TYPE_JSONSTRING:
		case OS210_TYPE_DEFINE:
		case OS210_TYPE_DEFSTR:	
		case OS210_TYPE_DEFSTRARRAY:
		case OS210_TYPE_UUIDARRAY:
		case OS210_TYPE_DEFUUIDARRAY:
		case OS210_TYPE_DEFNAMELIST:
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
		case OS210_TYPE_UUIDARRAY:
		case OS210_TYPE_DEFUUIDARRAY:
		case OS210_TYPE_DEFNAMELIST:
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
		case OS210_TYPE_BOOL:
			return JSON_ELEM_BOOL;
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
				if( (curr_elem->size=get_fixed_elemsize(elem_desc->type))<0)
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

struct struct_deal_ops
{
	int (*start)(void * addr, void * data,void *elem,void * para);
	int (*testelem)(void * addr, void * data,void *elem,void * para);
	int (*enterstruct)(void * addr,void * data, void * elem,void * para);
	int (*exitstruct)(void * addr,void * data,void * elem,void * para);
	int (*proc_func)(void * addr, void * data, void * elem,void * para);
	int (*finish)(void * addr, void * data,void *elem,void * para);
};

struct default_para
{
	int offset;
};

int  _convert_frame_func (void *addr, void * data, void * struct_template,
	struct struct_deal_ops * funcs,void * para)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;


	int offset=0;
	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int def_value;
	int ret;
	if(funcs->start!=NULL)
	{
		ret=funcs->start(addr,data,struct_template,para);
		if(ret<0)
			return ret;
	}

	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			if(funcs->exitstruct!=NULL)
			{
				ret=funcs->exitstruct(addr,data,curr_node,
					para);
				if(ret<0)
					return ret;
			}
			continue;
		}

		curr_elem=&curr_node->elem_list[curr_node->temp_var];
		if(funcs->testelem!=NULL)
		{
			if(!funcs->testelem(addr,data,curr_elem,para))
			{
				curr_node->temp_var++;
				continue;
			}
		}
			// throughout the node tree: into the sub_struct
		if(curr_elem->elem_desc->type==OS210_TYPE_ORGCHAIN)
		{
			curr_node->temp_var++;
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			if(funcs->enterstruct!=NULL)
			{
				ret=funcs->enterstruct(addr,data,curr_elem,
					para);
				if(ret<0)
					return ret;
			}
			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;

		ret=funcs->proc_func(addr,data,curr_elem,para);
		if(ret<0)
			return ret;
		// pre_fretch the define value
		if(_isvalidvalue(curr_elem->elem_desc->type) &&
			((int)curr_elem->ref & DEFINE_TAG))
		{
			if(elem_ops->get_int_value == NULL)
				return -EINVAL;
			int define_value;
			define_value = elem_ops->get_int_value(addr+curr_elem->offset,curr_elem);
			if((define_value<0)||define_value>=1024)
				return -EINVAL;
			curr_elem->ref=((int)curr_elem->ref&DEFINE_TAG)+define_value;			
		}
		curr_node->temp_var++;
	}while(1);
	if(funcs->finish==NULL)
		return 0;
	return funcs->finish(addr,data,struct_template,para);
}

int _elem_get_bin_length(void * value,void * elem)
{
	int ret;
	struct elem_template * curr_elem=elem;
	if(_ispointerelem(curr_elem->elem_desc->type))
	{
		if(_isdefineelem(curr_elem->elem_desc->type))
		{
			struct elem_template * temp_elem=curr_elem->ref;
			ret = (int)temp_elem->ref & 0x00000FFF;
			if(_isarrayelem(curr_elem->elem_desc->type))
				ret*=curr_elem->elem_desc->size;
		}
		else 
		{ 
			if(_isarrayelem(curr_elem->elem_desc->type))
			{
				ret=curr_elem->elem_desc->size*(int)curr_elem->elem_desc->ref;
				if((ret<0) ||(ret>DIGEST_SIZE*16))
					return -EINVAL;
			}
			else
			{
				ret=strnlen(value,DIGEST_SIZE*16);
				if(ret<DIGEST_SIZE*16)
						ret+=1;
			}
		}
	}
	else
	{
		if( (ret=get_fixed_elemsize(curr_elem->elem_desc->type))<0)
			ret=curr_elem->size;
	}
	return ret;
}
		
int    _elem_get_bin_value(void * addr,void * data,void * elem)
{
	int ret;
	struct elem_template * curr_elem=elem;
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	
	if(elem_ops->get_bin_value==NULL)
	{
		if((ret=_elem_get_bin_length(*(char **)(addr+curr_elem->offset),			elem))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
			Memcpy(data,*(char **)(addr+curr_elem->offset),ret);
		else
			Memcpy(data,addr+curr_elem->offset,ret);
	}
	else
	{
		ret=elem_ops->get_bin_value(addr+curr_elem->offset,
			data,curr_elem);
		if(ret<0)
			return ret;
	}
	return ret;
} 

int    _elem_set_bin_value(void * addr,void * data,void * elem)
{
	int ret;
	struct elem_template * curr_elem=elem;
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	
	if(elem_ops->set_bin_value==NULL)
	{
		if((ret=_elem_get_bin_length(data,elem))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
		{
			int tempret=Palloc0(addr+curr_elem->offset,ret);
			if(tempret<0)
				return tempret;
			Memcpy(*(char **)(addr+curr_elem->offset),data,ret);
		}
		else
			Memcpy(addr+curr_elem->offset,data,ret);
	}
	else
	{
		ret=elem_ops->set_bin_value(addr+curr_elem->offset,
			data,curr_elem);
		if(ret<0)
			return ret;
	}
	return ret;
} 

int    _elem_get_text_value(void * addr,char * text,void * elem)
{
	int ret;
	struct elem_template * curr_elem=elem;
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	
	if(elem_ops->get_text_value==NULL)
	{
		if((ret=_elem_get_bin_length(*(char **)(addr+curr_elem->offset),			elem))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
			Memcpy(text,*(char **)(addr+curr_elem->offset),ret);
		else
			Memcpy(text,addr+curr_elem->offset,ret);
	}
	else
	{
		ret=elem_ops->get_text_value(addr+curr_elem->offset,
			text,curr_elem);
		if(ret<0)
			return ret;
	}
	return ret;
} 

int    _elem_set_text_value(void * addr,char * text,void * elem)
{
	int ret;
	struct elem_template * curr_elem=elem;
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	
	if(elem_ops->set_text_value==NULL)
	{
		if((ret=_elem_get_bin_length(text,elem))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
		{
			int tempret=Palloc0(addr+curr_elem->offset,ret);
			if(tempret<0)
				return tempret;
			Memcpy(*(char **)(addr+curr_elem->offset),text,ret);
		}
		else
			Memcpy(addr+curr_elem->offset,text,ret);
	}
	else
	{
		ret=elem_ops->set_text_value(addr+curr_elem->offset,
			text,curr_elem);
		if(ret<0)
			return ret;
	}
	return ret;
} 


int proc_struct_2_blob(void * addr,void * data,void * elem,void * para)
{
	struct default_para  * my_para = para;
	struct elem_template	* curr_elem=elem;
	int ret;
	
	ret = _elem_get_bin_value(addr,data+my_para->offset,elem);
	if(ret>=0)
		my_para->offset+=ret;
	return ret;
} 

int struct_2_blob(void * addr, void * blob, void * struct_template)
{
	int ret;
	struct struct_deal_ops struct_2_blob_ops =
	{
		.proc_func=&proc_struct_2_blob,
	};	
	static struct default_para my_para;
	my_para.offset=0;
	ret = _convert_frame_func(addr,blob,struct_template,&struct_2_blob_ops,		&my_para);
	if(ret<0)
		return ret;
	return my_para.offset;
}

struct part_deal_para
{
	int offset;
	int flag;	
};

int part_deal_test(void * addr,void * data,void * elem,void *para)
{
	struct part_deal_para * my_para=para;
	struct elem_template * curr_elem=elem;
	if(curr_elem->elem_desc->type == OS210_TYPE_ORGCHAIN)
	{
		STRUCT_NODE * temp_node=curr_elem->ref;
		return temp_node->flag & my_para->flag;
	}
	return curr_elem->flag & my_para->flag;	
}

int struct_2_part_blob(void * addr,void * blob, void * struct_template,int flag)
{
	int ret;
	struct struct_deal_ops struct_2_blob_ops =
	{
		.testelem=part_deal_test,
		.proc_func=&proc_struct_2_blob,
	};	
	static struct part_deal_para my_para;
	my_para.flag=flag;
	my_para.offset=0;
	ret= _convert_frame_func(addr,blob,struct_template,&struct_2_blob_ops,		&my_para);
	if(ret<0)
		return ret;
	return my_para.offset;
}


int proc_blob_2_struct(void * addr,void * data,void * elem,void * para)
{
	struct default_para  * my_para = para;
	struct elem_template	* curr_elem=elem;
	int ret;
	ret = _elem_set_bin_value(addr,data+my_para->offset,elem);
	if(ret>=0)
		my_para->offset+=ret;
	return ret;
} 

int blob_2_struct(void * blob, void * addr, void * struct_template)
{
	int ret;
	struct struct_deal_ops blob_2_struct_ops =
	{
		.proc_func=&proc_blob_2_struct,
	};	
	static struct default_para my_para;
	my_para.offset=0;
	ret = _convert_frame_func(addr,blob,struct_template,&blob_2_struct_ops,		&my_para);
	if(ret<0)
		return ret;
	return my_para.offset;
}


int blob_2_part_struct(void * blob,void * addr, void * struct_template,int flag)
{
	int ret;
	struct struct_deal_ops blob_2_struct_ops =
	{
		.testelem=part_deal_test,
		.proc_func=&proc_blob_2_struct,
	};	
	static struct part_deal_para my_para;
	my_para.flag=flag;
	my_para.offset=0;
	ret= _convert_frame_func(addr,blob,struct_template,&blob_2_struct_ops,		&my_para);
	if(ret<0)
		return ret;
	return my_para.offset;
}

int struct_read_elem(char * name,void * addr, void * elem_data,void * struct_template)
{
	STRUCT_NODE * curr_node=struct_template;
	ELEM_OPS * elem_ops;
	int ret;
	struct elem_template * curr_elem= _get_elem_by_name(curr_node,name);
	if(curr_elem==NULL)
		return -EINVAL;
	
	return _elem_get_bin_value(addr,elem_data,curr_elem);
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
	return _elem_get_text_value(addr,text,curr_elem);
}

int struct_write_elem(char * name,void * addr, void * elem_data,void * struct_template)
{
	STRUCT_NODE * curr_node=struct_template;
	int ret;
	struct elem_template * curr_elem= _get_elem_by_name(curr_node,name);
	if(curr_elem==NULL)
		return -EINVAL;
	return _elem_set_bin_value(addr,elem_data,curr_elem);
}

int struct_write_elem_text(char * name,void * addr, char * text,void * struct_template)
{
	STRUCT_NODE * curr_node=struct_template;
	struct struct_elem_attr * curr_desc;
	int ret;
	struct elem_template * curr_elem= _get_elem_by_name(curr_node,name);
	if(curr_elem==NULL)
		return -EINVAL;
	return _elem_set_text_value(addr,text,curr_elem);
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
				if((text[i]==0) || (text[i]==','))
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

int _stripjsonarraychar(char * text,int len)
{
	int i;
	int offset=0;
	int bracket_order=0;
	int quota_order=0;
	enum stat_list
	{
		STRIP_START,
		STRIP_INBRACKET,
		STRIP_INELEM,
		STRIP_FINISH,
	};
	int state=STRIP_START;

	for(i=0;i<len;i++)
	{
		switch(state)
		{
			case STRIP_START:
				if((*(text+i)==' ')||(*(text+i)=='\t'))
				{
					offset++;
					break;
				}
				if(*(text+i)=='[')
				{
					offset++;
					bracket_order++;
					state=STRIP_INBRACKET;
				}
				else
					return -EINVAL;
				break;
			case STRIP_INBRACKET:
				if(Ischarinset(*(text+i)," \t\n\r,")
					||(*(text+i)==','))
				{
					offset++;
					break;
				}
				if(*(text+i)=='\"')
				{
					if(quota_order==0)
					{
						quota_order++;
						state=STRIP_INELEM;
						offset++;
						break;
					}
					return -EINVAL;
				}
				if(*(text+i)==']')
				{
					bracket_order--;
					if(bracket_order==0)
					{
						offset++;
						state=STRIP_FINISH;
						break;
					}
					return -EINVAL;
				}
				if(Ischarinset(*(text+i),":{}"))
					return -EINVAL;
				state=STRIP_INELEM;
				break;
			case 	STRIP_INELEM:
				if(*(text+i)==',')
				{
					*(text+i-offset)=*(text+i);
					if((quota_order==0)&&
						(bracket_order==1))
						state=STRIP_INBRACKET;
				}	
				else if(*(text+i)=='\"')
				{
					if(bracket_order==1)
					{
						if(quota_order==1)
						{
							quota_order--;
							*(text+i-offset)=',';
							state=STRIP_INBRACKET;
						}
						else
							return -EINVAL;
					}
					else
						*(text+i-offset)=*(text+i);
				}
				else if(*(text+i)=='[')
				{
					if(quota_order==0)
						bracket_order++;
					*(text+i-offset)=*(text+i);
				}
				else if(*(text+i)==']')
				{
					if(quota_order==0)
					{
						bracket_order--;
						offset++;
					}
				}
				else
				{
					*(text+i-offset)=*(text+i);
				}
				break;
			default:
				return -EINVAL;
		}
		if(state==STRIP_FINISH)
		{
			*(text+i-offset+1)=0;
			break;
		}
						
	}
	if(state!=STRIP_FINISH)
		return -EINVAL;
	return i-offset;
	
}

int _setvaluefromjson(void * addr,void * node,void * elem)
{
	struct elem_template * curr_elem=elem;
	int ret;
	char buf[DIGEST_SIZE*16];
	switch(json_get_type(node))
	{
		case JSON_ELEM_STRING:
			ret=_elem_set_text_value(addr,json_get_valuestr(node),curr_elem);
			break;		
		case JSON_ELEM_NUM:
		case JSON_ELEM_BOOL:
			{
				long long tempdata;
				ret=json_node_getvalue(node,&tempdata,sizeof(long long));
				if(ret<0)
					return ret;
				ret=_elem_set_bin_value(addr,&tempdata,curr_elem);
			}
			break;		
		case JSON_ELEM_ARRAY:
			{
				ret=json_print_str(node,buf);
				if(ret<0)
					return ret;
				ret=_stripjsonarraychar(buf,ret);	
				if(ret<0)
					return ret;
				ret=_elem_set_text_value(addr,buf,curr_elem);
			}
			
			break;
		case JSON_ELEM_MAP:
		default:
			return -EINVAL;
	}
	return 0;
}

int _print_elem_name(void * data,void * elem,void * para)
{
	struct default_para  * my_para = para;
	char * json_str=data;
	struct elem_template	* curr_elem=elem;
	int ret,text_len;
	// print this elem's name
	text_len=strnlen(curr_elem->elem_desc->name,64);
	*(json_str+my_para->offset)='\"';
	Memcpy(json_str+my_para->offset+1,curr_elem->elem_desc->name,text_len);
	ret=text_len+1;	
	my_para->offset+=ret;
	*(json_str+my_para->offset)='\"';
	*(json_str+my_para->offset+1)=':';
	ret+=2;
	my_para->offset+=2;
	return ret;
}

struct tojson_para
{
	int offset;
};

int _tojson_start(void * addr, void * data,void *elem,void * para)
{
	struct default_para * my_para=para;
	char * json_str=data;
	
	*json_str='{';
	my_para->offset=1;
	return 1;	
}

int _tojson_enterstruct(void * addr,void * data, void * elem,void * para)
{
	struct default_para * my_para=para;
	char * json_str=data;
	_print_elem_name(data,elem,para);
	*(json_str+my_para->offset)='{';
	my_para->offset+=1;
	return my_para->offset;
}
int _tojson_exitstruct(void * addr,void * data, void * elem,void * para)
{
	struct default_para * my_para=para;
	char * json_str=data;

	*(json_str+my_para->offset)='}';
	my_para->offset+=1;
	return my_para->offset;
}

int _tojson_proc_func(void * addr, void * data, void * elem,void * para)
{
	struct default_para  * my_para = para;
	char * json_str=data;
	struct elem_template	* curr_elem=elem;
	int ret,text_len;
	char buf[512];
	// get this elem's ops
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	if(elem_ops==NULL)
		return -EINVAL;
	_print_elem_name(data,elem,para);
	text_len=_elem_get_text_value(addr,buf,curr_elem);
	if(text_len<0)
		return text_len;
	text_len = _getjsonstr(json_str+my_para->offset,buf,text_len,_getelemjsontype(curr_elem->elem_desc->type));
	*(json_str+my_para->offset+text_len)=',';
	text_len+=1;
	my_para->offset+=text_len;
	return ret+text_len;
}

int _tojson_finish(void * addr, void * data,void *elem,void * para)
{
	struct default_para * my_para=para;
	char * json_str=data;
	
	*(json_str+my_para->offset)='}';
	my_para->offset+=1;
	*(json_str+my_para->offset)='\0';
	return 1;	
}

int struct_2_json(void * addr, char * json_str, void * struct_template)
{
	int ret;
	struct struct_deal_ops struct_2_json_ops =
	{
		.start=&_tojson_start,
		.enterstruct=&_tojson_enterstruct,
		.exitstruct=&_tojson_exitstruct,
		.proc_func=&_tojson_proc_func,
		.finish=&_tojson_finish,
	};	
	static struct default_para my_para;
	my_para.offset=0;
	ret = _convert_frame_func(addr,json_str,struct_template,&struct_2_json_ops,		&my_para);
	if(ret<0)
		return ret;
	return my_para.offset;
}
int struct_2_part_json(void * addr, char * json_str, void * struct_template,int flag)
{
	int ret;
	struct struct_deal_ops struct_2_part_json_ops =
	{
		.start=&_tojson_start,
		.testelem=part_deal_test,
		.enterstruct=&_tojson_enterstruct,
		.exitstruct=&_tojson_exitstruct,
		.proc_func=&_tojson_proc_func,
		.finish=&_tojson_finish,
	};	
	static struct part_deal_para my_para;
	my_para.offset=0;
	my_para.flag=flag;
	ret = _convert_frame_func(addr,json_str,struct_template,&struct_2_part_json_ops,		&my_para);
	if(ret<0)
		return ret;
	return my_para.offset;
}

struct jsonto_para
{
	void * json_node;
};

int _jsonto_start(void * addr, void * data,void *elem,void * para)
{
	// json node init
	if(json_get_type(data) != JSON_ELEM_MAP)
		return -EINVAL;
	return 1;	
}

int _jsonto_enterstruct(void * addr,void * data, void * elem,void * para)
{
	struct jsonto_para * my_para=para;
	struct elem_template	* curr_elem=elem;
	void * temp_json_node=json_find_elem(curr_elem->elem_desc->name,my_para->json_node);
	if(json_get_type(temp_json_node) != JSON_ELEM_MAP)
		return -EINVAL;
	my_para->json_node=temp_json_node;
	return 1;
}
int _jsonto_exitstruct(void * addr,void * data, void * elem,void * para)
{
	struct jsonto_para * my_para=para;
	void * temp_json_node=json_get_father(my_para->json_node);
	my_para->json_node=temp_json_node;

	return 1;
}

int _jsonto_proc_func(void * addr, void * data, void * elem,void * para)
{
	struct jsonto_para * my_para=para;
	struct elem_template	* curr_elem=elem;
	void * temp_json_node=json_find_elem(curr_elem->elem_desc->name,my_para->json_node);
	int ret,text_len;

	return _setvaluefromjson(addr,temp_json_node,curr_elem);
}

int json_2_struct(void * root, void * addr, void * struct_template)
{
	int ret;
	struct struct_deal_ops json_2_struct_ops =
	{
		.start=&_jsonto_start,
		.enterstruct=&_jsonto_enterstruct,
		.exitstruct=&_jsonto_exitstruct,
		.proc_func=&_jsonto_proc_func,
	};	
	static struct jsonto_para my_para;
	my_para.json_node=root;
	ret = _convert_frame_func(addr,root,struct_template,&json_2_struct_ops,		&my_para);
	if(ret<0)
		return ret;
	return 0;
}

struct part_jsonto_para
{
	void * json_node;
	int flag;
};
int part_json_test(void * addr,void * data,void * elem,void *para)
{
	struct part_jsonto_para * my_para=para;
	struct elem_template * curr_elem=elem;
	if(curr_elem->elem_desc->type == OS210_TYPE_ORGCHAIN)
	{
		STRUCT_NODE * temp_node=curr_elem->ref;
		return temp_node->flag & my_para->flag;
	}
	return curr_elem->flag & my_para->flag;	
}

int json_2_part_struct(void * addr, void * root, void * struct_template,int flag)
{
	int ret;
	struct struct_deal_ops json_2_part_struct_ops =
	{
		.start=&_jsonto_start,
		.testelem=part_json_test,
		.enterstruct=&_jsonto_enterstruct,
		.exitstruct=&_jsonto_exitstruct,
		.proc_func=&_jsonto_proc_func,
	};	
	static struct part_jsonto_para my_para;
	my_para.json_node=root;
	my_para.flag=flag;
	ret = _convert_frame_func(addr,root,struct_template,&json_2_part_struct_ops,		&my_para);
	if(ret<0)
		return ret;
	return 0;
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
