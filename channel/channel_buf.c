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


typedef struct cube_channel_buf
{
	UINT16 bufsize;
	BYTE * buf;
	UINT16 last_read_tail;
	UINT16 last_write_tail;
}CHANNEL_BUF; 


void * channel_buf_create(int size)
{
	int ret;
	CHANNEL_BUF * new_buf;
	if(size<0)
		return NULL;
	if(size>32767)
		return NULL;

	ret=Galloc0(&new_buf,sizeof(CHANNEL_BUF));
	if(new_buf==NULL)
		return NULL;
	new_buf->bufsize=size;		
	ret=Galloc0(&new_buf->buf,size);
	if(new_buf->buf==NULL)
	{
		Free(new_buf);
		return NULL;	
	}
	return new_buf;
}

void channel_buf_free(void * buf)
{
	CHANNEL_BUF * old_buf=buf;
	Free(old_buf->buf);
	Free(old_buf);	
}

int channel_buf_read(void * buf,BYTE * data,int size)
{
	CHANNEL_BUF * curr_buf=buf;
	int left_data;
	int ret=0;
	if(curr_buf->last_read_tail==curr_buf->last_write_tail)
		return 0;

	if(curr_buf->last_read_tail<curr_buf->last_write_tail)
	{
		left_data=curr_buf->last_write_tail-curr_buf->last_read_tail;
		if(size<left_data)
		{
			Memcpy(data,curr_buf->buf+curr_buf->last_read_tail,size);
			curr_buf->last_read_tail+=size;		
			ret=size;
		}
		else
		{
			Memcpy(data,curr_buf->buf+curr_buf->last_read_tail,left_data);
			curr_buf->last_read_tail=curr_buf->last_write_tail;		
			ret=left_data;
		}

	}
	else
	{
		left_data=curr_buf->bufsize-curr_buf->last_read_tail+curr_buf->last_write_tail;
		int tail_block_size=curr_buf->bufsize-curr_buf->last_read_tail;
		if(size<tail_block_size)
		{
			Memcpy(data,curr_buf->buf+curr_buf->last_read_tail,size);
			curr_buf->last_read_tail+=size;		
			ret=size;
		}
		else
		{
			Memcpy(data,curr_buf->buf+curr_buf->last_read_tail,tail_block_size);
			if(size<left_data)
			{
				Memcpy(data+tail_block_size,curr_buf->buf,size-tail_block_size);
				curr_buf->last_read_tail=size-tail_block_size;
				ret=size;	

			}
			else
			{
				Memcpy(data+tail_block_size,curr_buf->buf,left_data-tail_block_size);
				curr_buf->last_read_tail=curr_buf->last_write_tail;		
				ret=left_data;	

			}
		}
	}
	return ret;	
}
 
int channel_buf_write(void * buf,BYTE * data,int size)
{
	CHANNEL_BUF * curr_buf=buf;
	int left_data;
	int ret=0;
	if(curr_buf->last_read_tail-1==curr_buf->last_write_tail)
		return 0;

	if(curr_buf->last_read_tail>curr_buf->last_write_tail)
	{
		left_data=curr_buf->last_read_tail-1-curr_buf->last_write_tail;
		if(size<left_data)
		{
			Memcpy(curr_buf->buf+curr_buf->last_write_tail,data,size);
			curr_buf->last_write_tail+=size;		
			ret=size;
		}
		else
		{
			Memcpy(curr_buf->buf+curr_buf->last_write_tail,data,left_data);
			curr_buf->last_write_tail=curr_buf->last_read_tail-1;		
			ret=left_data;
		}

	}
	else
	{
		left_data=curr_buf->bufsize-curr_buf->last_write_tail+curr_buf->last_read_tail-1;
		int tail_block_size=curr_buf->bufsize-curr_buf->last_write_tail;
		if(size<tail_block_size)
		{
			Memcpy(curr_buf->buf+curr_buf->last_write_tail,data,size);
			curr_buf->last_write_tail+=size;		
			ret=size;
		}
		else
		{
			Memcpy(curr_buf->buf+curr_buf->last_write_tail,data,tail_block_size);
			if(size<left_data)
			{
				Memcpy(curr_buf->buf,data+tail_block_size,size-tail_block_size);
				curr_buf->last_write_tail=size-tail_block_size;
				ret=size;	

			}
			else
			{
				Memcpy(curr_buf->buf,data+tail_block_size,left_data-tail_block_size);
				curr_buf->last_write_tail=curr_buf->last_read_tail-1;		
				ret=left_data;	

			}
		}
	}
	return ret;	
}
 
