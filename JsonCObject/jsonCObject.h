/*
 * Parse.h
 *
 *  Created on: Jun 27, 2018
 *      Author: Like.Z
 */

#ifndef JSONCOBJECT_H_
#define JSONCOBJECT_H_
#include "CTypeInfo/type_info.h"
/*
 * Before this, set locate LC_CTYPE "*.UTF-8" for Unicode char transform between utf-8 and utf-16.
 * e.g. UTF-16 char "\u040f" in JSON string
 *
 * 	if(!strstr(setlocale(LC_CTYPE, NULL),"UTF-8"))
		if(!strstr(setlocale(LC_CTYPE, ""),"UTF-8"))
			setlocale(LC_CTYPE, "en_US.UTF-8");
 * */

extern int json_cto_object_init();
extern int json_cto_object(char* , ObjectInfo*);
extern int json_cf_object(char* ,size_t , ObjectInfo*);
extern int json_cto_object_destroy();

extern int verify_test(int i,char* s);
#endif /* JSONCOBJECT_H_ */
