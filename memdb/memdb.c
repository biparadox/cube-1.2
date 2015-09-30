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
//#include "../include/kernel_comp.h"
//#include "../include/list.h"

#endif

#include "../include/data_type.h"
#include "../include/list.h"
#include "../include/string.h"
#include "../include/alloc.h"
#include "../include/struct_deal.h"
#include "../include/valuelist.h"
#include "../include/memdb.h"

enum base_struct_type
{
	TYPE_STRUCT_DESC,
	TYPE_DB_HEADER,
	TYPE_MESSAGE_HEAD,
	TYPE_PROC_HEAD,
	TYPE_BASE_END=0x100,
};

enum base_cube_db
{
	DB_STRUCT_DESC,
	DB_STRUCT_TEMPLATE,
};

struct struct_elem_attr elem_attr_desc_array[] =
{
	{"name",OS210_TYPE_ESTRING,sizeof(char *),NULL,0},
	{"type",OS210_TYPE_ENUM,sizeof(int),&elem_type_valuelist,0},
	{"size",OS210_TYPE_INT,sizeof(int),NULL,0},
	{"ref",OS210_TYPE_UUID,DIGEST_SIZE,NULL,0},
	{"attr",OS210_TYPE_FLAG,sizeof(int),&elem_attr_flaglist,0},
	{NULL,OS210_TYPE_ENDDATA,0,NULL,0}
};

struct cube_struct_type_list
{
	struct list_head list;
	int struct_type;
	int sub_type;
	BYTE uuid[DIGEST_SIZE];
	int    elem_no;
	struct struct_elem_attr * elem_list;
}STRUCT_DESC_ELEM;


struct cube_core_lib {
	int struct_type;
	int sub_type;
	struct list_head head_list;
};


int read_struct_json_desc(char * root, void ** attr_list)
{
	JSON_NODE * root_node = (JSON_NODE *)root;
	JSON_NODE * curr_node = (JSON_NODE *)root;
	int elem_no;
	struct struct_elem_attr * 
l

	if(json_get_type(root_node)!= JSON_ELEM_MAP)
		return -EINVAL;

	curr_node=find_json_elem("desc",root);
	if(curr_node==NULL)
		return -EINVAL;
	if(json_get_type(curr_node)!=JSON_ELEM_MAP)
		return -EINVAL;	
	elem_no	
	

}

int replace_

int store_struct_desc(char * name,void * attr_list)
{
	

}
void * get_struct_desc(char * name)
{
	
}

int register_struct_template(int type, int subtype,void * struct_desc)
{

}

int memdb_init()
{
	elem_type_valuelist=elem_type_valuelist_array;
	elem_attr_flaglist=elem_attr_flaglist_array;
	elem_attr_desc=elem_attr_desc_array;

	
	memset(&struct_desc_list,0,sizeof(struct cube_struct_type_list));
	INIT_LIST_HEAD(&struct_desc_list.list);
	

	return 0;
}

