#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../include/data_type.h"
#include "../include/list.h"
#include "../include/string.h"
#include "../include/alloc.h"
#include "../include/json.h"
#include "../include/struct_deal.h"
#include "../include/basefunc.h"
#include "../include/memdb.h"
#include "../include/message.h"
#include "../include/routine.h"
#include "../include/channel.h"

#include "routine_internal.h"

static void * subroutine_list;
static void * channel_list;
static void * message_list;
static void (*sleep_func)(int);
static int sleep_para;

int routine_setuuid(void * proc)
{
	int ret;
	ROUTINE * routine=(ROUTINE *)proc;
	if(routine==NULL)
		return -EINVAL;
	ret=comp_proc_uuid(myproc_context->uuid,routine->name,routine->uuid);
	if(ret<0)
		return -EINVAL;
	return 0;
}

int channel_setuuid(void * channel)
{
	int ret;
	CHANNEL * my_channel=(CHANNEL *)channel;
	if(channel==NULL)
		return -EINVAL;
	ret=comp_proc_uuid(myproc_context->uuid,my_channel->name,my_channel->uuid);
	if(ret<0)
		return -EINVAL;
	return 0;
}

void * routine_register(char * name,int type,void * routine_ops, void * state_list)
{
	int ret;
	ROUTINE * new_routine;
	ret=Galloc0(&new_routine,sizeof(ROUTINE));
	if(ret<0)
		return NULL;
	
	Strncpy(new_routine->name,name,DIGEST_SIZE);
	routine_setuuid(new_routine);		
	new_routine->state=ROUTINE_INIT;
	new_routine->type=type;
	new_routine->ops=routine_ops;
	
	new_routine->state_list=state_list;
	hashlist_add_elem(subroutine_list,new_routine);
	return new_routine;

}

void * channel_register(char * name,int type)
{
	int ret;
	CHANNEL * new_channel;
	ret=Galloc0(&new_channel,sizeof(CHANNEL));
	if(ret<0)
		return NULL;
	
	new_channel=channel_create(name,type);
	if(new_channel==NULL)
		return NULL;
	channel_setuuid(new_channel);		
	
	hashlist_add_elem(channel_list,new_channel);
	return new_channel;
}

void * _subroutine_getfirst()
{
	return hashlist_get_first(subroutine_list);
}

void * _subroutine_getnext()
{
	return hashlist_get_next(subroutine_list);
}

void * _channel_getfirst()
{
	return hashlist_get_first(channel_list);
}

void * _channel_getnext()
{
	return hashlist_get_next(channel_list);
}

int routine_init(void * para)
{
	int ret;
	struct routine_para * routine_para=para;

	if(para!=NULL)
	{
		sleep_func=routine_para->sleep_func;
		sleep_para=routine_para->sleep_para;
	}
	else
	{
		sleep_func=NULL;
		sleep_para=0;
	}


	subroutine_list=init_hash_list(10,0,0);	
	channel_list=init_hash_list(8,0,0);	
	message_list=init_list_queue();	
	if(message_list==NULL)
		return -EINVAL;
	ret=Galloc0(&myproc_context,sizeof(struct proc_context));
	if(ret<0)
		return ret;
	ret=_routine_switch_init();
	return 0;	
}

void * routine_start(void * pointer)
{
	int ret;
	int count=0;
	do
	{
		ret=_routine_manage_start();
		if(ret<0)
			break;
		ret=_routine_switch_start();	
		if(ret<0)
			break;
		ret=_routine_dispatch_start();
		if(ret<0)
			break;
		ret=_routine_channel_start();
		if(ret<0)
			break;
		count++;
		if(sleep_func!=NULL)
			sleep_func(sleep_para);
	}while(ret>0);
	return count;

}

void * _get_message_list(void * routine)
{
	if(routine==NULL)
		return message_list;
	return 	((ROUTINE *)routine)->message_list;
}
