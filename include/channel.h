/*************************************************
*       project:        973 trust demo, zhongan secure os 
*                       and trust standard verify
*	name:		message_struct.h
*	write date:    	2011-08-04
*	auther:    	Hu jun
*       content:        this file describe the module's extern struct 
*       changelog:       
*************************************************/
#ifndef _CUBE_CHANNEL_H
#define _CUBE_CHANNEL_H

#include "data_type.h"

enum channel_type
{
    CHANNEL_READ=0x01,
    CHANNEL_WRITE=0x02,   // finishing the msg's loading
    CHANNEL_RDWR=0x03,   // finishing the msg's loading
    CHANNEL_BIN=0x10,
    CHANNEL_JSON=0x20,
    CHANNEL_BLK=0x30,
    CHANNEL_SHUTDOWN=0x1000,   // finishing the msg's loading
};


#define CHANNEL_DIRECT_MASK  0x0F
#define CHANNEL_STREAM_MASK  0xF0

enum channel_state
{
    CHANNEL_INIT=0x01,
    CHANNEL_WORK,   // finishing the msg's loading
    CHANNEL_FULL,   // finishing the msg's loading
    CHANNEL_ERROR=0xffff,
};

typedef struct tube_channel
{
	BYTE uuid[DIGEST_SIZE];
	char name[DIGEST_SIZE];
	int  type;
	int  state;

	void * read_buf;
	void * write_buf;
	void * temp_msg;
	void * temp_node;

}CHANNEL;

void * channel_create(char * name,int type);
void * channel_register(char * name, int type);
int channel_write(void * channel,BYTE * data, int size);
int channel_read(void * channel,BYTE * data, int size);

int channel_inner_read(void * channel,BYTE * data,int size);
int channel_inner_write(void * channel,BYTE * data,int size);
void * channel_get(char * name);
void * channel_get_byuuid(BYTE * uuid);

#endif
