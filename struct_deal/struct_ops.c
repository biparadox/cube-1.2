#include <string.h>
#include "../include/errno.h"
#include "../include/data_type.h"
#include "../include/alloc.h"
//#include "../include/string.h"
#include "../include/struct_deal.h"
#include "struct_ops.h"
#include "struct_attr.h"


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
		len=Strlen(src)+1;
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
	Memcpy(*dest,src,len);
	return len;			
}

int estring_get_length (void * value,void * attr)
{

	struct elem_template * elem_attr=attr;
	struct struct_elem_attr * elem_desc = elem_attr->elem_desc;
	int retval;
	retval = Strlen(value);
	if (retval<0)
		retval = -EINVAL;
	retval++;
	return retval;
}

int _digest_to_uuid(BYTE * digest, char * uuid)
{
	BYTE char_value;
	int i,j,k;
	k=0;
	for(i=0;i<DIGEST_SIZE;i++)
	{
		int tempdata;
		char_value=digest[i];

		for(j=0;j<2;j++)
		{
			tempdata=char_value>>4;
			if(tempdata>9)
				*(uuid+k)=tempdata-10+'a';
			else
				*(uuid+k)=tempdata+'0';
			k++;
			if(j!=1)
				char_value<<=4;
				
		}
	}
	return DIGEST_SIZE*2;
}

int _uuid_to_digest(char * uuid,BYTE * digest)
{
	int i;
	BYTE char_value;
	for(i=0;i<DIGEST_SIZE*2;i++)
	{
		if((uuid[i]>='0')&&(uuid[i]<='9'))
			char_value=uuid[i]-'0';
		else if((uuid[i]>='a')&&(uuid[i]<='f'))
			char_value=uuid[i]-'a'+10;
		else if((uuid[i]>='A')&&(uuid[i]<='F'))
			char_value=uuid[i]-'A'+10;
		else
			return -EINVAL;
		if(i%2==0)
			digest[i/2]=char_value<<4;
		else
			digest[i/2]+=char_value;
	}
	return 0;
}

int uuid_get_text_value(void * addr, void * data,void * elem_template)
{
	return _digest_to_uuid(addr,data);
}


int uuid_set_text_value(void * addr, char * text,void * elem_template)
{
	return _uuid_to_digest(text,addr);
}
int uuidarray_get_text_value(void * addr, void * data,void * elem_template)
{
	int i,j,retval;
	char * text=data;
	struct elem_template * curr_elem=elem_template;
	BYTE * digest=*(BYTE **)addr;
	int offset=0;
	int array_num=(int)curr_elem->elem_desc->ref;
	
	if((array_num<0)|| (array_num>DIGEST_SIZE*2))
		return -EINVAL;
	for(i=0;i<array_num;i++)
	{
		if(i!=0)
			*(text+offset++)=',';
		_digest_to_uuid(digest+i*DIGEST_SIZE,text+offset);
		offset+=DIGEST_SIZE*2;
	}	

	*(text+offset)=0;
	return offset+1;
}

int uuidarray_set_text_value(void * addr, char * text,void * elem_template)
{
	int i,j,retval;
	struct elem_template * curr_elem=elem_template;
	int offset=0;
	int array_num=(int)curr_elem->elem_desc->ref;
	
	if((array_num<0)|| (array_num>DIGEST_SIZE*2))
		return -EINVAL;
	retval=Palloc0(addr,DIGEST_SIZE*array_num);
	if(retval<0)
		return retval;
	BYTE * digest=*(BYTE **)addr;
	for(i=0;i<array_num;i++)
	{
		while((*(text+offset)==',')||(*(text+offset)==' '))
			offset++;
		retval=_uuid_to_digest(text+offset,digest+i*DIGEST_SIZE);
		if(retval<0)
			return retval;
		offset+=DIGEST_SIZE*2;
	}	
	return offset;
}

int defuuidarray_get_text_value(void * addr, void * data,void * elem_template)
{
	char * text=data;
	struct elem_template * curr_elem=elem_template;
	BYTE * digest=addr;
	int offset=0;
	_digest_to_uuid(digest,text+offset);
	offset+=DIGEST_SIZE*2;

	*(text+offset++)=',';
	*(text+offset)=0;
	return offset+1;
}

int defuuidarray_set_text_value(void * addr, char * text,void * elem_template)
{
	int retval;
	struct elem_template * curr_elem=elem_template;
	int offset=0;

	BYTE * digest=addr;

	while((*(text+offset)==',')||(*(text+offset)==' '))
		offset++;
	retval=_uuid_to_digest(text+offset,digest);
	if(retval<0)
		return retval;
	offset+=DIGEST_SIZE*2;
	return offset;
}

int namelist_get_bin_value(void * addr, void * data,void * elem_template)
{
	NAME2VALUE * namelist=addr;

	int addroffset=0;
	int offset=0;
	struct elem_template * curr_elem=elem_template;
	int textlen=0;
	textlen=Strlen(*(char **)namelist);
	if(textlen>DIGEST_SIZE)
		return -EINVAL;
	Memcpy(data,*(char **)namelist,textlen+1);
	
	offset+=textlen+1;
	Memcpy(data+offset,&namelist->value,sizeof(int));
	offset+=sizeof(int);
	return offset;
}

int namelist_set_bin_value(void * addr, void * data,void * elem_template)
{
	int retval;
	NAME2VALUE * namelist=addr;
	struct elem_template * curr_elem=elem_template;
	int offset=0;
	int addroffset=0;
	int textlen=0;
	textlen=Strlen((char *)data);
	if(textlen>DIGEST_SIZE)
		return -EINVAL;
	retval=Palloc0(namelist,textlen+1);
	if(retval<0)
		return -ENOMEM;
	Memcpy(*(char **)namelist,data+offset,textlen+1);
	offset+=textlen+1;
	Memcpy(&namelist->value,data+offset,sizeof(int));
	offset+=sizeof(int);
	return offset;
}

int namelist_get_text_value(void * addr, void * data,void * elem_template)
{
	NAME2VALUE * namelist=addr;
	int addroffset=0;
	int offset=0;
	char * text=data;
	char * name;
	int  value;
	struct elem_template * curr_elem=elem_template;
	int textlen=0;

	textlen=Strlen(namelist->name);
	if(textlen>DIGEST_SIZE)
		return -EINVAL;
	Memcpy(text+offset,namelist->name,textlen);
	offset+=textlen;
	value=namelist->value;

	curr_elem->index++;
	if(value<curr_elem->index)
		return -EINVAL;
	if(value>curr_elem->index)
	{
		text[offset++]='=';
		textlen=Itoa(value,data+offset);
		if(textlen<0)
			return textlen;
		offset+=textlen;
		curr_elem->index=value;
	}
	text[offset++]=',';
	text[offset++]=0;
	return offset;
}

int namelist_set_text_value(void * addr, char * text,void * elem_template)
{
	int i,j,retval;
	NAME2VALUE * namelist=addr;
	struct elem_template * curr_elem=elem_template;
	int offset=0;
	int addroffset=0;
	int textlen=0;
	int namelen=0;
	char * name;
	int  value;

	char buf[128];

	textlen=Getfiledfromstr(buf,text+offset,',',128);
	if(textlen>DIGEST_SIZE*2)
		return -EINVAL;
	namelen=Getfiledfromstr(buf+DIGEST_SIZE*2,buf,'=',textlen);
	curr_elem->index++;
	if(namelen==textlen)
	{
		value=curr_elem->index;
	}
	else
	{
		value=Atoi(buf+namelen+1,textlen-namelen);
		if(value<curr_elem->index)
			return -EINVAL;
		curr_elem->index=value;
	}
	retval=Palloc0(&(namelist->name),namelen+1);
	if(retval<0)
		return -ENOMEM;
	Memcpy(namelist->name,text+offset,namelen);
	*(namelist->name+namelen)=0;
	offset+=textlen+1;
	namelist->value=value;
	return offset;
}

int define_get_text_value(void * addr,char * text,void * elem_template){
	char * blob = *(char **)addr;
	struct elem_template * curr_elem=elem_template;
	int def_value;

	def_value=_elem_get_defvalue(curr_elem,addr);
	if(def_value<=0)
		return def_value;
	Memcpy(text,blob,def_value);
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
	if(curr_elem->elem_desc->type == CUBE_TYPE_STRING)
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

int int_get_text_value(void * elem,char * text,void * elem_attr)
{
	struct elem_template * curr_elem=elem_attr;
	int i;
	long long value=0;
	int len;
	char buffer[DIGEST_SIZE];
	Memcpy(&value,elem,curr_elem->size);
	i=1;
	len=2;
	char *pch=text;
	while(value/i)
	{
		i*=10;
		len++;
	}
	if(value==0)
		i=10;
	while(i/=10)
	{
		*pch++=value/i+'0';
		value%=i;
	}	
	*pch='\0';
	return len;
}
int int_set_text_value(void * addr,char * text,void * elem_attr)
{
	struct elem_template * curr_elem=elem_attr;
	char * string=text;
	int ret=0;
	int i;
	int base=10;
	int temp_value;

	int str_len;

	str_len=strnlen(string,DIGEST_SIZE);

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
					ret=0;
					Memcpy(addr,&ret,curr_elem->size);
					return str_len+1;
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
		}
		break;
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
	Memcpy(addr,&ret,curr_elem->size);
	return str_len+1;
}

ELEM_OPS string_convert_ops =
{
	.get_int_value=get_string_value,
};

ELEM_OPS bindata_convert_ops =
{
};

ELEM_OPS uuid_convert_ops =
{
	.get_text_value = uuid_get_text_value,
	.set_text_value = uuid_set_text_value,
};
ELEM_OPS uuidarray_convert_ops =
{
	.get_text_value = uuidarray_get_text_value,
	.set_text_value = uuidarray_set_text_value,
};
ELEM_OPS defuuidarray_convert_ops =
{
	.get_text_value = defuuidarray_get_text_value,
	.set_text_value = defuuidarray_set_text_value,
};
ELEM_OPS estring_convert_ops =
{
	.elem_get_length = estring_get_length,
};
ELEM_OPS define_convert_ops =
{
};

ELEM_OPS int_convert_ops =
{
	.get_int_value=get_int_value,
	.get_text_value = int_get_text_value,
	.set_text_value = int_set_text_value,
};
ELEM_OPS defnamelist_convert_ops =
{
	.get_bin_value= namelist_get_bin_value,
	.set_bin_value= namelist_set_bin_value,
	.get_text_value= namelist_get_text_value,
	.set_text_value= namelist_set_text_value,
};

