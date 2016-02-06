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

#endif

#include "../include/data_type.h"
#include "../include/list.h"
#include "../include/attrlist.h"
#include "../include/string.h"
#include "../include/alloc.h"
#include "../include/json.h"
#include "../include/struct_deal.h"
#include "../include/valuelist.h"
#include "../include/basefunc.h"
#include "../include/memdb.h"

#include "memdb_internal.h"

int read_namelist_json_desc(void * root,BYTE * uuid)
{
	int ret;
	struct struct_namelist * namelist;
	UUID_HEAD * head;
	
	void * temp_node;
	char buf[1024];

	ret=Galloc0(&namelist,sizeof(struct struct_namelist));
	if(ret<0)
		return ret;
	namelist->head.type=DB_NAMELIST;

	temp_node=json_find_elem("elemlist",root);
	if(temp_node==NULL)
		return -EINVAL;
	namelist_template=memdb_get_template(DB_NAMELIST,0);
	if(namelist==NULL)
		return -EINVAL;

	ret=json_2_struct(root,namelist,namelist_template);
	namelist->elem_no=json_get_elemno(temp_node);

	ret=memdb_comp_uuid(namelist);
	if(ret<0)
		return ret;	
	ret=memdb_store(namelist,DB_NAMELIST,0);
	if(ret<0)
		return ret;	
	ret=memdb_store_index(namelist,NULL,0);
	if(ret<0)
		return ret;
	
	return ret;	
}

int read_typelist_json_desc(void * root,BYTE * uuid)
{
	int ret;
	struct struct_namelist * namelist;
	struct struct_namelist * baselist;
	struct struct_typelist * typelist;
	struct struct_typelist * basetypelist;

	int * temp_node;
	char buf[1024];
	void * namelist_template;
	void * typelist_template;

	ret=Galloc0(&namelist,sizeof(struct struct_namelist));
	if(ret<0)
		return ret;

	ret=Galloc0(&typelist,sizeof(struct struct_typelist));
	if(ret<0)
		return ret;
	
//      store the namelist (if exists) and compute the uuid

	namelist->head.type=DB_NAMELIST;
	temp_node=json_find_elem("elemlist",root);
	if(temp_node==NULL)
		return -EINVAL;

	namelist_template=memdb_get_template(DB_NAMELIST,0);
	if(namelist_template==NULL)
		return -EINVAL;

	typelist_template=memdb_get_template(DB_TYPELIST,0);
	if(typelist_template==NULL)
		return -EINVAL;

	ret=json_2_part_struct(root,namelist,namelist_template,CUBE_ELEM_FLAG_INPUT);
	if(ret<0)
		return ret;

	ret=memdb_comp_uuid(namelist);
	if(ret<0)
		return ret;	
	ret=memdb_store(namelist,DB_NAMELIST,0);

//      generate the typelist struct
	
	Memcpy(typelist->head.name,namelist->head.name,DIGEST_SIZE);
	typelist->head.type=DB_TYPELIST;

	typelist->elem_no=namelist->elem_no;
	Memcpy(typelist->uuid,namelist->head.uuid,DIGEST_SIZE);
	ret=memdb_comp_uuid(typelist);
	if(ret<0)
		return ret;
	typelist->tail_desc=namelist;

//	merge the new typelist function

	baselist=memdb_find_byname("baselist",DB_NAMELIST,0);
	if(baselist==NULL)
		return -EINVAL;

	memdb_remove(baselist,DB_INDEX,0);

	ret=_merge_namelist(baselist,namelist);
	if(ret<0)
		return ret;
	ret=memdb_store_index(baselist,NULL,0);
	if(ret<0)
		return ret;

//     replace the old baselist to new baselist
	basetypelist=memdb_find_byname("baselist",DB_TYPELIST,0);
	if(basetypelist==NULL)
		return -EINVAL;

	memdb_remove(basetypelist,DB_INDEX,0);

	Memcpy(basetypelist->uuid,baselist->head.uuid,DIGEST_SIZE);

	basetypelist->elem_no=baselist->elem_no;

	ret=memdb_comp_uuid(basetypelist);
	if(ret<0)
		return ret;
	basetypelist->tail_desc=baselist;
	ret=memdb_store_index(basetypelist,NULL,0);
	if(ret<0)
		return ret;

//	reset the typelist ref in head.typelist
 
	ret=struct_set_ref(namelist_template,"head.type",baselist->elemlist);
	ret=struct_set_ref(typelist_template,"head.type",baselist->elemlist);
	void * subtypelist_template=memdb_get_template(DB_SUBTYPELIST,0);
	if(subtypelist_template==NULL)
		return -EINVAL;
	ret=struct_set_ref(subtypelist_template,"head.type",baselist->elemlist);
	ret=struct_set_ref(subtypelist_template,"type",baselist->elemlist);
	void * recordtype_template=memdb_get_template(DB_RECORDTYPE,0);
	if(recordtype_template==NULL)
		return -EINVAL;
	ret=struct_set_ref(recordtype_template,"head.type",baselist->elemlist);
	ret=struct_set_ref(recordtype_template,"type",baselist->elemlist);
	
	struct struct_namelist * baseflag = memdb_find_byname("baseflag",DB_NAMELIST,0);
	if(baseflag==NULL)
		return -EINVAL;
	struct_set_ref(recordtype_template,"index.flag",baseflag->elemlist);

	return ret;	
}

int read_subtypelist_json_desc(void * root,BYTE * uuid)
{
	int ret;
	struct struct_namelist * namelist;
	struct struct_subtypelist * subtypelist;

	int * temp_node;
	char buf[1024];
	void * namelist_template;
	void * subtypelist_template;

	ret=Galloc0(&namelist,sizeof(struct struct_namelist));
	if(ret<0)
		return ret;

	ret=Galloc0(&subtypelist,sizeof(struct struct_subtypelist));
	if(ret<0)
		return ret;
	
//      store the namelist (if exists) and compute the uuid

	namelist->head.type=DB_NAMELIST;
	temp_node=json_find_elem("elemlist",root);
	if(temp_node==NULL)
		return -EINVAL;

	namelist_template=memdb_get_template(DB_NAMELIST,0);
	if(namelist_template==NULL)
		return -EINVAL;

	subtypelist_template=memdb_get_template(DB_SUBTYPELIST,0);
	if(subtypelist_template==NULL)
		return -EINVAL;

	ret=json_2_part_struct(root,namelist,namelist_template,CUBE_ELEM_FLAG_INPUT);
	if(ret<0)
		return ret;

	ret=memdb_comp_uuid(namelist);
	if(ret<0)
		return ret;	
	ret=memdb_store(namelist,DB_NAMELIST,0);

//      generate the typelist struct
	
	temp_node=json_find_elem("type",root);
	if(temp_node==NULL)
		return -EINVAL;
	
	ret=struct_write_elem_text("type",subtypelist,json_get_valuestr(temp_node),subtypelist_template);
	if(ret<0)
		return ret;

	Memcpy(subtypelist->head.name,namelist->head.name,DIGEST_SIZE);
	subtypelist->head.type=DB_SUBTYPELIST;

	subtypelist->elem_no=namelist->elem_no;
	Memcpy(subtypelist->uuid,namelist->head.uuid,DIGEST_SIZE);
	ret=memdb_comp_uuid(subtypelist);
	if(ret<0)
		return ret;
	subtypelist->tail_desc=namelist;

	ret=memdb_store(subtypelist,DB_SUBTYPELIST,0);
	if(ret<0)
		return ret;

	ret=memdb_store_index(subtypelist,NULL,0);
	if(ret<0)
		return ret;

	return 0;	
}

int _read_struct_json(void * root,void ** record)
{
	int ret;
	void * root_node = root;
	void * father_node = root;
	void * curr_node = root;
	void * temp_node = NULL;
	void * desc_node = NULL;
	void * struct_template ; 
	int i;
	int elem_no;
	int elem_size;

	struct struct_desc_record * struct_desc_record;
	struct struct_desc_record * child_desc_record;
	struct struct_namelist * ref_namelist;
	BYTE struct_uuid[DIGEST_SIZE];

	struct elem_attr_octet * struct_desc_octet; 	
	struct elem_attr_octet * elem_desc_octet; 

	struct struct_elem_attr * struct_desc;
	struct struct_elem_attr * elem_desc;

	// get the struct_desc_record  's self template	
	struct_template=memdb_get_template(DB_STRUCT_DESC,0);
	if(struct_template==NULL)
		return -EINVAL;

	// generate the struct_desc_record struct

	ret=Galloc0(&struct_desc_record,sizeof(struct struct_desc_record));
	if(ret<0)
		return ret;

	ret=json_2_struct(root_node,struct_desc_record,struct_template);

	if(ret<0)
		return ret;

	// convert all the ref name to ref uuid
        // and generate the struct_desc list

	ret=Galloc0(&struct_desc,sizeof(struct struct_elem_attr)*(struct_desc_record->elem_no+1));
	if(ret<0)
		return ret;
	struct_desc_octet=struct_desc_record->elem_desc;

	struct_desc_record->tail_desc=struct_desc;

	for(i=0;i<struct_desc_record->elem_no;i++)
	{
		elem_desc_octet=struct_desc_octet+i;
		elem_desc=struct_desc+i;

		// duplicate all the value except ref		

		ret=Galloc0(&elem_desc->name,Strnlen(elem_desc_octet->name,DIGEST_SIZE));
		if(ret<0)
			return ret;
		Strncpy(elem_desc->name,elem_desc_octet->name,DIGEST_SIZE);

		elem_desc->type=elem_desc_octet->type;
		elem_desc->size=elem_desc_octet->size;
		if(elem_desc_octet->def[0]==0)
			elem_desc->def=NULL;
		else
			elem_desc->def=elem_desc_octet->def;
		
		// if their is no valid ref
		if((elem_desc_octet->type == CUBE_TYPE_SUBSTRUCT)
			||(elem_desc_octet->type == CUBE_TYPE_ARRAY))
		{
			
			if(elem_desc_octet->ref[0]!=0)
			{
				child_desc_record=memdb_find(elem_desc_octet->ref,DB_STRUCT_DESC,0);
			}		
			else
			{
				child_desc_record=memdb_find_byname(elem_desc_octet->ref_name,DB_STRUCT_DESC,0);	
			}
			if(child_desc_record==NULL)
				return -EINVAL;
			elem_desc->ref=child_desc_record->tail_desc;	
			if(elem_desc->ref==NULL)
				return -EINVAL;
			Memcpy(elem_desc_octet->ref,child_desc_record->head.uuid,DIGEST_SIZE);
		}

		else if((elem_desc_octet->type == CUBE_TYPE_ENUM)
			||(elem_desc_octet->type == CUBE_TYPE_FLAG))
		{
			if(elem_desc_octet->ref[0]!=0)
			{
				ref_namelist=memdb_find(elem_desc_octet->ref,DB_NAMELIST,0);
			}		
			else
			{
				ref_namelist=memdb_find_byname(elem_desc_octet->ref_name,DB_NAMELIST,0);	
			}
			if(ref_namelist==NULL)
				return -EINVAL;
			elem_desc->ref=ref_namelist->elemlist;	
			if(elem_desc->ref==NULL)
				return -EINVAL;
			Memcpy(elem_desc_octet->ref,ref_namelist->head.uuid,DIGEST_SIZE);

		}

	}
	struct_desc_record->head.type=DB_STRUCT_DESC;
	ret=_comp_struct_digest(struct_desc_record->head.uuid,struct_desc_record);
	if(ret<0)
		return ret;
	ret=memdb_store(struct_desc_record,DB_STRUCT_DESC,0);
	if(ret<0)
		return ret;
	

	// convett the ref

	// compute the struct's template


/*
	json_node_set_pointer(father_node,struct_desc);

	i=0;
	do
	{
		if(i==0)
		{
			curr_node=json_get_first_child(father_node);
			if(curr_node==NULL)
				return -EINVAL;
		}
		if(i==elem_no)
		{
			struct_desc[i].type=CUBE_TYPE_ENDDATA;
				
			curr_node=father_node;
			ret=Galloc(&struct_desc_record,sizeof(struct struct_desc_record));
			if(ret<0)
				return ret;
			struct_desc_record->head.type=DB_STRUCT_DESC;		
			struct_desc_record->head.subtype=0;		
			struct_desc_record->elem_no=elem_no;		
			memset(struct_desc_record->head.uuid,0,DIGEST_SIZE);
			struct_desc_record->elem_desc=struct_desc;
			ret=_comp_struct_digest(struct_desc_record->head.uuid,struct_desc_record);
			if(ret<0)
				return ret;
			memdb_store(struct_desc_record,DB_STRUCT_DESC,0);
			if(curr_node==root_node)
				break;
			curr_node=json_get_father(curr_node);
			father_node=json_get_father(curr_node);
			struct_desc=json_node_get_pointer(father_node);
			i=json_node_get_no(father_node);
			memcpy(struct_desc[i].ref_uuid,struct_desc_record->head.uuid,DIGEST_SIZE);
			curr_node=json_get_next_child(father_node);
			elem_no = json_get_elemno(father_node);
			i++;
			continue;
		}
		if(json_get_type(curr_node)!=JSON_ELEM_MAP)
			return -EINVAL;
		
		json_node_set_no(father_node,i);
		temp_node=json_find_elem("name",curr_node);
		if(temp_node==NULL)
			return -EINVAL;
		char *valuestr;
		int value;
		valuestr =json_get_valuestr(temp_node);
		ret=struct_write_elem_text("name",&struct_desc[i],valuestr,struct_template);
		if(ret<0)
			return -EINVAL;
		temp_node=json_find_elem("type",curr_node);
		if(temp_node==NULL)	
			return -EINVAL;
		// deal with type
		valuestr = json_get_valuestr(temp_node);
		ret=struct_write_elem_text("type",&struct_desc[i],valuestr,struct_template);
		if(struct_desc[i].type == CUBE_TYPE_SUBSTRUCT)
		{
			struct_desc[i].size=sizeof(void *);
			temp_node=json_find_elem("ref",curr_node);
			if(temp_node==NULL)
				return -EINVAL;
			json_node_set_no(temp_node,0);
			elem_no = json_get_elemno(temp_node);
			i=0;

			ret=Galloc(&struct_desc,sizeof(struct elem_attr_octet)*(elem_no+1));
			if(ret<0)
				return ret;
			json_node_set_pointer(temp_node,struct_desc);
			father_node=temp_node;
			continue;
		}	
		elem_size=get_fixed_elemsize(struct_desc[i].type);
		// compute size
		if(elem_size>0)
			struct_desc[i].size=elem_size;
		else
		{
			temp_node=json_find_elem("size",curr_node);
			if(temp_node==NULL)
				return -EINVAL;
			ret=json_node_getvalue(temp_node,&value,sizeof(int));
			if(ret<0)
				return ret;
			ret=struct_write_elem("size",&struct_desc[i],&value,struct_template);
		}	
		// compute ref	
		memset(struct_desc[i].ref_uuid,0,DIGEST_SIZE);
		temp_node=json_find_elem("ref",curr_node);
		if(temp_node!=NULL)
		{
			switch(struct_desc[i].type)
			{
				case CUBE_TYPE_DEFINE:
				case CUBE_TYPE_DEFSTR:
				case CUBE_TYPE_DEFSTRARRAY:
					valuestr=json_get_valuestr(temp_node);
					if(ret<0)
						return ret;
					strncpy(struct_desc[i].ref_uuid,valuestr,DIGEST_SIZE);	
					break;
				case CUBE_TYPE_ENUM:
				case CUBE_TYPE_FLAG:
					break;
				default:
					return -EINVAL;
			}
		}
			
		curr_node=json_get_next_child(father_node);
		i++;

	}while(curr_node!=root_node);
*/
	*record=struct_desc_record;
	return 0;
}


int read_struct_json_desc(void * root, BYTE * uuid)
{
	int ret;
	// get the struct desc db
	void * temp_node ;
	struct struct_desc_record * struct_desc_record;

	if(json_get_type(root)!= JSON_ELEM_MAP)
		return -EINVAL;
	ret = _read_struct_json(root,&struct_desc_record);
	if(ret<0)
		return ret;
	
	memcpy(uuid,struct_desc_record->head.uuid,DIGEST_SIZE);
	temp_node=json_find_elem("name",root);
	if(temp_node!=NULL)
		strncpy(struct_desc_record->head.name,json_get_valuestr(temp_node),DIGEST_SIZE);
	memdb_store(struct_desc_record,DB_STRUCT_DESC,0);
	memdb_store_index(struct_desc_record,NULL,0);
	return 0;
}

int read_recorddef_json_desc(void * root,BYTE * uuid)
{
	int ret;
	int i;
	// get the struct desc db
	void * temp_node ;
	struct struct_desc_record * struct_desc_record;
	struct struct_recordtype * record_type;
	void * struct_template;
	char buffer[DIGEST_SIZE*2];

	if(json_get_type(root)!= JSON_ELEM_MAP)
		return -EINVAL;

	struct_template=memdb_get_template(DB_RECORDTYPE,0);
	if(struct_template==NULL)
		return -EINVAL;

	ret=Galloc0(&record_type,sizeof(struct struct_recordtype));
	if(ret<0)
		return ret;

	record_type->head.type=DB_RECORDTYPE;

	temp_node=json_find_elem("type",root);
	if(temp_node==NULL)
		return -EINVAL;
	

	ret=struct_write_elem_text("type",record_type,json_get_valuestr(temp_node),struct_template);
	if(ret<0)
		return ret;

	struct struct_subtypelist * subtypelist=memdb_get_subtypelist(record_type->type);
	if(subtypelist==NULL)
		return -EINVAL;

	struct struct_namelist * subnamelist=subtypelist->tail_desc;
	ret=struct_set_ref(struct_template,"subtype",subnamelist->elemlist);

	if(ret<0)
		return ret;

	ret=json_2_struct(root,record_type,struct_template);
	if(ret<0)
		return ret;

	temp_node=json_find_elem("struct_name",root);
	if(temp_node!=NULL)
	{
		struct_desc_record=memdb_find_byname(json_get_valuestr(temp_node),DB_STRUCT_DESC,0);
		if(struct_desc_record==NULL)
			return -EINVAL;
		Memcpy(record_type->uuid,struct_desc_record->head.uuid,DIGEST_SIZE);
	}
	else
	{
		struct_desc_record=memdb_find(struct_desc_record->head.uuid,DB_STRUCT_DESC,0);
		if(struct_desc_record==NULL)
			return -EINVAL;
	}
	
	ret=memdb_comp_uuid(record_type);
	if(ret<0)
		return ret;
	struct_template=create_struct_template(struct_desc_record->tail_desc);
	if(struct_template==NULL)
		return -EINVAL;
	for(i=0;i<record_type->flag_no;i++)
	{
		struct_set_flag(struct_template,record_type->index[i].flag,
			(char *)record_type->index[i].elemlist);

	}
	record_type->tail_desc=struct_template;

	memdb_store(record_type,DB_RECORDTYPE,0);

	memdb_store_index(record_type,NULL,0);
	_set_dynamic_db_bytype(record_type);
	return 0;
}


UUID_HEAD * _read_one_record(int type,int subtype,void * temp_node,void * record_template)
{
	int ret;
	int record_size;
	UUID_HEAD * head;
	void * record;	

	record_size=struct_size(record_template);
	if(record_size<0)
		return NULL;
	ret=Galloc0(&head,sizeof(UUID_HEAD)+record_size);
	if(ret<0)
		return ret;
	record=(void *)head+sizeof(UUID_HEAD);
	ret=json_2_struct(temp_node,record,record_template);
	if(ret<0)
	{
		Free(head);
		return NULL;
	}
	
	head->type=type;
	head->subtype=subtype;
	ret=memdb_comp_uuid(head);
	if(ret<0)
	{
		Free(head);
		return NULL;
	}
	memdb_store(head,type,subtype);
	return head;

}

int read_record_json_desc(void * root,BYTE * uuid)
{
	int ret;
	int i;
	// get the struct desc db
	void * temp_node ;
	void * record_node;

	void * record_template;
	void  * record;
	UUID_HEAD * head;

	
//	struct struct_desc_record * struct_desc_record;
//	struct struct_recordtype * record_type;
	void * struct_template;
	char buffer[DIGEST_SIZE*2];
	int type;
	int subtype;
	int record_size;

	if(json_get_type(root)!= JSON_ELEM_MAP)
		return -EINVAL;

	temp_node=json_find_elem("type",root);
	if(temp_node==NULL)
		return -EINVAL;
	type=memdb_get_typeno(json_get_valuestr(temp_node));
	if(type<0)
		return type;
	temp_node=json_find_elem("subtype",root);
	if(temp_node==NULL)
		subtype=0;
	else
	{
		subtype=memdb_get_subtypeno(type,json_get_valuestr(temp_node));
		if(subtype<0)
			return subtype;
	}

	
	temp_node=json_find_elem("record",root);
	if(temp_node==NULL)
		return -EINVAL;

	record_template=memdb_get_template(type,subtype);
	if(record_template==NULL)
		return -EINVAL;

	if(json_get_type(temp_node)==JSON_ELEM_MAP)
	{	
		head=_read_one_record(type,subtype,temp_node,record_template);
		if(head==NULL)
			return -EINVAL;
		return 1;
	}
	else if(json_get_type(temp_node)==JSON_ELEM_ARRAY)
	{
		int record_no=json_get_elemno(temp_node);
		if(record_no==0)
			return 0;
		record_node=json_get_first_child(temp_node);
		for(i=0;i<record_no;i++)
		{
			if(record_node==NULL)
				return -EINVAL;
			head=_read_one_record(type,subtype,record_node,record_template);
			if(head==NULL)
				return -EINVAL;
			if(i<record_no-1)
				record_node=json_get_next_child(temp_node);
		}
		return record_no;
	}

	return -EINVAL;
	
}

int memdb_read_desc(void * root, BYTE * uuid)
{
	NAME2POINTER funclist[]=
	{
		{"namelist",&read_namelist_json_desc},
		{"typelist",&read_typelist_json_desc},
		{"subtypelist",&read_subtypelist_json_desc},
		{"struct",&read_struct_json_desc},
		{"record_define",&read_recorddef_json_desc},
		{"record",&read_record_json_desc},
		{NULL,NULL},

	};	

	int (*read_json_func)(void * root, BYTE * uuid)=NULL;

	void * temp_node;
	int i;
	char * typestr;
	temp_node=json_find_elem("info-type",root);
	if(temp_node==NULL)
	{
		read_json_func=&read_record_json_desc;
	}
	else
	{
		typestr = json_get_valuestr(temp_node);
		if(typestr==NULL)
			return -EINVAL;

		i=0;
		while(funclist[i].name!=NULL)
		{
			if(strcmp(funclist[i].name,typestr)==0)
			{
				read_json_func=funclist[i].pointer;
				break;
			}
			i++;
		}
	}		
	if(read_json_func==NULL)
		return -EINVAL;
	return read_json_func(root,uuid);
	
}
