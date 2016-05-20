
#ifndef DISPATCH_STRUCT_H
#define DISPATCH_STRUCT_H

#define MATCH_FLAG 0x8000

/*
enum dispatch_match_op
{
	MATCH_AND=0x01,
	MATCH_OR,
	MATCH_NOT,
};

enum dispatch_area_op
{
	MSG_AREA_HEAD=0x01,
	MSG_AREA_RECORD,
	MSG_AREA_EXPAND,
};

enum dispatch_target_type
{
    MSG_TARGET_LOCAL=0x01,
    MSG_TARGET_NAME,
    MSG_TARGET_UUID,
    MSG_TARGET_RECORD,
    MSG_TARGET_EXPAND,
    MSG_TARGET_CONN,
    MSG_TARGET_MIXUUID,
    MSG_TARGET_ERROR=0xFFFF,
};*/

typedef struct policy_list
{	
	Record_List head;
	int state;
	int policy_num;
	void * curr;
}POLICY_LIST;

typedef struct tagmatch_rule
{
	int op;
	int area;
	int type;
	int subtype;
	void * match_template;
	void * value;
}__attribute__((packed)) MATCH_RULE;

static NAME2VALUE match_op_type_list[] =
{
	{"AND",DISPATCH_MATCH_AND},
	{"OR",DISPATCH_MATCH_OR},
	{"NOT",DISPATCH_MATCH_NOT},
	{NULL,0},
};

static NAME2VALUE message_area_type_list[] =
{
	{"HEAD",MATCH_AREA_HEAD},
	{"RECORD",MATCH_AREA_RECORD},
	{"EXPAND",MATCH_AREA_EXPAND},
	{NULL,0},
};

static NAME2VALUE router_target_type_list[] =
{
	{"LOCAL",ROUTER_TARGET_LOCAL},
	{"NAME",ROUTER_TARGET_NAME},
	{"UUID",ROUTER_TARGET_UUID},
	{"RECORD",ROUTER_TARGET_RECORD},
	{"EXPAND",ROUTER_TARGET_EXPAND},
	{"CHANNEL",ROUTER_TARGET_CHANNEL},
	{"PORT",ROUTER_TARGET_PORT},
	{"MIXUUID",ROUTER_TARGET_MIXUUID},
	{"ERROR",ROUTER_TARGET_ERROR},
	{NULL,0}
};

static struct struct_elem_attr match_rule_desc[] =
{
	{"op",CUBE_TYPE_ENUM,0,&match_op_type_list},
	{"area",CUBE_TYPE_ENUM,0,&message_area_type_list},
	{"type",CUBE_TYPE_RECORDTYPE,sizeof(int),NULL},
	{"subtype",CUBE_TYPE_RECORDSUBTYPE,sizeof(int),NULL},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL}
};

static NAME2VALUE message_flow_type_valuelist[]=
{
	{"INIT",MSG_FLOW_INIT},
	{"LOCAL",MSG_FLOW_LOCAL},
	{"DELIVER",MSG_FLOW_DELIVER},
	{"QUERY",MSG_FLOW_QUERY},
	{"RESPONSE",MSG_FLOW_RESPONSE},
	{"ASPECT",MSG_FLOW_ASPECT},
	{"ASPECT_LOCAL",MSG_FLOW_ASPECT_LOCAL},
	{"ASPECT_RETURN",MSG_FLOW_ASPECT_RETURN},
	{"TRANS",MSG_FLOW_TRANS},
	{"DRECV",MSG_FLOW_DRECV},
	{"QRECV",MSG_FLOW_QRECV},
	{"FINISH",MSG_FLOW_FINISH},
	{"ERROR",MSG_FLOW_ERROR},
	{"NULL",0},
};


typedef struct tagrouter_rule
{
	int type;
	int state;
	int target_type;
	int target_mode;
	char * target_name;
}__attribute__((packed)) ROUTER_RULE;

struct struct_elem_attr router_rule_desc[] =
{
	{"type",CUBE_TYPE_ENUM,0,&message_flow_type_valuelist},
	{"state",CUBE_TYPE_FLAG,0,&message_flow_type_valuelist},
	{"target_type",CUBE_TYPE_ENUM,0,&router_target_type_list},
	{"target_mode",CUBE_TYPE_ENUM,0,sizeof(int),NULL},
	{"target_name",CUBE_TYPE_ESTRING,DIGEST_SIZE*4,NULL},
	{NULL,CUBE_TYPE_ENDDATA,0,NULL}
};
typedef struct tagdispatch_rule
{
	char sender[DIGEST_SIZE];
	POLICY_LIST match_list;
	POLICY_LIST router_list;
}  DISPATCH_RULE;

#endif // DISPATCH_STRUCT_H
