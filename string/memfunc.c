/**
 * Copyright [2015] Tianfu Ma (matianfu@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * File: buddy.c
 *
 * Created on: Jun 5, 2015
 * Author: Tianfu Ma (matianfu@gmail.com)
 */

#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/string.h"

const int maxnamelen=DIGEST_SIZE*8+1;
void * Memcpy(void * dest,void * src, unsigned int count)
{
	if(dest == src)
		return src;
	char * d=(char *)dest;
	char * s=(char *)src;
	while(count-->0)
		*d++=*s++;
	return dest;
}

void * Memset(void * s,int c, int n)
{
	const unsigned char uc=c;
	unsigned char * su;
	for(su=s;n>0;++su,--n)
		*su=uc;
	return s;
}

int Getfiledfromstr(char * name,char * str,char IFS,int maxsize)
{
	int offset=0;
	int i=0;
	int limit=maxsize;
	if(limit==0)
		limit=maxnamelen;
	while(str[i]==' ')
		i++;
	if(str[i]==0)
		return 0;
	
	for(;offset<limit;i++)
	{
		if((str[i]==IFS) || (str[i]==0))
		{
			name[offset]=0;
			return i;
		}
		name[offset++]=str[i];
	}
	name[0]=0;
	return i;
}

int Itoa(int n,char *str)
{
	int i=0,isNegative=0;
	int len=0;
	char temp;
	if((isNegative=n)<0)
		n=-n;
	do
	{
		str[len++]=n%10+'0';
		n=n/10;
	}while(n>0);
	if(isNegative<0)
		str[len++]='-';
	str[len]=0;
	for(i=0;i<len/2;i++)
	{
		temp=str[i];
		str[i]=str[len-1-i];
		str[len-1-i]=temp;	
	}
	return len;
}

static inline int _isdigit(char c)
{
	if((c>='0') && (c<='9'))
		return 1;
	return 0;
}
static inline int _get_char_value(char c)
{
	if(_isdigit(c))
		return c-'0';
	if((c>='a') && (c<='f'))
		return c-'a'+9;
	if((c>='A') && (c<='F'))
		return c-'a'+9;
	return -EINVAL;
}
int Atoi(char * string,int maxlen)
{
	int ret=0;
	int i;
	int base=10;
	int temp_value;
	int str_len;
	if(maxlen==0)
		str_len=strnlen(string,DIGEST_SIZE);
	else
		str_len=strnlen(string,maxlen);

	// process the head
	for(i=0;i<str_len;i++)
	{
		if(string[i]==0)
			break;
		if(string[i]==' ')
		{
			i++;
			continue;
		}
		// change the base
		if(string[i]=='0')
		{
			switch(string[i+1])
			{
				case 0:
					return 0;
				case 'b':
				case 'B':
					i+=2;
					base=2;
					break;
				case 'x':
				case 'X':
					i+=2;
					base=16;
					break;
				default:
					i++;
					base=8;
					break;

			}
			break;
		}
		break;
	}
	for(;i<str_len;i++)
	{
		if(string[i]==0)
			break;
		temp_value=_get_char_value(string[i]);
		if((temp_value <0)||(temp_value>=base))
			return -EINVAL;
		ret=ret*base+temp_value;		
	}
	return ret;
}
static inline int _get_lowest_bit(long long value)
{
	int i;
	int ret=0;
	int offset=sizeof(long long)*8/2;

	long long mask[6]=
	{
		0x00000000FFFFFFFF,
		0x0000FFFF0000FFFF,
		0x00FF00FF00FF00FF,
		0x0F0F0F0F0F0F0F0F,
		0x3333333333333333,
		0x7777777777777777,
	};
//	long long mask=-1;
	if(value==0)
		return 0;
	for(i=0;i<6;i++)
	{
		if(!(mask[i]&value))
		{
			ret+=offset;
		}
		else
		{
			value&=mask[i];
		}
		offset/=2;
	}
	return ret+1;	
}

int    Getlowestbit(BYTE  * addr,int size,int bit)
{
	long long test=0;
	int ret=0;
	int i;
	if(size<=0)
		return 0;
	if(size<=8)
	{
		memcpy(&test,addr,size);	
		if(bit)
			return _get_lowest_bit(test);	
		else
			return _get_lowest_bit(~test);
	}
	for(i=0;i<size;i+=8)
	{
		test=0;
		if(i+8>size)
			memcpy(&test,addr+i,size-i);
		else
			memcpy(&test,addr+i,8);
		if(bit)
		{
			if(test==0)
			{
				ret+=64;
				continue;
			}
			return ret+_get_lowest_bit(test);
		}
		else
		{
			if(test==-1)
			{
				ret+=64;
				continue;
			}
			return ret+_get_lowest_bit(~test);
		}
	}
	return 0;
} 
