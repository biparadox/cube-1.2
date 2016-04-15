/*************************************************
*       Hige security Linux Operating System Project
*
*	File description: 	Linux core database function model header file 
*	File name:		logic_baselib.h
*	date:    	2008-05-09
*	Author:    	Hu jun
*************************************************/

#ifndef _CUBE_MEMDB_H
#define _CUBE_MEMDB_H

//#include "../include/struct_deal.h"
//#include "../include/valuelist.h"

#define MAX_RECORD_NUM 65536

//NAME2VALUE * elem_type_valuelist;
//NAME2VALUE * elem_attr_flaglist;

//struct struct_elem_attr * elem_attr_desc; 

int memdb_init(void);
int read_json_desc(void * root, BYTE * uuid);

enum new_struct_elem_type
{
	CUBE_TYPE_ELEMTYPE=0x60,
	CUBE_TYPE_RECORDTYPE,
	CUBE_TYPE_RECORDSUBTYPE,
};

enum base_cube_db
{
	DB_INDEX=0x01,
	DB_NAMELIST,
	DB_STRUCT_DESC,
	DB_TYPELIST,
	DB_SUBTYPELIST,
	DB_CONVERTLIST,
	DB_RECORDTYPE,
	DB_BASEEND=0x10,
	DB_DTYPE_START=0x100,
};

typedef struct index_elem
{
	UUID_HEAD head;
	int flag;
	BYTE uuid[DIGEST_SIZE];
}INDEX_ELEM;

void * memdb_get_dblist(int type,int subtype);
int  memdb_init();

void * memdb_store(void * data,int type,int subtype,char * name);
void * memdb_get_first(int type,int subtype);
void * memdb_get_next(int type,int subtype);
void * memdb_remove(void * uuid,int type,int subtype);
int memdb_remove_byname(char * name,int type,int subtype);
int memdb_remove_record(void * record);
int memdb_free_record(void * record);

void * memdb_find(void * data,int type,int subtype);
void * memdb_find_byname(char * name,int type,int subtype);


int memdb_set_template(int type, int subtype, void * struct_template);
void * memdb_get_template(int type, int subtype);
int  memdb_set_index(int type,int subtype,int flag,char * elem_list);
int memdb_store_index(void * record,char * name,int flag);
INDEX_ELEM * memdb_find_index_byuuid(BYTE* uuid);

int memdb_get_elem_type(void * elem);
int memdb_get_elem_subtype(void * elem);
int memdb_is_elem_namelist(void * elem);

void * memdb_get_subtypelist(int type);
int  memdb_get_typeno(char * typestr);
int  memdb_get_subtypeno(int typeno,char * typestr);
void  * memdb_get_recordtype(int type,int subtype);
int memdb_print(void * data,char * json_str);

int memdb_read_desc(void * root,BYTE * uuid);

#endif

