
#ifndef DISPATCH_STRUCT_H
#define DISPATCH_STRUCT_H

#define MATCH_FLAG 0x8000

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
};

typedef struct tagmatch_rule
{
	int op;
	int area;
	int type;
	int subtype;
	void * match_template;
	void * value;
}__attribute__((packed)) MATCH_RULE;

typedef struct tagrouter_rule
{
	int type;
	int target_type;
	int target_mode;
	char * target_name;
}__attribute__((packed)) ROUTER_RULE;

typedef struct tagdispatch_rule
{
	char proc[DIGEST_SIZE];
	void * match_list;
	void * router_list;
}  DISPATCH_RULE;

#endif // DISPATCH_STRUCT_H
