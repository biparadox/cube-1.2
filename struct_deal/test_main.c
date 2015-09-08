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
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/struct_deal.h"

struct connect_login
{
    char user[DIGEST_SIZE];
    char * passwd;
//    char nonce[DIGEST_SIZE];
} __attribute__((packed));


struct verify_login
{
	struct connect_login login_info;
	char nonce_len[4];
   	char *nonce;
} __attribute__((packed));

static struct struct_elem_attr connect_login_desc[]=
{
    {"user",OS210_TYPE_STRING,DIGEST_SIZE,NULL,0},
    {"passwd",OS210_TYPE_ESTRING,sizeof(char *),NULL,0},
    {NULL,OS210_TYPE_ENDDATA,0,NULL}
};

static struct struct_elem_attr verify_login_desc[]=
{
    {"login_info",OS210_TYPE_ORGCHAIN,0,&connect_login_desc,0},
    {"nonce_len",OS210_TYPE_STRING,4,NULL,0},
    {"nonce",OS210_TYPE_DEFINE,sizeof(char *),"nonce_len",0},
    {NULL,OS210_TYPE_ENDDATA,0,NULL,0}
};

int main() {

//	struct connect_login test_login={"HuJun","openstack"};
	struct verify_login test_login={{"HuJun","openstack"},"0x20","AAAAAAA"};
	char buffer[512];
	char text[512];
	int ret;
	struct verify_login * recover_struct;
	int stroffset=0;


  	Gmeminit();
  	Cmeminit();
	Tmeminit();
	struct_deal_init();

	recover_struct=Calloc(sizeof(struct verify_login));
    	void * struct_template=create_struct_template(verify_login_desc);
	ret=struct_2_blob(&test_login,buffer,struct_template);	
	ret=struct_read_elem("login_info.passwd",&test_login,text,struct_template);
	ret=blob_2_text(buffer,text,struct_template,&stroffset);
	printf("%s\n",text);
//	ret=blob_2_text(buffer,text,struct_template);	
//	ret=text_2_blob(buffer,text,struct_template);	
	ret=blob_2_struct(buffer,recover_struct,struct_template);
    	free_struct_template(struct_template);
	Cfree(recover_struct);
  	Tmemdestroy();
 	Gmemdestroy();
 	Cmemdestroy();
	return 0;
}
