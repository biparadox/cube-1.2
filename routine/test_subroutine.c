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
#include "../include/memdb.h"
#include "../include/message.h"
#include "../include/routine.h"

#include "routine_internal.h"


struct sub1_context
{
	int a;
	int b;	
};

struct sub2_context
{
	int a;
	int b;	
	int c;	
};

int sub1_init(void * proc,void * para)
{
	
	ROUTINE * this=(ROUTINE *)proc;
	int ret;
	
	struct sub1_context * context;
	
	ret=Galloc(&context,sizeof(struct sub1_context));
	if(ret<0)
		return -ENOMEM;
	this->context=context;
	context->a=1;
	context->b=1;
	return 0;
}
	

int sub1_start(void * proc,void * para)
{
	SUBROUTINE_INIT
	
	struct sub1_context * context= this->context;	

	context->a++;
	WAIT()
	context->b++;	
	return 0;	
}

int sub2_init(void * proc,void * para)
{
	
	ROUTINE * this=(ROUTINE *)proc;
	int ret;
	
	struct sub2_context * context;
	
	ret=Galloc(&context,sizeof(struct sub1_context));
	if(ret<0)
		return -ENOMEM;
	this->context=context;
	context->a=1;
	context->b=1;
	context->c=context->a+context->b;
	return 0;
}
	

int sub2_start(void * proc,void * para)
{
	SUBROUTINE_INIT
	
	struct sub1_context * context= this->context;	

	context->a++;
	WAIT()
	context->b++;	
	WAIT()
	context->c=context->a+context->b;
	return 0;	
}
