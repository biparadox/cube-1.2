
#ifndef DISPATCH_STRUCT_H
#define DISPATCH_STRUCT_H

#define MATCH_FLAG 0x8000

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
	void * match_policy;
	void * router_rule;
}  DISPATCH_POLICY;

#endif // ROUTER_STRUCT_H
