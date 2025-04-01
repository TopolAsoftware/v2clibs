/*
 *  Copyright (c) 2014-2015 Oleg Vlasenko <vop@unity.net>
 *  All Rights Reserved.
 *
 *  This file is part of the the TopolA managment software.
 *
 *  TaRemote is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  TaRemote is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TopolA; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307 USA.
 */

// Converts json params to v2_wo_lst.

#include "v2_wo_json.h"

// ERROR_CODE 136XX

/* ============================================================= */
int v2_wo_json_parse(JSON_Object *in_json) {
    JSON_Value *jval=NULL;
    JSON_Value_Type jtype=JSONNull;
    const char *jname=NULL;
    double in_number;
    int rc=0;
    int x;

    for(x=0; x<json_object_get_count(in_json) && !rc; x++) {
	if(!(jname=json_object_get_name(in_json, x))) continue;
	jval=json_object_get_value(in_json, jname);
	jtype=json_type(jval);

	if(jtype==JSONString) {
	    v2_add_lstr_tail(&v2_wo_lst, (char *)jname, (char *)json_string(jval), 0, 0);
	    continue;
	}

	if(jtype==JSONNumber) {
	    in_number=json_number(jval);
	    if(in_number - (long)in_number) {
		v2_add_lstr_tail(&v2_wo_lst, (char *)jname, v2_strtmp("%.2f", in_number), 0, 0);
	    } else {
		v2_add_lstr_tail(&v2_wo_lst, (char *)jname, v2_strtmp("%ld", (long)in_number), 0, 0);
	    }
            continue;
	}

	if(jtype==JSONObject) {
            rc=v2_wo_json_parse(json_object(jval));
            continue;
	}
    }

    return(rc);
}
/* ============================================================= */
int v2_wo_json(char *in_json) {
    JSON_Value *val=NULL;
    JSON_Value_Type jtype=JSONNull;
    JSON_Object *jobj=NULL;

    if(!(val=json_parse_string(in_json))) return(13601);

    if((jtype=json_value_get_type(val)) != JSONObject) {
	json_value_free(val);
        return(13602);
    }

    jobj=json_object(val);

    return(v2_wo_json_parse(jobj));

}
/* ============================================================= */
