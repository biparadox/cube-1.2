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
#include "../include/dispatch.h"

#include "routine_internal.h"

static ROUTINE * dispatch_proc;

int _routine_dispatch_init()
{
	int ret;
	ret=Galloc0(&dispatch_proc,sizeof(ROUTINE));
	if(ret<0)
		return ret;	
	Strncpy(dispatch_proc->name,"dispatch_proc",DIGEST_SIZE);

	ret=comp_proc_uuid(myproc_context->uuid,dispatch_proc->name,dispatch_proc->uuid);
	if(ret<0)
		return -EINVAL;
	return 0;
}

int _routine_dispatch_recv_start()
{
	int ret;
	int count=0;
	void * policy;
	void * match_rule;
	void * route_rule;
	void * msg;
	void * message_list=_get_recv_message_list(NULL);

	do
	{
		ret=list_queue_get(message_list,&msg);
		if(ret<0)
			return ret;
		if(msg==NULL)
			break;
		ret=dispatch_policy_getfirst(&policy);
		if(ret<0)
			return -EINVAL;
		while(policy!=NULL)
		{
			ret=dispatch_match_message(policy,msg);
			if(ret>0)
			{
				ret=dispatch_policy_getfirstrouterule(policy,&route_rule);
//				if(route_rule!=NULL)
//				{
//					
//				}
			}
			ret=dispatch_policy_getnext(&policy);
		}
		
	}while(1);

	return count;
}

int _routine_dispatch_send_start()
{
	int ret;
	int count=0;
	void * policy;
	void * match_rule;
	void * route_rule;
	void * msg;
	void * message_list=_get_send_message_list(NULL);
	do
	{
		ret=list_queue_get(message_list,&msg);
		if(ret<0)
			return ret;
		if(msg==NULL)
			break;
		ret=dispatch_policy_getfirst(&policy);
		if(ret<0)
			return -EINVAL;
		while(policy!=NULL)
		{
			ret=dispatch_match_message(policy,msg);
			if(ret>0)
			{
				ret=dispatch_policy_getfirstrouterule(policy,&route_rule);
//				if(route_rule!=NULL)
//				{
//					
//				}
			}
			ret=dispatch_policy_getnext(&policy);
		}
		
	}while(1);

	return count;
}
