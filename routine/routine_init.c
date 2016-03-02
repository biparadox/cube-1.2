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

#include "routine_internal.h"

static void * subroutine_list;
static void * channel_list;

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

void * subroutine_getfirst()
{
	return hashlist_get_first(subroutine_list);
}

void * subroutine_getnext()
{
	return hashlist_get_next(subroutine_list);
}

int routine_init()
{
	int ret;
	subroutine_list=init_hash_list(10,0,0);	
	channel_list=init_hash_list(8,0,0);	
	ret=Galloc0(&myproc_context,sizeof(struct proc_context));
	if(ret<0)
		return ret;
	return 0;	
}

