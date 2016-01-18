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
#include "../include/string.h"
#include "../include/alloc.h"
#include "../include/json.h"
#include "../include/struct_deal.h"
#include "../include/valuelist.h"
#include "../include/basefunc.h"
#include "../include/memdb.h"

#include "memdb_internal.h"

int _comp_struct_digest(BYTE * digest,void * record);
int _comp_db_digest(BYTE * digest,void * record);
int _comp_namelist_digest(BYTE * digest,void * namelist);

void *  _get_dynamic_db_bytype(int type,int subtype)
{
	void * db_list;
	DB_DESC * memdb;
	db_list=hashlist_get_first(dynamic_db_list);
	while(db_list!=NULL)
	{
		memdb=hashlist_get_desc(db_list);
		if(memdb->head.type == type)
		{
			if(memdb->head.subtype == subtype)
				return db_list;
		}
		db_list=hashlist_get_next(dynamic_db_list);
	}	 
	return NULL;
}

void * _set_dynamic_db_bytype(void * record_def)
{
	int ret;
	DB_DESC * memdb;
	struct struct_recordtype * record_type=record_def; 
	void * db_list=init_hash_list(8,record_type->type,record_type->subtype);
	if(db_list==NULL)
		return NULL;

	ret=Galloc0(&memdb,sizeof(DB_DESC));
	if(ret<0)
		return NULL;
	Memcpy(memdb->head.uuid,record_type->head.uuid,DIGEST_SIZE);
	Strncpy(memdb->head.name,record_type->head.name,DIGEST_SIZE);
	memdb->head.type=record_type->type;
	memdb->head.subtype=record_type->subtype;
	hashlist_set_desc(db_list,memdb);
	hashlist_add_elem(dynamic_db_list,db_list);
	memdb_set_template(record_type->type,record_type->subtype,record_type->tail_desc);
	return db_list;

}

int memdb_is_elem_namelist(void * elem)
{
	return _is_type_namelist(memdb_get_elem_type(elem));
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

int memdb_set_template(int type, int subtype,void * struct_template)
{
	DB_DESC * db_desc = Calloc0(sizeof(DB_DESC));
	if(db_desc == NULL)
		return -ENOMEM;
	void * db_list = memdb_get_dblist(type,subtype);
	if(db_list == NULL)
		return NULL;
	db_desc->head.type=type;
	db_desc->head.subtype=subtype;
	db_desc->struct_template = struct_template;
	return hashlist_set_desc(db_list,db_desc);
}

void * memdb_get_template(int type, int subtype)
{
	DB_DESC * db_desc;
	void * db_list = memdb_get_dblist(type,subtype);
	if(db_list == NULL)
		return NULL;
	db_desc = hashlist_get_desc(db_list);
	if(db_desc==NULL)
		return NULL;
	return db_desc->struct_template;
	
}

int memdb_set_index(int type,int subtype,int flag,char * elem_list)
{
	void * base_struct_template;
	int ret;

	// init the index db
	base_struct_template=memdb_get_template(type,subtype);
	if(base_struct_template==NULL)
		return -EINVAL;
	if(flag==0)
		return -EINVAL;
	ret=struct_set_flag(base_struct_template,flag,elem_list);
	return ret;
}
int memdb_store_index(void * record, char * name,int flag)
{
	int ret;
	BYTE buffer[4096];
	UUID_HEAD * head = record;
	if(head==NULL)
		return -EINVAL;
	void * db_list = memdb_get_dblist(DB_INDEX,0);
	if(db_list == NULL)
		return NULL;
	
	INDEX_ELEM * index;
	INDEX_ELEM * findindex;

	ret=Galloc(&index,sizeof(INDEX_ELEM));
	if(ret<0)
		return -ENOMEM;
	
	Memcpy(index->uuid,head->uuid,DIGEST_SIZE);
	index->flag=flag;
	index->head.type=head->type;
	index->head.subtype=head->subtype;
	if(name==NULL)
		Memcpy(index->head.name,head->name,DIGEST_SIZE);
	else
	{
		memset(index->head.name,0,DIGEST_SIZE);
		strncpy(index->head.name,name,DIGEST_SIZE);
	}

	if(flag==0)
	{
		Memcpy(index->head.uuid,head->uuid,DIGEST_SIZE);
	}
	else
	{
		void * db_template;
		db_template=memdb_get_template(head->type,head->subtype);
		if(db_template==NULL)
		{
			Free(index);
			return -EINVAL;
		}
		ret=struct_2_part_blob(record,buffer,db_template,flag);
		if(ret<0)
			return -EINVAL;

		ret=calculate_context_sm3(buffer,ret,index->head.uuid);
		if(ret<0)
			return ret;	
	}
	findindex=hashlist_find_elem(db_list,index);
	if(findindex!=NULL)
	{
		if(!memcmp(findindex->head.uuid,index->head.uuid,DIGEST_SIZE))
			return -EINVAL;
		Free(findindex);	
	}
	return hashlist_add_elem(db_list,index);
}

INDEX_ELEM * memdb_find_index_byuuid(BYTE * uuid)
{
	INDEX_ELEM * index;
	index=memdb_get_first(DB_INDEX,0);
	while(index!=NULL)
	{
		if(!memcmp(uuid,index->uuid,DIGEST_SIZE))
		{
			return index;	
		}
		index=memdb_get_next(DB_INDEX,0);
	}
	return NULL;	
	
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
	ret=_comp_struct_digest(record->head.uuid,record);
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
	memcpy(uuid,record->head.uuid,DIGEST_SIZE);
	ret=memdb_store(record,DB_STRUCT_DESC,0);
	ret=memdb_store_index(record,NULL,0);
	return ret;
}

/*
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
	dblist=memdb_get_dblist(type,subtype);
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
}*/

int memdb_init()
{
	int ret;
	int i;
	void * struct_template; 
	void * pointer=&static_db_list;
	void * base_struct_template;
	void * namelist_template;
	void * typelist_template;
	void * subtypelist_template;
	void * recordtype_template;
	void * index_template;
	struct struct_namelist  * baselist;
	struct struct_typelist  * basetypelist;

	// init those basic template
	head_template= create_struct_template(&uuid_head_desc);
	if(head_template==NULL)
		return -EINVAL;

	elem_template= create_struct_template(&elem_attr_octet_desc);
	if(elem_template==NULL)
		return -EINVAL;

	ret=Galloc(pointer,sizeof(void *)*TYPE_BASE_END);
	if(ret<0)
		return ret;

	// init the four start list
	memset(static_db_list,0,sizeof(void *)*TYPE_BASE_END);
	static_db_list[DB_INDEX]=init_hash_list(10,DB_INDEX,0);
	static_db_list[DB_STRUCT_DESC]=init_hash_list(8,DB_STRUCT_DESC,0);
	static_db_list[DB_NAMELIST]=init_hash_list(8,DB_NAMELIST,0);
	static_db_list[DB_TYPELIST]=init_hash_list(8,DB_TYPELIST,0);
	static_db_list[DB_SUBTYPELIST]=init_hash_list(8,DB_SUBTYPELIST,0);
	static_db_list[DB_RECORDTYPE]=init_hash_list(8,DB_RECORDTYPE,0);

	if(static_db_list[DB_STRUCT_DESC]==NULL)
	{
		return -EINVAL;
	}


	// init the index db's template
	base_struct_template=create_struct_template(&struct_index_desc);
//	struct_set_flag(base_struct_template,CUBE_ELEM_FLAG_TEMP,"name,type");
//	struct_set_flag(base_struct_template,CUBE_ELEM_FLAG_TEMP<<1,"name,type,size");
	
	memdb_set_template(DB_INDEX,0,base_struct_template);

	memdb_set_index(DB_INDEX,0,CUBE_ELEM_FLAG_INDEX,"head.name,head.type,head.subtype,flag,uuid");	

	// init and set struct db's template 
	base_struct_template=create_struct_template(&struct_define_desc);
	memdb_set_template(DB_STRUCT_DESC,0,base_struct_template);
	memdb_set_index(DB_STRUCT_DESC,0,CUBE_ELEM_FLAG_INDEX,"head.name,head.type,head.subtype,elem_no,elem_desc");

	
	// init and set namelist

	namelist_template=create_struct_template(&struct_namelist_desc);
	struct_set_flag(namelist_template,CUBE_ELEM_FLAG_INDEX,"head.name,head.type,elem_no,elemlist");
	struct_set_flag(namelist_template,CUBE_ELEM_FLAG_INPUT,"head.name,elem_no,elemlist");


	memdb_set_template(DB_NAMELIST,0,namelist_template);

	// init and set typelist
	// notice that the first typelist's name is baselist, 
	// and it stores the complete local typelist 
  	// if new typelist add in the typelist db, 
	// its name-value list will add in the main typelist
	// if there is any conflict in new list and the baselist
	// the store action of new list will be failed
	 
	typelist_template=create_struct_template(&struct_typelist_desc);
	struct_set_flag(typelist_template,CUBE_ELEM_FLAG_INDEX,"head.name,head.type,elem_no,uuid");
	memdb_set_template(DB_TYPELIST,0,typelist_template);

	ret=Galloc0(&baselist,sizeof(struct struct_namelist));
	if(ret<0)
		return ret;
	Strncpy(baselist->head.name,"baselist",DIGEST_SIZE);
	baselist->head.type=DB_NAMELIST;
	for(i=0;struct_type_baselist[i].name!=NULL;i++);
	baselist->elem_no=i;
	ret=Galloc0(&baselist->elemlist,sizeof(NAME2VALUE)*baselist->elem_no);
	for(i=0;i<baselist->elem_no;i++)
	{	
		baselist->elemlist[i].name=struct_type_baselist[i].name;	
		baselist->elemlist[i].value=struct_type_baselist[i].value;	
	}
	memdb_comp_uuid(baselist);
	ret=memdb_store(baselist,DB_NAMELIST,0);
	if(ret<0)
		return ret;
	ret=memdb_store_index(baselist,NULL,0);
	if(ret<0)
		return ret;

	ret=Galloc0(&basetypelist,sizeof(struct struct_typelist));
	if(ret<0)
		return ret;
	Strncpy(basetypelist->head.name,"baselist",DIGEST_SIZE);
	basetypelist->head.type=DB_TYPELIST;
	basetypelist->elem_no=baselist->elem_no;
	Memcpy(basetypelist->uuid,baselist->head.uuid,DIGEST_SIZE);
	memdb_comp_uuid(basetypelist);
	ret=memdb_store(basetypelist,DB_TYPELIST,0);
	if(ret<0)
		return ret;
	ret=memdb_store_index(basetypelist,NULL,0);
	if(ret<0)
		return ret;

	struct_set_ref(namelist_template,"head.type",baselist->elemlist);
	struct_set_ref(typelist_template,"head.type",baselist->elemlist);

	// compute the base typelist
		


	//  build the subtypelist database

	subtypelist_template=create_struct_template(&struct_subtypelist_desc);
	struct_set_ref(subtypelist_template,"head.type",baselist->elemlist);
	struct_set_ref(subtypelist_template,"type",baselist->elemlist);
	struct_set_flag(subtypelist_template,CUBE_ELEM_FLAG_INDEX,"head.name,head.type,type,elem_no,uuid");
	
	memdb_set_template(DB_SUBTYPELIST,0,subtypelist_template);

	//  build the recordtype database

	recordtype_template=create_struct_template(&struct_recordtype_desc);
	struct_set_ref(recordtype_template,"head.type",baselist->elemlist);
	struct_set_ref(recordtype_template,"type",baselist->elemlist);
	struct_set_flag(recordtype_template,CUBE_ELEM_FLAG_KEY,"head.name,head.type,type,subtype,uuid");

	memdb_set_template(DB_RECORDTYPE,0,recordtype_template);

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
void * memdb_find_byname(char * name,int type,int subtype)
{
	int ret;
	void * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_find_elem_byname(db_list,name);
}
void * memdb_get_first(int type,int subtype)
{
	int ret;
	void * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_get_first(db_list);
}

void * memdb_get_next(int type,int subtype)
{
	int ret;
	void * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_get_next(db_list);
}

void * memdb_remove(void * baselist,int type,int subtype)
{
	int ret;
	void * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_remove_elem(db_list,baselist);
}

int memdb_reset_baselist()
{
	struct struct_namelist * baselist=memdb_find_byname("baselist",DB_TYPELIST,0);
	if(baselist==NULL)
		return -EINVAL;
	if(baselist->elemlist==NULL)
		return -EINVAL;

	void * typelist_template=memdb_get_template(DB_TYPELIST,0);
	if(typelist_template==NULL)
		return -EINVAL;
	struct_set_ref(typelist_template,"head.type",baselist->elemlist);

	void * subtypelist_template=memdb_get_template(DB_SUBTYPELIST,0);
	if(subtypelist_template==NULL)
		return -EINVAL;
	struct_set_ref(subtypelist_template,"head.type",baselist->elemlist);
	struct_set_ref(subtypelist_template,"type",baselist->elemlist);
	return 0;
}

void * memdb_get_subtypelist(int type)
{
	struct struct_subtypelist * subtypelist;	
	subtypelist=memdb_get_first(DB_SUBTYPELIST,0);

	while(subtypelist!=NULL)
	{
		if(subtypelist->type==type)
			return subtypelist;
		subtypelist=memdb_get_next(DB_SUBTYPELIST,0);
	}
	return NULL;
}

int memdb_get_typeno(char * typestr)
{
	struct struct_typelist * baselist=memdb_find_byname("baselist",DB_TYPELIST,0);
	if(baselist==NULL)
		return -EINVAL;
	if(baselist->tail_desc==NULL)
		return -EINVAL;
	return _get_value_namelist(typestr,baselist->tail_desc);

}

int memdb_get_subtypeno(int typeno,char * typestr)
{
	struct struct_subtypelist  * subtypelist=memdb_get_subtypelist(typeno);
	if(subtypelist==NULL)
		return -EINVAL;
	return _get_value_namelist(typestr,subtypelist->tail_desc);

}

int memdb_print(void * data,char * json_str)
{
	struct struct_desc_record * struct_record=data;
	void * struct_template ; 
	int ret;
	int offset;

	struct_template=memdb_get_template(struct_record->head.type,struct_record->head.subtype);
	
	if(memdb_is_dynamic(struct_record->head.type))
	{
		offset=struct_2_json(data+sizeof(UUID_HEAD),json_str,struct_template);
	}
	else
	{
		offset=struct_2_json(struct_record,json_str,struct_template);
	}
	if(offset<0)
		return offset;
	return offset;
}

int memdb_print_namelist(void * namelist,char * json_str)
{
	int ret;
	UUID_HEAD * head=namelist;
	void * namelist_template;
	if(head==NULL)
		return -EINVAL;
	if(json_str==NULL)
		return -EINVAL;
	namelist_template = memdb_get_template(head->type,head->subtype);
	if(namelist_template == NULL)
		return -EINVAL;
	ret=struct_2_json(namelist,json_str,namelist_template);
	return ret;	
}

int memdb_print_index(void * index,char * json_str)
{
	int ret;
	UUID_HEAD * head=index;
	void * index_template;
	if(head==NULL)
		return -EINVAL;
	if(json_str==NULL)
		return -EINVAL;
	index_template = memdb_get_template(DB_INDEX,0);
	if(index_template == NULL)
		return -EINVAL;
	ret=struct_2_json(index,json_str,index_template);
	return ret;	
}

int _comp_namelist_digest(BYTE * digest,void * namelist)
{
	int ret;
	struct struct_namelist * list=namelist;
	int type = list->head.type;
	BYTE buffer[4096];
	if( ! _is_type_namelist(type))
		return -EINVAL;

	void * namelist_template;

	namelist_template=memdb_get_template(type,0);
	if(namelist_template==NULL)
		return -EINVAL;

	ret=struct_2_part_blob(namelist,buffer,namelist_template,CUBE_ELEM_FLAG_KEY);

	if(ret<0)
		return ret;	

	ret=calculate_context_sm3(buffer,ret,digest);
	if(ret<0)
		return ret;	
	return 0;
}


int memdb_comp_uuid(void * record)
{
	BYTE buf[4096];	
	int blob_size;
	UUID_HEAD * head=record;
	void * struct_template=memdb_get_template(head->type,head->subtype);
	if(struct_template == NULL)
		return -EINVAL;
	if(record==NULL)
		return -EINVAL;
	
	blob_size=struct_2_part_blob(record,buf,struct_template,CUBE_ELEM_FLAG_INDEX);
	if(blob_size<0)
		return blob_size;
	calculate_context_sm3(buf,blob_size,&head->uuid);
	
	return blob_size;
}

int _comp_struct_digest(BYTE * digest,void * record)
{
	BYTE buf[4096];	
	int offset=0;
	int ret;
	int i;
	struct struct_desc_record * struct_record=record;
	void * base_struct_template=memdb_get_template(DB_STRUCT_DESC,0);
	if(record==NULL)
		return -EINVAL;
	int blob_size;
	
	blob_size=struct_2_part_blob(struct_record,buf,base_struct_template,CUBE_ELEM_FLAG_INDEX);

	calculate_context_sm3(buf,blob_size,digest);
	return blob_size;
}

int _merge_namelist(void * list1, void * list2)
{
	struct struct_namelist * namelist1 = list1;
	struct struct_namelist * namelist2 = list2;
	int ret;
	int elem_no;
	int i,j,k;
	NAME2VALUE  * buf;

	elem_no = namelist1->elem_no+namelist2->elem_no;

	ret = Galloc0(&buf,sizeof(NAME2VALUE)*elem_no);
	if(ret<0)
		return ret;
	j=0;
	k=0;
	for(i=0;i<elem_no;i++)
	{
		if(j==namelist1->elem_no)
		{
			buf[i].value=namelist2->elemlist[k].value;
			buf[i].name=namelist2->elemlist[k++].name;
			continue;
		}
		if(k==namelist2->elem_no)
		{
			buf[i].value=namelist1->elemlist[j].value;
			buf[i].name=namelist1->elemlist[j++].name;
			continue;
		}

		if(namelist1->elemlist[j].value<namelist2->elemlist[k].value)
		{
			buf[i].value=namelist1->elemlist[j].value;
			buf[i].name=namelist1->elemlist[j++].name;
		}
		else if(namelist1->elemlist[j].value>namelist2->elemlist[k].value)
		{
			buf[i].value=namelist2->elemlist[k].value;
			buf[i].name=namelist2->elemlist[k++].name;
		}
		else
		{
			buf[i].value=namelist1->elemlist[j].value;
			buf[i].name=namelist1->elemlist[j++].name;
			elem_no--;
			k++;
		}
	}
	Free0(namelist1->elemlist);
	namelist1->elem_no=elem_no;
	namelist1->elemlist=buf;
	_comp_namelist_digest(namelist1->head.uuid,namelist1);
	return elem_no;
}

int read_namelist_json_desc(void * root,BYTE * uuid)
{
	int ret;
	struct struct_namelist * namelist;

	int * temp_node;
	char buf[1024];
	void * namelist_template;

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

int store_struct_desc(char * name,void * attr_list)
{
	

}
void * get_struct_desc(char * name)
{
	
}

int register_struct_template(int type, int subtype,void * struct_desc)
{

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
