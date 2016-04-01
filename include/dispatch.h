#ifndef ROUTER_H
#define ROUTER_H

#define DIGEST_SIZE 32

typedef struct tagmatch_rule
{
    int op;
    int area;
    char expand_type[4];
    char * seg;
    char * value;
}__attribute__((packed))MATCH_RULE;

typedef struct tagrouter_rule
{
     int type;
     int state;
     int target_type;
     int define_area;
     char target_expand[4];
     char * target_name;
}__attribute__((packed))ROUTER_RULE;

enum match_op_type
{
    MATCH_OP_AND=0x01,
    MATCH_OP_OR=0x02,
    MATCH_OP_NOT=0x04,
    MATCH_OP_ERR=0xFF,
};

enum message_area_type
{
    MATCH_AREA_HEAD=0x01,
    MATCH_AREA_RECORD=0x02,
    MATCH_AREA_EXPAND=0x04,
    MATCH_AREA_ERR=0xFF,
};


enum message_target_type
{
    MSG_TARGET_LOCAL=0x01,
    MSG_TARGET_NAME=0x02,
    MSG_TARGET_UUID=0x04,
    MSG_TARGET_RECORD=0x08,
    MSG_TARGET_EXPAND=0x10,
    MSG_TARGET_SPLIT=0x20,
    MSG_TARGET_CONN=0x40,
    MSG_TARGET_MIXUUID=0x80,
    MSG_TARGET_ERROR=0xFFFF,
};

int router_policy_init();
int router_read_cfg(char * filename);

int router_policy_getfirst(void ** policy);
int router_policy_getnext(void ** policy);
int aspect_policy_getfirst(void ** policy);
int aspect_policy_getnext(void ** policy);

int router_policy_gettype(void * policy);
int router_policy_match_message(void * policy,void * message,char * sender_proc);
int router_find_match_policy(void * message,void **msg_policy,char * sender_proc);
int router_find_local_policy(void * message,void **msg_policy,char * sender_proc);
int router_find_aspect_policy(void * message,void **msg_policy,char * sender_proc);
int router_find_aspect_local_policy(void * message,void **msg_policy,char * sender_proc);

void * router_get_first_duprule(void * policy);
void * router_get_next_duprule(void * policy);
int router_set_dup_flow(void * message,void * dup_rule,void **dup_msg);
int router_push_site(void * message,char * name,char * type);
int router_push_aspect_site(void * message,char * proc,char * point);
int router_check_sitestack(void * message,char * type);
int router_pop_site(void * message, char * type);
int router_pop_aspect_site(void * message, char * proc);
int router_dup_activemsg_info (void * message);


#endif // ROUTER_H
