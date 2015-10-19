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
#include "../include/basefunc.h"
#include "../include/memdb.h"

enum base_struct_type
{
	TYPE_STRUCT_DESC,
	TYPE_BASE_END=0x10,
	TYPE_DB_LIST=0x100,
};


struct elem_attr_octet
{
	char * name;
	enum os210_struct_elem_type type;
	int size;     //³¤¶ÈÖµ,¶Ô±ä³¤±äÁ¿,ÔòÎª×î´ó³¤¶ÈÖµ	
	char ref_uuid[DIGEST_SIZE];
	int  attr;
};

static void ** static_db_list;

static void * dynamic_db_list;

enum base_cube_db
{
	DB_STRUCT_DESC,
	DB_NAMEVALUE_TABLE,
};

struct struct_elem_attr elem_attr_octet_desc[] =
{
	{"name",OS210_TYPE_ESTRING,sizeof(char *),NULL,0},
	{"type",OS210_TYPE_ENUM,sizeof(int),&elem_type_valuelist,0},
	{"size",OS210_TYPE_INT,sizeof(int),NULL,0},
	{"ref",OS210_TYPE_UUID,DIGEST_SIZE,NULL,0},
	{"attr",OS210_TYPE_FLAG,sizeof(int),&elem_attr_flaglist,0},
	{NULL,OS210_TYPE_ENDDATA,0,NULL,0}
};

static void * base_struct_template;

struct struct_desc_record
{
	UUID_HEAD head;
	int elem_no;
	struct elem_attr_octet * elem_desc_list;
};

struct db_desc_record
{
	UUID_HEAD head;
	BYTE format_uuid[DIGEST_SIZE];
	void * struct_template;	
};

int _comp_struct_digest(BYTE * digest,void * record);
int _comp_db_digest(BYTE * digest,void * record);

void *  _get_dynamic_db_bytype(int type,int subtype)
{
	struct memdb_desc_record * memdb;
	memdb=hashlist_get_first(dynamic_db_list);
	while(memdb!=NULL)
	{
		if(memdb->head.type == type)
		{
			if(memdb->head.subtype == subtype)
				return memdb;
		}
		memdb=hashlist_get_next(dynamic_db_list);
	}	 
	return NULL;
}

void * memdb_get_dblist(int type, int subtype)
{
	void * db_list;
	if(type<0) 
		return NULL;
	if(type< TYPE_BASE_END)
		db_list=static_db_list[type];
	else if(type == TYPE_DB_LIST)
		db_list=dynamic_db_list;
	else if(type >TYPE_DB_LIST)
		db_list=_get_dynamic_db_bytype(type,subtype);
	else
		return NULL;
	return db_list;
}

void * _build_struct_desc(void ** elem_attr,int type,int subtype,char * name)
{
	int ret;
	struct struct_desc_record * record;
	int len;
	if(name!=NULL)
	{
		len=strlen(name);
		if(len>DIGEST_SIZE)
			return -EINVAL;
	}
	ret=Galloc(&record,sizeof(struct struct_desc_record));
	if(ret<0)
		return NULL;
	record->head.type=type;		
	record->head.subtype=subtype;		
	memset(record->head.uuid,0,DIGEST_SIZE);
	memset(record->head.name,0,DIGEST_SIZE);
	record->elem_desc_list=elem_attr;
	ret=_comp_struct_digest(record->head.uuid,struct_desc_record);
	if(ret<0)
		return ret;
	if(name!=NULL)
	{
		memcpy(record->head.name,name,len);
	}
	return record;
}

int memdb_register_struct(void ** elem_attr,char * name,BYTE * uuid)
{
	struct struct_desc_record * record;
	int ret;

	record=_build_struct_desc(elem_attr,0,0,name);
	if(record==NULL)
		return -EINVAL;
	memcpy(uuid,record->uuid,DIGEST_SIZE);
	ret=memdb_store(record,TYPE_STRUCT_DESC,0);
	return ret;
	
}

int memdb_register_db(BYTE * uuid,int type,int subtype,char * name)
{
	struct struct_desc_record * record;
	void * dblist;
	int ret;
	void * struct_template;
	void * struct_desc;
	if(type<0)
		return -EINVAL;
	if(type==TYPE_STRUCT_DESC)
		return -EINVAL;
	record=memdb_find(uuid,TYPE_STRUCT_DESC,0);
	if(record==NULL)
		return -EINVAL;
	dblist=memdb_get_dblist(tyoe,subtype);
	if(dblist!=NULL)
		return -EINVAL;
	
	dblist=init_hash_list(8,type,subtype);
	if(dblist==NULL)
		return NULL;
	
	
	
	if(type< TYPE_BASE_END)
	{
		if(static_db_list[type]!=NULL)
			return -EINVAL;
		ret=Galloc(&record,sizeof(struct memdb_desc_record));
		if(ret<0)
			return ret;
		static_db_list[type]=record;
	}
	else if(type == TYPE_DB_LIST)
		return -EINVAL;
	else if(type >TYPE_DB_LIST)
	{
		record=_get_dynamic_db_bytype(type,subtype);
		if(record!=NULL)
			return -EINVAL;
		ret=Galloc(&record,sizeof(struct memdb_desc_record));
		if(ret<0)
			return ret;
		ret=hashlist_add_elem(dynamic_db_list,record);
		if(ret<0)
			return ret;
	}
	else
}

int memdb_init()
{
	int ret;
	void * struct_template; 
	void * pointer=&static_db_list;
	ret=Galloc(pointer,sizeof(void *)*TYPE_BASE_END);
	if(ret<0)
		return ret;
	memset(static_db_list,0,sizeof(void *)*TYPE_BASE_END);
	static_db_list[TYPE_STRUCT_DESC]=init_hash_list(8,TYPE_STRUCT_DESC,0);
	if(static_db_list[TYPE_STRUCT_DESC]==NULL)
	{
		return -EINVAL;
	}

	base_struct_template=create_struct_template(&elem_attr_octet_desc);
	
	hashlist_set_desc(static_db_list[TYPE_STRUCT_DESC],base_struct_template);
	dynamic_db_list=init_hash_list(8,TYPE_DB_LIST,0);
	return 0;
}




int  memdb_store(void * data,int type,int subtype)
{
	int ret;
	void * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return -EINVAL;
	return hashlist_add_elem(db_list,data);
}

void * memdb_find(void * data,int type,int subtype)
{
	int ret;
	void * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_find_elem(db_list,data);
}

int _comp_struct_digest(BYTE * digest,void * record)
{
	BYTE buf[4096];	
	int offset=0;
	int ret;
	int i;
	struct struct_desc_record * struct_record=record;
	if(record==NULL)
		return -EINVAL;
	memcpy(buf+offset,&struct_record->head.type,sizeof(int));
	offset+=sizeof(int);
	memcpy(buf+offset,&struct_record->head.subtype,sizeof(int));
	offset+=sizeof(int);

	for(i=0;i<struct_record->elem_no;i++)
	{
		ret=struct_2_blob(&struct_record->elem_desc_list[i],buf+offset,base_struct_template);
		if(ret<0)
			return ret;
		offset+=ret;	
	}
	calculate_context_sm3(buf,offset,digest);
	return 0;
}

int read_struct_json_desc(void * root, BYTE * uuid)
{
	int ret;
	void * struct_db = memdb_get_dblist(TYPE_STRUCT_DESC,0);
	void * root_node = root;
	void * curr_node = root;
	void * temp_node = NULL;
	void * struct_template ; 
	int i;
	int elem_no;
	struct struct_desc_record * struct_desc_record;
	BYTE struct_uuid[DIGEST_SIZE];

	struct elem_attr_octet * struct_desc; 	

	struct elem_attr_octet * elem_desc; 

	if(json_get_type(root_node)!= JSON_ELEM_MAP)
		return -EINVAL;

	curr_node=find_json_elem("desc",root);
	if(curr_node==NULL)
		return -EINVAL;
	if(json_get_type(curr_node)!=JSON_ELEM_ARRAY)
		return -EINVAL;	

	root_node=curr_node;
	struct_template=hashlist_get_desc(static_db_list[TYPE_STRUCT_DESC]);
	
	elem_no = json_get_elemno(curr_node);
	
	ret=Galloc(&struct_desc,sizeof(struct elem_attr_octet)*(elem_no+1));
	if(ret<0)
		return ret;
	json_node_set_pointer(curr_node,struct_desc);

	i=0;
	do
	{
		if(i==0)
		{
			curr_node=get_first_json_child(curr_node);
			if(curr_node==NULL)
				return -EINVAL;
		}
		if(i==elem_no)
		{
			struct_desc[i].type=OS210_TYPE_ENDDATA;
				
			curr_node=get_json_father(curr_node);
			ret=Galloc(&struct_desc_record,sizeof(struct struct_desc_record));
			if(ret<0)
				return ret;
			struct_desc_record->head.type=TYPE_STRUCT_DESC;		
			struct_desc_record->head.subtype=0;		
			memset(struct_desc_record->head.uuid,0,DIGEST_SIZE);
			struct_desc_record->elem_desc_list=struct_desc;
			ret=_comp_struct_digest(struct_desc_record->head.uuid,struct_desc_record);
			if(ret<0)
				return ret;
			memdb_store(struct_desc_record,TYPE_STRUCT_DESC,0);
			struct_desc=json_node_get_pointer(curr_node);
			i=json_node_get_no(curr_node);
			memcpy(struct_desc[i].ref_uuid,struct_desc_record->head.uuid,DIGEST_SIZE);
			continue;
		}
		if(json_get_type(curr_node)==JSON_ELEM_MAP)
		{
			json_node_set_no(get_json_father(curr_node),i);
			temp_node=find_json_elem("name",curr_node);
			if(temp_node==NULL)
				return -EINVAL;
			char value[128];
			ret=json_node_getvalue(temp_node,value,128);
			ret=struct_write_elem_text("name",&struct_desc[i],value,struct_template);
			if(ret<0)
				return -EINVAL;
			struct_desc[i].type=OS210_TYPE_ORGCHAIN;
			struct_desc[i].size=DIGEST_SIZE;
			struct_desc[i].attr=0;

			elem_no = json_get_elemno(curr_node);
			i=0;

			ret=Galloc(&struct_desc,sizeof(struct elem_attr_octet)*(elem_no+1));
			if(ret<0)
				return ret;
			json_node_set_pointer(curr_node,struct_desc);
			continue;
			
		}	
		ret=json_2_struct(curr_node,&struct_desc[i],struct_template);
		if(ret<0)
			return ret;
		curr_node=get_next_json_child(get_json_father(curr_node));

	}while(curr_node!=root_node);

	ret=Galloc(&struct_desc_record,sizeof(struct struct_desc_record));
	if(ret<0)
		return ret;
	struct_desc_record->head.type=TYPE_STRUCT_DESC;		
	struct_desc_record->head.subtype=0;		
	memset(struct_desc_record->head.uuid,0,DIGEST_SIZE);
	struct_desc_record->elem_desc_list=struct_desc;
	ret=_comp_struct_digest(struct_desc_record->head.uuid,struct_desc_record);
	if(ret<0)
		return ret;
	memdb_store(struct_desc_record,TYPE_STRUCT_DESC,0);
	return 0;

}

int store_struct_desc(char * name,void * attr_list)
{
	

}
void * get_struct_desc(char * name)
{
	
}

int register_struct_template(int type, int subtype,void * struct_desc)
{

}

