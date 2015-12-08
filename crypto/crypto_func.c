#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
//#include "common.h"
#include "../include/data_type.h"
//#include "../list.h"
#include "sm3.h"
#include "sha1.h"
#include "../include/crypto_func.h"

//int file_to_hash(int argc, char *argv[])
int digest_to_uuid(BYTE *digest,char *uuid)
{
	int i,j,k,retval;
	unsigned char char_value;
	retval=DIGEST_SIZE;
	k=0;
	for(i=0;i<retval;i++)
	{
		int tempdata;
		char_value=digest[i];

		for(j=0;j<2;j++)
		{
			tempdata=char_value>>4;
			if(tempdata>9)
				*(uuid+k)=tempdata-10+'a';
			else
				*(uuid+k)=tempdata+'0';
			k++;
			if(j!=1)
				char_value<<=4;
				
		}
	}
	return 0;
}
#define PCR_SIZE 20
int extend_pcr_sm3digest(BYTE * pcr_value,BYTE * sm3digest)
{
	BYTE buffer[DIGEST_SIZE*2];
	BYTE digest[DIGEST_SIZE];
	memcpy(buffer,pcr_value,PCR_SIZE);
	memcpy(buffer+PCR_SIZE,sm3digest,DIGEST_SIZE);
	calculate_context_sha1(buffer,PCR_SIZE+DIGEST_SIZE,digest);
	memcpy(pcr_value,digest,PCR_SIZE);
	return 0;
}

int comp_proc_uuid(char * dev_uuid,char * name,char * conn_uuid)
{
	int len;
	int i;
	BYTE buffer[DIGEST_SIZE*4];
	BYTE digest[DIGEST_SIZE];
	memset(buffer,0,DIGEST_SIZE*4);
	len=strlen(dev_uuid);
	if(len<DIGEST_SIZE*2)
		memcpy(buffer,dev_uuid,len);
	else
		memcpy(buffer,dev_uuid,DIGEST_SIZE*2);
	if(name!=NULL)
	{
		len=strlen(name);
		if(len<DIGEST_SIZE*2)
		{
			memcpy(buffer+DIGEST_SIZE*2,name,len);
		}
		else 
		{
			memcpy(buffer+DIGEST_SIZE*2,name,DIGEST_SIZE*2);
		}
	}
	calculate_context_sm3(buffer,DIGEST_SIZE*4,digest);
	digest_to_uuid(digest,conn_uuid);
	return 0;
}
