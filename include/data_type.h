/*************************************************
*       project:        973 trust demo, zhongan secure os 
*                       and trust standard verify
*	name:		data_type.h
*	write date:    	2011-08-04
*	auther:    	Hu jun
*       content:        this file describe some basic data type 
*       changelog:       
*************************************************/
#ifndef _OS210_DATA_TYPE_H
#define _OS210_DATA_TYPE_H
#define NULL 0
//#ifndef TSS_PLATFORM_H
typedef unsigned char         BYTE;
typedef unsigned short int  UINT16;
typedef unsigned int        UINT32;
typedef unsigned long int   UINT64;
//#endif

typedef unsigned short int    WORD;
typedef unsigned int         DWORD;


#define BITSTRING (unsigned char *)
#define OS210_DEBUG  

#define DIGEST_SIZE	32

typedef struct tagVar_String
{
    UINT32 length;   //´®³¤¶È
    BYTE *  String;	  //´®ÄÚÈÝ
} __attribute__((packed)) V_String;  

#define IS_ERR(ptr) (ptr-4096 <0)

#ifdef OS210_DEBUG
#define os210_dbg(format, arg...)		\
	printk(KERN_DEBUG, format , ## arg)
#else
#define os210_dbg(format, arg...)  do { } while (0)
#endif
#endif
