#ifdef KERNEL_MODE

#include <linux/string.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>

#else

#include<string.h>
#include<errno.h>
//#include "../include/kernel_comp.h"
#include "../include/list.h"
#endif

#include "../include/data_type.h"
#include "../include/struct_deal.h"
#include "../include/alloc.h"
#include "../include/attrlist.h"
#include "struct_ops.h"
#include "struct_internal.h"
#define DIGEST_SIZE 32


static inline int IsValueEnd(char c)
{
    if((c==' ')||(c==',')||(c=='\0')||(c=='}')||(c==']'))
    {
        return 1;
    }
    return 0;
}
static inline int IsSplitChar(char c)
{
    if((c==',')||(c==' ')||(c=='\r')||(c=='\n')||(c=='\t'))
    {
        return 1;
    }
    return 0;
}

void * get_first_json_child(void * father)
{
    JSON_NODE * father_node = (JSON_NODE *)father;
    father_node->curr_child =(Record_List *) (father_node->childlist.list.next);
    if(father_node->curr_child == &(father_node->childlist.list))
        return NULL;
    return father_node->curr_child->record;
}

void * get_next_json_child(void * father)
{
    JSON_NODE * father_node = (JSON_NODE *)father;
    if(father_node->curr_child == &(father_node->childlist.list))
        return NULL;
    father_node->curr_child = father_node->curr_child->list.next;
    return father_node->curr_child->record;
}

void * get_json_father(void * child)
{
    if(child==NULL)
	return NULL;
    JSON_NODE * child_node = (JSON_NODE *)child;
    return child_node->father;
}

int json_get_type(void * node)
{
    if(node==NULL)
		return -EINVAL;
    JSON_NODE * json_node = (JSON_NODE *)node;
    return json_node->elem_type;
}

char * json_get_valuestr(void * node)
{
    if(node==NULL)
		return -EINVAL;
    JSON_NODE * json_node = (JSON_NODE *)node;
    return json_node->value_str;
}

int json_get_elemno(void * node)
{
    if(node==NULL)
		return -EINVAL;
    JSON_NODE * json_node = (JSON_NODE *)node;
    return json_node->elem_no;
}

Record_List * get_new_Record_List(void * record)
{
    Record_List * newrecord = Calloc(sizeof(Record_List));
    if(newrecord == NULL)
        return NULL;
    INIT_LIST_HEAD (&(newrecord->list));
    newrecord->record=record;
    return newrecord;
}

static inline int get_json_numvalue(char * valuestr,char * json_str)
{
    int i;
    int point=0;
     if(json_str[0]!='.')
    {
        if((json_str[0]<'0')||(json_str[0]>'9'))
            return -EINVAL;
    }
    for(i=0;i<1024;i++)
    {
        if(json_str[i]==0)
            return -EINVAL;
        if(IsValueEnd(json_str[i]))
            break;
        valuestr[i]=json_str[i];
    }
    if(i==0)
        return -EINVAL;
    if(i==1024)
        return -EINVAL;
    valuestr[i]=0;
    return i;
}

void * find_json_elem(char * name,void * root)
{
	JSON_NODE * root_node = (JSON_NODE * )root;
	JSON_NODE * this_node ;

	if(root_node->elem_type!=JSON_ELEM_MAP)
		return NULL;
        this_node = (JSON_NODE *)get_first_json_child(root);
	
	while(this_node != NULL)
	{
		if(strncmp(name,this_node->name,DIGEST_SIZE*2)==0)
			break;
		this_node=(JSON_NODE *)get_next_json_child(root);
	}
	return this_node;

}

static inline int get_json_boolvalue(char * valuestr,char * json_str)
{
   int i;
   if((json_str[0]!='b')||(json_str[0]!='B')
            ||(json_str[0]!='f')||(json_str[0]!='F'))
        return -EINVAL;
   for(i=0;i<6;i++)
   {
       if(json_str[i]==0)
           return -EINVAL;
       if(IsValueEnd(json_str[i]))
           break;
       valuestr[i]=json_str[i];
   }
   if(i==0)
        return -EINVAL;
   if(i==6)
        return -EINVAL;
   valuestr[i]=0;
   return i;
}

static inline int get_json_nullvalue(char * valuestr,char * json_str)
{
    int i;
    if((json_str[0]!='n')||(json_str[0]!='N'))
        return -EINVAL;
   for(i=0;i<6;i++)
   {
       if(json_str[i]==0)
           return -EINVAL;
       if(IsValueEnd(json_str[i]))
           break;
       valuestr[i]=json_str[i];
   }
   if(i==0)
        return -EINVAL;
   if(i==6)
        return -EINVAL;
   valuestr[i]=0;
   return i;

}

static inline int get_json_strvalue(char * valuestr,char * json_str)
{
    int i;
    int offset=0;
    for(i=0;i<1024;i++)
    {
        if(json_str[i]=='\"')
            break;
        if(json_str[i]==0)
            return -EINVAL;
	if(json_str[i]=='\\')
	    valuestr[offset]=json_str[++i];
	else
            valuestr[offset]=json_str[i];
	offset++;
    }
    if(i==1024)
        return -EINVAL;
    valuestr[offset]=0;
    return i;
}

void * get_json_node(void * father)
{
    JSON_NODE * newnode;
    JSON_NODE * father_node=(JSON_NODE *)father;
    newnode=Calloc(sizeof(JSON_NODE));
    if(newnode==NULL)
        return NULL;
    memset(newnode,0,sizeof(JSON_NODE));
    INIT_LIST_HEAD(&(newnode->childlist.list));
    newnode->father=father;
    if(father!=NULL)
        newnode->layer=father_node->layer+1;
    Record_List * newrecord = get_new_Record_List(newnode);
    if(newrecord == NULL)
        return NULL;
    if(father_node!=NULL)
    {
        list_add_tail(newrecord,&(father_node->childlist.list));
        father_node->curr_child=newrecord;
    }
    return newnode;
}

void * get_json_value(void * father, int value_type)
{
    JSON_VALUE * newvalue;
    JSON_NODE * father_node=(JSON_NODE *)father;
    if((value_type !=JSON_ELEM_INIT)&&
           (value_type !=JSON_ELEM_NUM)&&
           (value_type !=JSON_ELEM_STRING)&&
           (value_type !=JSON_ELEM_BOOL))
        return NULL;
    if(father_node==NULL)
        return NULL;
    newvalue=Calloc(sizeof(JSON_VALUE));
    if(newvalue==NULL)
        return NULL;
    memset(newvalue,0,sizeof(JSON_VALUE));
    Record_List * newrecord = get_new_Record_List(newvalue);
    if(newrecord == NULL)
        return NULL;
    list_add(newrecord,&(father_node->childlist.list));
    father_node->curr_child=newrecord;
    return newvalue;
}
int json_add_child(JSON_NODE * curr_node,void * child)
{
    Record_List * newrecord = get_new_Record_List(child);
    if(newrecord == NULL)
        return NULL;
    list_add(newrecord,&(curr_node->childlist.list));
    curr_node->curr_child=newrecord;
}

int json_solve_str(void ** root, char *str)
{
    JSON_NODE * root_node;
    JSON_NODE * father_node;
    JSON_NODE * curr_node;
    JSON_VALUE * curr_value;
    int value_type;
    char value_buffer[1024];

    char * tempstr;
    int i;
    int offset=0;
    int state=0;
    int ret;

    // give the root node value

    root_node=get_json_node(NULL);
    if(root_node==NULL)
        return -ENOMEM;
    father_node=NULL;
    curr_node=root_node;
    curr_node->layer=0;
    root_node->elem_type=JSON_ELEM_INIT;

    curr_node->solve_state=SOLVE_INIT;


    while(str[offset]!='\0')
    {
        switch(curr_node->solve_state)
        {
            case SOLVE_INIT:
                while(str[offset]!=0)
                {
                    if(!IsSplitChar(str[offset]))
                        break;
                    offset++;
                }
                if(str[offset]!='{')
                    return -EINVAL;
                // get an object node,then switch to the SOLVE_OBJECT
                father_node=curr_node;
                curr_node=get_json_node(father_node);
                curr_node->elem_type=JSON_ELEM_MAP;
                curr_node->solve_state=SOLVE_MAP;
                offset++;
                break;
           case SOLVE_MAP:
                while(str[offset]!=0)
                {
                    if(!IsSplitChar(str[offset]))
                        break;
                    offset++;
                }
                if(str[offset]!='\"'){
                    // if this map is empty,then finish this MAP
                    if(str[offset]=='}')
                    {
                        offset++;
                        curr_node->solve_state=SOLVE_UPGRADE;
                        break;
                    }
                    // if we should to find another elem
                    if(str[offset]==',')
                    {
                        offset++;
                        break;
                    }
                    else
                        return -EINVAL;
                }
                // we should build a name:value json node
                father_node=curr_node;
                curr_node=get_json_node(father_node);
                curr_node->elem_str=str+offset;
                offset++;
                curr_node->solve_state=SOLVE_NAME;
                break;
           case SOLVE_NAME:
                ret=get_json_strvalue(value_buffer,str+offset);
                if(ret<0)
                    return ret;
                if(ret>=DIGEST_SIZE*2)
                    return ret;
                offset+=ret;
		{
			 int len=strlen(value_buffer);
			 if(len<=DIGEST_SIZE*2)
               	        	 memcpy(curr_node->name,value_buffer,len+1);
			 else
               	        	 memcpy(curr_node->name,value_buffer,DIGEST_SIZE*2);
               		 offset++;
		}
                while(str[offset]!=0)
                {
                    if(!IsSplitChar(str[offset]))
                        break;
                    offset++;
                }
                if(str[offset]!=':')
                    return -EINVAL;
                offset++;
                curr_node->solve_state=SOLVE_VALUE;
                break;
           case SOLVE_VALUE:
                while(str[offset]!=0)
                {
                    if(!IsSplitChar(str[offset]))
                        break;
                    offset++;
                }
                if(str[offset]=='{')
                {
                // get an object node,then switch to the SOLVE_MAP
                   curr_node->elem_type=JSON_ELEM_MAP;
                   offset++;
                   curr_node->solve_state=SOLVE_MAP;
                   break;
                }
                if(str[offset]=='[')
                {
                // get an array node,then switch to the SOLVE_ARRAY
                    curr_node->elem_type=JSON_ELEM_ARRAY;
                    offset++;
                    curr_node->solve_state=SOLVE_ARRAY;
                    break;
                }
                if(str[offset]=='\"')   // value is JSON_STRING
                {
                    offset++;
                    i=get_json_strvalue(value_buffer,str+offset);
                    if(i>=0)
                    {
                        offset+=i+1;
                        curr_node->elem_type=JSON_ELEM_STRING;
                    }
                    else
                        return -EINVAL;
                }
                else
                {
                     i=get_json_numvalue(value_buffer,str+offset);
                     if(i>0)
                     {
                         offset+=i;
                         curr_node->elem_type=JSON_ELEM_NUM;
                     }
                     else
                     {
                          i=get_json_boolvalue(value_buffer,str+offset);
                          if(i>0)
                          {
                               offset+=i;
                               curr_node->elem_type=JSON_ELEM_BOOL;
                           }
                           else
                           {
                                 i=get_json_nullvalue(value_buffer,str+offset);
                                 if(i>0)
                                 {
                                       offset+=i;
                                       curr_node->elem_type=JSON_ELEM_NULL;
                                 }
                                 else
                                     return -EINVAL;

                            }

                       }
                 }
                 ret=dup_str(&curr_node->value_str,value_buffer,0);
                 curr_node->solve_state=SOLVE_UPGRADE;
                 break;
           case SOLVE_ARRAY:
                while(str[offset]!=0)
                {
                    if(!IsSplitChar(str[offset]))
                        break;
                    offset++;
                }
                if(str[offset]==']')
                {
                    offset++;
                    curr_node->solve_state=SOLVE_UPGRADE;
                    break;
                }
                // if we should to find another elem
                if(str[offset]==',')
                {
                    offset++;
                    break;
                }

            // we should build a name:value json node
                father_node=curr_node;
                curr_node=get_json_node(father_node);
                curr_node->elem_str=str+offset;
 //             offset++;
                curr_node->solve_state=SOLVE_VALUE;
                break;
            case SOLVE_UPGRADE:  // get value process
                curr_node->solve_state=SOLVE_FINISH;
                if(father_node->elem_type==JSON_ELEM_INIT)
                    break;
                curr_node=father_node;
                father_node=curr_node->father;
                break;

            default:
                return -EINVAL;
        }
        if(curr_node->solve_state==SOLVE_FINISH)
            break;
    }
    *root=curr_node;
    return offset;
}

