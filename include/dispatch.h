#ifndef ROUTER_H
#define ROUTER_H

#define DIGEST_SIZE 32

enum match_op_type
{
    DISPATCH_MATCH_AND=0x01,
    DISPATCH_MATCH_OR,
    DISPATCH_MATCH_NOT,	
    DISPATCH_MATCH_ERROR,
};

enum message_area_type
{
    MATCH_AREA_NULL=0x01,	
    MATCH_AREA_HEAD=0x02,
    MATCH_AREA_RECORD=0x04,
    MATCH_AREA_EXPAND=0x08,
    MATCH_AREA_ERR=0xFF,
};

enum router_target_type
{
    ROUTER_TARGET_LOCAL=0x01,
    ROUTER_TARGET_NAME=0x02,
    ROUTER_TARGET_UUID=0x04,
    ROUTER_TARGET_RECORD=0x08,
    ROUTER_TARGET_EXPAND=0x10,
    ROUTER_TARGET_CHANNEL=0x40,
    ROUTER_TARGET_PORT=0x80,
    ROUTER_TARGET_MIXUUID=0x100,
    ROUTER_TARGET_ERROR=0xFFFF,
};

int dispatch_policy_init(void * object);
void * dispatch_create_rule(void * object);


void * dispatch_create_match_list(void);
void * dispatch_create_route_list(void);


void * dispatch_read_match_policy(void * policy_node);
int dispatch_add_match_policy(void * list,void * policy);

void * dispatch_read_route_policy(void * policy_node);
int dispatch_add_route_policy(void * list,void * policy);

void * dispatch_read_policy(void * policy_node);
int dispatch_add_policy(void * list,void * policy);

void * match_policy_getfirst(void * list);
void * match_policy_getnext(void * list);


void * router_policy_getfirst(void * list);
void * router_policy_getnext(void * list);

void * dispatch_policy_getfirst();
void * dispatch_policy_getnext();

int match_message(void * match_rule,void * message);

void * dispatch_route_rule_getfirst(void * rule);
void * dispatch_route_rule_getnext(void * policy);
int  	aspect_policy_getfirst(void ** policy);
int aspect_policy_getnext(void ** policy);

int router_find_local_policy(void * message,void **msg_policy,char * sender_proc);
int router_find_aspect_policy(void * message,void **msg_policy,char * sender_proc);
int router_find_aspect_local_policy(void * message,void **msg_policy,char * sender_proc);

int router_push_site(void * message,char * name,char * type);
int router_push_aspect_site(void * message,char * proc,char * point);
int router_check_sitestack(void * message,char * type);
int router_pop_site(void * message, char * type);
int router_pop_aspect_site(void * message, char * proc);
int router_dup_activemsg_info (void * message);


#endif // DISPATCH_H
