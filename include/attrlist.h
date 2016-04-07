#ifndef _ATTR_LIST_H
#define _ATTR_LIST_H       

#ifndef _OS210_DATA_TYPE_H
#include "data_type.h"
#endif
//#include "../include/extern_interface.h"

/*
#ifdef USER_MODE

#include "list.h"

#else
#include <linux/slab.h>
#endif
*/
typedef struct tagOs210_Record_List_type
{
	struct list_head list;
	void * record;
} __attribute__((packed)) Record_List;  


static __inline__ struct list_head * find_elem_in_list(
				struct list_head * head,
				int (*fn)(struct list_head * head1, struct list_head * head2),
				struct list_head * elem)
{
		struct list_head * temp;
		Record_List * ptr;
		if(elem==NULL){
			printk("---------------elem null\n");
			return NULL;
}
		if(head==NULL){
				
			printk("---------------head null\n");
			return NULL;	
			}	
		temp=head;
		do
		{
		ptr=list_entry(temp,Record_List,list);
			if(fn(temp,elem)==0)
			{
				return temp;	
			}
			temp=temp->next;
		}while(temp!=head);
		return NULL;
}

/* *************************************************************
 *  名称:  在队列中寻找一个值域与特定标记一致的元素
 *  描述:  在队列中寻找其元素值域与tag一致的元素
 *  输入:  head:  队列头
 *         fn:    比较函数,比较元素值域是否与tag一致,若一致则返回0值
 *         tag:   作为比较的标记
 *  输出:  无
 *  返回:  所找到的第一个元素,若未找到元素,返回空指针 
 * *************************************************************/
static __inline__ struct list_head * find_elem_with_tag(
	struct list_head * head,
	int (*fn)(struct list_head * head, void * tag),
	void * tag)
{
	
		struct list_head * temp;
//		Record_List * ptr;
//		DAC_POLICY * pp;
	if(tag==NULL){
		return NULL;}
	if(head==NULL){
		return NULL;}	
			
	temp=head;
	//int i=0;
		
	do
	{

		if(fn(temp,tag)==0)
		{
			return temp;	
		}
		temp=temp->next;

	/*	//any use?
		i++;
		printk("search time %d\n",i);
		if(i==4)
			break;
		*/
	}while(temp!=head);
	return NULL;
}

static __inline__ struct list_head * typefind_elem_with_tag(
	struct list_head * head,
	int (*fn)(int findtype,struct list_head * head, void * tag),
	int findtype,void * tag)
{
	
		struct list_head * temp;
//		Record_List * ptr;
//		DAC_POLICY * pp;
	if(tag==NULL){
		return NULL;}
	if(head==NULL){
		return NULL;}	
			
	temp=head;
	//int i=0;
		
	do
	{

		if(fn(findtype,temp,tag)==0)
		{
			return temp;	
		}
		temp=temp->next;

	/*	//any use?
		i++;
		printk("search time %d\n",i);
		if(i==4)
			break;
		*/
	}while(temp!=head);
	return NULL;
}
/* *************************************************************
 *  名称:  队列偏序极值元素查找 
/* *************************************************************
 *  名称:  队列偏序极值元素查找 
 *  描述:  在队列查找与特定记录成偏序关系的元素的最小上界,如带目
 *  	   录文件名中最接近该文件的一个
 *  输入:  head:  队列头
 *         fn:    比较函数,比较元素值域是否与tag成偏序关系,0为一致
 *          	  1为大于,2为小于,负值为无关系
 *         elem:  作为比较的元素
 *  输出:  无
 *  返回:  所找到的第一个元素上界,若未找到元素,返回空指针 
 * *************************************************************/
static __inline__ struct list_head * find_elem_minupper_inlist(
	struct list_head * head,
	int (*fn)(struct list_head * head,struct list_head * elem),
	struct list_head * elem)
{
		struct list_head * temp, *curr;
		int retval;
		
		if(elem == NULL)
			return NULL;
		if(head==NULL)
			return NULL;	
				
		temp=head;
		curr=NULL;
		//int i=0;
		
		do
		{
			retval = fn(temp,elem);
			if(retval ==0)
			{
				return temp;	
			}
			if(retval == 1)
			{
				if(curr ==NULL)
				{
					curr=temp;
				}
				else if(fn(temp,curr)==2)
				{
					curr=temp;
				}
			}
			if(temp == temp->next)
				break;
			temp=temp->next;
		}while(temp!=head);
		return curr;
}

/* *************************************************************
 *  名称:  删除整个元素队列
 *  描述:  遍历并删除队列中所有元素值
 *  输入:  head:  队列头
 *         fn:    删除函数,删除队列元素及其值域
 *  输出:  无
 *  返回:  无 
 * *************************************************************/
static __inline__ void destroy_list(struct list_head * head,
				void (*fn)(struct list_head * elem))
{
		struct list_head * temp, *temp1;
		if(head==NULL)
			return;
		
		temp=head->next;
		while(temp!=head)
		{
			temp1=temp;
			temp=temp->next;
			fn(temp1);
		}
		fn(head);
		return ;	
}



static __inline__ void destroy_record_list_elem (struct list_head * elem)
{
	Record_List * record_elem;
	record_elem = list_entry( elem, Record_List,list); 
//	kfree(record_elem->record);
//	kfree(record_elem);
}

static __inline__ void destroy_record_list_elem_only (struct list_head * elem)
{
	Record_List * record_elem;
	record_elem = list_entry( elem, Record_List,list); 
//	kfree(record_elem);
}
static void destroy_record_list (struct list_head * head)
{
	destroy_list(head,destroy_record_list_elem);
}
static void destroy_record_list_only (struct list_head * head)
{
	destroy_list(head,destroy_record_list_elem_only);
}

static __inline__ Record_List * get_record_from_list(Record_List * record,Record_List * Head)
{
	struct list_head * list, * head;
	head = &(Head->list);
	list = &(record->list);
	if(list == NULL)
		return NULL;
	if(list->next == head)
		return NULL;
	return list_entry(list->next,Record_List,list);	

}
static __inline__ int get_record_list_no(Record_List * root)
{
	struct list_head * list;
	int no=0;
	list=root->list.next;
	while(list!=&(root->list))
	{
		no++;
		list=list->next;
	}
	return no;
}
#endif
