#include <string.h>
#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/alloc.h"
//#include "../include/string.h"
#include "../include/struct_deal.h"
#include "struct_ops.h"


//const int deftag=0x00FFF000;

int dup_str(char ** dest,char * src, int size)
{
	int len;
	int ret;
	*dest=NULL;
	if(src==NULL)
		return 0;
	if(size == 0)
	{
		len=strlen(src)+1;
	}
	else
	{
		len=strnlen(src,size);
		if(len!=size)
			len++;
	}
	ret=Palloc((void **)dest,len);
	if(ret<0)
		return ret;
	memcpy(*dest,src,len);
	return len;			
}

int string_get_bin_value(void * addr, void * elem_data,  void * elem_attr);
int string_set_bin_value(void * addr, void * elem_data,  void * elem_attr);

int estring_get_bin_value(void * addr, void * elem_data,  void * elem_attr);
int estring_set_bin_value(void * addr, void * elem_data,  void * elem_attr);


int estring_get_length(void * value,void * elem_attr);

//}

int estring_get_bin_value(void * addr,void * elem_data,void * elem_template){
	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;

	int retval;
	char * estring;
	estring = *(char **)addr;
	// if the string is an empty string
	if (estring == NULL)
		{
			retval = 1;
			memset(elem_data, 0, retval);
		}
		else
		{

			retval = strlen(estring);
			if (retval<0)
				return -EINVAL;
			retval++;
			memcpy(elem_data, estring, retval);
		}

		return retval;
}

int estring_get_length (void * value,void * attr)
{

	struct elem_template * elem_attr=attr;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int retval;
	retval = strlen(value);
	if (retval<0)
		retval = -EINVAL;
	retval++;
	return retval;
}


int estring_set_bin_value(void * addr, void * elem_data, void * elem_template){

	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int retval;
	char * estring;
	retval = estring_get_length(elem_data,elem_template);
	if (retval<0)
		return retval;
	int ret = Palloc(addr,retval);
	if (ret <0)
		return -ENOMEM;
	memcpy(*(char **)addr, elem_data, retval);

	return retval;

}
int estring_get_text_value(void * addr, char * text, void * elem_template){

	char * blob = *(char **)addr;
	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int retval;
	retval=strlen(blob)+1;
	memcpy(text,blob,retval);
	return retval;
}

int define_get_bin_value(void * addr,void * elem_data,void * elem_template){
	struct elem_template * curr_elem=elem_template;
	struct struct_elem_attr * elem_desc = curr_elem->elem_desc;

	struct elem_template * temp_elem;
	int retval;
	int def_offset;
	int def_value;
	ELEM_OPS * elem_ops;
	temp_elem=curr_elem->ref;
	if(temp_elem==NULL)
		return -EINVAL;
	def_offset=temp_elem->offset;

	if(curr_elem->offset <def_offset)
		return -EINVAL;

	elem_ops=struct_deal_ops[temp_elem->elem_desc->type];
	if(elem_ops==NULL)
		return -EINVAL;
	def_value=elem_ops->get_int_value(addr-(curr_elem->offset-def_offset),temp_elem);
	
	if(def_value==0)
		return 0;
	if(def_value<0)
		return -EINVAL;
	unsigned char * defdata;
	defdata = *(unsigned char **)addr;
	// if the string is an empty string
	if (defdata == NULL)
	{
		return -EINVAL;
	}
	retval =def_value;
	memcpy(elem_data, defdata, retval);

	return retval;
}

int define_set_bin_value(void * elem,void * addr,void * elem_template){
	struct elem_template * curr_elem=elem_template;
	struct struct_elem_attr * elem_desc = curr_elem->elem_desc;

	struct elem_template * temp_elem;
	int retval;
	int def_offset;
	int def_value;
	ELEM_OPS * elem_ops;
	temp_elem=curr_elem->ref;
	if(temp_elem==NULL)
		return -EINVAL;

	def_value=(int)temp_elem->ref & 0x00000FFF;
	
	if(def_value==0)
		return 0;
	if(def_value<0)
		return -EINVAL;
	retval = Palloc(addr,def_value);
	if(retval<0)
		return retval;
	unsigned char * defdata;
	defdata = *(unsigned char **)addr;
	// if the string is an empty string
	if (defdata == NULL)
	{
		return -EINVAL;
	}
	memcpy(defdata,addr,def_value);

	return def_value;
}

int define_get_text_value(void * addr,char * text,void * elem_template){
	char * blob = *(char **)addr;
	struct elem_template * curr_elem=elem_template;
	struct struct_elem_attr * elem_desc = curr_elem->elem_desc;

	struct elem_template * temp_elem;
	int retval;
	int def_offset;
	int def_value;
	ELEM_OPS * elem_ops;
	temp_elem=curr_elem->ref;
	if(temp_elem==NULL)
		return -EINVAL;
	def_value=(int)temp_elem->ref & 0x00000FFF;
	
	if(def_value==0)
		return 0;
	memcpy(text,blob,def_value);
	return def_value;
}
static inline int _isdigit(char c)
{
	if((c>='0') && (c<='9'))
		return 1;
	return 0;
}

static inline int _get_char_value(char c)
{
	if(_isdigit(c))
		return c-'0';
	if((c>='a') && (c<='f'))
		return c-'a'+9;
	if((c>='A') && (c<='F'))
		return c-'a'+9;
	return -EINVAL;
}

int get_string_value(void * addr,void * elem_attr)
{
	struct elem_template * curr_elem=elem_attr;
	char * string=addr;
	int ret=0;
	int i;
	int base=10;
	int temp_value;
	int str_len;
	if(curr_elem->elem_desc->type == OS210_TYPE_STRING)
	{
		str_len=strnlen(string,curr_elem->size);
	}
	else
		str_len=strnlen(string,16);

	// process the head
	for(i=0;i<str_len;i++)
	{
		if(string[i]==0)
			break;
		if(string[i]==' ')
		{
			i++;
			continue;
		}
		// change the base
		if(string[i]=='0')
		{
			switch(string[i+1])
			{
				case 0:
					return 0;
				case 'b':
				case 'B':
					i+=2;
					base=2;
					break;
				case 'x':
				case 'X':
					i+=2;
					base=16;
					break;
				default:
					i++;
					base=8;
					break;

			}
			break;
		}
		
	}
	for(;i<str_len;i++)
	{
		if(string[i]==0)
			break;
		temp_value=_get_char_value(string[i]);
		if((temp_value <0)||(temp_value>=base))
			return -EINVAL;
		ret=ret*base+temp_value;		
	}
	return ret;
}
	
int get_int_value(void * addr,void * elem_attr)
{
	return *(int *)addr; 
}

ELEM_OPS string_convert_ops =
{
	.get_int_value=get_string_value,
};
ELEM_OPS int_convert_ops =
{
	.get_int_value=get_int_value,
};
ELEM_OPS bindata_convert_ops =
{
//	.calculate_offset=Calculate_offset,
};

ELEM_OPS estring_convert_ops =
{
	.get_bin_value = estring_get_bin_value,
	.set_bin_value = estring_set_bin_value,
	.get_text_value = estring_get_text_value,
//	.set_text_value = estring_set_text_value,
	.elem_get_length = estring_get_length,
};
ELEM_OPS define_convert_ops =
{
	.get_bin_value = define_get_bin_value,
	.set_bin_value = define_set_bin_value,
	.get_text_value = define_get_text_value,
//	.set_text_value = define_set_text_value,
//	.elem_alloc_size = estring_alloc_size,
};
