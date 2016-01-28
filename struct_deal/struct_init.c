#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/struct_deal.h"
#include "../include/string.h"
#include "../include/json.h"
#include "struct_ops.h"
#include "struct_attr.h"

struct InitElemInfo_struct
{
	enum cube_struct_elem_type type;
	void * enum_ops; 
	int  flag;
	int  offset;
};

static struct InitElemInfo_struct InitElemInfo [] =
{
	{CUBE_TYPE_STRING,&string_convert_ops,ELEM_ATTR_VALUE,-1},
	{CUBE_TYPE_ESTRING,&estring_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_POINTER,0},
	{CUBE_TYPE_BINDATA,&bindata_convert_ops,0,-1},
	{CUBE_TYPE_DEFINE,&define_convert_ops,ELEM_ATTR_POINTER|ELEM_ATTR_DEFINE,0},
	{CUBE_TYPE_UUID,&uuid_convert_ops,0,DIGEST_SIZE},
	{CUBE_TYPE_UUIDARRAY,&uuidarray_convert_ops,ELEM_ATTR_POINTER|ELEM_ATTR_ARRAY,0},
	{CUBE_TYPE_DEFUUIDARRAY,&defuuidarray_convert_ops,ELEM_ATTR_POINTER|ELEM_ATTR_ARRAY|ELEM_ATTR_DEFINE,0},
	{CUBE_TYPE_DEFNAMELIST,&defnamelist_convert_ops,ELEM_ATTR_POINTER|ELEM_ATTR_DEFINE,sizeof(void *)+sizeof(int)},
	{CUBE_TYPE_INT,&int_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_NUM,sizeof(int)},
	{CUBE_TYPE_UCHAR,&int_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_NUM,sizeof(char)},
	{CUBE_TYPE_USHORT,&int_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_NUM,sizeof(short)},
	{CUBE_TYPE_LONGLONG,&int_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_NUM,sizeof(long long)},
	{TPM_TYPE_UINT64,&int_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_NUM,8},
	{TPM_TYPE_UINT32,&int_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_NUM,4},
	{TPM_TYPE_UINT16,&int_convert_ops,ELEM_ATTR_VALUE|ELEM_ATTR_NUM,2},
	{CUBE_TYPE_ENUM,&enum_convert_ops,0,sizeof(int)},
	{CUBE_TYPE_FLAG,&flag_convert_ops,0,sizeof(int)},
	{CUBE_TYPE_SUBSTRUCT,NULL,ELEM_ATTR_SUBSET,0},
	{CUBE_TYPE_ARRAY,NULL,ELEM_ATTR_POINTER|ELEM_ATTR_ARRAY|ELEM_ATTR_SUBSET,0},
	{CUBE_TYPE_ENDDATA,NULL,ELEM_ATTR_EMPTY,0},
};

void ** struct_deal_ops;
int * struct_deal_flag;
int * struct_deal_offset;
static void * ops_list[CUBE_TYPE_ENDDATA];
static int  flag_list[CUBE_TYPE_ENDDATA];
static int  offset_list[CUBE_TYPE_ENDDATA];

int struct_register_ops(int value,void * pointer,int flag,int offset)
{
	if((value<0) || (value>=CUBE_TYPE_ENDDATA))
		return -EINVAL;
	ops_list[value]=pointer;
	flag_list[value]=flag;
	offset_list[value]=offset;
	return 0; 
}

static inline int _testelemattr(int type, int flag)
{
	if((type<0) || (type >=CUBE_TYPE_ENDDATA))
		return -EINVAL;
	return flag_list[type]&flag;

}

int _isdefineelem(int type)
{
	return _testelemattr(type,ELEM_ATTR_DEFINE);
}

int _isarrayelem(int type)
{
	return _testelemattr(type,ELEM_ATTR_ARRAY);
}

int _ispointerelem(int type)
{
	return _testelemattr(type,ELEM_ATTR_POINTER);
}

int _issubsetelem(int type)
{
	return _testelemattr(type,ELEM_ATTR_SUBSET);
}

int _isvalidvalue(int type)
{
	return _testelemattr(type,ELEM_ATTR_VALUE);
}

int _isnumelem(int type)
{
	return _testelemattr(type,ELEM_ATTR_NUM);
}

int _isboolelem(int type)
{
	return _testelemattr(type,ELEM_ATTR_BOOL);
}

int _isemptyelem(int type)
{
	return _testelemattr(type,ELEM_ATTR_EMPTY);
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

ELEM_OPS * _elem_get_ops(void * elem)
{
	struct elem_template * curr_elem=elem;
	if(curr_elem==NULL)
		return NULL;
	if(curr_elem->elem_desc==NULL)
		return NULL;
	if((curr_elem->elem_desc->type<0)|| (curr_elem->elem_desc->type>CUBE_TYPE_ENDDATA))
		return NULL;
	return struct_deal_ops[curr_elem->elem_desc->type];  

}

void * _elem_get_addr(void * elem,void * addr)
{
	int offset_array[10];
	int limit=0;
	int i;
	memset(offset_array,0,sizeof(offset_array));

	struct elem_template * curr_elem=elem;

	while(curr_elem!=NULL)
	{
		if(_issubsetelem(curr_elem->elem_desc->type))
		{
			if(_ispointerelem(curr_elem->elem_desc->type))
			{
				offset_array[limit]+=curr_elem->size * curr_elem->index;
				offset_array[++limit]=curr_elem->offset;
			}
			else 
			{
				offset_array[limit]+=curr_elem->offset+curr_elem->size*curr_elem->index;
			}
		}
		else
		{
			offset_array[limit]+=curr_elem->offset;
		}
		curr_elem=curr_elem->father;
	}
	

	for(i=limit;i>0;i--)
	{
		addr=*((void **)(addr+offset_array[i]));
		if(addr==NULL)
			return NULL;
		
	}
	return addr+offset_array[0];
}

int _elem_get_offset(void * elem)
{
	int offset=0;
	struct elem_template * curr_elem=elem;

	while(curr_elem!=NULL)
	{
		if(_issubsetelem(curr_elem->elem_desc->type) &&
			_ispointerelem(curr_elem->elem_desc->type))
			offset+=curr_elem->offset +
				curr_elem->size * curr_elem->index;
		else
			offset+=curr_elem->offset;
		
		curr_elem=curr_elem->father;
	}
	return offset;	
	
}

int _elem_get_defvalue(void * elem,void * addr)
{
	struct elem_template * curr_elem=elem;
	struct elem_template * temp_elem=curr_elem->def;
	struct elem_template * temp_subset;
	ELEM_OPS * elem_ops;
	int define_value;
	void * def_addr;
	if(temp_elem==NULL)
		return -EINVAL;
	elem_ops=_elem_get_ops(temp_elem);
	if(elem_ops==NULL)
		return NULL;

	if(elem_ops->get_int_value == NULL)
		return -EINVAL;

	// now compute the define elem's offset

	// if define elem is the first layer child
	if(temp_elem->father!=NULL)
	{
		if(!_is_elem_in_subset(curr_elem,temp_elem->father))
			return -EINVAL;	
	}

	def_addr=_elem_get_addr(temp_elem,addr);
	// if define elem is an elem in curr_elem's father subset
	define_value=elem_ops->get_int_value(def_addr,temp_elem);
	if((define_value<0) || (define_value >=1024))
		return -EINVAL;
	return define_value;
}

int _elem_set_defvalue(void * elem,void * addr,int value)
{
	struct elem_template * curr_elem=elem;
	struct elem_template * temp_elem=curr_elem->def;
	ELEM_OPS * elem_ops;
	void * def_addr;
	int ret;
	if(temp_elem==NULL)
		return -EINVAL;
	
	if((value<0)|| (value>1024))
		return -EINVAL;
	

	elem_ops=_elem_get_ops(temp_elem);
	if(elem_ops==NULL)
		return NULL;

	// now compute the define elem's offset

	def_addr=_elem_get_addr(temp_elem,addr);
	// if define elem is an elem in curr_elem's father subset
	
	char buffer[DIGEST_SIZE];
	if((temp_elem->elem_desc->type == CUBE_TYPE_STRING)
		||(temp_elem->elem_desc->type == CUBE_TYPE_ESTRING))
	{
		ret=Itoa(value,buffer);
		if(ret<0)
			return -EINVAL;
		if(elem_ops->set_text_value==NULL)
		{
			int str_len=strlen(buffer);
			if(_ispointerelem(temp_elem->elem_desc->type))
			{
				int tempret=Palloc0(def_addr,str_len+1);
				if(tempret<0)
					return tempret;
				Memcpy(*(char **)def_addr,buffer,str_len+1);
			}
			else
			{
				Memcpy(def_addr,buffer,str_len);
			}
			
		}
		else
		{
			ret=elem_ops->set_text_value(def_addr,buffer,temp_elem);
			if(ret<0)
				return -EINVAL;
		}
	}
	else
	{
		if(elem_ops->set_bin_value==NULL)
		{
			ret=get_fixed_elemsize(temp_elem->elem_desc->type);
			if(ret<=0)
				return -EINVAL;
			Memcpy(def_addr,&value,ret);
		}
		else
		{
			ret=elem_ops->set_bin_value(def_addr,&value,temp_elem);
			if(ret<0)
				return -EINVAL;
		}
	}
	return value;
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
	
	for(i=0;InitElemInfo[i].type!=CUBE_TYPE_ENDDATA;i++)
	{
		ret=struct_register_ops(InitElemInfo[i].type,
			InitElemInfo[i].enum_ops,
			InitElemInfo[i].flag,
			InitElemInfo[i].offset);
		if(ret<0)
			return ret;
	}
	return 0;		
}

int _count_struct_num(struct struct_elem_attr * struct_desc)
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

STRUCT_NODE * _get_root_node(STRUCT_NODE * start_node)
{
	while( start_node->parent != NULL)
		start_node=start_node->parent;
	return start_node;
}

char * _get_next_name(char * name,char * buffer)
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
		if(!_issubsetelem(curr_elem->elem_desc->type))
			return NULL;
		node=curr_elem->ref;
	}while(node != NULL);
	return NULL;
}

int get_fixed_elemsize(int type)
{
	if(offset_list[type]>0)
		return offset_list[type];
	if(_ispointerelem(type))
		return sizeof(void *);
	return offset_list[type];
	
}

int _getelemjsontype(int type)
{
	if((type<0)|| (type>CUBE_TYPE_ENDDATA))
		return -EINVAL; 
	if(_isnumelem(type))
		return JSON_ELEM_NUM;
	if(_isboolelem(type))
		return JSON_ELEM_BOOL;
	if(_issubsetelem(type))
		return JSON_ELEM_MAP;
	if(_isarrayelem(type))
		return JSON_ELEM_ARRAY;
	if(_isemptyelem(type))
		return 0;
	return JSON_ELEM_STRING;
		
}
/*
int _getelemjsontype(int type)
{
	switch(type)
	{
		case CUBE_TYPE_STRING:
		case CUBE_TYPE_UUID:
		case CUBE_TYPE_ENUM:
		case CUBE_TYPE_FLAG:
		case CUBE_TYPE_DEFENUM:
		case CUBE_TYPE_DEFFLAG:
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
*/
void * _elem_get_ref(void * elem)
{
	struct elem_template *  curr_elem=elem;
	if(curr_elem==NULL)	
		return NULL;
	if(curr_elem->ref!=NULL)
		return curr_elem->ref;
	return curr_elem->elem_desc->ref;
}

int _elem_set_ref(void * elem,void * ref)
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

