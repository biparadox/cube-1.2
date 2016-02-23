/*************************************************
*       project:        973 trust demo, zhongan secure os 
*                       and trust standard verify
*	name:		message_struct.h
*	write date:    	2011-08-04
*	auther:    	Hu jun
*       content:        this file describe the module's extern struct 
*       changelog:       
*************************************************/
#ifndef _CUBE_TPM_H
#define _CUBE_TPM_H

enum TPM_command_code
{
    TPM_ORD_SHA1Start=1,  // just init a msg_box
};

typedef struct TPM_INCOMING_HEAD  //强制访问控制标记
{
   UINT16 tag;            // should be "MESG" to indicate that this information is a message's begin 
   UINT32 paramSize;          //  the message's version, now is 0x00010001
   enum TPM_command_code ordinal;
} __attribute__((packed)) TPM_IN_HEAD;
// if load message error, function return negtive value

#endif
