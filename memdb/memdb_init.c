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
#include "../include/basefunc.h"
#include "../include/memdb.h"
#include "../struct_deal/struct_ops.h"

#include "valuelist.h"
#include "type_desc.h"
#include "memdb_internal.h"


ELEM_OPS enumtype_convert_ops;
ELEM_OPS recordtype_convert_ops;
ELEM_OPS subtype_convert_ops;

static struct InitElemInfo_struct MemdbElemInfo[] =
{
	{CUBE_TYPE_ELEMTYPE,&enumtype_convert_ops,0,sizeof(int)},
	{CUBE_TYPE_RECORDTYPE,&recordtype_convert_ops,ELEM_ATTR_VALUE,sizeof(int)},
	{CUBE_TYPE_RECORDSUBTYPE,&subtype_convert_ops,ELEM_ATTR_DEFINE,sizeof(int)},
	{CUBE_TYPE_ENDDATA,NULL,ELEM_ATTR_EMPTY,0},

};

/*
struct struct_elem_attr index_list_desc[] =
{
	{"flag",CUBE_TYPE_FLAG,sizeof(int),&elem_attr_flaglist_array,NULL},
	{"elemlist",CUBE_TYPE_ESTRING,sizeof(void *),NULL,NULL},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};

struct struct_elem_attr struct_namelist_desc[] =
{
	{"elem_no",CUBE_TYPE_INT,sizeof(int),NULL,NULL},
	{"elemlist",CUBE_TYPE_DEFNAMELIST,sizeof(void *),NULL,"elem_no"},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};
struct struct_elem_attr struct_typelist_desc[] =
{
	{"head",CUBE_TYPE_SUBSTRUCT,1,&uuid_head_desc,NULL},
	{"elem_no",CUBE_TYPE_INT,sizeof(int),NULL,NULL},
	{"uuid",CUBE_TYPE_UUID,DIGEST_SIZE,NULL,NULL},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};

struct struct_elem_attr struct_subtypelist_desc[] =
{
	{"head",CUBE_TYPE_SUBSTRUCT,1,&uuid_head_desc,NULL},
	{"type",CUBE_TYPE_ENUM,sizeof(int),NULL},
	{"elem_no",CUBE_TYPE_INT,sizeof(int),NULL,NULL},
	{"uuid",CUBE_TYPE_UUID,DIGEST_SIZE,NULL,NULL},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};

struct struct_elem_attr struct_define_desc[] =
{
	{"head",CUBE_TYPE_SUBSTRUCT,1,&uuid_head_desc,NULL},
	{"elem_no",CUBE_TYPE_INT,sizeof(int),NULL,NULL},
	{"elem_desc",CUBE_TYPE_ARRAY,sizeof(void *),&elem_attr_octet_desc,"elem_no"},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};

struct struct_elem_attr struct_record__desc[] =
{
	{"head",CUBE_TYPE_SUBSTRUCT,1,&uuid_head_desc,NULL},
	{"elem_no",CUBE_TYPE_INT,sizeof(int),NULL,NULL},
	{"elemlist",CUBE_TYPE_DEFNAMELIST,sizeof(void *),&elem_attr_octet_desc,"elem_no"},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};

struct struct_elem_attr struct_recordtype_desc[] =
{
	{"head",CUBE_TYPE_SUBSTRUCT,1,&uuid_head_desc,NULL},
	{"type",CUBE_TYPE_ENUM,sizeof(int),NULL},
	{"subtype",CUBE_TYPE_ENUM,sizeof(int),NULL},
	{"uuid",CUBE_TYPE_UUID,DIGEST_SIZE,NULL,NULL},
	{"flag_no",CUBE_TYPE_INT,sizeof(int),NULL,NULL},
	{"index",CUBE_TYPE_ARRAY,sizeof(void *),&index_list_desc,"flag_no"},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};


struct struct_elem_attr struct_index_desc[] =
{
	{"head",CUBE_TYPE_SUBSTRUCT,1,&uuid_head_desc,NULL},
	{"flag",CUBE_TYPE_INT,sizeof(int),NULL,NULL},
	{"uuid",CUBE_TYPE_UUID,DIGEST_SIZE,NULL,NULL},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};
struct struct_elem_attr namelist_attr_desc[] =
{
	{"uuid",CUBE_TYPE_UUID,DIGEST_SIZE,NULL},
	{"name",CUBE_TYPE_STRING,DIGEST_SIZE,NULL},
	{"num",CUBE_TYPE_INT,sizeof(int),NULL},
	{"namelist",CUBE_TYPE_DEFNAMELIST,sizeof(void *),"num"},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL}
};


struct struct_elem_attr struct_record_head_desc[] = 
{
	{"head",CUBE_TYPE_SUBSTRUCT,0,&uuid_head_desc},
	{"elem_no",CUBE_TYPE_INT,sizeof(int),NULL},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL}
};
*/

int _namelist_tail_func(void * memdb,void * record)
{
	DB_RECORD * db_record=record;
	struct struct_namelist * namelist=db_record->record;
	db_record->tail=namelist->elemlist;
	return 0;
}

static inline int _get_namelist_no(void * list)
{
	NAME2VALUE * namelist=list;
	int i=0;

	if(list==NULL)
		return -EINVAL;

	while(namelist[i].name!=NULL)
		i++;
	return i;
}

static inline int _get_value_namelist(char * name,void * list)
{
	struct struct_namelist * namelist=list;
	int i;
	for(i=0;i<namelist->elem_no;i++)
	{
		if(!strcmp(namelist->elemlist[i].name,name))
		{
			return namelist->elemlist[i].value;
		}
	}
	return 0;
}

void * _clone_namelist(void * list1)
{
	struct struct_namelist * namelist1 = list1;
	struct struct_namelist * newnamelist;
	int ret;
	int i;

	int elem_no = namelist1->elem_no;

	ret=Galloc0(&newnamelist,sizeof(struct struct_namelist));
	if(ret<0)
		return NULL;
	ret = Galloc0(&newnamelist->elemlist,sizeof(NAME2VALUE)*elem_no);
	if(ret<0)
		return NULL;
	newnamelist->elem_no=elem_no;
	for(i=0;i<elem_no;i++)
	{
		newnamelist->elemlist[i].value=namelist1->elemlist[i].value;
		newnamelist->elemlist[i].name=namelist1->elemlist[i].name;
	}
	return newnamelist;
}




void * _merge_namelist(void * list1, void * list2)
{
	struct struct_namelist * namelist1 = list1;
	struct struct_namelist * namelist2 = list2;
	struct struct_namelist * newnamelist;
	int ret;
	int elem_no;
	int i,j,k;
	NAME2VALUE  * buf;

	elem_no = namelist1->elem_no+namelist2->elem_no;

	ret = Galloc0(&buf,sizeof(NAME2VALUE)*elem_no);
	if(ret<0)
		return NULL;
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

	
	ret=Galloc0(&newnamelist,sizeof(struct struct_namelist));
	if(ret<0)
		return NULL;
	ret = Galloc0(&newnamelist->elemlist,sizeof(NAME2VALUE)*elem_no);
	if(ret<0)
		return NULL;
	newnamelist->elem_no=elem_no;
	for(i=0;i<elem_no;i++)
	{
		newnamelist->elemlist[i].value=buf[i].value;
		newnamelist->elemlist[i].name=buf[i].name;
	}

	Free0(buf);
	return newnamelist;
}

void * _struct_octet_to_attr(void * octet_array,int elem_no)
{
	int ret,i;
	struct elem_attr_octet * struct_desc_octet=octet_array; 	
	struct elem_attr_octet * elem_desc_octet; 
	struct struct_elem_attr * struct_desc;
	struct struct_elem_attr * elem_desc;
	DB_RECORD * child_desc_record;
	DB_RECORD * ref_namelist;

	ret=Galloc0(&struct_desc,sizeof(struct struct_elem_attr)*(elem_no+1));
	if(ret<0)
		return NULL;
	for(i=0;i<elem_no;i++)
	{
		elem_desc_octet=struct_desc_octet+i;
		elem_desc=struct_desc+i;

		// duplicate all the value except ref		

		elem_desc->name=elem_desc_octet->name;
		elem_desc->type=elem_desc_octet->type;
		elem_desc->size=elem_desc_octet->size;
		if(elem_desc_octet->def[0]==0)
			elem_desc->def=NULL;
		else
			elem_desc->def=elem_desc_octet->def;
		
		// if their is no valid ref
		if(_issubsetelem(elem_desc_octet->type))
		{
			
			child_desc_record=memdb_find(elem_desc_octet->ref,DB_STRUCT_DESC,0);
			if(child_desc_record==NULL)
				return NULL;
			elem_desc->ref=child_desc_record->tail;	
			if(elem_desc->ref==NULL)
				return NULL;
		}

		else if(_isnamelistelem(elem_desc_octet->type))
		{
			ref_namelist=memdb_find(elem_desc_octet->ref,DB_NAMELIST,0);
			if(ref_namelist==NULL)
				return NULL;
			elem_desc->ref=ref_namelist->tail;	
			if(elem_desc->ref==NULL)
				return NULL;
		}
	}
	return struct_desc; 
} 

int _struct_desc_tail_func(void * memdb,void * record)
{
	DB_RECORD * db_record=record;
	struct struct_desc_record * struct_desc_octet=db_record->record;

	db_record->tail=_struct_octet_to_attr(struct_desc_octet->elem_desc,struct_desc_octet->elem_no);
	if(db_record->tail==NULL)
		return -EINVAL;
	return 0;
}

int _typelist_tail_func(void * memdb,void * record)
{
	DB_RECORD * db_record=record;
	struct struct_typelist * typelist=db_record->record;
	DB_RECORD * temp_record;
	temp_record=memdb_find(DB_NAMELIST,0,typelist->uuid);
	if(temp_record==NULL)
		return NULL;

	db_record->tail=temp_record->record;
	if(db_record->tail==NULL)
		return -EINVAL;
	return 0;
}

int _subtypelist_tail_func(void * memdb,void * record)
{
	DB_RECORD * db_record=record;
	struct struct_subtypelist * subtypelist=db_record->record;
	DB_RECORD * temp_record;
	temp_record=memdb_find(DB_NAMELIST,0,subtypelist->uuid);
	if(temp_record==NULL)
		return NULL;

	db_record->tail=temp_record->record;
	if(db_record->tail==NULL)
		return -EINVAL;
	return 0;
}

int _recordtype_tail_func(void * memdb,void * record)
{
	DB_RECORD * db_record=record;
	struct struct_recordtype * recordtype = db_record->record;
	DB_RECORD * temp_record;
	void * struct_template;
	int i;
	char * index_elems=NULL;
	temp_record=memdb_find(DB_STRUCT_DESC,0,recordtype->uuid);
	if(temp_record==NULL)
		return -EINVAL;

	for(i=0;i<recordtype->flag_no;i++)
	{
		if(recordtype->index[i].flag==CUBE_ELEM_FLAG_INDEX)
		{
			index_elems=recordtype->index[i].elemlist;		
			break;
		}
	}
	
	temp_record=memdb_register_db(recordtype->type,recordtype->subtype,
		temp_record->tail,NULL,index_elems);
	if(temp_record==NULL)
		return -EINVAL;
	struct_template=memdb_get_template(recordtype->type,recordtype->subtype);
	if(struct_template==NULL)
		return -EINVAL; 

	for(i=0;i<recordtype->flag_no;i++)
	{
		if(recordtype->index[i].flag!=CUBE_ELEM_FLAG_INDEX)
		{
			struct_set_flag(struct_template,recordtype->index[i].flag,
				recordtype->index[i].elemlist);		
		}
	}
	db_record->tail=struct_template;
	return 0;
}

int memdb_register_db(int type,int subtype,void * struct_desc,void * tail_func,char * index_elems)
{
	struct memdb_desc * memdb;
	int ret;
	if(type<0)
		return -EINVAL;

	memdb=memdb_get_dblist(type,subtype);
	if(memdb!=NULL)
		return -EINVAL;

	if(type<DB_DTYPE_START)
	{
		ret=Galloc0(&static_db_list[type],sizeof(struct memdb_desc));
		if(ret<0)
			return -EINVAL;
		memdb=static_db_list[type];
	}
	else if(type==DB_DTYPE_START)
	{
		ret=Galloc0(&dynamic_db_list,sizeof(struct memdb_desc));
		if(ret<0)
			return -EINVAL;
		memdb=dynamic_db_list;
		
	}
	else
	{
		return -EINVAL;
	}
		
	memdb->record_db=init_hash_list(8,type,subtype);
	memdb->struct_template=create_struct_template(struct_desc);
	memdb->type=type;
	memdb->subtype=subtype;
	memdb->tail_func=tail_func;
	if(index_elems!=NULL)
	{
		ret=struct_set_flag(memdb->struct_template,CUBE_ELEM_FLAG_INDEX,index_elems);
		if(ret<0)
			return ret;
	}
	else
	{
		ret=struct_set_allflag(memdb->struct_template,CUBE_ELEM_FLAG_INDEX);
		if(ret<0)
			return ret;
	}
	return 0;
}

int memdb_get_elem_type(void * elem)
{
	UUID_HEAD * head=elem;
	if(head==NULL)
		return -EINVAL;
	return head->type;
}

int memdb_get_elem_subtype(void * elem)
{
	UUID_HEAD * head=elem;
	if(head==NULL)
		return -EINVAL;
	return head->subtype;
}

int memdb_is_dynamic(int type)
{
	if(type>=DB_DTYPE_START)
		return 1;
	return 0;
}

void * memdb_get_first(int type,int subtype)
{
	int ret;
	struct memdb_desc * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_get_first(db_list->record_db);
}

void * memdb_get_next(int type,int subtype)
{
	int ret;
	struct memdb_desc * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_get_next(db_list->record_db);
}


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

void * memdb_get_dblist(int type, int subtype)
{
	void * db_list;
	if(type<0) 
		return NULL;
	if(type< DB_DTYPE_START)
		db_list=static_db_list[type];
	else if(type == DB_DTYPE_START)
		db_list=dynamic_db_list;
	else if(type >DB_DTYPE_START)
		db_list=_get_dynamic_db_bytype(type,subtype);
	else
		return NULL;
	return db_list;
}

/*
void * _set_dynamic_db_bytype(int type,int subtype,void * record_def)
{
	int ret;
	DB_DESC * memdb;
	struct struct_recordtype * record_type=record_def; 
	void * db_list=init_hash_list(8,type,subtype);
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
*/
void * memdb_get_template(int type, int subtype)
{
	struct memdb_desc * db_list;
	db_list = memdb_get_dblist(type,subtype);
	if(db_list == NULL)
		return NULL;
	return db_list->struct_template;
	
}
/*
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
*/
int memdb_init()
{
	int ret;
	int i;
	void * struct_template; 
	void * base_struct_template;
	void * namelist_template;
	void * typelist_template;
	void * subtypelist_template;
	void * recordtype_template;
	void * index_template;
	struct struct_namelist  * baselist;
	struct struct_typelist  * basetypelist;
	struct struct_namelist * templist, *templist1;
	DB_RECORD *record;
	struct memdb_desc * curr_memdb;


	// alloc memspace for static database 
	ret=Galloc0(&static_db_list,sizeof(void *)*DB_DTYPE_START);
	if(ret<0)
		return ret;

	// generate two init namelist

	
	templist=Talloc(sizeof(struct struct_namelist));
	templist->elem_no=_get_namelist_no(&elem_type_valuelist_array);
	templist->elemlist=&elem_type_valuelist_array;

	
	templist1=Talloc(sizeof(struct struct_namelist));
	templist1->elem_no=_get_namelist_no(&memdb_elem_type_array);
	templist1->elemlist=&memdb_elem_type_array;

	ret=Galloc0(&elemenumlist,sizeof(struct struct_namelist));
	if(ret<0)
		return ret;

	elemenumlist=_merge_namelist(templist,templist1);


	ret=Galloc0(&typeenumlist,sizeof(struct struct_namelist));
	if(ret<0)
		return ret;

	templist->elem_no=_get_namelist_no(&struct_type_baselist);
	templist->elemlist=&struct_type_baselist;
	
	typeenumlist=_clone_namelist(templist);
	

	// register the new elem ops
	for(i=0;MemdbElemInfo[i].type!=CUBE_TYPE_ENDDATA;i++)
	{
		ret=struct_register_ops(MemdbElemInfo[i].type,
			MemdbElemInfo[i].enum_ops,
			MemdbElemInfo[i].flag,
			MemdbElemInfo[i].offset);
		if(ret<0)
			return ret;
	}
	

	// init those head template and elem template
	head_template= create_struct_template(&uuid_head_desc);
	if(head_template==NULL)
		return -EINVAL;

	elem_template= create_struct_template(&elem_attr_octet_desc);
	if(elem_template==NULL)
		return -EINVAL;

	// init the DB_NAMELIST lib
	ret=memdb_register_db(DB_NAMELIST,0,&struct_namelist_desc,&_namelist_tail_func,NULL);
	if(ret<0)
		return ret;

	// store the two init record of DB_NAMELIST
	record = memdb_store(elemenumlist,DB_NAMELIST,0,"elemenumlist");
	if(record==NULL)
		return -EINVAL;

	record = memdb_store(typeenumlist,DB_NAMELIST,0,"typeenumlist");
	if(record==NULL)
		return -EINVAL;

	// init the DB_STRUCT_DESC lib
	ret=memdb_register_db(DB_STRUCT_DESC,0,&struct_define_desc,&_struct_desc_tail_func,"elem_no,elem_desc.name,elem_desc.type,elem_desc.size,elem_desc.ref,elem_desc.def");
	if(ret<0)
		return ret;

	// init the DB_TYPELIST lib
	ret=memdb_register_db(DB_TYPELIST,0,&struct_typelist_desc,&_typelist_tail_func,NULL);
	if(ret<0)
		return ret;

	// init the DB_SUBTYPELIST lib
	ret=memdb_register_db(DB_SUBTYPELIST,0,&struct_subtypelist_desc,&_subtypelist_tail_func,NULL);
	if(ret<0)
		return ret;

	// init the DB_RECORDTYPE lib
	ret=memdb_register_db(DB_RECORDTYPE,0,&struct_recordtype_desc,&_recordtype_tail_func,NULL);
	if(ret<0)
		return ret;

	// store the init typelist

	dynamic_db_list=init_hash_list(8,DB_DTYPE_START,0);

	return 0;
}

void *  memdb_store(void * data,int type,int subtype,char * name)
{
	int ret;
	struct memdb_desc * db_list;
	UUID_HEAD * head;
	DB_RECORD * record;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	ret=Galloc0(&record,sizeof(DB_RECORD));
	if(ret<0)
		return -EINVAL;

	record->head.type=type;
	record->head.subtype=subtype;
	Strncpy(record->head.name,name,DIGEST_SIZE);
	record->record=data;

	if(db_list->tail_func!=NULL)
	{
		ret=db_list->tail_func(db_list,record);	
		if(ret<0)
			return NULL;
	}
	ret=memdb_comp_uuid(record);
	if(ret<0)
		return NULL;

	ret=hashlist_add_elem(db_list->record_db,record);
	if(ret<0)
		return NULL;

	return record;
}

int memdb_store_record(void * record)
{
	int ret;
	struct memdb_desc * db_list;
	UUID_HEAD * head;
	DB_RECORD * db_record=record;
	if(db_record==NULL)
		return -EINVAL; 
	if(db_record->record==NULL)
		return -EINVAL;	
	db_list=memdb_get_dblist(db_record->head.type,db_record->head.subtype);
	if(db_list==NULL)
		return -EINVAL;

	if(db_record->tail==NULL)
	{
		if(db_list->tail_func!=NULL)
		{
			ret=db_list->tail_func(db_list,db_record);	
			if(ret<0)
				return ret;
		}
	}
	ret=memdb_comp_uuid(db_record);
	if(ret<0)
		return ret;

	ret=hashlist_add_elem(db_list->record_db,db_record);
	if(ret<0)
		return ret;

	return 1;
}

void * memdb_find(void * data,int type,int subtype)
{
	int ret;
	struct memdb_desc * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_find_elem(db_list->record_db,data);
}
/*
void * memdb_find_byname(char * name,int type,int subtype)
{
	int ret;
	void * db_list;
	db_list=memdb_get_dblist(type,subtype);
	if(db_list==NULL)
		return NULL;
	return hashlist_find_elem_byname(db_list,name);
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
*/
void * memdb_get_subtypelist(int type)
{
	DB_RECORD * record;
	struct struct_subtypelist * subtypelist;	
	record=memdb_get_first(DB_SUBTYPELIST,0);
	subtypelist=record->record;

	while(subtypelist!=NULL)
	{
		if(subtypelist->type==type)
			return subtypelist;
		subtypelist=memdb_get_next(DB_SUBTYPELIST,0);
	}
	return NULL;
}

int memdb_print(void * data,char * json_str)
{
	DB_RECORD * record = data;
	void * struct_template ; 
	int ret;
	int offset;
	char * buf="\"record\":";

	struct_template=memdb_get_template(record->head.type,record->head.subtype);
	if(struct_template==NULL)
		return -EINVAL;	
	Strcpy(json_str,"{\"head\":");
	offset=Strlen(json_str);
	ret=struct_2_json(&record->head,json_str+offset,head_template);
	if(ret<0)
		return ret;
	offset+=ret;
	ret=Strlen(buf);
	Memcpy(json_str+offset,buf,ret);
	offset+=ret;
	ret=struct_2_json(record->record,json_str+offset,struct_template);
	if(ret<0)
		return ret;
	offset+=ret;
	json_str[offset++]='}';
	
	return offset;
}

/*
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
*/

int memdb_comp_uuid(void * record)
{
	BYTE * buf;	
	int blob_size;
	DB_RECORD * db_record=record;
	void * struct_template=memdb_get_template(db_record->head.type,db_record->head.subtype);
	if(struct_template == NULL)
		return -EINVAL;
	if(record==NULL)
		return -EINVAL;
	buf=Talloc(4000);
	if(buf==NULL)
		return -ENOMEM;
	
	blob_size=struct_2_part_blob(db_record->record,buf,struct_template,CUBE_ELEM_FLAG_INDEX);
	if(blob_size>0)
	{
		calculate_context_sm3(buf,blob_size,&db_record->head.uuid);
	}
	Free(buf);
	return blob_size;
}
