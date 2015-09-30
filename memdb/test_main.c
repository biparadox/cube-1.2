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

//	struct connect_login test_login={"HuJun","openstack"};
	char json_buffer[1024];
	int fd;
	int ret;
	void * root_node;

	mem_init();
	
	fd=open("login.json",O_RDONLY);
	if(fd<0)
		return fd;
	
	ret=read(fd,json_buffer,1024);
	if(ret<0)
		return -EIO;
	printf("%s\n",json_buffer);
	close(fd);

	memdb_init();

	json_solve_str(&root_node,json_buffer);
	
	return 0;
}
