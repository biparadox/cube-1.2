#ifndef _MEMDB_INTERNAL_H
#define _MEMDB_INTERNAL_H

typedef struct db_record
{
	UUID_HEAD head;
	int name_no;
	char ** names;
	void * record;
	void * tail;
}DB_RECORD;

struct elem_attr_octet
{
	char * name;
	enum cube_struct_elem_type type;
	int size;     	
	char ref[DIGEST_SIZE];
	char ref_name[DIGEST_SIZE];
	char def[DIGEST_SIZE];
};


struct memdb_desc
{
	BYTE uuid[DIGEST_SIZE];
	int type;
	int subtype;
	void * struct_template;
	int (* tail_func)(void * memdb,void * record);
	void * record_db;
};


struct memdb_desc ** static_db_list;
struct memdb_desc * dynamic_db_list;


void * elem_template;
extern void * head_template;
void * index_template;

// static type define

struct struct_desc_record
{
	int elem_no;
	struct elem_attr_octet * elem_desc;
}__attribute__((packed));

struct struct_namelist
{
	int elem_no;
	NAME2VALUE * elemlist;
}__attribute__((packed));

struct struct_typelist
{
	int elem_no;
	BYTE uuid[DIGEST_SIZE];
}__attribute__((packed));

struct struct_subtypelist
{
	int type;
	int elem_no;
	BYTE uuid[DIGEST_SIZE];
}__attribute__((packed));


struct flag_index
{
	int flag;
	void * elemlist;
}__attribute__((packed));

struct struct_recordtype
{
	int type;
	int subtype;
	BYTE uuid[DIGEST_SIZE];
	int flag_no;
	struct flag_index * index;
}__attribute__((packed));


// the 2 init namelist

struct struct_namelist *elemenumlist;
struct struct_namelist *typeenumlist;


typedef struct memdb_desc_record
{
	UUID_HEAD head;
	void * struct_template;	
}DB_DESC;

int memdb_get_elem_type(void * elem);

int memdb_get_elem_subtype(void * elem);

int memdb_is_dynamic(int type);

void * _merge_namelist(void * list1, void * list2);
#endif
