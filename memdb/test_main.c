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
	int fd;
	int ret;
	void * root_node;
	BYTE uuid[DIGEST_SIZE];

	mem_init();
	struct_deal_init();
	memdb_init();

// test namelist reading start
	
	fd=open("typelist.json",O_RDONLY);
	if(fd<0)
		return fd;

	
	ret=read(fd,json_buffer,1024);
	if(ret<0)
		return -EIO;
	printf("%s\n",json_buffer);
	close(fd);

	ret=json_solve_str(&root_node,json_buffer);
	if(ret<0)
	{
		printf("solve json str error!\n");
		return ret;
	}


	ret=read_json_desc(root_node,uuid);

	void * findlist;
	findlist=memdb_find(uuid,DB_NAMELIST,0);
	void * memdb_template = memdb_get_template(DB_NAMELIST,0);
	ret=struct_2_json(findlist,json_buffer,memdb_template);
	if(ret<0)
		return -EINVAL;
	printf("%s\n",json_buffer);
//    test namelist reading finish

/*
//    test struct reading start
	fd=open("loginstruct.json",O_RDONLY);
	if(fd<0)
		return fd;

	
	ret=read(fd,json_buffer,1024);
	if(ret<0)
		return -EIO;
	printf("%s\n",json_buffer);
	close(fd);

	ret=json_solve_str(&root_node,json_buffer);
	if(ret<0)
	{
		printf("solve json str error!\n");
		return ret;
	}


	ret=read_json_desc(root_node,uuid);

	findlist=memdb_find_byname("login_verify",DB_STRUCT_DESC,0);
	if(findlist==NULL)
		return -EINVAL;
	ret=memdb_print_struct(findlist,json_buffer);
//	memdb_template = memdb_gettemplate(DB_STRUCT_DESC,0);
//	ret=struct_2_json(findlist,json_buffer,memdb_template);
	if(ret<0)
		return -EINVAL;
	printf("%s\n",json_buffer);

//     test struct reading finish

*/
	return 0;
}
