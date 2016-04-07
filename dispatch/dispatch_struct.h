
#ifndef ROUTER_STRUCT_H
#define ROUTER_STRUCT_H

enum match_operation
{
	DISPATCH_MATCH_PARALLEL,
	DISPATCH_MATCH_SERIAL,
};

enum message_area_type
{
    MATCH_AREA_HEAD=0x01,
    MATCH_AREA_RECORD=0x02,
    MATCH_AREA_EXPAND=0x04,
    MATCH_AREA_ERR=0xFF,
};

typedef struct tagmatch_rule
{
	int op;
	int area;
	int type;
	int subtype;
	void * match_template;
	void * value;
};

static struct struct_elem_attr match_rule_desc[] =
{
    {"op",OS210_TYPE_ENUM,sizeof(int),&match_rule_op_valuelist},
    {"area",OS210_TYPE_ENUM,sizeof(int),&message_area_valuelist},
    {"expand_type",OS210_TYPE_STRING,4,NULL},
    {"seg",OS210_TYPE_ESTRING,DIGEST_SIZE*2,NULL},
    {"value",OS210_TYPE_ESTRING,1024,NULL},
	{NULL,OS210_TYPE_ENDDATA,0,NULL}
};

static struct struct_elem_attr router_rule_desc[] =
{
    {"type",OS210_TYPE_FLAG,sizeof(int),&message_flow_valuelist},
    {"state",OS210_TYPE_ENUM,sizeof(int),&message_flow_valuelist},
    {"target_type",OS210_TYPE_ENUM,sizeof(int),&message_target_type_valuelist},
    {"define_area",OS210_TYPE_ENUM,sizeof(int),&message_area_valuelist},
    {"target_expand",OS210_TYPE_STRING,4,NULL},
    {"target_name",OS210_TYPE_ESTRING,DIGEST_SIZE*2,NULL},
    {NULL,OS210_TYPE_ENDDATA,0,NULL}
};

struct expand_flow_trace
{
    int  data_size;
    char tag[4];                 // this should be "FTRE" and "APRE"
    int  record_num;
    char *trace_record;
} __attribute__((packed));

struct expand_aspect_point
{
    int  data_size;
    char tag[4];                 // this should be "APRE"
    int  record_num;
    char * aspect_proc;
    char * aspect_point;
} __attribute__((packed));

static struct struct_elem_attr expand_flow_trace_desc[] =
{
    {"data_size",OS210_TYPE_INT,sizeof(int),0},
    {"tag",OS210_TYPE_STRING,4,0},
    {"record_num",OS210_TYPE_INT,sizeof(int),0},
    {"trace_record",OS210_TYPE_DEFSTRARRAY,DIGEST_SIZE*2,"record_num"},
    {NULL,OS210_TYPE_ENDDATA,0,NULL}
};

static struct struct_elem_attr expand_aspect_point_desc[] =
{
    {"data_size",OS210_TYPE_INT,sizeof(int),0},
    {"tag",OS210_TYPE_STRING,4,0},
    {"record_num",OS210_TYPE_INT,sizeof(int),0},
    {"aspect_proc",OS210_TYPE_DEFSTRARRAY,DIGEST_SIZE*2,"record_num"},
    {"aspect_point",OS210_TYPE_DEFSTRARRAY,DIGEST_SIZE*2,"record_num"},
    {NULL,OS210_TYPE_ENDDATA,0,NULL}
};

#endif // ROUTER_STRUCT_H
