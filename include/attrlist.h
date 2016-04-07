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
 *  ����:  �ڶ�����Ѱ��һ��ֵ�����ض����һ�µ�Ԫ��
 *  ����:  �ڶ�����Ѱ����Ԫ��ֵ����tagһ�µ�Ԫ��
 *  ����:  head:  ����ͷ
 *         fn:    �ȽϺ���,�Ƚ�Ԫ��ֵ���Ƿ���tagһ��,��һ���򷵻�0ֵ
 *         tag:   ��Ϊ�Ƚϵı��
 *  ���:  ��
 *  ����:  ���ҵ��ĵ�һ��Ԫ��,��δ�ҵ�Ԫ��,���ؿ�ָ�� 
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
 *  ����:  ����ƫ��ֵԪ�ز��� 
/* *************************************************************
 *  ����:  ����ƫ��ֵԪ�ز��� 
 *  ����:  �ڶ��в������ض���¼��ƫ���ϵ��Ԫ�ص���С�Ͻ�,���Ŀ
 *  	   ¼�ļ�������ӽ����ļ���һ��
 *  ����:  head:  ����ͷ
 *         fn:    �ȽϺ���,�Ƚ�Ԫ��ֵ���Ƿ���tag��ƫ���ϵ,0Ϊһ��
 *          	  1Ϊ����,2ΪС��,��ֵΪ�޹�ϵ
 *         elem:  ��Ϊ�Ƚϵ�Ԫ��
 *  ���:  ��
 *  ����:  ���ҵ��ĵ�һ��Ԫ���Ͻ�,��δ�ҵ�Ԫ��,���ؿ�ָ�� 
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
 *  ����:  ɾ������Ԫ�ض���
 *  ����:  ������ɾ������������Ԫ��ֵ
 *  ����:  head:  ����ͷ
 *         fn:    ɾ������,ɾ������Ԫ�ؼ���ֵ��
 *  ���:  ��
 *  ����:  �� 
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
