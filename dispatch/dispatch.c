#include "../include/data_type.h"
#include "../include/errno.h"
#include "../include/list.h"
#include "../include/attrlist.h"
#include "../include/string.h"
#include "../include/alloc.h"
#include "../include/json.h"
#include "../include/struct_deal.h"
#include "../include/basefunc.h"
#include "../include/memdb.h"
#include "../include/message.h"
#include "../include/dispatch.h"

#include "dispatch_struct.h"


static POLICY_LIST process_policy;
static POLICY_LIST aspect_policy;

static void * match_policy_template;
static void * route_policy_template;

static inline int _init_policy_list(void * list)
{
    	POLICY_LIST * policy_list=(POLICY_LIST *)list;
   	memset(policy_list,0,sizeof(POLICY_LIST));
   	INIT_LIST_HEAD(&(policy_list->head.list));
   	policy_list->head.record=NULL;
	policy_list->policy_num=0;
	return 0;

}

int dispatch_policy_init(void * object )
{
	int ret;
	ret=_init_policy_list(&process_policy);
	ret=_init_policy_list(&aspect_policy);
	return 0;
}

void * dispatch_policy_create()
{
	int ret;
	DISPATCH_RULE * rule;
	ret=Galloc0(&rule,sizeof(DISPATCH_RULE));
	if(rule==NULL)
		return NULL;
	_init_policy_list(&rule->match_list);
	_init_policy_list(&rule->router_list);
	return rule;
	
}

int read_policy_from_buffer(char * buffer, int max_len)
{
	int offset=0;
	int ret;
	void * root;
	int solve_offset;
	int count=0;
	void * policy;

	while(offset<max_len)
	{
      	        solve_offset=json_solve_str(&root,buffer+offset);
        	if(solve_offset<=0)
			break;
		offset+=solve_offset;

        	policy=dispatch_read_rule_policy(root);
		if(policy==NULL)
			break;
		
		if(dispatch_policy_is_aspect(policy))
		{
			ret=dispatch_add_rule_policy(&aspect_policy,policy);
		}
		else
		{
			ret=dispatch_add_rule_policy(&process_policy,policy);
		}
		if(ret<0)
			return ret;
		count++;
	}
	return count;
}

void * dispatch_read_rule_policy(void * policy_node)
{
    void * match_rule_template=create_struct_template(&match_rule_desc);
    void * router_rule_template=create_struct_template(&router_rule_desc);
    void * match_policy_node;
    void * router_policy_node;
    void * match_rule_node;
    void * router_rule_node;
    void * temp_node;
    char buffer[1024];
    int ret;
    MATCH_RULE * temp_match_rule;
    ROUTER_RULE * temp_router_rule;

    MESSAGE_POLICY * policy=malloc(sizeof(MESSAGE_POLICY));
    if(policy==NULL)
        return -EINVAL;
    __message_policy_init(policy);

    // get the match policy json node
    match_policy_node=find_json_elem("MATCH_POLICY",json_node);
    if(match_policy_node==NULL)
        return -EINVAL;

    // get the match policy json node
    router_policy_node=find_json_elem("ROUTER_POLICY",json_node);
    if(router_policy_node==NULL)
        return -EINVAL;

    // read the match policy
    // first,read the sender proc value

    temp_node=find_json_elem("sender",match_policy_node);
    if(temp_node==NULL)
        policy->sender_proc=NULL;
    else
    {
        ret=get_json_value_from_node(temp_node,buffer,1024);
        if((ret<0) ||(ret>=1024))
            return -EINVAL;
        policy->sender_proc=dup_str(buffer,ret+1);
    }

    // second,read the match rule

    temp_node = find_json_elem("rules",match_policy_node);
    if(temp_node==NULL)
        return -EINVAL;

    match_rule_node=get_first_json_child(temp_node);
    while(match_rule_node!=NULL)
    {
        temp_match_rule=malloc(sizeof(MATCH_RULE));
        ret=json_2_struct(match_rule_node,temp_match_rule,match_rule_template);
        if(ret<0)
            return -EINVAL;
        __router_policy_add(policy->match_policy_list,temp_match_rule);
        match_rule_node=get_next_json_child(temp_node);
    }

    // read the router policy
    // first,read the main router policy

    router_rule_node=find_json_elem("main_policy",router_policy_node);
    if(router_rule_node==NULL)
        return -EINVAL;
    temp_router_rule=malloc(sizeof(ROUTER_RULE));
    ret=json_2_struct(router_rule_node,temp_router_rule,router_rule_template);
    if(ret<0)
        return -EINVAL;
    policy->main_router_rule=temp_router_rule;

    // second,read the dup rule

    temp_node = find_json_elem("dup_policy",router_policy_node);
    if(temp_node==NULL)
    {
        *message_policy=policy;
        return 0;
    }
    router_rule_node=get_first_json_child(temp_node);
    while(router_rule_node!=NULL)
    {
        temp_router_rule=malloc(sizeof(ROUTER_RULE));
        ret=json_2_struct(router_rule_node,temp_router_rule,router_rule_template);
        if(ret<0)
            return -EINVAL;
        __router_policy_add(policy->dup_router_rule,temp_router_rule);

}

/*
int __is_policy_aspectpolicy(void * policy)
{
	MESSAGE_POLICY * test_policy=policy;
	if(policy==NULL)
		return -EINVAL;
	if(test_policy->main_router_rule->state & MSG_FLOW_ASPECT)
		return 1;
	return 0;
}

int router_policy_init()
{
    	int retval;
   	retval=  register_record_type("FTRE",expand_flow_trace_desc,get_entity_lib_ops("FTRE"));
    	if(retval<0)
    	{
	    printf("register FTRE struct error!\n");
	    return -EINVAL;
    	}
    	retval=  register_record_type("APRE",expand_aspect_point_desc,get_entity_lib_ops("APRE"));
    	if(retval<0)
    	{
	    printf("register APRE struct error!\n");
	    return -EINVAL;
    	}	
	retval= __router_policy_init(&main_router_policy);
	if(retval<0)
		return retval;	

	retval= __router_policy_init(&local_router_policy);
	if(retval<0)
		return retval;	

	retval= __router_policy_init(&aspect_router_policy);
	if(retval<0)
		return retval;	

   	return __router_policy_init(&aspect_local_policy);
}

int router_policy_gettype(void * policy)
{
    MESSAGE_POLICY * router_policy;
    ROUTER_RULE  * main_rule;
    if(policy==NULL)
	    return -EINVAL;
    main_rule=((MESSAGE_POLICY *)policy)->main_router_rule;
    if(main_rule==NULL)
	    return -EINVAL;

    return main_rule->type;
}

int __router_policy_getfirst(void * policy_list,void ** policy)
{
	Record_List * recordhead;
	Record_List * newrecord;
    ROUTER_POLICY_LIST * router_policy=(ROUTER_POLICY_LIST *)policy_list;

    if(router_policy->state<0)
		return -EINVAL;

    recordhead = &(router_policy->head);
	if(recordhead==NULL)
		return -ENOMEM;

	newrecord = (Record_List *)(recordhead->list.next);
	*policy=newrecord->record;
    router_policy->curr=newrecord;
	return 0;
}

int router_policy_getfirst(void ** policy)
{
    return __router_policy_getfirst(&main_router_policy,policy);
}

int local_policy_getfirst(void ** policy)
{
    return __router_policy_getfirst(&local_router_policy,policy);
}

int aspect_local_getfirst(void ** policy)
{
    return __router_policy_getfirst(&aspect_local_policy,policy);
}

int aspect_policy_getfirst(void ** policy)
{
    return __router_policy_getfirst(&aspect_router_policy,policy);
}

int __router_policy_getnext(void * policy_list,void ** policy)
{
	Record_List * recordhead;
	Record_List * newrecord;
    ROUTER_POLICY_LIST * router_policy=(ROUTER_POLICY_LIST *)policy_list;

    if(router_policy->state<0)
		return -EINVAL;

    if(router_policy->curr==NULL)
		return -EINVAL;
    if(router_policy->curr==&(router_policy->head.list))
   {
	 *policy=NULL;
	 return 0;
   }

    recordhead = router_policy->curr;
	if(recordhead==NULL)
		return -EINVAL;

	newrecord = (Record_List *)(recordhead->list.next);
	*policy=newrecord->record;
    router_policy->curr=newrecord;
	return 0;
}

int router_policy_getnext(void ** policy)
{
    return __router_policy_getnext(&main_router_policy,policy);
}

int local_policy_getnext(void ** policy)
{
    return __router_policy_getnext(&local_router_policy,policy);
}

int aspect_policy_getnext(void ** policy)
{
    return __router_policy_getnext(&aspect_router_policy,policy);
}

int aspect_local_getnext(void ** policy)
{
    return __router_policy_getnext(&aspect_local_policy,policy);
}

int __router_policy_add(void * policy_list,void * policy)
{
	Record_List * recordhead;
	Record_List * newrecord;
    	ROUTER_POLICY_LIST * router_policy=(ROUTER_POLICY_LIST *)policy_list;

    	if(router_policy->state<0)
		return -EINVAL;
   	recordhead = &(router_policy->head);
	if(recordhead==NULL)
		return -ENOMEM;

	newrecord = kmalloc(sizeof(Record_List),GFP_KERNEL);
	if(newrecord==NULL)
		return -ENOMEM;
	INIT_LIST_HEAD(&(newrecord->list));
	newrecord->record=policy;
	list_add_tail(&(newrecord->list),recordhead);
    	router_policy->policy_num++;
	return 0;
}

int router_policy_add(void * policy)
{
   int ret;
   int type;
   type=router_policy_gettype(policy); 
   switch(type)
   {
	   case  MSG_FLOW_LOCAL:
    		   ret = __router_policy_add(&local_router_policy,policy);
		   break;
	   case  MSG_FLOW_DELIVER:
	   case  MSG_FLOW_QUERY:
    		   ret= __router_policy_add(&main_router_policy,policy);
		   break;
	   case  MSG_FLOW_ASPECT_LOCAL:
    		   ret= __router_policy_add(&aspect_local_policy,policy);
		   break;
	   case  MSG_FLOW_ASPECT:
    		   ret= __router_policy_add(&aspect_router_policy,policy);
		   break;
	   default:
		   return -EINVAL;
   }
   return ret;
}

int __router_policy_del(void * policy_list,void * policy)
{
	Record_List * recordhead;
	Record_List * newrecord;
    ROUTER_POLICY_LIST * router_policy=(ROUTER_POLICY_LIST *)policy_list;

    if(router_policy->state<0)
		return -EINVAL;
    recordhead = &(router_policy->head);
	if(recordhead==NULL)
		return -ENOMEM;

    if(router_policy->policy_num==0)
	{
		return 0;
	}
	newrecord = (Record_List *)(recordhead->list.prev);
	list_del(&(newrecord->list));
    	router_policy->policy_num--;
	kfree(newrecord);
	return 0;
}
int router_policy_del(void * policy)
{
    return __router_policy_del(&main_router_policy,policy);
}

int __router_policy_reset( void * policy_list )
{
	int ret;
	void * policy;
    ROUTER_POLICY_LIST * router_policy=(ROUTER_POLICY_LIST *)policy_list;
    if(router_policy->state<0)
		return -EINVAL;

    router_policy->state = -1;

    while(router_policy->policy_num>0)
	{
		if(ret<=0)
			break;
	}
	return 0;
}
int router_policy_reset()
{
    return __router_policy_reset(&main_router_policy);
}

int __message_policy_init(void * policy)
{
    MESSAGE_POLICY * message_policy=(MESSAGE_POLICY *)policy;
    if(message_policy==NULL)
        return -EINVAL;
    message_policy->sender_proc=NULL;
    message_policy->match_policy_list=
            (ROUTER_POLICY_LIST *)malloc(sizeof(ROUTER_POLICY_LIST));
    if(message_policy->match_policy_list==NULL)
        return -EINVAL;

    message_policy->dup_router_rule =
            (ROUTER_POLICY_LIST *)malloc(sizeof(ROUTER_POLICY_LIST));
    if(message_policy->dup_router_rule==NULL)
        return -EINVAL;
    message_policy->main_router_rule =
            (ROUTER_RULE *)malloc(sizeof(ROUTER_RULE));
    if(message_policy->main_router_rule==NULL)
        return -EINVAL;
    message_policy->sender_proc=NULL;
    __router_policy_init(message_policy->match_policy_list);

    memset(message_policy->main_router_rule,0,sizeof(ROUTER_RULE));
    __router_policy_init(message_policy->dup_router_rule);
    return 0;
}
*/
#define MAX_LINE_LEN 1024
//int read_cfg_buffer(FILE * stream, char * buf, int size)
    /*  Read text data from config file,
     *  ignore the ^# line and remove the \n character
     *  stream: the config file stream
     *  buf: the buffer to store the cfg data
     *  size: read data size
     *
     *  return value: read data size,
     *  negative value if it has special error
     *  */
/*
{
    long offset=0;
    long curr_offset;
    char buffer[MAX_LINE_LEN];
    char * retptr;
    int len;

    while(offset<size)
    {
        curr_offset=ftell(stream);
        retptr=fgets(buffer,MAX_LINE_LEN,stream);

        // end of the file
        if(retptr==NULL)
            break;
        len=strlen(buffer);
        if(len==0)
            break;
        // commet line
        if(buffer[0]=='#')
            continue;
        while((buffer[len-1]=='\r')||(buffer[len-1]=='\n'))
        {
            len--;
            if(len==0)
                continue;
            buffer[len]==0;
        }
        // this line is too long to read
        if(len>size)
            return -EINVAL;

        // out of the bound
        if(len+offset>size)
        {
            fseek(stream,curr_offset,SEEK_SET);
            break;
        }
        memcpy(buf+offset,buffer,len);
        offset+=len;
	buf[offset]=0;
    }
    return offset;
}

int read_one_policy(void ** message_policy,void * json_node)
{
    void * match_rule_template=create_struct_template(&match_rule_desc);
    void * router_rule_template=create_struct_template(&router_rule_desc);
    void * match_policy_node;
    void * router_policy_node;
    void * match_rule_node;
    void * router_rule_node;
    void * temp_node;
    char buffer[1024];
    int ret;
    MATCH_RULE * temp_match_rule;
    ROUTER_RULE * temp_router_rule;

    MESSAGE_POLICY * policy=malloc(sizeof(MESSAGE_POLICY));
    if(policy==NULL)
        return -EINVAL;
    __message_policy_init(policy);

    // get the match policy json node
    match_policy_node=find_json_elem("MATCH_POLICY",json_node);
    if(match_policy_node==NULL)
        return -EINVAL;

    // get the match policy json node
    router_policy_node=find_json_elem("ROUTER_POLICY",json_node);
    if(router_policy_node==NULL)
        return -EINVAL;

    // read the match policy
    // first,read the sender proc value

    temp_node=find_json_elem("sender",match_policy_node);
    if(temp_node==NULL)
        policy->sender_proc=NULL;
    else
    {
        ret=get_json_value_from_node(temp_node,buffer,1024);
        if((ret<0) ||(ret>=1024))
            return -EINVAL;
        policy->sender_proc=dup_str(buffer,ret+1);
    }

    // second,read the match rule

    temp_node = find_json_elem("rules",match_policy_node);
    if(temp_node==NULL)
        return -EINVAL;

    match_rule_node=get_first_json_child(temp_node);
    while(match_rule_node!=NULL)
    {
        temp_match_rule=malloc(sizeof(MATCH_RULE));
        ret=json_2_struct(match_rule_node,temp_match_rule,match_rule_template);
        if(ret<0)
            return -EINVAL;
        __router_policy_add(policy->match_policy_list,temp_match_rule);
        match_rule_node=get_next_json_child(temp_node);
    }

    // read the router policy
    // first,read the main router policy

    router_rule_node=find_json_elem("main_policy",router_policy_node);
    if(router_rule_node==NULL)
        return -EINVAL;
    temp_router_rule=malloc(sizeof(ROUTER_RULE));
    ret=json_2_struct(router_rule_node,temp_router_rule,router_rule_template);
    if(ret<0)
        return -EINVAL;
    policy->main_router_rule=temp_router_rule;

    // second,read the dup rule

    temp_node = find_json_elem("dup_policy",router_policy_node);
    if(temp_node==NULL)
    {
        *message_policy=policy;
        return 0;
    }
    router_rule_node=get_first_json_child(temp_node);
    while(router_rule_node!=NULL)
    {
        temp_router_rule=malloc(sizeof(ROUTER_RULE));
        ret=json_2_struct(router_rule_node,temp_router_rule,router_rule_template);
        if(ret<0)
            return -EINVAL;
        __router_policy_add(policy->dup_router_rule,temp_router_rule);
        router_rule_node=get_next_json_child(temp_node);
    }
    *message_policy=policy;
    return 0;
}

int router_read_cfg(char * filename)
{
    const int bufsize=1024;
    char buffer[bufsize];
    void * root;
    int read_offset;
    int solve_offset;
    int buffer_left=0;
    int policy_num=0;
    void * policy;
    int ret;

    FILE * fp = fopen(filename,"r");
    if(fp==NULL)
        return -EINVAL;

    do {

        // when the file reading is not finished, we should read new data to the buffer
        if(fp != NULL)
        {
            read_offset=read_cfg_buffer(fp,buffer+buffer_left,bufsize-buffer_left);
            if(read_offset<0)
                return -EINVAL;
            else if(read_offset==0)
            {
                fclose(fp);
                fp=NULL;
            }
        }
        printf("policy %d is %s\n",policy_num+1,buffer);

        solve_offset=json_solve_str(&root,buffer);
        if(solve_offset<=0)
		break;
        ret=read_one_policy(&policy,root);

        if(ret<0)
		break;
        router_policy_add(policy);
        policy_num++;
	read_offset+=buffer_left;
        buffer_left=read_offset-solve_offset;
        if(buffer_left>0)
	{
            Memcpy(buffer,buffer+solve_offset,buffer_left);
	    buffer[buffer_left]=0;
	}
        else
        {
            if(fp==NULL)
                break;
        }
    }while(1);
    return policy_num;
}

int match_message_head(void * match_rule,void * message)
{
    int ret;
    MESSAGE_HEAD * msg_head=get_message_head(message);
    if(msg_head==NULL)
        return -EINVAL;
    MATCH_RULE * rule=(MATCH_RULE *)match_rule;
    char buffer[128];
    ret=message_comp_head_elem_text(message,rule->seg,rule->value);
	return ret;
}
*/
/*
int match_message_record(void * match_rule,void * message)
{
    int ret;
    MESSAGE_HEAD * msg_head=get_message_head(message);
    if(msg_head==NULL)
        return -EINVAL;
    void * record_template = load_record_template(msg_head->record_type);
    if(record_template==NULL)
        return -EINVAL;
    MATCH_RULE * rule=(MATCH_RULE *)match_rule;
    char buffer[1024];
    void * record;
    ret=message_comp_elem_text(message,rule->seg,0,rule->value);
	
    return ret;
}

int match_message_expand(void * match_rule,void * message)
{
    int ret;
    int i;
    MESSAGE_HEAD * msg_head=get_message_head(message);
    MESSAGE_EXPAND * expand;
    if(msg_head==NULL)
        return -EINVAL;
    MATCH_RULE * rule=(MATCH_RULE *)match_rule;
    char buffer[1024];
    void * expand_template = load_record_template(rule->expand_type);
    if(expand_template==NULL)
        return -EINVAL;

   for(i=0;i<5;i++)
   {
        ret=message_get_expand(message,&expand,i);
        if(expand==NULL)
            return -EINVAL;
        if(strncmp(expand->tag,rule->expand_type,4)==0)
            break;
   }
   if(i==5)
       return -EINVAL;
    ret=struct_read_elem(rule->seg,expand,buffer,expand_template);
    if(ret<0)
        return ret;
    buffer[ret]=0;

    ret=strncmp(buffer,rule->value,ret);
    if(ret==0)
        return 1;
    return 0;
}

int rule_match_message(void * match_rule,void * message)
{
    int ret;
    MATCH_RULE * rule = (MATCH_RULE *)match_rule;
    if(rule==NULL)
        return -EINVAL;

    switch(rule->area)
    {
        case MATCH_AREA_HEAD:
            ret= match_message_head(rule,message);
            break;
        case MATCH_AREA_RECORD:
            ret= match_message_record(rule,message);
            break;
        case MATCH_AREA_EXPAND:
            ret= match_message_expand(rule,message);
            break;
        case MATCH_AREA_ERR:
        default:
            return -EINVAL;
    }
    return ret;
}

void * get_first_match_rule(void * policy)
{
    MESSAGE_POLICY * msg_policy=(MESSAGE_POLICY *)policy;
    void * rule;
    int ret;
    ret=__router_policy_getfirst(msg_policy->match_policy_list,&rule);
    if(ret<0)
        return NULL;
    return rule;
}

void * get_next_match_rule(void * policy)
{
    MESSAGE_POLICY * msg_policy=(MESSAGE_POLICY *)policy;
    void * rule;
    int ret;
    ret=__router_policy_getnext(msg_policy->match_policy_list,&rule);
    if(ret<0)
        return NULL;
    return rule;
}

void * get_main_router_rule(void * policy)
{
    MESSAGE_POLICY * msg_policy=(MESSAGE_POLICY *)policy;
    void * rule;
    int ret;
    return msg_policy->main_router_rule;
}

void * router_get_first_duprule(void * policy)
{
    MESSAGE_POLICY * msg_policy=(MESSAGE_POLICY *)policy;
    void * rule;
    int ret;
    ret=__router_policy_getfirst(msg_policy->dup_router_rule,&rule);
    if(ret<0)
        return NULL;
    return rule;
}

void * router_get_next_duprule(void * policy)
{
    MESSAGE_POLICY * msg_policy=(MESSAGE_POLICY *)policy;
    void * rule;
    int ret;
    ret=__router_policy_getnext(msg_policy->dup_router_rule,&rule);
    if(ret<0)
        return NULL;
    return rule;
}


int router_policy_match_message(void * policy,void * message,char * sender_proc)
{
    int ret;
    MESSAGE_POLICY * msg_policy=(MESSAGE_POLICY *)policy;
    if(policy==NULL)
        return -EINVAL;
    if(message==NULL)
        return -EINVAL;
    if(sender_proc!=NULL)
    {
	    if(strncmp(msg_policy->sender_proc,sender_proc,DIGEST_SIZE*2)!=0)
		    return 0;
    }
    void * rule=get_first_match_rule(policy);
    while(rule!=NULL)
    {
        ret=rule_match_message(rule,message);
        if(ret<0)
            return -EINVAL;
        if(ret==0)
            return 0;
        rule=get_next_match_rule(policy);
    }
    return ret;
}

int router_find_match_policy(void * message,void **msg_policy,char * sender_proc)
{
    void * policy;
    int ret;
    *msg_policy=NULL;
    ret=router_policy_getfirst(&policy);
    if(ret<0)
        return ret;
    while(policy!=NULL)
    {
        ret=router_policy_match_message(policy,message,sender_proc);
        if(ret<0)
            return ret;
        if(ret>0)
        {
            *msg_policy=policy;
            return ret;
        }
    	ret=router_policy_getnext(&policy);
    	if(ret<0)
             return ret;
    }
    return ret;
}

int router_find_aspect_policy(void * message,void **msg_policy,char * sender_proc)
{
    void * policy;
    int ret;
    *msg_policy=NULL;
    ret=aspect_policy_getfirst(&policy);
    if(ret<0)
        return ret;
    while(policy!=NULL)
    {
        ret=router_policy_match_message(policy,message,sender_proc);
        if(ret<0)
            return ret;
        if(ret>0)
        {
            *msg_policy=policy;
            return ret;
        }
    	ret=aspect_policy_getnext(&policy);
    	if(ret<0)
             return ret;
    }
    return ret;
}


int router_find_local_policy(void * message,void **msg_policy,char * sender_proc)
{
    void * policy;
    int ret;
    *msg_policy=NULL;
    ret=local_policy_getfirst(&policy);
    if(ret<0)
        return ret;
    while(policy!=NULL)
    {
        ret=router_policy_match_message(policy,message,sender_proc);
        if(ret<0)
            return ret;
        if(ret>0)
        {
            *msg_policy=policy;
            return ret;
        }
    	ret=local_policy_getnext(&policy);
    	if(ret<0)
             return ret;
    }
    return ret;
}

int router_find_aspect_local_policy(void * message,void **msg_policy,char * sender_proc)
{
    void * policy;
    int ret;
    *msg_policy=NULL;
    ret=aspect_local_getfirst(&policy);
    if(ret<0)
        return ret;
    while(policy!=NULL)
    {
        ret=router_policy_match_message(policy,message,sender_proc);
        if(ret<0)
            return ret;
        if(ret>0)
        {
            *msg_policy=policy;
            return ret;
        }
    	ret=aspect_local_getnext(&policy);
    	if(ret<0)
             return ret;
    }
    return ret;
}




int rule_get_target(void * router_rule,void * message,void **result)
{
    char * target;
    int ret;
    ROUTER_RULE * rule= (ROUTER_RULE *)router_rule;
    *result=NULL;
    switch(rule->target_type){
	    case MSG_TARGET_NAME:
	    case MSG_TARGET_LOCAL:
		    target=dup_str(rule->target_name,DIGEST_SIZE*2);
		    break;
	    case MSG_TARGET_UUID:
		    target=dup_str(rule->target_name,DIGEST_SIZE*2);
		    break;
	    case MSG_TARGET_CONN:
		    target=dup_str(rule->target_name,DIGEST_SIZE*2);
		    break;
	    case MSG_TARGET_RECORD:
		   {
			ret=message_read_elem(message,rule->target_name,0,&target);
			if(ret<0)
				return -EINVAL;
			if(target==NULL)
				return -EINVAL;
			break;
		   }

	    case MSG_TARGET_SPLIT:
		    return -EINVAL;
	    case MSG_TARGET_MIXUUID:
		    {
			    void * json_root;
			    void * json_elem;
			    const int repeat_time=2;
			    char mixvalue[repeat_time][DIGEST_SIZE*2];
			    char tag[16];
			    char uuid[DIGEST_SIZE*2];
			    int i;

			    ret=json_solve_str(&json_root,rule->target_name);
			    if(ret<0)
				    return ret;
			    if(json_get_type(json_root)!=JSON_ELEM_MAP)
				    return -EINVAL;
			
			    memset(uuid,0,DIGEST_SIZE*2);

			    char * value;
			    char name[DIGEST_SIZE*2];

			    for(i=0;i<repeat_time;i++)
			    {
			   	 memset(mixvalue[i],0,DIGEST_SIZE*2);

				 if(i==0)
				 {
			      	 	 // first json_elem is the node uuid value
			       		json_elem=get_first_json_child(json_root);
				 }
				 else
				 {
					json_elem=get_next_json_child(json_root);

				 }
				 ret=get_json_name_from_node(json_elem,name);
			    	 if(ret<0)
					return ret;
				 if(json_get_type(json_elem)!=JSON_ELEM_STRING)
					return -EINVAL;

				 ret=get_json_value_from_node(json_elem,tag,15);
				// tag is "RECORD"  means this elem defines an elem in the message's record
				 if(strcmp(tag,"RECORD")==0)
				 {
					void * record;	
					ret=message_read_elem(message,name,0,&record);
					if(ret<0)
						return ret;
				        int len=strlen(record);
					if(len<DIGEST_SIZE*2)
					{
						Memcpy(mixvalue[i],record,len);
					}
					else
					{
						Memcpy(mixvalue[i],record,DIGEST_SIZE*2);
					}
				}
				// tag is "name" means this elem defines remote proc's value
				else if(strcmp(tag,"NAME")==0)
				{
				        int len=strlen(name);
					if(len<DIGEST_SIZE*2)
					{
						Memcpy(mixvalue[i],name,len);
					}
					else
					{
						Memcpy(mixvalue[i],name,DIGEST_SIZE*2);
					}
				}
			    	else if(strncmp(tag,"EXPAND",6)==0)
			    	{
					char * type=tag+7;
			       		if(strlen(type)!=4)
					{
						return -EINVAL;	
					}		
					void * expand;
				        ret = message_get_define_expand(message,&expand,type);
					if(ret<0)
						return -EINVAL;
					if(expand==NULL)
						return -EINVAL;
					void * struct_template=load_record_template(type);
					if(struct_template==NULL)
						return -EINVAL;
					ret=struct_read_elem(name,expand,mixvalue[i],struct_template);		
					free_struct_template(struct_template);
					if(ret<0)
						return ret;
			    	}
		    		else
			    	{
			    		return -EINVAL;
			    	}
			  	if(i==0)
					continue;
			  	if(i==1)
			   	{
					comp_proc_uuid(mixvalue[0],mixvalue[1],uuid);
				  	continue;
			    	}
			   	comp_proc_uuid(uuid,mixvalue[i],uuid);
		
			  }
		   	 target=malloc(DIGEST_SIZE*2);
		 	 Memcpy(target,uuid,DIGEST_SIZE*2);
		   	 break;
		   }
		    
	    case MSG_TARGET_ERROR:
	    default:
		    return -EINVAL;
    }
    *result=target;
    return rule->target_type;	

}

int router_dup_activemsg_info (void * message)
{
	int ret;
	struct message_box * msg_box=message;
	MESSAGE_HEAD * msg_head;
	if(message==NULL)
		return -EINVAL;

	void * active_msg=message_get_activemsg(message);
	if(active_msg==NULL)
		return 0;
	
	msg_head=get_message_head(active_msg);
	message_set_flow(msg_box,msg_head->flow);
	message_set_state(msg_box,msg_head->state);
//	if(msg_head->flow & MSG_FLOW_RESPONSE)
//	{
	
	void * flow_expand;
	ret=message_get_define_expand(active_msg,&flow_expand,"FTRE");
	if(flow_expand!=NULL) 
	{
		void * struct_template =load_record_template("FTRE");
		void * new_expand = clone_struct(flow_expand,struct_template);

		message_add_expand(message,new_expand);
	}
	ret=message_get_define_expand(active_msg,&flow_expand,"APRE");
	if(flow_expand!=NULL) 
	{
		void * struct_template =load_record_template("APRE");
		void * new_expand = clone_struct(flow_expand,struct_template);

		message_add_expand(message,new_expand);
	}

//	}

	return 1;

}

int router_set_local_flow(void * message,void * local_rule)
{
    int ret;
    char * target;
    if(message==NULL)
        return -EINVAL;
    if(local_rule==NULL)
        return -EINVAL;
    ROUTER_RULE * rule= (ROUTER_RULE *)local_rule;
    if(rule==NULL)
	return -EINVAL;
    message_set_state(message,MSG_FLOW_LOCAL);
    ret=rule_get_target(rule,message,&target);
    if(ret<0)
	return ret;
    message_set_receiver(message,target);
    return 0;    
}

int router_set_aspect_local_flow(void * message,void * msg_policy)
{
    int ret;
    char * target;
    if(message==NULL)
        return -EINVAL;
    if(msg_policy==NULL)
        return -EINVAL;
    MESSAGE_POLICY * policy= (MESSAGE_POLICY *)msg_policy;

    ROUTER_RULE * rule = policy->main_router_rule;
    if(rule==NULL)
	return -EINVAL;
    message_set_state(message,MSG_FLOW_ASPECT_LOCAL);
    ret=rule_get_target(rule,message,&target);
    if(ret<0)
	return ret;
    message_set_receiver(message,target);
    return 0;    
}

int router_set_main_flow(void * message,void * msg_policy)
{
    int ret;
    if(message==NULL)
        return -EINVAL;
    if(msg_policy==NULL)
        return -EINVAL;
    MESSAGE_POLICY * policy= (MESSAGE_POLICY *)msg_policy;

    ROUTER_RULE * rule = policy->main_router_rule;

    if(rule==NULL)
        return -EINVAL;

    // local message deliver
    if(rule->type&MSG_FLOW_LOCAL)
    {
        router_set_local_flow(message,rule);
	return 0;
    }

    // else ,if message is a new message, init the message
   if( (message_get_state(message)<=MSG_FLOW_INIT)
   	 ||( message_get_state(message)==MSG_FLOW_FINISH))
   {
	   message_set_flow(message,rule->type);
	   if(rule->state==MSG_FLOW_RESPONSE)
	   {
	  	 message_set_state(message,MSG_FLOW_RESPONSE);
	   }
	   else
	   {
	  	 message_set_state(message,MSG_FLOW_DELIVER);
	   }
   }


   int flow = message_get_flow(message);

  if(flow==MSG_FLOW_INIT)
  {
   	message_set_flow(message,rule->type);
  }
  else
  {
	message_set_flow(message,flow|rule->type);
  }
   message_set_state(message,rule->state);


   switch(message_get_state(message))
   {
           case MSG_FLOW_DELIVER:
	   case MSG_FLOW_QUERY:
	   {
		   char buffer[DIGEST_SIZE*2];   
		   char * target;
		   int target_type;
		   target_type=rule_get_target(rule,message,&target);
		   if(target_type<0)
		   	return target_type;
		    switch (target_type)
		    {
			    case MSG_TARGET_UUID:
			    case MSG_TARGET_MIXUUID:
    				set_message_head(message,"receiver_uuid",target);
				break;
			    case MSG_TARGET_NAME:
			    case MSG_TARGET_RECORD:
				buffer[0]='@';
				strncpy(buffer+1,target,DIGEST_SIZE*2-1);
				set_message_head(message,"receiver_uuid",buffer);  
				break;
			    case MSG_TARGET_CONN:
				buffer[0]=':';
				strncpy(buffer+1,target,DIGEST_SIZE*2-1);
				set_message_head(message,"receiver_uuid",buffer);  
    				break;
		    }
		    break;

	   }
           case MSG_FLOW_RESPONSE:
	   {
		void * active_msg=message_get_activemsg(message);
		if(active_msg==NULL)
			break;
		void * flow_expand;
		ret=message_remove_expand(active_msg,"FTRE",&flow_expand);
		if(flow_expand!=NULL) 
		{
			message_add_expand(message,flow_expand);
		}
		else
		{
			const char * sender_uuid=message_get_sender(active_msg);
			if(sender_uuid==NULL)
				return -EINVAL;
			message_set_receiver(message,sender_uuid);
		}
		break;

	   }
           case MSG_FLOW_ASPECT:
                    return -EINVAL;

            default:
               return -EINVAL;

    }
    return 0;
}

int router_set_aspect_flow(void * message,void * msg_policy)
{
    int ret;
    if(message==NULL)
        return -EINVAL;
    if(msg_policy==NULL)
        return -EINVAL;
    MESSAGE_POLICY * policy= (MESSAGE_POLICY *)msg_policy;

    ROUTER_RULE * rule = policy->main_router_rule;

    if(rule==NULL)
        return -EINVAL;

    if(!(rule->type & (MSG_FLOW_ASPECT| MSG_FLOW_ASPECT_LOCAL)))
    {
	return -EINVAL;
    }
   
    int flow=message_get_flow(message);
    message_set_flow(message, flow | rule->type);


   char buffer[DIGEST_SIZE*2];   
   char * target;
   int target_type;
   target_type=rule_get_target(rule,message,&target);
   if(target_type<0)
   	return target_type;
    switch (target_type)
    {
	    case MSG_TARGET_LOCAL:
		message_set_receiver(message,target);
		break;
	    case MSG_TARGET_UUID:
	    case MSG_TARGET_MIXUUID:
		message_set_receiver(message,target);
		break;
	    case MSG_TARGET_NAME:
		buffer[0]='@';
		strncpy(buffer+1,target,DIGEST_SIZE*2-1);
		message_set_receiver(message,buffer);  
		break;
	    case MSG_TARGET_CONN:
		buffer[0]=':';
		strncpy(buffer+1,target,DIGEST_SIZE*2-1);
		message_set_receiver(message,buffer);  
		break;
	   default:
		return -EINVAL;
    }
   // message_set_state(message,MSG_FLOW_ASPECT);
    return 0;
}

int router_set_dup_flow(void * src_msg,void * dup_rule,void **dup_msg)
{
    int ret;
    void * message;
    if(src_msg==NULL)
        return -EINVAL;
    if(dup_rule==NULL)
        return -EINVAL;
    ROUTER_RULE * rule = (ROUTER_RULE *)dup_rule;

    if(rule==NULL)
        return -EINVAL;

   message=message_clone(src_msg);
   if(message==NULL)
	   return -EINVAL;

   *dup_msg=message;

    // local message deliver
    if(rule->type&MSG_FLOW_LOCAL)
    {
        router_set_local_flow(message,rule);
	return 0;
    }

    // init the message's flow and state
    message_set_flow(message,rule->type);
    message_set_state(message,rule->state);

   switch(message_get_state(message))
   {
           case MSG_FLOW_DELIVER:
	   {
		   char buffer[DIGEST_SIZE*2];   
		   char * target;
		   int target_type;
		   target_type=rule_get_target(rule,message,&target);
		   if(target_type<0)
		   	return target_type;
		    switch (target_type)
		    {
			    case MSG_TARGET_UUID:
    				set_message_head(message,"receiver_uuid",target);
				break;
			    case MSG_TARGET_NAME:
				buffer[0]='@';
				strncpy(buffer+1,target,DIGEST_SIZE*2-1);
				set_message_head(message,"receiver_uuid",buffer);  
				break;
			    case MSG_TARGET_CONN:
				buffer[0]=':';
				strncpy(buffer+1,target,DIGEST_SIZE*2-1);
				set_message_head(message,"receiver_uuid",buffer);  
    				break;
		    }
		    break;

	   }
           case MSG_FLOW_RESPONSE:
	   {
		break;
	   }
           case MSG_FLOW_ASPECT:
                    return -EINVAL;

            default:
                    return -EINVAL;

    }
    return 0;
}

int router_push_site(void * message,char * name,char * type)
{
	struct expand_flow_trace * flow_trace;
	int ret;
	int len;
	int index;

	if(strcmp(type,"FTRE")!=0)
		return -EINVAL;

	index=message_get_define_expand(message,&flow_trace,type);
	if(index<0)
		return -EINVAL;
	if(flow_trace==NULL)
	{
		// build a new flow_trace
		flow_trace=malloc(sizeof(struct expand_flow_trace));
		flow_trace->data_size=0;
		flow_trace->record_num=1;
		memcpy(flow_trace->tag, type,4);
		flow_trace->trace_record=dup_str(name,DIGEST_SIZE*2);
		message_add_expand(message,flow_trace);	
	}
	else
	{
		int curr_offset=flow_trace->record_num*DIGEST_SIZE*2;
		flow_trace->record_num++;
		char * buffer=malloc(curr_offset+DIGEST_SIZE*2);
		memcpy(buffer,flow_trace->trace_record,curr_offset);
		len=strlen(name);
		if(len<DIGEST_SIZE*2)
			memcpy(buffer+curr_offset,name,len+1);
		else
			memcpy(buffer+curr_offset,name,DIGEST_SIZE*2);
		free(flow_trace->trace_record);
		flow_trace->trace_record=buffer;
		message_replace_define_expand(message,flow_trace,type);
	}
	return 0;
}

int router_push_aspect_site(void * message,char * proc,char * point)
{
	struct expand_aspect_point * aspect_point;
	int ret;
	int len;
	int index;

	index=message_get_define_expand(message,&aspect_point,"APRE");
	if(index<0)
		return -EINVAL;
	if(aspect_point==NULL)
	{
		// build a new flow_trace
		aspect_point=malloc(sizeof(struct expand_aspect_point));
		aspect_point->data_size=0;
		aspect_point->record_num=1;
		memcpy(aspect_point->tag,"APRE",4);
		aspect_point->aspect_proc=dup_str(proc,DIGEST_SIZE*2);
		aspect_point->aspect_point=dup_str(point,DIGEST_SIZE*2);
		message_add_expand(message,aspect_point);	
	}
	else
	{
		int curr_offset=aspect_point->record_num*DIGEST_SIZE*2;
		char * buffer=malloc(curr_offset+DIGEST_SIZE*2);
		memcpy(buffer,aspect_point->aspect_proc,curr_offset);
		len=strlen(proc);
		if(len<DIGEST_SIZE*2)
			memcpy(buffer+curr_offset,proc,len+1);
		else
			memcpy(buffer+curr_offset,proc,DIGEST_SIZE*2);
		free(aspect_point->aspect_proc);
		aspect_point->aspect_proc=buffer;

		curr_offset=aspect_point->record_num*DIGEST_SIZE*2;
		buffer=malloc(curr_offset+DIGEST_SIZE*2);
		memcpy(buffer,aspect_point->aspect_point,curr_offset);
		len=strlen(point);
		if(len<DIGEST_SIZE*2)
			memcpy(buffer+curr_offset,point,len+1);
		else
			memcpy(buffer+curr_offset,point,DIGEST_SIZE*2);
		free(aspect_point->aspect_point);
		aspect_point->aspect_point=buffer;

		aspect_point->record_num++;

		message_replace_define_expand(message,aspect_point,"APRE");
	}
	return 0;
}

int router_check_sitestack(void * message,char * type)
{
	struct expand_flow_trace * flow_trace;
	int ret;
	ret=message_get_define_expand(message,&flow_trace,type);
	if(ret<0)
		return -EINVAL;
	if(flow_trace==NULL)
		return 0;
	return flow_trace->record_num;
}
int router_pop_site(void * message, char * type)
{
	struct expand_flow_trace * flow_trace;
	int ret;
	int len;
	if(strcmp(type,"FTRE")!=0)
			return -EINVAL;

	MESSAGE_HEAD * msg_head =get_message_head(message);

	ret=message_get_define_expand(message,&flow_trace,type);
	if(ret<0)
		return -EINVAL;
	if(flow_trace==NULL)
		return 0;
	if(flow_trace->record_num<1)
		return 0;

	int curr_offset=(--(flow_trace->record_num))*DIGEST_SIZE*2;
	memcpy(msg_head->receiver_uuid,flow_trace->trace_record+curr_offset,DIGEST_SIZE*2);

	if(flow_trace->record_num==0)
	{
		free(flow_trace->trace_record);
		message_remove_expand(message,type,&flow_trace);
	}
	return 1;
}

int router_pop_aspect_site(void * message, char * proc)
{
	struct expand_aspect_point * aspect_point;
	int ret;
	int len;

	MESSAGE_HEAD * msg_head =get_message_head(message);

	ret=message_get_define_expand(message,&aspect_point,"APRE");
	if(ret<0)
		return -EINVAL;
	if(aspect_point==NULL)
		return 0;
	if(aspect_point->record_num<1)
		return 0;

	int curr_offset=(--(aspect_point->record_num))*DIGEST_SIZE*2;
	memcpy(msg_head->receiver_uuid,aspect_point->aspect_point+curr_offset,DIGEST_SIZE*2);
	memcpy(proc,aspect_point->aspect_proc+curr_offset,DIGEST_SIZE*2);

	if(aspect_point->record_num==0)
	{
		free(aspect_point->aspect_proc);
		free(aspect_point->aspect_point);
		message_remove_expand(message,"APRE",&aspect_point);
	}
	return 1;
}*/
