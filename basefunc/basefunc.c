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
 * File: basefunc.c
 *
 * Created on: Jun 5, 2015
 * Author: Tianfu Ma (matianfu@gmail.com)
 */

#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/string.h"
#include "../include/list.h"
#include "../include/basefunc.h"
#include "attrlist.h"

const int db_order=10;
const int hash_num=1024;  // 2^db_order

const int subdb_order=8;
const int hash_subnum=256;

#if db_order>8
inline int get_hash_index(char * uuid)
{
	return uuid[0]<<(db_order-8)+uuid[1]>>(16-db_order);
}
#elif db_order<=8
inline int get_hash_index(char * uuid)
{
	return uuid[0]>>(8-db_order);
}
#endif

#if subdb_order>8
inline int get_hash_subindex(char * uuid)
{
	return uuid[0]<<(subdb_order-8)+uuid[1]>>(16-subdb_order);
}
#elif subdb_order<=8
inline int get_hash_subindex(char * uuid)
{
	return uuid[0]>>(8-subdb_order);
}
#endif

struct uuid_elem_desc
{
	char * name;
	void * template;
};

typedef struct taguuid_hashlist
{
	int hash_num;
	void * desc;
	Record_List * hash_table;
	int curr_index;
	struct list_head * curr_head;
}UUID_LIST;



void * init_hash_list(int order,int type,int subtype)
{
	int ret;
	int i;
	if(order<0)
		return -EINVAL;
	if(order>10)
		return -EINVAL;
	UUID_LIST * hash_head;
	hash_head=Calloc(sizeof(UUID_LIST));
	if(hash_head==NULL)
		return -ENOMEM;
	hash_head->hash_num=1<<order;
	hash_head->desc=NULL;
	hash_head->curr_index=0;
	ret=Galloc(&hash_head->hash_table,sizeof(Record_List)*hash_head->hash_num);
	if(ret<0)
		return -ENOMEM;
	for(i=0;i<hash_num;i++)
	{
		INIT_LIST_HEAD(&(hash_head->hash_table[i].list));
		hash_head->hash_table[i].record=NULL;
	}
	return hash_head;
}


int hashlist_add_elem(void * hashlist,void * elem)
{
	Record_List * new_record;
	UUID_LIST * uuid_list= (UUID_LIST *)hashlist;
	int hindex;
	new_record=Calloc(sizeof(Record_List));
	if(new_record==NULL)
		return -ENOMEM;
	INIT_LIST_HEAD(&(new_record->list));
	new_record->record=elem;

	if(uuid_list->hash_num==256)
		hindex=get_hash_subindex(elem);
	else if(uuid_list->hash_num==1024)
		hindex=get_hash_index(elem);
	else
		return -EINVAL;
	
	list_add_tail(&new_record->list,&uuid_list->hash_table[hindex].list);
	return 0;
}

static __inline__ int comp_uuid(void * src,void * desc)
{
	int i;
	unsigned int * src_array=(int *)src;
	unsigned int * desc_array = (int *)desc;

	for(i=0;i<DIGEST_SIZE/sizeof(int);i++)
	{
		if(src_array[i]>desc_array[i])
			return 1;
		if(src_array[i]<desc_array[i])
			return -1;
	}
	return 0;
}

Record_List * list_find_uuidelem(void * list,void * elem)
{
	Record_List * head = list;
	Record_List * curr = head->list.next;
	Record_List * temp;
	
	while(curr!=head)
	{
		if(comp_uuid(curr->record,elem)==0)
			return curr;
		curr= (Record_List *)curr->list.next;
	}
	return NULL;
}

Record_List * _hashlist_find_elem(void * hashlist,void * elem)
{
	Record_List * new_record;
	UUID_LIST * uuid_list= (UUID_LIST *)hashlist;
	int hindex;

	if(uuid_list->hash_num==1<<subdb_order)
		hindex=get_hash_subindex(elem);
	else if(uuid_list->hash_num==1<<db_order)
		hindex=get_hash_index(elem);
	else
		return NULL;
	return list_find_uuidelem(&uuid_list->hash_table[hindex],elem);
}

void * hashlist_find_elem(void * hashlist,void * elem)
{
	Record_List * curr_elem;
	curr_elem=_hashlist_find_elem(hashlist,elem);
	if(curr_elem==NULL)
		return NULL;
	return curr_elem->record;
} 

Record_List  * _hashlist_remove_elem(void * hashlist,void * elem)
{
	Record_List * curr_elem;
	curr_elem=_hashlist_find_elem(hashlist,elem);
	if(curr_elem==NULL)
		return NULL;
	list_del(curr_elem);
	return curr_elem;		
}

void * hashlist_remove_elem(void * hashlist,void * elem)
{
	Record_List * curr_elem;
	void * temp;
	curr_elem=_hashlist_remove_elem(hashlist,elem);
	if(curr_elem==NULL)
		return curr_elem;
	temp=curr_elem->record;
	Free(curr_elem);
	return temp;	
}

typedef struct tagpointer_stack
{
	void ** top;
	void ** curr; 
	int size;
}POINTER_STACK;


void * init_pointer_stack(int size)
{
	POINTER_STACK * stack;
	BYTE * buffer;
	buffer=Talloc(sizeof(POINTER_STACK)+sizeof(void *)*size);
	if(buffer==NULL)
		return -ENOMEM;
	stack=(POINTER_STACK *)buffer;
	stack->top=(void **)(buffer+sizeof(POINTER_STACK));
	stack->curr=stack->top;
	stack->size=size;
	return stack;
}
void free_pointer_stack(void * stack)
{	
	Free(stack);
	return;
}

int pointer_stack_push(void * pointer_stack,void * pointer)
{
	POINTER_STACK * stack;
	stack=(POINTER_STACK *)pointer_stack;
	if(stack->curr+1>=stack->top+stack->size)
		return -ENOSPC;
	*(stack->curr)=pointer;
	stack->curr++;
	return 0;
}


void * pointer_stack_pop(void * pointer_stack)
{
	POINTER_STACK * stack;
	stack=(POINTER_STACK *)pointer_stack;
	if(--stack->curr<stack->top)
		return -ERANGE;
	return *(stack->curr);
}

typedef struct tagpointer_queue
{
	void ** buffer;
	int size;
	int head;
	int tail;
}POINTER_QUEUE;

void * init_pointer_queue(int size)
{
	POINTER_QUEUE * queue;
	BYTE * buffer;
	buffer=Talloc(sizeof(POINTER_QUEUE)+sizeof(void *)*size);
	if(buffer==NULL)
		return -ENOMEM;
	queue=(POINTER_QUEUE *)buffer;
	memset(queue,0,sizeof(POINTER_QUEUE)+sizeof(void *)*size);
	queue->buffer=(void **)(buffer+sizeof(POINTER_QUEUE));
	queue->size=size;
	queue->head=-1;
	return queue;
}

void free_pointer_queue(void * queue)
{	
	Free(queue);
	return;
}

int pointer_queue_put(void * pointer_queue,void * pointer)
{
	POINTER_QUEUE * queue;
	queue=(POINTER_QUEUE *)pointer_queue;
	if(queue->head==-1)
		queue->head=0;
	else if(queue->head ==queue->size-1)
	{
		if(queue->tail==0)
			return -ENOSPC;
		queue->head=0;
	}
	else
	{
		if(queue->head+1==queue->tail)
			return -ENOSPC;
		queue->head++;
	}
	queue->buffer[queue->head]=pointer;
	return 0;
}


int pointer_queue_get(void * pointer_queue,void **pointer)
{
	POINTER_QUEUE * queue;
	queue=(POINTER_QUEUE *)pointer_queue;
	if(queue->head==-1)
		return -EINVAL;
	*pointer=queue->buffer[queue->tail];
	if(queue->tail==queue->head)
	{
		queue->head=-1;
		queue->tail=0;
		return 0;
	}
	if(queue->tail ==queue->size-1)
		queue->tail=0;
	else
		queue->tail++;
	return 0;
}
