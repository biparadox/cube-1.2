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
