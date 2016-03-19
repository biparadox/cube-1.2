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
 * File: main.c
 *
 * Created on: Jun 5, 2015
 * Author: Tianfu Ma (matianfu@gmail.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/basefunc.h"
#include "../include/struct_deal.h"
#include "../include/channel.h"

int main() {

	void * channel;
	char inner_buffer[8192];
	char extern_buffer[8192];
	int i;
	char c;
	int ret;
	int ex_offset=0;
	int in_offset=0;

	mem_init();
	struct_deal_init();

	channel=channel_create("test_channel",CHANNEL_RDWR);
	if(channel==NULL)
		return -EINVAL;

	c='0';
	for(i=0;i<4096;i++)
	{
		extern_buffer[i]=c;
		c++;
		if(c>'z')
			c='0';		
	}	
	
	int size=200;
	ret=channel_write(channel,extern_buffer+ex_offset,size);
	printf("channel write %d, return %d!\n",size,ret);
	ex_offset+=ret;

	size=300;
	ret=channel_inner_read(channel,inner_buffer+in_offset,size);
	printf("channel read %d, return %d!\n",size,ret);
	in_offset+=ret;



	return 0;
}
