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
#include "../include/data_type.h"
#include "../include/string.h"

int main() {


//	struct connect_login test_login={"HuJun","openstack"};
	char * src=" hello,  world!";
	int i;
	char buffer[20];
	Memset(buffer,0,20);
	Memcpy(buffer,src,9);
	printf("\n%s\n",buffer);
	i=Getfiledfromstr(buffer,src,',',0);
	printf("\n%s\n %c",buffer,src[i]);
	i=Getfiledfromstr(buffer,src+i+1,',',0);
	printf("\n%s\n",buffer);
	i=Itoa(-216342,buffer);
	printf("\n%s\n",buffer);
	return 0;
}
