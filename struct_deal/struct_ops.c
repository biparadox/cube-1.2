#ifdef KERNEL_MODE

#include <linux/string.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>

#else

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<time.h>
//#include "../include/kernel_comp.h"
//#include "../include/list.h"

#endif

#include "../include/data_type.h"
#include "../include/alloc.h"
#include "../include/struct_deal.h"
#include "struct_ops.h"


static inline int elem_alloc(void ** pointer,int size)
{
	if(Tisinmem(pointer))
	{
		pointer=Talloc(size);
		if(pointer==NULL)
			return -ENOMEM;
		return 0;
	}
	return Galloc(pointer,size);
}

int string_elem_2_blob(void * addr, void * elem_data,  void * elem_attr);
int string_blob_2_elem(void * addr, void * elem_data,  void * elem_attr);

int estring_elem_2_blob(void * addr, void * elem_data,  void * elem_attr);
int estring_blob_2_elem(void * addr, void * elem_data,  void * elem_attr);


int estring_alloc_size(void * value,void * elem_attr);

//}
ELEM_OPS string_convert_ops =
{
//	.calculate_offset=Calculate_offset,
};
ELEM_OPS bindata_convert_ops =
{
//	.calculate_offset=Calculate_offset,
};

ELEM_OPS estring_convert_ops =
{
	.elem_2_blob = estring_elem_2_blob,
	.blob_2_elem = estring_blob_2_elem,
	.elem_alloc_size = estring_alloc_size,
//	.calculate_offset=Calculate_offset,
};

/*
int string_elem_2_blob(void * addr, void * elem_data,  void * elem_template){

	struct struct_elem_attr * elem_attr=elem_template;
	int retval;
	retval = elem_attr->size;
	memset(elem_data, 0, sizeof(elem_attr->size));
	strncpy(elem_data, addr, retval);
	return retval;

}
*/
int estring_elem_2_blob(void * addr,void * elem_data,void * elem_template){
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
//			if ((elem_desc->size != 0) && (retval>elem_desc->size))
//				return -EINVAL;
			retval++;
			memcpy(elem_data, estring, retval);
		}

		return retval;
}

int estring_alloc_size (void * value,void * attr)
{

	struct elem_template * elem_attr=attr;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int retval;
	retval = strlen(value);
	if (retval<0)
		retval = -EINVAL;
//	if ((elem_desc->size != 0) && (retval>elem_desc->size))
//		retval = -EINVAL;
	retval++;
	return retval;
	
}


int estring_blob_2_elem(void * addr, void * elem_data, void * elem_template){

	struct elem_template * elem_attr=elem_template;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int retval;
	char * estring;
	retval = estring_alloc_size(elem_data,elem_template);
	if (retval<0)
		return retval;
	int ret = elem_alloc(addr,retval);
	if (ret <0)
		return -ENOMEM;
	memcpy(*(char **)addr, elem_data, retval);

	return retval;

}
