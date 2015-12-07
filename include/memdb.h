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

NAME2VALUE * elem_type_valuelist;
NAME2VALUE * elem_attr_flaglist;

struct struct_elem_attr * elem_attr_desc; 

int memdb_init();
int read_json_desc(void * root, BYTE * uuid);
int memdb_store(void * record, int type,int subtype);

enum base_cube_db
{
	DB_INDEX=0x01,
	DB_STRUCT_DESC,
	DB_NAMELIST,
	DB_TYPELIST,
	DB_SUBTYPELIST,
	DB_CONVERTLIST,
};

typedef struct index_elem
{
	UUID_HEAD head;
	int flag;
	BYTE uuid[DIGEST_SIZE];
}INDEX_ELEM;

int memdb_set_template(int type, int subtype, void * struct_template);
void * memdb_get_template(int type, int subtype);
int  memdb_set_index(int type,int subtype,int flag,char * elem_list);
int memdb_store_index(void * record,char * name,int flag);
int memdb_get_elem_type(void * elem);
int memdb_get_elem_subtype(void * elem);
int memdb_is_elem_namelist(void * elem);


int memdb_get_typeno(char * typestr);
int memdb_get_subtypeno(int typeno,char * subtypestr);

/*
typedef struct tagPolicyHead{
   	 BYTE NodeSequence[20];      
   	 BYTE UserName[40];            
   	 BYTE PolicyType[4];              
   	 BYTE PolicyVersion[8];        
   	 UINT32  RecordNum;	       
   	 UINT32 Reserved;   		
}__attribute__((packed)) POLICY_HEAD;
typedef struct trust_policy_ops
{
	char name[5];
	void * (*initlib)(void * lib);
//	void * (*gethandle)(char * policytype);
	void * (*find)(void * lib,void * tag);
	void * (*typefind)(int findtype,void * lib,void * tag);
	void * (*gettag)(void * lib,void * policy);
	void * (*insert)(void * lib,void * policy);
	void * (*modify)(void * lib,void * policy,char * name,void * newvalue);
	void * (*remove)(void * lib,void * tag);
	void * (*getfirst)(void * lib);
	void * (*getnext)(void * lib);
	int (*comp)(void * policy1,void * policy2);
	int (*comptag)(void * policy,void * policy_tag);
	int (*typecomptag)(int type,void * policy,void * policy_tag);
	int (*hashfunc)(void * policy);
	void (*destroyelem)(void * lib,void * policy);
	void (*destroylib)(void * lib);
}TPLIB_OPS;

typedef struct tagpolicy_lib
{
	char policy_type[5];
	void * struct_template;
	struct trust_policy_ops * policy_ops;
	void * handle;
	void * curr_record;
}POLICY_LIB;

int logic_baselib_init(void);
void * find_policy_lib(char * policy_type);
void * logic_get_policy_struct_template(char * policy_type);
int register_policy_lib(char * policy_type,struct trust_policy_ops * policy_ops);

extern struct trust_policy_ops sublabel_policy_ops; 
extern struct trust_policy_ops objlabel_policy_ops; 
extern struct trust_policy_ops authuser_policy_ops; 
extern struct trust_policy_ops dac_policy_ops; 
extern struct trust_policy_ops audit_policy_ops; 


void * find_record_type(char * record_type);
int register_record_type(char * type,struct struct_elem_attr * desc);
void * load_record_desc(char  * type);
void * load_record_template(char  * type);
void * load_record_ops(char  * type);

void * general_initlib(void * lib);
void * general_find(void * lib, void * tag);
void * general_typefind(int findtype,void * lib, void * tag);
void * general_insert(void * lib,void * policy);
int general_modify(void * lib,void * policy,char * name,void * newvalue);
void * general_remove(void * lib,void * tag);
void * general_getfirst(void * lib);
void * general_getnext(void * lib);
void * general_destroyelem(void * lib,void * policy);
void * general_destroylib(void * lib);
void * entity_get_uuid(void * lib,void * policy);
int  entity_comp_uuid(void * head, void * uuid);
extern struct trust_policy_ops general_lib_ops;
struct trust_policy_ops *  get_entity_lib_ops(char * policy_type);
*/
#endif

