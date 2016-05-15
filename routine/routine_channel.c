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
	int bufsize;
	int offset=0;

	BYTE buffer[PAGE_SIZE];	
	
	CHANNEL * channel;
	void * message_list;
	void * new_msg;

	message_list=_get_message_list(NULL);
	if(message_list==NULL)
		return -EINVAL;

	channel=_channel_getfirst();

	while(channel!=NULL)
	{
		count++;
		bufsize=channel_inner_read(channel,buffer,PAGE_SIZE);
		if(bufsize<0)
			return bufsize;
		if(bufsize>0)
		{
			
			switch(channel->type&CHANNEL_STREAM_MASK)
			{
				// bin message format is the default format
				case 0x00:		
				case 0x10:
					break;
				case 0x20:
					break;
				case 0x30:

					do {
						ret=json_2_message(buffer+offset,&new_msg);
						if(ret<=0)
							break;
						if(new_msg!=NULL)
							list_queue_put(message_list,new_msg);
						if(bufsize-offset-ret<10)
							break;
						offset+=ret;
					}while(1);

					break;
				default:
					break;
			}
		}
		channel=_channel_getnext();
	}
	
	return count;
}
