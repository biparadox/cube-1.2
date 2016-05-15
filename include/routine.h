/*************************************************
*       project:        973 trust demo, zhongan secure os 
*                       and trust standard verify
*	name:		message_struct.h
*	write date:    	2011-08-04
*	auther:    	Hu jun
*       content:        this file describe the module's extern struct 
*       changelog:       
*************************************************/
#ifndef _CUBE_ROUTINE_H
#define _CUBE_ROUTINE_H

#include "data_type.h"

enum routine_state
{
    ROUTINE_INIT=1,  // just init a msg_box
    ROUTINE_RUNNING,   // finishing the msg's loading
    ROUTINE_SLEEP,	//begin to add msg's record , at that time the message head can't be set
    ROUTINE_READY,	
    ROUTINE_STOP,	//begin to add msg's expand , at that time the message ohead can't be set  ,and we can't add record again
    ROUTINE_ZOMBIE,	//begin to add msg's expand , at that time the message ohead can't be set  ,and we can't add record again
    ROUTINE_ERROR=0xffff,
};


enum routine_type
{
	ROUTINE_SYSTEM=1,
	ROUTINE_SOURCE,
	ROUTINE_RESPONSE,
};

struct routine_ops
{
	struct struct_elem_attr * init_para_desc;	
	struct struct_elem_attr * start_para_desc;	

	int (*init)(void * proc,void * para);
	int (*start)(void * proc,void * para);
	int (*exit)(void * proc,void * para);

};

struct routine_para
{
	void (*sleep_func)(int);
	int  sleep_para; 	
};

int routine_init(void * para);
void * routine_start(void *);
void * routine_register(char * name,int type,void * ops,void * state_list);
int routine_unload();

#endif
