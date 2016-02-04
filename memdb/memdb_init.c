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


	// alloc memspace for static database 
	ret=Galloc0(&static_db_list,sizeof(void *)*DB_TYPELIST);
	if(ret<0)
		return ret;

	// init those basic template
	head_template= create_struct_template(&uuid_head_desc);
	if(head_template==NULL)
		return -EINVAL;

	elem_template= create_struct_template(&elem_attr_octet_desc);
	if(elem_template==NULL)
		return -EINVAL;


	// init the four start list
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
