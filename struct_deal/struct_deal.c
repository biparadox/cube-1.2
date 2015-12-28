#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/struct_deal.h"
#include "struct_ops.h"
#include "struct_attr.h"
#include "../include/string.h"
#include "../include/json.h"

static VALUE2POINTER InitFuncList [] =
{
	{CUBE_TYPE_STRING,&string_convert_ops},
	{CUBE_TYPE_ESTRING,&estring_convert_ops},
	{CUBE_TYPE_BINDATA,&bindata_convert_ops},
	{CUBE_TYPE_DEFINE,&define_convert_ops},
	{CUBE_TYPE_UUID,&uuid_convert_ops},
	{CUBE_TYPE_UUIDARRAY,&uuidarray_convert_ops},
	{CUBE_TYPE_DEFUUIDARRAY,&defuuidarray_convert_ops},
	{CUBE_TYPE_DEFNAMELIST,&defnamelist_convert_ops},
	{CUBE_TYPE_INT,&int_convert_ops},
	{CUBE_TYPE_ENUM,&enum_convert_ops},
	{CUBE_TYPE_FLAG,&flag_convert_ops},
	{CUBE_TYPE_ENDDATA,NULL},
};

void ** struct_deal_ops;
static void * ops_list[CUBE_TYPE_ENDDATA];

static inline int struct_register_ops(int value,void * pointer)
{
	if((value<0) || (value>=CUBE_TYPE_ENDDATA))
		return -EINVAL;
	struct_deal_ops=&ops_list;
	struct_deal_ops[value]=pointer;
	return 0; 
}

int iselemneeddef(int type)
{
	if(_isdefineelem(type))
		return 1;
	switch(type)
	{
		case CUBE_TYPE_ENUM:
		case CUBE_TYPE_FLAG:
		case CUBE_TYPE_SUBSTRUCT:
			return 1;
		default:
			break;
	}	
	return 0;
}


int struct_deal_init()
{
	int count;
	int ret;
	int i;
	struct_deal_ops=&ops_list;
	for(i=0;i<CUBE_TYPE_ENDDATA;i++)
	{
		struct_deal_ops[i]=NULL;
	}
	
	for(i=0;InitFuncList[i].value!=CUBE_TYPE_ENDDATA;i++)
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
	struct struct_elem_attr * struct_desc;
	struct elem_template * elem_list;
	int temp_var;
	
}STRUCT_NODE;

static inline int _count_struct_num(struct struct_elem_attr * struct_desc)
{
	int i=0;
	while(struct_desc[i].name!=NULL)
	{
		if(struct_desc[i].type==CUBE_TYPE_ENDDATA)
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
		if(curr_elem==NULL)
			return -EINVAL;
		if(curr_elem->elem_desc->type != CUBE_TYPE_SUBSTRUCT)
			return NULL;
		node=curr_elem->ref;
	}while(node != NULL);
	return NULL;
}


int get_fixed_elemsize(int type)
{

	switch(type)
	{
		case CUBE_TYPE_STRING:
			return -1;
		case CUBE_TYPE_UUID:
			return DIGEST_SIZE;
		case CUBE_TYPE_ENUM:
		case CUBE_TYPE_FLAG:
		case CUBE_TYPE_TIME:
		case CUBE_TYPE_BOOL:
			return sizeof(int);
		case CUBE_TYPE_BINDATA: 
		case CUBE_TYPE_HEXDATA:	 
			return -1;
		case CUBE_TYPE_ESTRING:
		case CUBE_TYPE_JSONSTRING:
		case CUBE_TYPE_DEFINE:
		case CUBE_TYPE_DEFSTR:	
		case CUBE_TYPE_DEFSTRARRAY:
		case CUBE_TYPE_UUIDARRAY:
		case CUBE_TYPE_DEFUUIDARRAY:
			return sizeof(char *);
		case CUBE_TYPE_DEFNAMELIST:
			return sizeof(char *)+sizeof(int);
		case CUBE_TYPE_BINARRAY:
		case CUBE_TYPE_BITMAP:	 
			return -1;
		case CUBE_TYPE_INT:
			return sizeof(int);
		case CUBE_TYPE_UCHAR:    
			return sizeof(char);
		case CUBE_TYPE_USHORT:   
			return sizeof(short);
		case CUBE_TYPE_LONGLONG: 
		case TPM_TYPE_UINT64:
			return sizeof(long long);
		case TPM_TYPE_UINT32:
			return sizeof(int);
		case TPM_TYPE_UINT16:
			return 2;
		case CUBE_TYPE_NODATA:
			return 0;
        	case CUBE_TYPE_CHOICE:
		case CUBE_TYPE_ENDDATA:
			return 0;
		case CUBE_TYPE_SUBSTRUCT:
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
		case CUBE_TYPE_STRING:
		case CUBE_TYPE_UUID:
		case CUBE_TYPE_ENUM:
		case CUBE_TYPE_FLAG:
		case CUBE_TYPE_TIME:
		case CUBE_TYPE_BINDATA: 
		case CUBE_TYPE_HEXDATA:	 
		case CUBE_TYPE_ESTRING:
		case CUBE_TYPE_JSONSTRING:
		case CUBE_TYPE_DEFINE:
		case CUBE_TYPE_DEFSTR:	
			return JSON_ELEM_STRING;
		case CUBE_TYPE_DEFSTRARRAY:
		case CUBE_TYPE_BINARRAY:
		case CUBE_TYPE_UUIDARRAY:
		case CUBE_TYPE_DEFUUIDARRAY:
		case CUBE_TYPE_DEFNAMELIST:
		case CUBE_TYPE_BITMAP:	 
			return JSON_ELEM_ARRAY;
		case CUBE_TYPE_INT:
		case CUBE_TYPE_UCHAR:    
		case CUBE_TYPE_USHORT:   
		case CUBE_TYPE_LONGLONG: 
		case TPM_TYPE_UINT64:
		case TPM_TYPE_UINT32:
		case TPM_TYPE_UINT16:
			return JSON_ELEM_NUM;
		case CUBE_TYPE_BOOL:
			return JSON_ELEM_BOOL;
		case CUBE_TYPE_NODATA:
        	case CUBE_TYPE_CHOICE:
		case CUBE_TYPE_ENDDATA:
			return 0;
		case CUBE_TYPE_SUBSTRUCT:
			return JSON_ELEM_MAP;
		
		default:
			break;
	}
	return -EINVAL;
}

static inline void * _elem_get_ref(void * elem)
{
	struct elem_template *  curr_elem=elem;
	if(curr_elem==NULL)	
		return NULL;
	if(curr_elem->ref!=NULL)
		return curr_elem->ref;
	return curr_elem->elem_desc->ref;
}

static inline int _elem_set_ref(void * elem,void * ref)
{
	struct elem_template *  curr_elem=elem;
	if(curr_elem==NULL)	
		return -EINVAL;
	curr_elem->ref=ref;
	return 0;
}

void * struct_get_ref(void * struct_template,char * name)
{
	struct elem_template *  curr_elem;
	curr_elem=_get_elem_by_name(struct_template,name);
	return _elem_get_ref(curr_elem);
}

int struct_set_ref(void * struct_template,char * name,void * ref)
{
	struct elem_template *  curr_elem;
	curr_elem=_get_elem_by_name(struct_template,name);
	return _elem_set_ref(curr_elem,ref);
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

int  _convert_frame_func (void *addr, void * data, void * struct_template,
	struct struct_deal_ops * funcs,void * para)
{
	STRUCT_NODE * root_node=struct_template;
	STRUCT_NODE * curr_node=root_node;
	STRUCT_NODE * temp_node;
	curr_node->temp_var=0;
	struct elem_template * curr_elem;


	int offset=0;
//	struct struct_elem_attr * curr_desc;
	ELEM_OPS * elem_ops;
	int def_value;
	int ret;
	if(funcs->start!=NULL)
	{
		ret=funcs->start(addr,data,struct_template,para);
		if(ret<0)
			return ret;
	}

//	curr_desc=root_node->struct_desc;

	do{
		// throughout the node tree: back
		if(curr_node->temp_var == curr_node->elem_no)
		{
			if(curr_node==root_node)
				break;
			temp_node=curr_node;
			curr_node=curr_node->parent;
			curr_elem=&curr_node->elem_list[curr_node->temp_var];
			if(funcs->exitstruct!=NULL)
			{
				ret=funcs->exitstruct(addr,data,curr_elem,
					para);
				if(ret<0)
					return ret;
			}
			curr_elem->index++;
			if((curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
				||(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY))
			{	
				if(curr_elem->index>=curr_elem->limit)
				{
					curr_node->temp_var++;
					curr_elem->limit=0;
				}
			}
			else
				return -EINVAL;
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
		if((curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
			||(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY))
		{
			if(funcs->enterstruct!=NULL)
			{
				ret=funcs->enterstruct(addr,data,curr_elem,
					para);
				if(ret<0)
					return ret;
			}
			if(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY)
			{
				if(curr_elem->limit==0)
				{
					curr_node->temp_var++;
					continue;
				}
			}
			curr_node=curr_elem->ref;
			curr_node->temp_var=0;
			continue;
		}
		// get this elem's ops
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return -EINVAL;

		ret=funcs->proc_func(addr,data,curr_elem,para);
		if(ret<0)
			return ret;
		curr_node->temp_var++;
	}while(1);
	if(funcs->finish==NULL)
		return 0;
	return funcs->finish(addr,data,struct_template,para);
}


struct create_para
{
	int offset;
	int curr_offset;
	STRUCT_NODE * curr_node;
	struct elem_template * parent_elem;
};

int _create_template_start(void * addr,void * data,void * elem, void * para)
{
	int ret;
	int i;
	struct create_para * my_para = para;
	my_para->offset=0;
	my_para->curr_offset=0;
	STRUCT_NODE * root_node=elem;
	STRUCT_NODE * temp_node;
	my_para->parent_elem=NULL;
	my_para->curr_node=root_node;
	// prepare the root node's elem_list
	root_node->elem_no=_count_struct_num(root_node->struct_desc);
	
	ret=Palloc0(&(root_node->elem_list),
		sizeof(struct elem_template)*root_node->elem_no);
	if(ret<0)
		return ret;

	// prepare the struct node for SUBSTRUCT elem and ARRAY elem

	for( i=0;i<root_node->elem_no;i++)
	{
		root_node->elem_list[i].elem_desc=&(root_node->struct_desc[i]);
		if((root_node->struct_desc[i].type == CUBE_TYPE_SUBSTRUCT) ||
			(root_node->struct_desc[i].type == CUBE_TYPE_ARRAY))
		{
			
			ret = Palloc0(&temp_node,sizeof(STRUCT_NODE)); 
			if(ret<0)
				return ret;
			root_node->elem_list[i].ref=temp_node;
			temp_node->struct_desc=root_node->struct_desc[i].ref;
			temp_node->parent=root_node;
		}
	} 
	return 0;
}

int _create_template_enterstruct(void * addr,void * data,void * elem, void * para)
{
	int ret;
	int i;
	struct create_para * my_para = para;
	struct elem_template * curr_elem=elem;
	STRUCT_NODE * curr_node = curr_elem->ref;
	STRUCT_NODE * temp_node;
	// prepare the root node's elem_list
	curr_node->elem_no=_count_struct_num(curr_node->struct_desc);
	
	ret=Palloc0(&(curr_node->elem_list),
		sizeof(struct elem_template)*curr_node->elem_no);
	if(ret<0)
		return ret;

	// prepare the struct node for SUBSTRUCT elem and ARRAY elem

	for( i=0;i<curr_node->elem_no;i++)
	{
		curr_node->elem_list[i].elem_desc=&(curr_node->struct_desc[i]);
		if((curr_node->struct_desc[i].type == CUBE_TYPE_SUBSTRUCT) ||
			(curr_node->struct_desc[i].type == CUBE_TYPE_ARRAY))
		{
			
			ret = Palloc0(&temp_node,sizeof(STRUCT_NODE)); 
			if(ret<0)
				return ret;
			curr_node->elem_list[i].ref=temp_node;
			temp_node->struct_desc=curr_node->struct_desc[i].ref;
			temp_node->parent=curr_node;
		}
	} 
	curr_elem->offset=my_para->curr_offset;
//	curr_node->offset=my_para->curr_offset;
	if(curr_elem->elem_desc->type == CUBE_TYPE_ARRAY)
		my_para->curr_offset=0;
	if(curr_elem->father==NULL)
		curr_elem->father=my_para->parent_elem;
	curr_elem->limit=1;
	my_para->parent_elem=curr_elem;
	my_para->curr_offset=0;
	return 0;
}

int _create_template_exitstruct(void * addr,void * data,void * elem, void * para)
{
	int ret;
	int i;
	struct create_para * my_para = para;
	struct elem_template * curr_elem=elem;
	if(curr_elem->elem_desc->type == CUBE_TYPE_ARRAY)
	{
		struct elem_template * temp_elem;
		temp_elem= _get_elem_by_name(my_para->curr_node,curr_elem->elem_desc->def);
		if(temp_elem==NULL)
			return -EINVAL;
		if(!_isvalidvalue(temp_elem->elem_desc->type))
			return -EINVAL;
		curr_elem->def=temp_elem;
		curr_elem->size=my_para->curr_offset;
		my_para->curr_offset=curr_elem->offset+sizeof(void *);
		curr_elem->limit=0;
	}
	else
	{
		curr_elem->size=my_para->curr_offset;
		curr_elem->limit=curr_elem->elem_desc->size;
		if(curr_elem->limit==0)
			curr_elem->limit=1;
		my_para->curr_offset=curr_elem->offset+curr_elem->size*curr_elem->limit;
	}
	my_para->parent_elem=curr_elem->father;
	curr_elem->index=curr_elem->limit;
	return 0;
}

int _create_template_proc_func(void * addr,void * data,void * elem, void * para)
{
	ELEM_OPS * elem_ops;
	struct create_para * my_para = para;
	struct elem_template * curr_elem=elem;
	STRUCT_NODE * curr_node=my_para->curr_node;
	if(_isdefineelem(curr_elem->elem_desc->type))
	{
		struct elem_template * temp_elem;
		temp_elem= _get_elem_by_name(curr_node,curr_elem->elem_desc->def);
		if(temp_elem==NULL)
			return -EINVAL;
		if(!_isvalidvalue(temp_elem->elem_desc->type))
			return -EINVAL;
		curr_elem->def=temp_elem;
		curr_elem->offset=my_para->curr_offset;
		curr_elem->size=sizeof(void *);
		my_para->curr_offset+=curr_elem->size;
	}
	else
	{
		elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
		if(elem_ops==NULL)
			return NULL;
		curr_elem->ref=curr_elem->elem_desc->ref;
		curr_elem->offset=my_para->curr_offset;
		if(elem_ops->elem_size==NULL)
		{
			if((curr_elem->size=get_fixed_elemsize(curr_elem->elem_desc->type))<0)
				curr_elem->size=curr_elem->elem_desc->size;
		}
		else
		{
			curr_elem->size=elem_ops->elem_size(curr_elem->elem_desc);
		}
		my_para->curr_offset+=curr_elem->size;
		
	}
	curr_elem->father=my_para->parent_elem;
	return 0;
}

int _create_template_finish(void * addr,void * data,void * elem, void * para)
{
	struct create_para * my_para = para;
	return 0;
}

void * create_struct_template(struct struct_elem_attr * struct_desc)
{
	int ret;
	struct struct_deal_ops create_template_ops =
	{
		.start=_create_template_start,
		.enterstruct=_create_template_enterstruct,
		.exitstruct=_create_template_exitstruct,
		.proc_func=_create_template_proc_func,
		.finish=_create_template_finish,
	};

	static struct create_para my_para;
	
	STRUCT_NODE * root_node = Calloc0(sizeof(STRUCT_NODE));
	if(root_node==NULL)
		return NULL;
	root_node->struct_desc=struct_desc;
	ret = _convert_frame_func(NULL,NULL,root_node,&create_template_ops,
		&my_para);
	if(ret<0)
		return NULL;
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
		if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
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
/*
	while(last_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
	{
		last_elem=&root_node->elem_list[root_node->elem_no-1];
	}
*/
	return last_elem;
}

int struct_size(void * struct_template)
{
	struct elem_template * last_elem=_get_last_elem(struct_template);
	ELEM_OPS * elem_ops;
	int total_size;

	if(last_elem->elem_desc->type == CUBE_TYPE_SUBSTRUCT)
		return last_elem->offset+last_elem->size*last_elem->elem_desc->size;

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
		if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
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


struct default_para
{
	int offset;
};

int _elem_get_bin_length(void * value,void * elem,void * addr)
{
	int ret;
	struct elem_template * curr_elem=elem;
	if(_ispointerelem(curr_elem->elem_desc->type))
	{
		if(_isdefineelem(curr_elem->elem_desc->type))
		{
			if(addr==NULL)
				return -EINVAL;
			ret=_elem_get_defvalue(curr_elem,addr);
			if(ret<0)
				return ret;
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
	void * elem_addr;
	elem_addr=_elem_get_addr(elem,addr);
	
	
	if(elem_ops->get_bin_value==NULL)
	{
		
		if((ret=_elem_get_bin_length(*(char **)elem_addr,elem,addr))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
			Memcpy(data,*(char **)elem_addr,ret);
		else
			Memcpy(data,elem_addr,ret);
	}
	else
	{
		if(_isdefineelem(curr_elem->elem_desc->type))
		{
			int def_value=_elem_get_defvalue(curr_elem,addr);
			if(def_value<0)
				return def_value;
			int offset=0;
			int i;
			int addroffset=get_fixed_elemsize(curr_elem->elem_desc->type);
			if(addroffset<0)
				return addroffset;
			curr_elem->index=0;
			
			for(i=0;i<def_value;i++)
			{
				ret=elem_ops->get_bin_value(*(void **)elem_addr+addroffset*i,data+offset,curr_elem);
				if(ret<0)
					return ret;
				offset+=ret;
			}
			return offset;
		}
		else
		{
			ret=elem_ops->get_bin_value(elem_addr,
				data,curr_elem);
			if(ret<0)
				return ret;
		}
	}
	return ret;
} 

int    _elem_set_bin_value(void * addr,void * data,void * elem)
{
	int ret;
	struct elem_template * curr_elem=elem;
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	void * elem_addr;
	elem_addr=_elem_get_addr(elem,addr);
	
	if(elem_ops->set_bin_value==NULL)
	{
		if((ret=_elem_get_bin_length(data,elem,addr))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
		{
			int tempret=Palloc0(elem_addr,ret);
			if(tempret<0)
				return tempret;
			Memcpy(*(char **)elem_addr,data,ret);
		}
		else
			Memcpy(elem_addr,data,ret);
	}
	else
	{
		if(_isdefineelem(curr_elem->elem_desc->type))
		{
			int def_value=_elem_get_defvalue(curr_elem,addr);
			if(def_value<0)
				return def_value;
			int offset=0;
			int i;
			int addroffset=get_fixed_elemsize(curr_elem->elem_desc->type);
			if(addroffset<0)
				return addroffset;
			ret=Palloc0(elem_addr,addroffset*def_value);
			if(ret<0)
				return ret;
			curr_elem->index=0;
	
			for(i=0;i<def_value;i++)
			{
				ret=elem_ops->set_bin_value(*(void **)elem_addr+addroffset*i,data+offset,curr_elem);
				if(ret<0)
					return ret;
				offset+=ret;
			}
			return offset;
		}
		else
		{

			ret=elem_ops->set_bin_value(elem_addr,data,curr_elem);
			if(ret<0)
				return ret;
		}
	}
	return ret;
} 

int    _elem_get_text_value(void * addr,char * text,void * elem)
{
	int ret;
	void * elem_addr;
	elem_addr=_elem_get_addr(elem,addr);
	struct elem_template * curr_elem=elem;
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	
	if(elem_ops->get_text_value==NULL)
	{
		if((ret=_elem_get_bin_length(*(char **)elem_addr,elem,addr))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
			Memcpy(text,*(char **)elem_addr,ret);
		else
			Memcpy(text,elem_addr,ret);
	}
	else
	{
		if(_isdefineelem(curr_elem->elem_desc->type))
		{
			int def_value=_elem_get_defvalue(curr_elem,addr);
			if(def_value<0)
				return def_value;
			int offset=0;
			int i;
			int addroffset=get_fixed_elemsize(curr_elem->elem_desc->type);
			if(addroffset<0)
				return addroffset;
			
			curr_elem->index=0;
			for(i=0;i<def_value;i++)
			{
				ret=elem_ops->get_text_value(*(void **)elem_addr+addroffset*i,text+offset,curr_elem);
				if(ret<0)
					return ret;
				offset+=ret-1;
			}
			*(text+offset-1)=0;
			return offset;
		}
		else
		{
			ret=elem_ops->get_text_value(elem_addr,
				text,curr_elem);
			if(ret<0)
				return ret;
		}
	}
	return ret;
} 

int    _elem_set_text_value(void * addr,char * text,void * elem)
{
	int ret;
	struct elem_template * curr_elem=elem;
	ELEM_OPS * elem_ops=struct_deal_ops[curr_elem->elem_desc->type];
	
	void * elem_addr;
	elem_addr=_elem_get_addr(elem,addr);
	
	if(elem_ops->set_text_value==NULL)
	{
		if((ret=_elem_get_bin_length(text,elem,addr))<0)
			return ret;
		if(_ispointerelem(curr_elem->elem_desc->type))
		{
			int tempret=Palloc0(elem_addr,ret);
			if(tempret<0)
				return tempret;
			Memcpy(*(char **)elem_addr,text,ret);
		}
		else
		{
			int str_len=strlen(text);
			if(str_len>=ret)
			{
				Memcpy(elem_addr,text,ret);
			}
			else
			{
				Memcpy(elem_addr,text,str_len);
				Memset(elem_addr+str_len,0,ret-str_len);	
			}
		}
	}
	else
	{
		if(_isdefineelem(curr_elem->elem_desc->type))
		{
			int def_value=_elem_get_defvalue(curr_elem,addr);
			if(def_value<0)
				return def_value;
			int offset=0;
			int i;
			int addroffset=get_fixed_elemsize(curr_elem->elem_desc->type);
			if(addroffset<0)
				return addroffset;
			ret=Palloc0(elem_addr,addroffset*def_value);
			if(ret<0)
				return ret;
			curr_elem->index=0;
	
			for(i=0;i<def_value;i++)
			{
				ret=elem_ops->set_text_value(*(void **)elem_addr+addroffset*i,text+offset,curr_elem);
				if(ret<0)
					return ret;
				offset+=ret;
			}
			return offset;
		}
		else
		{
			ret=elem_ops->set_text_value(elem_addr,text,curr_elem);
			if(ret<0)
				return ret;
		}
	}
	return ret;
} 


int struct_2_blob_enterstruct(void * addr, void * data, void * elem,void * para)
{
	struct default_para  * my_para = para;
	struct elem_template	* curr_elem=elem;
	if(curr_elem->limit==0)
	{
		curr_elem->index=0;
		if(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY)
			curr_elem->limit=_elem_get_defvalue(curr_elem,addr);
		else if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
		{
			curr_elem->limit=curr_elem->elem_desc->size;
			if(curr_elem->limit==0)
				curr_elem->limit=1;
		}
	}
	return 0;		
}

int struct_2_blob_exitstruct(void * addr, void * data, void * elem,void * para)
{
	struct default_para  * my_para = para;
	struct elem_template	* curr_elem=elem;

	return 0;		
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
		.enterstruct=&struct_2_blob_enterstruct,
		.exitstruct=&struct_2_blob_exitstruct,
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
	if(curr_elem->elem_desc->type == CUBE_TYPE_SUBSTRUCT)
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
		.enterstruct=&struct_2_blob_enterstruct,
		.exitstruct=&struct_2_blob_exitstruct,
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


int blob_2_struct_enterstruct(void * addr, void * data, void * elem,void * para)
{
	int ret;
	struct default_para  * my_para = para;
	struct elem_template	* curr_elem=elem;
	void * elem_addr;
	if(curr_elem->limit==0)
	{
		curr_elem->index=0;
		if(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY)
		{
			curr_elem->limit=_elem_get_defvalue(curr_elem,addr);
			if(curr_elem->father==NULL)
				elem_addr=addr;
			else
				elem_addr=_elem_get_addr(curr_elem->father,addr);
			elem_addr+=curr_elem->offset;
			
			if(curr_elem->limit>0)
				ret=Palloc0(elem_addr,curr_elem->size*curr_elem->limit);
		}
		else if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
		{
			curr_elem->limit=curr_elem->elem_desc->size;
			if(curr_elem->limit==0)
				curr_elem->limit=1;
		}
	}
	return 0;		
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
		.enterstruct=&blob_2_struct_enterstruct,
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
		.enterstruct=&blob_2_struct_enterstruct,
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
			{
				int isinelem=0;
				int array_no=0;
				*json_str='[';
				str_offset++;
				for(i=0;i<text_len;i++)
				{
					if((text[i]==0) || (text[i]==','))
					{	
						if(isinelem)
						{
							*(json_str+str_offset++)='\"';
							isinelem=0;
						}
					}

					else
					{
						if(!isinelem)
						{
							if(array_no>0)
								*(json_str+str_offset++)=',';
							
							*(json_str+str_offset++)='\"';
							isinelem=1;
							array_no++;
						}
						*(json_str+str_offset++)=*(text+i);
					}
				}
				if((*(text+i-1)!=0)&&(*(text+i-1)!=','))
				{
					if(isinelem)
						*(json_str+str_offset++)='\"';
				}
				*(json_str+str_offset++)=']';
				*(json_str+str_offset)=0;
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
	struct elem_template * curr_elem=elem;
	if(curr_elem->limit==0)
	{
		curr_elem->index=0;
		if(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY)
			curr_elem->limit=_elem_get_defvalue(curr_elem,addr);
		else if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
		{
			curr_elem->limit=curr_elem->elem_desc->size;
			if(curr_elem->limit==0)
				curr_elem->limit=1;
		}
	}
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
	int ret;
	void * temp_json_node=json_find_elem(curr_elem->elem_desc->name,my_para->json_node);
	if(json_get_type(temp_json_node) != JSON_ELEM_MAP)
		return -EINVAL;
	my_para->json_node=temp_json_node;
	void * elem_addr;
	if(curr_elem->limit==0)
	{
		curr_elem->index=0;
		if(curr_elem->elem_desc->type==CUBE_TYPE_ARRAY)
		{
			curr_elem->limit=_elem_get_defvalue(curr_elem,addr);
			if(curr_elem->father==NULL)
				elem_addr=addr;
			else
				elem_addr=_elem_get_addr(curr_elem->father,addr);
			elem_addr+=curr_elem->offset;
			
			if(curr_elem->limit>0)
				ret=Palloc0(elem_addr,curr_elem->size*curr_elem->limit);
		}
		else if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
		{
			curr_elem->limit=curr_elem->elem_desc->size;
			if(curr_elem->limit==0)
				curr_elem->limit=1;
		}
	}
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
	int ret,text_len;
	void * temp_json_node=json_find_elem(curr_elem->elem_desc->name,my_para->json_node);
	if(temp_json_node==NULL)
		return 0;

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
	if(curr_elem->elem_desc->type == CUBE_TYPE_SUBSTRUCT)
	{
		STRUCT_NODE * temp_node=curr_elem->ref;
		return temp_node->flag & my_para->flag;
	}
	return curr_elem->flag & my_para->flag;	
}

int json_2_part_struct(void * root, void * addr, void * struct_template,int flag)
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
		if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
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
		if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
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
		if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
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
		if(curr_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
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
				if(temp_elem->elem_desc->type==CUBE_TYPE_SUBSTRUCT)
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
