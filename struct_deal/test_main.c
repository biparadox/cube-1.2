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
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/json.h"
#include "../include/struct_deal.h"

struct connect_login
{
    char user[DIGEST_SIZE];
    char * passwd;
//    char nonce[DIGEST_SIZE];
} __attribute__((packed));


struct verify_login
{
	char uuid[DIGEST_SIZE];
	struct connect_login login_info;
	char nonce_len[4];
   	char *nonce;
	int  listnum;
	char *uuidlist;
	void * namelist;
} __attribute__((packed));

static struct struct_elem_attr connect_login_desc[]=
{
    {"user",CUBE_TYPE_STRING,DIGEST_SIZE,NULL,NULL},
    {"passwd",CUBE_TYPE_ESTRING,sizeof(char *),NULL,NULL},
    {NULL,CUBE_TYPE_ENDDATA,0,NULL}
};

static struct struct_elem_attr verify_login_desc[]=
{
    {"uuid",CUBE_TYPE_UUID,DIGEST_SIZE,NULL,NULL},
    {"login_info",CUBE_TYPE_SUBSTRUCT,0,&connect_login_desc,NULL},
    {"nonce_len",CUBE_TYPE_STRING,4,NULL},
    {"nonce",CUBE_TYPE_DEFINE,sizeof(char *),NULL,"nonce_len"},
    {"listnum",CUBE_TYPE_INT,DIGEST_SIZE,NULL,NULL},
    {"uuidlist",CUBE_TYPE_DEFUUIDARRAY,DIGEST_SIZE,NULL,"listnum"},
    {"namelist",CUBE_TYPE_DEFNAMELIST,DIGEST_SIZE,NULL,"listnum"},
    {NULL,CUBE_TYPE_ENDDATA,0,NULL,NULL}
};

int main() {

//	struct connect_login test_login={"HuJun","openstack"};
	struct verify_login test_login={"",{"HuJun","openstack"},"0x20","",4,"",		""};
	char buffer[512];
	char buffer1[512];
	char text[512];
	char text1[512];
	void * root;
//	char * json_string = "{\"login_info\":{\"user\":\"HuJun\","
//		"\"passwd\":\"openstack\"},\"nonce_len\":\"0x20\","
//		"\"nonce\":\"AAAAAAAABBBBBBBBCCCCCCCCDDDDEEFG\"}";
	int ret;
	struct verify_login * recover_struct;
	struct verify_login * recover_struct1;
	
	//char * namelist= "login_info.user,login_info.passwd";
//	char * namelist= "login_info";
	int flag;
	int i;

  	mem_init();
	test_login.nonce=Talloc(0x20);
	memset(test_login.nonce,'A',0x20);
	memset(test_login.uuid,'B',0x20);
	test_login.uuidlist=Talloc(DIGEST_SIZE*4);
	memset(test_login.uuidlist,'C',DIGEST_SIZE*4);
	test_login.namelist=Talloc(sizeof(NAME2VALUE)*test_login.listnum);
	NAME2VALUE * namelist=test_login.namelist;

	for(i=0;i<test_login.listnum;i++)
	{
		namelist[i].name=Talloc(10);
		sprintf(namelist[i].name,"name_%d",i);
	}
	namelist[0].value=0;
	namelist[1].value=1;
	namelist[2].value=3;
	namelist[3].value=4;

	struct_deal_init();

//	recover_struct=Calloc(sizeof(struct verify_login));
    	void * struct_template=create_struct_template(verify_login_desc);
	recover_struct=Talloc(struct_size(struct_template));
	recover_struct1=Talloc(struct_size(struct_template));
	ret=struct_2_blob(&test_login,buffer,struct_template);	
	printf("get %d size blob!\n",ret);

/*
	ret=struct_set_flag(struct_template,CUBE_ELEM_FLAG_TEMP,namelist);
	ret=struct_2_part_blob(&test_login,buffer1,struct_template,CUBE_ELEM_FLAG_TEMP);
	printf("get %d size blob!\n",ret);
*/
	ret=blob_2_struct(buffer,recover_struct,struct_template);
	printf("read %d size blob!\n",ret);
/*
	ret=blob_2_part_struct(buffer,recover_struct1,struct_template,CUBE_ELEM_FLAG_TEMP);
	printf("read %d size blob!\n",ret);
*/
	ret=struct_2_json(recover_struct,text,struct_template);
	printf("read %d size to json %s!\n",ret,text);
/*
	ret=struct_2_part_json(&test_login,text1,struct_template,CUBE_ELEM_FLAG_TEMP);
	printf("read %d size to json %s!\n",ret,text1);
*/
	ret=json_solve_str(&root,text);
	ret=json_2_struct(root,recover_struct1,struct_template);
	printf("read %d size to json!",ret);

	ret=struct_2_json(recover_struct1,text1,struct_template);
	printf("recover struct %d size to json %s!\n",ret,text1);

//	ret=struct_read_elem_text("login_info.passwd",&test_login,text,struct_template);
//	printf("read passwd %s from struct!\n",text);
/*	
	ret=struct_set_flag(struct_template,CUBE_ELEM_FLAG_TEMP,namelist);
	flag=struct_get_flag(struct_template,"login_info.passwd");
	
	memset(buffer,0,500);
	ret=struct_2_part_blob(&test_login,buffer,struct_template,CUBE_ELEM_FLAG_TEMP);
	ret=struct_2_part_json(&test_login,text,struct_template,CUBE_ELEM_FLAG_TEMP);
	printf("%s\n",text);

	ret=blob_2_struct(buffer,&test_login,struct_template);	
	ret=struct_2_json(&test_login,text,struct_template);
	printf("%s\n",text);
	ret=json_solve_str(&root,text);
	ret=json_2_struct(root,recover_struct,struct_template);
	ret=struct_2_json(recover_struct,text1,struct_template);
	printf("%s\n",text1);
//	ret=text_2_struct(text,&test_login,struct_template);
//	ret=blob_2_text(buffer,text,struct_template);	
//	ret=text_2_blob(buffer,text,struct_template);	
	ret=blob_2_struct(buffer,recover_struct,struct_template);
*/
	
	struct_free_alloc(recover_struct,struct_template);
    	free_struct_template(struct_template);
	return 0;
}
