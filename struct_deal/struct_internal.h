#include"../include/data_type.h"
#include"../include/list.h"
#include"../include/attrlist.h"
#define os210_print_buf 4096
#define OS210_MAX_BUF   4096

//#define nulstring "NULL"

UINT16 Decode_UINT16(BYTE * in);

void UINT32ToArray(UINT32 i, BYTE * out);

void UINT64ToArray(UINT64 i, BYTE *out);

void UINT16ToArray(UINT16 i, BYTE * out);

UINT64 Decode_UINT64(BYTE *y);

UINT32 Decode_UINT32(BYTE * y);

#define  MAX_ARRAY_ELEM_NUM  128

enum json_solve_state
{
    JSON_SOLVE_INIT,
    JSON_SOLVE_PROCESS,
    JSON_SOLVE_FINISH,
    JSON_VALUE_TRANS
};

enum solve_state
{
    SOLVE_INIT,
    SOLVE_ARRAY,
    SOLVE_MAP,
    SOLVE_NAME,
    SOLVE_VALUE,
    SOLVE_UPGRADE,
    SOLVE_FINISH
};



typedef struct json_value_struct
{
     int value_type;
     char * value_str;
     void * value;
}JSON_VALUE;

typedef struct json_elem_node
{
    int elem_type;            //  this json elem's type, it can be
                             //  NUM,STRING,BOOL,MAP,ARRAY or NULL

    char name[DIGEST_SIZE*2]; // this json elem's name,
                              // if this json elem is the root elem,
                              //  its name is "__ROOT__"
    int solve_state;                // this json elem's state, it should
                              // be JSON_SOLVE_INIT,JSON_SOLVE_PROCESS
                              // ,JSON_SOLVE_FINISH and JSON_VALUE_TRANS;
    char * elem_str;          // this json elem's string
    char * value_str;         // this json elem's value string
    int  json_strlen;         //  if solve_state is JSON_SOLVE_PROCESS,
                              //  it is this json elem str's offset, if
                              // solve_state is JSON_SOLVE_FINISH,it is
                              // this json elem str's length                         // this
    int layer;                //
    int elem_no;              //
    Record_List childlist;    // this json's child list
    Record_List * curr_child; // when this json elme is solved, this
                              // pointer point to the child it was
                              // processing.
    struct json_elem_node * father;
}JSON_NODE;

const char * nulstring;
