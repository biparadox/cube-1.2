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
#include "../include/struct_deal.h"
#include "../include/memdb.h"



int main() {

	char json_buffer[4096];
	char print_buffer[4096];
	int fd;
	int ret;
	int readlen;
	int json_offset;
	void * root_node;
	void * findlist;
	void * memdb_template ;
	BYTE uuid[DIGEST_SIZE];

	mem_init();
	struct_deal_init();
	memdb_init();

// test namelist reading start

	memdb_template = memdb_get_template(DB_TYPELIST,0);
	
	fd=open("typelist.json",O_RDONLY);
	if(fd<0)
		return fd;

	
	readlen=read(fd,json_buffer,1024);
	if(readlen<0)
		return -EIO;
	printf("%s\n",json_buffer);
	close(fd);

	json_offset=0;
	ret=json_solve_str(&root_node,json_buffer);
	if(ret<0)
	{
		printf("solve json str error!\n");
		return ret;
	}
	json_offset+=ret;


	ret=read_json_desc(root_node,uuid);

	findlist=memdb_find(uuid,DB_TYPELIST,0);
	if(findlist!=NULL)
	{

		ret=struct_2_json(findlist,print_buffer,memdb_template);
		if(ret<0)
			return -EINVAL;
		printf("%s\n",print_buffer);
	}

	findlist=memdb_find_byname("baselist",DB_TYPELIST,0);
	ret=struct_2_json(findlist,print_buffer,memdb_template);
	if(ret<0)
		return -EINVAL;
	printf("%s\n",print_buffer);

	ret=json_solve_str(&root_node,json_buffer+json_offset);
	if(ret<0)
	{
		printf("solve json str error!\n");
		return ret;
	}
	json_offset+=ret;
	ret=read_json_desc(root_node,uuid);
	int msg_type = memdb_get_typeno("MESSAGE");
	if(msg_type<=0)
		return -EINVAL;
	findlist=memdb_get_subtypelist(msg_type);

	if(findlist!=NULL)
	{
		memdb_template = memdb_get_template(DB_SUBTYPELIST,0);
		ret=struct_2_json(findlist,print_buffer,memdb_template);
		if(ret<0)
			return -EINVAL;
		printf("%s\n",print_buffer);
	}
	return 0;
}
