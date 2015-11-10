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

enum base_struct_type
{
	TYPE_STRUCT_DESC=0x01,
	TYPE_STRUCT_NAMELIST=0x02,
	TYPE_BASE_END=0x10,
	TYPE_DB_LIST=0x100,
};


struct elem_attr_octet
{
	char * name;
	enum os210_struct_elem_type type;
	int size;     	
	char ref_uuid[DIGEST_SIZE];
};

struct namelist_struct
{
	BYTE uuid[DIGEST_SIZE];
	char name[DIGEST_SIZE];
	int num;
	void * namelist;
};

typedef struct index_elem
{
	BYTE uuid[DIGEST_SIZE];
	int type;
	int subtype;
	int flag;
	void * elem;
}INDEX_ELEM;

struct memdb_desc
{
	BYTE UUID[DIGEST_SIZE];
	int type;
	int subtype;
	void * struct_template;
};

struct struct_elem_attr namelist_attr_desc[] =
{
	{"uuid",OS210_TYPE_UUID,DIGEST_SIZE,NULL},
	{"name",OS210_TYPE_STRING,DIGEST_SIZE,NULL},
	{"num",OS210_TYPE_INT,sizeof(int),NULL},
	{"namelist",OS210_TYPE_DEFNAMELIST,sizeof(void *),"num"},
	{NULL,OS210_TYPE_ENDDATA,0,NULL}
};
struct struct_elem_attr uuid_head_desc[] =
{
	{"uuid",OS210_TYPE_UUID,DIGEST_SIZE,NULL},
	{"type",OS210_TYPE_INT,sizeof(int),NULL},
	{"subtype",OS210_TYPE_INT,sizeof(int),NULL},
	{"name",OS210_TYPE_STRING,DIGEST_SIZE,NULL},
	{NULL,OS210_TYPE_ENDDATA,0,NULL}
};

struct struct_elem_attr struct_record_head_desc[] = 
{
	{"head",OS210_TYPE_ORGCHAIN,0,&uuid_head_desc},
	{"elem_no",OS210_TYPE_INT,sizeof(int),NULL},
	{NULL,OS210_TYPE_ENDDATA,0,NULL}
};

static void ** static_db_list;

static void * dynamic_db_list;

struct struct_elem_attr elem_attr_octet_desc[] =
{
	{"name",OS210_TYPE_ESTRING,sizeof(char *),NULL},
	{"type",OS210_TYPE_ENUM,sizeof(int),&elem_type_valuelist_array},
	{"size",OS210_TYPE_INT,sizeof(int),NULL},
	{"ref",OS210_TYPE_UUID,DIGEST_SIZE,NULL},
	{NULL,OS210_TYPE_ENDDATA,0,NULL}
};

static void * base_struct_template;
static void * namelist_template;

struct struct_desc_record
{
	UUID_HEAD head;
	int elem_no;
	struct elem_attr_octet * elem_desc_list;
}__attribute__((packed));

typedef struct memdb_desc_record
{
	UUID_HEAD head;
	void * struct_template;	
}DB_DESC;

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
	ret=memdb_store(record,TYPE_STRUCT_DESC,0);
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
	void * struct_template; 
	void * pointer=&static_db_list;
	ret=Galloc(pointer,sizeof(void *)*TYPE_BASE_END);
	if(ret<0)
		return ret;
	memset(static_db_list,0,sizeof(void *)*TYPE_BASE_END);
	static_db_list[DB_STRUCT_DESC]=init_hash_list(8,DB_STRUCT_DESC,0);
	static_db_list[DB_NAMELIST]=init_hash_list(8,DB_NAMELIST,0);
	if(static_db_list[TYPE_STRUCT_DESC]==NULL)
	{
		return -EINVAL;
	}

	base_struct_template=create_struct_template(&elem_attr_octet_desc);
	namelist_template=create_struct_template(&namelist_attr_desc);
	struct_set_flag(namelist_template,OS210_ELEM_FLAG_KEY,"name,num,namelist");

	
	memdb_set_template(DB_STRUCT_DESC,0,base_struct_template);
	memdb_set_template(DB_NAMELIST,0,namelist_template);
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

int memdb_print_struct(void * data,char * json_str)
{
	struct struct_desc_record * struct_record=data;
	int stroffset=0;
	void * struct_template ; 
	int i, ret;
	struct elem_attr_octet * struct_desc; 	
	struct elem_attr_octet * elem_desc; 

	void * head_template = create_struct_template(&struct_record_head_desc);
	if(head_template==NULL)
		return -EINVAL;

	ret=struct_2_json(data,json_str,head_template);
	if(ret<0)
		return ret;
	free_struct_template(head_template);
	stroffset+=ret;

	json_str[stroffset-1]=',';
	json_str[stroffset++]='[';
	struct_template=memdb_get_template(DB_STRUCT_DESC,0);

	struct_desc=struct_record->elem_desc_list;
	elem_desc=struct_desc;

	for(i=0;i<struct_record->elem_no;i++)
	{
		ret=struct_2_json(elem_desc,json_str+stroffset,struct_template);
		if(ret<0)
			return ret;
		stroffset+=ret;
		json_str[stroffset++]=',';
		elem_desc++;
	}
	json_str[stroffset++]=']';
	json_str[stroffset++]='}';
	json_str[stroffset]=0;
	return stroffset;
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


int read_namelist_json_desc(void * root,BYTE * uuid)
{
	int ret;
	struct namelist_struct * namelist;
	int * temp_node;
	char buf[1024];
	void * namelist_template;

	ret=Galloc0(&namelist,sizeof(struct namelist_struct));
	if(ret<0)
		return ret;
	temp_node=json_find_elem("namelist",root);
	if(temp_node==NULL)
		return -EINVAL;
	namelist_template=memdb_get_template(DB_NAMELIST,0);
	if(namelist==NULL)
		return -EINVAL;

	ret=json_2_part_struct(root,namelist,namelist_template,OS210_ELEM_FLAG_KEY);
	namelist->num=json_get_elemno(temp_node);


	ret=struct_2_blob(namelist,buf,namelist_template);
	if(ret<0)
		return ret;
	ret=calculate_context_sm3(buf,ret,namelist->uuid);
	if(ret<0)
		return ret;	
	ret=memdb_store(namelist,DB_NAMELIST,0);
	if(ret<0)
		return ret;	
	memcpy(uuid,namelist->uuid,DIGEST_SIZE);
	
	return ret;	
}

int _read_struct_json(void * root,void ** record)
{
	int ret;
	void * root_node = root;
	void * father_node = root;
	void * curr_node = root;
	void * temp_node = NULL;
	void * struct_template ; 
	int i;
	int elem_no;
	int elem_size;
	struct struct_desc_record * struct_desc_record;
	BYTE struct_uuid[DIGEST_SIZE];

	struct elem_attr_octet * struct_desc; 	

	struct elem_attr_octet * elem_desc; 
	
	struct_template=memdb_get_template(DB_STRUCT_DESC,0);
	
	elem_no = json_get_elemno(father_node);
	
	ret=Galloc(&struct_desc,sizeof(struct elem_attr_octet)*(elem_no+1));
	if(ret<0)
		return ret;
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
			struct_desc[i].type=OS210_TYPE_ENDDATA;
				
			curr_node=father_node;
			ret=Galloc(&struct_desc_record,sizeof(struct struct_desc_record));
			if(ret<0)
				return ret;
			struct_desc_record->head.type=DB_STRUCT_DESC;		
			struct_desc_record->head.subtype=0;		
			struct_desc_record->elem_no=elem_no;		
			memset(struct_desc_record->head.uuid,0,DIGEST_SIZE);
			struct_desc_record->elem_desc_list=struct_desc;
			ret=_comp_struct_digest(struct_desc_record->head.uuid,struct_desc_record);
			if(ret<0)
				return ret;
			memdb_store(struct_desc_record,DB_STRUCT_DESC,0);
			if(curr_node==root_node)
				break;
			father_node=json_get_father(curr_node);
			struct_desc=json_node_get_pointer(curr_node);
			i=json_node_get_no(curr_node);
			memcpy(struct_desc[i].ref_uuid,struct_desc_record->head.uuid,DIGEST_SIZE);
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
		if(struct_desc[i].type == OS210_TYPE_ORGCHAIN)
		{
			struct_desc[i].size=DIGEST_SIZE;
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
				case OS210_TYPE_DEFINE:
				case OS210_TYPE_DEFSTR:
				case OS210_TYPE_DEFSTRARRAY:
					valuestr=json_get_valuestr(temp_node);
					if(ret<0)
						return ret;
					strncpy(struct_desc[i].ref_uuid,valuestr,DIGEST_SIZE);	
					break;
				case OS210_TYPE_ENUM:
				case OS210_TYPE_FLAG:
					break;
				default:
					return -EINVAL;
			}
		}
			
		curr_node=json_get_next_child(father_node);
		i++;

	}while(curr_node!=root_node);
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

	temp_node=json_find_elem("desc",root);
	if(temp_node==NULL)
		return -EINVAL;
	if(json_get_type(temp_node)!=JSON_ELEM_ARRAY)
		return -EINVAL;	

	ret = _read_struct_json(temp_node,&struct_desc_record);
	if(ret<0)
		return ret;
	
	memcpy(uuid,struct_desc_record->head.uuid,DIGEST_SIZE);
	temp_node=json_find_elem("name",root);
	if(temp_node!=NULL)
		strncpy(struct_desc_record->head.name,json_get_valuestr(temp_node),DIGEST_SIZE);
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

int read_record_json_desc(void * root,BYTE * uuid)
{

}

int read_memdb_json_desc(void * root,BYTE * uuid)
{
	int ret;
	// get the struct desc db
	void * struct_db = memdb_get_dblist(TYPE_STRUCT_DESC,0);
	void * root_node = root;
	void * father_node = root;
	void * curr_node = root;
	void * temp_node = NULL;
	void * struct_template ; 
	int i;
	int elem_no;
	int elem_size;
	struct struct_desc_record * struct_desc_record;
	BYTE struct_uuid[DIGEST_SIZE];

	struct elem_attr_octet * struct_desc; 	

	struct elem_attr_octet * elem_desc; 

	if(json_get_type(root_node)!= JSON_ELEM_MAP)
		return -EINVAL;

	father_node=json_find_elem("desc",root);
	if(father_node==NULL)
		return -EINVAL;
	if(json_get_type(father_node)!=JSON_ELEM_ARRAY)
		return -EINVAL;	

	root_node=father_node;
	struct_template=hashlist_get_desc(static_db_list[TYPE_STRUCT_DESC]);
	
	elem_no = json_get_elemno(father_node);
	
	ret=Galloc(&struct_desc,sizeof(struct elem_attr_octet)*(elem_no+1));
	if(ret<0)
		return ret;
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
			struct_desc[i].type=OS210_TYPE_ENDDATA;
				
			curr_node=father_node;
			father_node=json_get_father(curr_node);
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
			if(father_node==root_node)
				break;
			continue;
		}
		if(json_get_type(curr_node)!=JSON_ELEM_MAP)
			return -EINVAL;
		
		json_node_set_no(father_node,i);
		temp_node=json_find_elem("name",curr_node);
		if(temp_node==NULL)
			return -EINVAL;
		char value[128];
		ret=json_node_getvalue(temp_node,value,128);
		ret=struct_write_elem_text("name",&struct_desc[i],value,struct_template);
		if(ret<0)
			return -EINVAL;
		temp_node=json_find_elem("type",curr_node);
		if(temp_node==NULL)	
			return -EINVAL;
		// deal with type
		ret=json_node_getvalue(temp_node,value,DIGEST_SIZE*2);
		ret=struct_write_elem_text("type",&struct_desc[i],value,struct_template);
		if(struct_desc[i].type == OS210_TYPE_ORGCHAIN)
		{
			struct_desc[i].size=DIGEST_SIZE;
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
		ret=json_2_struct(curr_node,&struct_desc[i],struct_template);
		if(ret<0)
			return ret;
		elem_size=get_fixed_elemsize(struct_desc[i].type);
		// compute size
		if(elem_size>0)
			struct_desc[i].size=elem_size;
		else
		{
			temp_node=json_find_elem("size",curr_node);
			if(temp_node==NULL)
				return -EINVAL;
			ret=json_node_getvalue(temp_node,value,DIGEST_SIZE*2);
			if(ret<0)
				return ret;
			ret=struct_write_elem_text("size",&struct_desc[i],value,struct_template);
		}	
		// compute ref	
		memset(struct_desc[i].ref_uuid,0,DIGEST_SIZE);
		temp_node=json_find_elem("ref",curr_node);
		if(temp_node!=NULL)
		{
			switch(struct_desc[i].type)
			{
				case OS210_TYPE_DEFINE:
				case OS210_TYPE_DEFSTR:
				case OS210_TYPE_DEFSTRARRAY:
					ret=json_node_getvalue(temp_node,value,DIGEST_SIZE);
					if(ret<0)
						return ret;
					strncpy(struct_desc[i].ref_uuid,value,DIGEST_SIZE);	
					break;
				case OS210_TYPE_ENUM:
				case OS210_TYPE_FLAG:
					break;
				default:
					return -EINVAL;
			}
		}
			
		curr_node=json_get_next_child(father_node);
		i++;

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

int read_json_desc(void * root, BYTE * uuid)
{
	NAME2POINTER funclist[]=
	{
		{"namelist",&read_namelist_json_desc},
		{"struct",&read_struct_json_desc},
		{"record",&read_record_json_desc},
		{"memdb",&read_memdb_json_desc},
		{NULL,NULL},

	};	

	int (*read_json_func)(void * root, BYTE * uuid)=NULL;

	void * temp_node;
	int i;
	char * typestr;
	temp_node=json_find_elem("type",root);
	if(temp_node==NULL)
		return -EINVAL;
	typestr = json_get_valuestr(temp_node);
	if(typestr==NULL)
		return -EINVAL;

	while(funclist[i].name!=NULL)
	{
		if(strcmp(funclist[i].name,typestr)==0)
		{
			read_json_func=funclist[i].pointer;
			break;
		}
		i++;
	}		
	if(read_json_func==NULL)
		return -EINVAL;
	return read_json_func(root,uuid);
	
}


