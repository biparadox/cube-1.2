/*************************************************
*       project:        973 trust demo, zhongan secure os 
*                       and trust standard verify
*	name:		message_struct.h
*	write date:    	2011-08-04
*	auther:    	Hu jun
*       content:        this file describe the module's extern struct 
*       changelog:       
*************************************************/
#ifndef _CUBE_ROUTINE_INTERNAL_H
#define _CUBE_ROUTINE_INTERNAL_H

#include "../include/data_type.h"

struct proc_context
{
	BYTE uuid[DIGEST_SIZE];
	BYTE node_uuid[DIGEST_SIZE];
	char proc_name[DIGEST_SIZE];
	char node_name[DIGEST_SIZE];
};

struct proc_context * myproc_context;

int routine_setuuid(void * proc);


typedef struct routine_struct
{
	BYTE uuid[DIGEST_SIZE];
	char name[DIGEST_SIZE];
	enum routine_state state;
	enum routine_type type;
	struct routine_ops * ops;
	NAME2VALUE * state_list;

	void * entry;
	void * slot;
	void * system_data;
	void * protect_data;
	void * context;
	void * context_template;
}__attribute__((packed))  ROUTINE;

void * _subroutine_getfirst();
void * _subroutine_getnext();
void * subroutine_register(char * name,int type, void * routine_ops,void * state_list);

#define CONCAT01(a,b)	a##b
#define CONCAT02(a,b)	CONCAT01(a,b)
#define ROUTINE_RESUMED	4

#define WAIT()	\
CONCAT02(ENTRY,__LINE__):	\
do {				\
	if(this->entry== && CONCAT02(ENTRY,__LINE__))	\
	{						\
		this->entry=ROUTINE_RESUMED;		\
	}						\
	else						\
	{						\
		this->entry = &&CONCAT02(ENTRY,__LINE__);	\
		return this;				\
	}						\
}while(0);						\
	

#define SUBROUTINE_INIT			\
	ROUTINE * this = proc;		\
	if(this)			\
	{				\
		goto *this->entry;	\
	};				\
	
#endif
