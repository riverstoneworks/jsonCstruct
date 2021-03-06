/*
 * Parse_to_json.c
 *
 *  Created on: Aug 2, 2018
 *      Author: Like.Z
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "json_type.h"
#include "JsonCObject/jsonCObject.h"

static int convert(char** const ,size_t *, ObjectInfo* const );

static int numeric_conver(char** const  _string,size_t * str_len, ObjectInfo* const numeric){
	char s[24];
	int n;
	switch (numeric->typeInf->type) {
	case FLOAT:
		n = sprintf(s, "%G", *(float*) (numeric->offset));
		break;
	case DOUBLE:
		n = sprintf(s, "%lG", *(double*) (numeric->offset));
		break;
	case LDOUBLE:
		n = sprintf(s, "%.16LG", *(long double*) (numeric->offset));
		break;
	case INT8:
		n = sprintf(s, "%d", *(char*) (numeric->offset));
		break;
	case UINT8:
		n = sprintf(s, "%d", *(unsigned char*) (numeric->offset));
		break;
	case INT16:
		n = sprintf(s, "%d", *(short*) (numeric->offset));
		break;
	case UINT16:
		n = sprintf(s, "%d", *(unsigned short*) (numeric->offset));
		break;
	case INT32:
		n = sprintf(s, "%d", *(int*) (numeric->offset));
		break;
	case UINT32:
		n = sprintf(s, "%u", *(unsigned int*) (numeric->offset));
		break;
	case UINT64:
		n = sprintf(s, "%llu", *(unsigned long long*) (numeric->offset));
		break;
	case INT64:
		n = sprintf(s, "%lld", *(long long*) (numeric->offset));
		break;
	default:
		return -1;
	}
	if(n<0||n>*str_len){
		puts(strerror(errno));
		return -1;
	}
	strncpy(*_string,s,n);
	*_string+=n;
	*str_len-=n;
	return 0;
}

/* the format of _string: "..."
 * Only the characters encoded by UTF-8  can be accepted
<<<<<<< HEAD
 * UTF-8 code rule:
 * 2 Bytes : 1100xxxx 10xxxxxx
 * 3 Bytes : 1110xxxx 10xxxxxx 10xxxxxx
 * 4 Bytes : 1111xxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 5 Bytes : 11111xxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * ...
=======
>>>>>>> branch 'master' of https://github.com/riverstoneworks/JsonCObject.git
 */
static int string_conver(char** const _string, size_t* str_len, ObjectInfo* const string){
	if(!string->offset){
		if(*str_len>3)
			strncpy(*_string,"null",4);
		else
			return -1;
	}

	size_t l=*str_len-2,n=strlen((const char*)(string->offset));
	int i=0,j=1;
	if(l<n)
		return -1;
	else if(n>0){
		const char* const p=(char*)(string->offset);
		while (i < n && j < l - 1) {
			switch (p[i]) {
			case '\n':
				(*_string)[j++] = '\\';
				(*_string)[j++] = 'n';
				break;
			case '\r':
				(*_string)[j++] = '\\';
				(*_string)[j++] = 'r';
				break;
			case '\t':
				(*_string)[j++] = '\\';
				(*_string)[j++] = 't';
				break;
			case '\f':
				(*_string)[j++] = '\\';
				(*_string)[j++] = 'f';
				break;
			case '\b':
				(*_string)[j++] = '\\';
				(*_string)[j++] = 'b';
				break;
			case '"':
				(*_string)[j++] = '\\';
				(*_string)[j++] = '"';
				break;
			case '\\':
				(*_string)[j++] = '\\';
				(*_string)[j++] = '\\';
				break;
			default:
				(*_string)[j++] = p[i];
				char mask = 0xC0;
				switch (p[i] & mask) {
				case (char)0x80:
					return -1;
					break;
				case (char)0xC0:{
					char t = 0;
					while (t++ < 8) {
						if ((p[i + t] & 0xC0) == 0x80 && j < l)
							(*_string)[j++] = p[i + t];
						else
							return -1;
						mask = (mask >> 1);
						if ((p[i] & mask) != mask)
							break;
					}
					i += t;
					break;
				}default:;
				}
			}
			++i;
		}
		if(i<n)
			return -1;
	}

	(*_string)[0]=(*_string)[j++]='"';
	*_string+=j;
	*str_len-=j;
	return 0;
}

// the format of _string is: true OR false
static int boolean_conver(char** const _string,size_t* str_len, ObjectInfo* const boolean){

	int i=0;
	char* s=*(unsigned char*)(boolean->offset)?*str_len>3&&(i=4)?"true":NULL:*str_len>4&&(i=5)?"false":NULL;

	if(!s)
		return -1;

	strncpy(*_string,s,i);
	*str_len-=i;
	*_string+=i;
	return 0;
}

//Single ASCII character can be accepted
static int char_conver(char** const _string,size_t* str_len, ObjectInfo* const _char){
	return *str_len>2?(*(*_string)++='"')&&(*(*_string)++=*(char*)(_char->offset))&&(*(*_string)++='"')&&(*str_len-=3)&&0:-1;
}

// array[array_size, ele_size, poiter_to_array]
static int array_conver(char** const _string,size_t* str_len, ObjectInfo* const array){
	if(*str_len<2||!array->offset)
		return -1;
	else{
		*(*_string)++='[';
		*str_len-=2;
	}

	size_t n=array->typeInf->size[1];
	if(n<1)
		return 0;
	else{
		size_t el_size=array->typeInf->size[0]/n;
		ObjectInfo ot=*array->typeInf->subObjInfo;
		ot.offset=array->offset;

		while(n-->0){
			if (0 > convert(_string, str_len, &ot))
				return -1;
			if (n > 0) {
				if (*str_len < 2)
					return -1;
				ot.offset += el_size;
				*(*_string)++ = ',';
				--*str_len;
			}
		}
	}
	*(*_string)++=']';
	return 0;
}

static int object_conver(char** const _string,size_t * str_len, ObjectInfo* const object){
	if(*str_len<2)
		return -1;
	else{
		*(*_string)++='{';
		*str_len-=2;
	}

	const ObjectInfo *o = object->typeInf->subObjInfo;
	size_t n=object->typeInf->size[1];
	if(n>0&&o){

		int t;
		long base_addr = (long) object->offset;

		do{
			ObjectInfo obj = *o;
			obj.offset += base_addr;		//base + offset == attribute address
			t=strlen(obj.name);
			if (*str_len < t + 4)
				return -1;

			*(*_string)++='"';
			strncpy(*_string, obj.name,t);
			*((*_string)+=t)='"';
			*++(*_string)=':';
			++*_string;
			*str_len-=(t+3);
			if(0>convert(_string, str_len, &obj))
				return -1;
			else if(--n<1)
				break;
			else{
				if(*str_len<6){
					return -1;
				}else{
					++o;
					*(*_string)++ = ',';
					--*str_len;
				}
			}
		}while(1);
	}

	*(*_string)++='}';
	return 0;
}

static int ptr_conver(char** const _string,size_t * str_len, ObjectInfo* const object){
	void** p=(void**)object->offset;
	if(!*p)
		return *str_len>3?strncpy(*_string,"null",4)&&(*_string+=4)&&(*str_len-=4)&&0:-1;
	else{
		ObjectInfo o=*object->typeInf->subObjInfo;
		o.offset=*p;
		return convert(_string,str_len, &o);
	}
}

static int (* const conver_by_type[5])(char** const _string,size_t* str_len, ObjectInfo* const data)={
		numeric_conver,
		string_conver,
		array_conver,
		object_conver,
		boolean_conver,
};

static int convert(char** const _string,size_t * str_len, ObjectInfo* const objectInfo){

	int t=objectInfo->typeInf->type;

	if(t==ACHAR)
		return char_conver(_string,str_len,objectInfo);
	else if(t==PTR&&objectInfo->typeInf->subObjInfo->typeInf->type==OBJECT)
		return ptr_conver(_string,str_len,objectInfo);
	else{
		int i=J_R_STRING;

		if(t>=INT8&&t<=UINT64)
			i=J_R_NUMBER;
		else
			while(i<=J_R_BOOLEAN&&t!=i) ++i;

		if(i>J_R_BOOLEAN)
			return -1;
		else
			return conver_by_type[i](_string,str_len,objectInfo);
	}

}


int json_cf_object(char* string,size_t str_len, ObjectInfo* objectInfo){

	if(--str_len>1&&0>convert(&string,&str_len,objectInfo))
		return -1;
	else{
		*string='\0';
		return 0;
	}
}
