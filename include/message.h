/*************************************************
*       project:        973 trust demo, zhongan secure os 
*                       and trust standard verify
*	name:		message_struct.h
*	write date:    	2011-08-04
*	auther:    	Hu jun
*       content:        this file describe the module's extern struct 
*       changelog:       
*************************************************/
#ifndef _OS210_MESSAGE_H
#define _OS210_MESSAGE_H

#include "data_type.h"

enum message_box_state
{
    MSG_BOX_INIT=1,  // just init a msg_box
    MSG_BOX_ADD,   // finishing the msg's loading
    MSG_BOX_EXPAND,	//begin to add msg's record , at that time the message head can't be set
    MSG_BOX_DEAL,	//begin to add msg's expand , at that time the message head can't be set  ,and we can't add record again
    MSG_BOX_LOADDATA=0x1000,    // read data to the msg box, if all the data were read,change state to MSG_BOX_REBUILD
    MSG_BOX_REBUILDING,
    MSG_BOX_RECOVER,  	//finish the record set and expand set, now the message can output data
    MSG_BOX_CUT, // load message box's head
    MSG_BOX_READ,
    MSG_BOX_ERROR=0xffff,
};

enum message_flow_type
{
    MSG_FLOW_INIT=0x01,
    MSG_FLOW_LOCAL=0x02,
    MSG_FLOW_DELIVER=0x04,
    MSG_FLOW_QUERY=0x08,
    MSG_FLOW_RESPONSE=0x10,
    MSG_FLOW_ASPECT=0x020,
    MSG_FLOW_ASPECT_LOCAL=0x040,
    MSG_FLOW_ASPECT_RETURN=0x080,
    MSG_FLOW_WAIT=0x0400,
    MSG_FLOW_TRANS=0x1000,
    MSG_FLOW_DRECV=0x2000,
    MSG_FLOW_QRECV=0x4000,
    MSG_FLOW_FINISH=0x8000,
    MSG_FLOW_ERROR=0xFFFF,
};

enum message_flag
{
	MSG_FLAG_INTERNAL=0x01,
	MSG_FLAG_CRYPT=0x02,
	MSG_FLAG_SIGN=0x04,
	MSG_FLAG_ZIP=0x08,
	MSG_FLAG_VERIFY=0x10,
};

typedef struct tagMessage_Head  //ǿ�Ʒ��ʿ��Ʊ��
{
   char tag[4];            // should be "MESG" to indicate that this information is a message's begin 
   int  version;          //  the message's version, now is 0x00010001
   char sender_uuid[DIGEST_SIZE];     // sender's uuid, or '@' followed with a name, or ':' followed with a connector's name
   char receiver_uuid[DIGEST_SIZE];   // receiver's uuid, or '@" followed with a name, or ':' followed with a connector's name
   int  flow;
   int  state;
   int  flag;
   char record_type[4];
   char record_subtype[4];
   int  record_num;
   int  record_size;
   int  expand_num;   
   int  expand_size;
   BYTE nonce[DIGEST_SIZE];
} __attribute__((packed)) MSG_HEAD;

typedef struct tagMessage_Expand_Data  //general expand data struct
{
   int  data_size;   //this expand data's size
   char tag[4];      //expand data's type
   BYTE data[0];
} __attribute__((packed)) MSG_EXPAND;

typedef struct expand_extra_info  //expand data struct to store one or more DIGEST_SIZE info
{
   int  data_size;   //this expand data's size
   char tag[4];      //expand data's type, 
   BYTE  uuid[DIGEST_SIZE];
} __attribute__((packed)) MSGEX_UUID;

struct connect_login
{
	char user[DIGEST_SIZE];
	char passwd[DIGEST_SIZE];
	char nonce[DIGEST_SIZE];
} __attribute__((packed));

struct connect_return
{
	int retval;
	int ret_data_size;
	BYTE * ret_data;
	char nonce[DIGEST_SIZE];
} __attribute__((packed));

struct request_cmd
{
	char tag[4];
	char etag[4];
	long long curr_time;
	long long last_time;
	char  uuid[DIGEST_SIZE*2];
} __attribute__((packed));


int message_get_state(void * message);

void * message_init(char * tag, int version);
int message_record_init(void * message);
void message_free(void * message);
int message_free_blob(void * message);

void * message_get_activemsg(void * message);
const char * message_get_recordtype(void * message);
const char * message_get_sender(void * message);
const char * message_get_receiver(void * message);
int message_set_receiver(void * message,const char * receiver_uuid);

void * message_create(char * type,void * active_msg);
void * message_clone(void * message);
int  message_add_record(void * message,void * record);
int  message_add_record_blob(void * message,int record_size, BYTE * record);


int  message_add_expand(void * message,void * expand);
int  message_add_expand_blob(void * message,void * expand);

int  message_remove_expand(void * message,char * type,void ** expand);
int  message_remove_indexed_expand(void * message,int index,void ** expand) ;


int  message_get_record(void * message, void ** msg_record,int record_no);
int  message_get_expand(void * message, void ** msg_expand,int expand_no);


void * load_record_template(char * type);
void * load_record_desc(char * type);
int read_message_from_blob(void * message,void * blob,int blob_size);
int read_message_head(void * message,void * blob,int blob_size);
int read_message_data(void * message,void * blob,int data_size);
int read_message_from_src(void * message,void * src,
           int (*read_func)(void *,char *,int size));

int read_message_head_elem(void * message,char * item_name, void * value);

int load_message_record(void * message,void ** record); 
int load_all_record(void * message); 
// load next record from message , it should be called after read_message_data finished, or load_message_expand or another load_message_record;
// that is, message_box's state must be MSG_BOX_LOAD,MSG_BOX_LOAD_EXPAND or MSG_BOX_LOAD_RECORD
// if load record success, function return value is 1 and the message_box's state is MSG_BOX_LOAD_RECORD 
// if all the record in message are loaded, function return 0 and the message_box's state is MSG_BOX_LOAD_FINISH
// if load message error, function return negtive value

int set_message_head(void * message,char * item_name, void * value);
int message_set_flag(void * message, int flag);
int message_set_state(void * message, int state);
int message_get_state(void * message);
int message_set_flow(void * message, int flow);
int message_get_flow(void * message);
int message_get_flag(void * message);
int message_read_elem(void * message,char * item_name, int index, void ** value);
int message_comp_head_elem_text(void * message,char * item_name, char * text);
int message_comp_elem_text(void * message,char * item_name, int index, char * text);
int message_comp_expand_elem_text(void * message,char * item_name, int index, char * text);

MSG_HEAD * get_message_head(void * message);
int message_output_blob(void * message, BYTE ** blob);
int message_output_text(void * message, char * text);

//   //  _________________________________________________________________________ //

int message_load_record(void * message);
int message_load_expand(void * message);

int message_get_define_expand(void * message,void ** addr,char * type);
int add_message_define_expand(void * message, void * expand, char * type);

int add_message_expand_state(void * message,int state_machine_no,int state);
int add_message_expand_identify(void * message,char * user_name,int type,int blob_size,BYTE * blob);
int add_message_expand_forward(void * message,char * sender_uuid,char * sender_name,char * receiver_uuid, char * receiver_name);

int add_message_expand(void * message, int record_size, int expand_no,BYTE * expand);

char * get_message_expand_type(void * message,int expand_no);
int  get_message_expand(void * message, void ** msg_expand,int expand_no);

//void * build_server_syn_message(char * service,char * local_uuid,char * proc_name);
//void * build_client_ack_message(void * message_box,char * local_uuid,char * proc_name,void * temp_conn);
int message_get_blob(void * message, void ** blob);
int message_set_blob(void * message,void * blob, int size);

int message_record_blob2struct(void * message);
int message_record_struct2blob(void * message);
int message_expand_struct2blob(void * message);
int message_2_json(void * message,char * json_str);
int json_2_message(char * json_str,void ** message);
int message_output_record_blob(void * message, BYTE ** blob);

#endif
