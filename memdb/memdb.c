
#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/list.h"
#include "../include/attrlist.h"
#include "../include/string.h"
#include "../include/alloc.h"
#include "../include/json.h"
#include "../include/struct_deal.h"
#include "../include/basefunc.h"
#include "../include/memdb.h"

#include "memdb_internal.h"

int read_namelist_json_desc(void * root,void * record)
{
	int ret;
	struct struct_namelist * namelist;
	DB_RECORD * db_record=record;
	void * namelist_template;
	
	void * temp_node;

	ret=Galloc0(&namelist,sizeof(struct struct_namelist));
	if(ret<0)
		return ret;
	if(db_record->head.type!=DB_NAMELIST)
		return -EINVAL;

	temp_node=json_find_elem("elemlist",root);
	if(temp_node==NULL)
		return -EINVAL;
	namelist_template=memdb_get_template(DB_NAMELIST,0);
	if(namelist_template==NULL)
		return -EINVAL;

	ret=json_2_struct(root,namelist,namelist_template);
//	namelist->elem_no=json_get_elemno(temp_node);
	db_record->record=namelist;

	ret=memdb_comp_uuid(db_record);
	if(ret<0)
		return ret;	
	ret=memdb_store_record(db_record);
	return ret;	
}

int read_typelist_json_desc(void * root,void * record)
{
	int ret;
	struct struct_namelist * namelist;
	struct struct_namelist * baselist;
	struct struct_typelist * typelist;
	struct struct_typelist * basetypelist;

	int * temp_node;
	void * namelist_template;
	void * typelist_template;
	DB_RECORD * db_record=record;
	DB_RECORD * namelist_record;


	ret=Galloc0(&typelist,sizeof(struct struct_typelist));
	if(ret<0)
		return ret;
	if(db_record->head.type!=DB_TYPELIST)
		return -EINVAL;
	
//      store the namelist (if exists) and compute the uuid

	temp_node=json_find_elem("uuid",root);
	if(temp_node == NULL)
	{
		// this typelist use namelist describe, 
		// we should finish namelist store first	
		ret=Galloc0(&namelist_record,sizeof(DB_RECORD));
		if(ret<0)
			return ret;
		namelist_record->head.type=DB_NAMELIST;
		ret=read_namelist_json_desc(root,namelist_record);
		if(ret<0)
			return ret;
		Memcpy(typelist->uuid,namelist_record->head.uuid,DIGEST_SIZE);
		namelist=namelist_record->record;
		typelist->elem_no=namelist->elem_no;			
	}
	else
	{
		typelist_template=memdb_get_template(DB_TYPELIST,0);
		if(typelist_template==NULL)
			return -EINVAL;

		ret=json_2_struct(root,typelist,typelist_template);
		namelist_record=memdb_find(typelist->uuid,DB_NAMELIST,0);
		if(namelist_record==NULL)
			return -EINVAL;
		namelist=namelist_record->record;
		if(typelist->elem_no==0)
			typelist->elem_no=namelist->elem_no;
	}

	db_record->record=typelist;
	ret=memdb_store_record(db_record);

	// merge the typelist 

	namelist_record=memdb_find_byname("typeenumlist",DB_NAMELIST,0);
	
	if(namelist_record==NULL)
		return -EINVAL;

	baselist=_merge_namelist(namelist_record->record,db_record->tail);
	if(baselist==NULL)
		return -EINVAL;

	ret=memdb_remove_byname("typeenumlist",DB_NAMELIST,0);
	if(ret<0)
		return -EINVAL;
	
	memdb_store(baselist,DB_NAMELIST,0,"typeenumlist");
	typeenumlist=baselist;

	return ret;	
}
		
int read_subtypelist_json_desc(void * root,void * record)
{
	int ret;
	struct struct_namelist * namelist;
	struct struct_subtypelist * subtypelist;

	int * temp_node;
	void * namelist_template;
	void * subtypelist_template;
	DB_RECORD * db_record=record;
	DB_RECORD * namelist_record;


	ret=Galloc0(&subtypelist,sizeof(struct struct_subtypelist));
	if(ret<0)
		return ret;
	if(db_record->head.type!=DB_SUBTYPELIST)
		return -EINVAL;
	
//      store the namelist (if exists) and compute the uuid

	temp_node=json_find_elem("uuid",root);
	if(temp_node == NULL)
	{
		// this typelist use namelist describe, 
		// we should finish namelist store first	
		ret=Galloc0(&namelist_record,sizeof(DB_RECORD));
		if(ret<0)
			return ret;
		namelist_record->head.type=DB_NAMELIST;
		ret=read_namelist_json_desc(root,namelist_record);
		if(ret<0)
		{
			Free0(namelist_record);
			return ret;
		}
		Memcpy(subtypelist->uuid,namelist_record->head.uuid,DIGEST_SIZE);
		namelist=namelist_record->record;
		subtypelist->elem_no=namelist->elem_no;			
		temp_node=json_find_elem("type",root);
		if(temp_node==NULL)
		{
			return -EINVAL;
		}
		subtypelist->type=memdb_get_typeno(json_get_valuestr(temp_node));
		if(subtypelist->type<0)
		{
			return -EINVAL;
		}
	
	}
	else
	{
		subtypelist_template=memdb_get_template(DB_SUBTYPELIST,0);
		if(subtypelist_template==NULL)
			return -EINVAL;

		ret=json_2_struct(root,subtypelist,subtypelist_template);
		namelist_record=memdb_find(subtypelist->uuid,DB_NAMELIST,0);
		if(namelist_record==NULL)
			return -EINVAL;
		namelist=namelist_record->record;
		if(subtypelist->elem_no==0)
			subtypelist->elem_no=namelist->elem_no;
	}

	db_record->record=subtypelist;
	ret=memdb_store_record(db_record);

	return ret;	
}
		

int read_default_json_desc(void * root,void * record)
{

	int ret;
	void * data;
	void * struct_template;

	DB_RECORD * db_record=record;
	
	void * temp_node;

	if(db_record->head.type <=0)
		return -EINVAL;

	struct_template=memdb_get_template(db_record->head.type,db_record->head.subtype);
	if(struct_template==NULL)
		return -EINVAL;

	ret=Galloc0(&data,struct_size(struct_template));
	if(ret<0)
		return ret;

	ret=json_2_struct(root,data,struct_template);
//	namelist->elem_no=json_get_elemno(temp_node);
	db_record->record=data;

	ret=memdb_comp_uuid(db_record);
	if(ret<0)
		return ret;	
	ret=memdb_store_record(db_record);
	return ret;	
}

int read_recordtype_json_desc(void * root,void * record)
{

	int ret;
	void * data;
	void * struct_template;

	DB_RECORD * db_record=record;
	DB_RECORD * struct_record;
	struct struct_desc_record * struct_desc;
	struct struct_recordtype * recordtype;
	
	void * temp_node;

	if(db_record->head.type <=0)
		return -EINVAL;

	struct_template=memdb_get_template(db_record->head.type,db_record->head.subtype);
	if(struct_template==NULL)
		return -EINVAL;

	ret=Galloc0(&recordtype,sizeof(struct struct_recordtype));
	if(ret<0)
		return ret;

	temp_node=json_find_elem("uuid",root);
	if(temp_node==NULL)
		return -EINVAL;
	char * uuid_str=json_get_valuestr(temp_node);
	
	if(!Isvaliduuid(uuid_str))
	{
		struct_record=memdb_find_byname(uuid_str,DB_STRUCT_DESC,0);
		if(struct_record==NULL)
			return -EINVAL;		
		Memcpy(recordtype->uuid,struct_record->head.uuid,DIGEST_SIZE);
		ret=json_remove_node(temp_node);
	}

	ret=json_2_struct(root,data,struct_template);
//	namelist->elem_no=json_get_elemno(temp_node);
	db_record->record=data;

	ret=memdb_comp_uuid(db_record);
	if(ret<0)
		return ret;	
	ret=memdb_store_record(db_record);
	return ret;	
}


	// convett the ref

	// compute the struct's template


/*
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
		Free0(head);
		return NULL;
	}
	
	head->type=type;
	head->subtype=subtype;
	ret=memdb_comp_uuid(head);
	if(ret<0)
	{
		Free0(head);
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
*/
int memdb_read_desc(void * root, BYTE * uuid)
{
/*
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
*/
	void * temp_node;
	void * head_node;
	void * record_node;
	int i;
	int ret;
	char * typestr;
	DB_RECORD * db_record;
	


	head_node=json_find_elem("head",root);
	if(head_node==NULL)
	{
		return -EINVAL;
	}
	record_node=json_find_elem("record",root);
	if(record_node==NULL)
	{
		return -EINVAL;
	}
	
	
	ret=Galloc0(&db_record,sizeof(DB_RECORD));
	
	ret=json_2_struct(head_node,&(db_record->head),head_template);
	if(ret<0)
		return -EINVAL;	
	
	if(db_record->head.type<DB_DTYPE_START)
	{
		switch(db_record->head.type)
		{
			case DB_NAMELIST:
				ret=read_namelist_json_desc(record_node,db_record);
				break;
			case DB_STRUCT_DESC:
				ret=read_default_json_desc(record_node,db_record);
				break;
			case DB_TYPELIST:
				ret=read_typelist_json_desc(record_node,db_record);
				break;
			case DB_SUBTYPELIST:
				ret=read_subtypelist_json_desc(record_node,db_record);
				break;
			case DB_CONVERTLIST:
				break;
			case DB_RECORDTYPE:
				ret=read_recordtype_json_desc(record_node,db_record);
				break;
			default:
				return -EINVAL;
		}
	}
	else
	{
		ret=read_default_json_desc(record_node,db_record);

	}

	return ret;
	
}
