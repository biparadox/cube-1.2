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

static ROUTINE * channel_proc;

int _routine_channel_init()
{
	int ret;
	ret=Galloc0(&channel_proc,sizeof(ROUTINE));
	if(ret<0)
		return ret;	
	Strncpy(channel_proc->name,"channel_proc",DIGEST_SIZE);

	ret=comp_proc_uuid(myproc_context->uuid,channel_proc->name,channel_proc->uuid);
	if(ret<0)
		return -EINVAL;
	return 0;
}

int _routine_channel_start()
{
	int ret;
	int count=0;

	return count;
}
