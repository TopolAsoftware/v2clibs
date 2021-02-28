/*
 *  Copyright (c) 2015-2016 Oleg Vlasenko <vop@unity.net>
 *  All Rights Reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// ERROR_CODE 174XX : 17400 - 17449

#include "v2_jrpc.h"


/* =========================================================== */
static int v2_jrpc_parse(json_box_t *in_box) {
    //char strtmp[MAX_STRING_LEN];
    json_lst_t *json_tmp=NULL;
    int rc=0;

    v2_freestr(&v2_jrpc.jsonrpc);
    v2_freestr(&v2_jrpc.method);
    v2_freestr(&v2_jrpc.id_str);
    v2_jrpc.id=0;

    v2_jrpc.json_par=NULL;

    if(!in_box) return(17400);

    FOR_LST(json_tmp, in_box->lst) {

	if(v2_json_type(json_tmp) == JS_STRING) {
	    if(!v2_strcmp(json_tmp->id, "jsonrpc")) {
		v2_let_var(&v2_jrpc.jsonrpc, json_tmp->str); // Not allocate
		continue;
	    } else if(!v2_strcmp(json_tmp->id, "method")) {
		v2_let_var(&v2_jrpc.method, json_tmp->str); // Not allocate
		continue;
	    } else if(!v2_strcmp(json_tmp->id, "id")) {
		v2_let_var(&v2_jrpc.id_str, json_tmp->str);
		continue;
	    }
	}

	if(v2_json_type(json_tmp) == JS_LONG) {
	    if(!v2_strcmp(json_tmp->id, "id")) {
		v2_jrpc.id=json_tmp->lnum;
		continue;
	    }
	}

	if(v2_json_type(json_tmp) == JS_OBJECT) {
	    if(!v2_strcmp(json_tmp->id, "params")) {
		v2_jrpc.json_par=json_tmp->child;
		continue;
	    }
	}

	v2_jrpc.error("-32700 Unknown \"%s\": %s = '%s'", v2_json_type_str(json_tmp), json_tmp->id, v2_as(json_tmp->str)); // Make it before free
	return(0);
    }

    if(!v2_jrpc.jsonrpc) {
	v2_jrpc.error("-32600 Require 'jsonrpc' field");
        return(0);
    }

    if(v2_strcmp(v2_jrpc.jsonrpc, "2.0")) {
	v2_jrpc.error("-32600 Wrong version of JSON-RPC request, supported 2.0 inly");
        return(0);
    }

    if(!v2_jrpc.id && !v2_jrpc.id_str) {
	v2_jrpc.error("-32600 Require 'id' field");
        return(0);
    }

    if(!v2_jrpc.method) {
	v2_jrpc.error("-32600 Require 'method' field");
        return(0);
    }

    // Starts answer
    v2_jrpc.beg(); // error removes all previouse json data

    if(v2_jrpc.to_method) {
	if((rc=v2_jrpc.to_method(&v2_jrpc))) {
	    if((rc==V2_JRPC_NOT_FOUND)) {
		v2_jrpc.error("-32601 Method '%s' not found%s", v2_jrpc.method, v2_strcmp(v2_jrpc.method, "help")?", try 'help' method":"");
	    } else {
		v2_jrpc.error("-32603 Internal error code rc=%d", rc);
	    }
	} else {
	    v2_jrpc.end();
	}
    } else {
	v2_jrpc.error("-32601 Method \"%s\" not defined", v2_jrpc.method);
    }
    //if(v2_jrpc.is_sent) return(0); // Error already sent

    return(0);
}
/* =========================================================== */
static int v2_jrpc_error(char *in_str, ...) {
    //str_lst_t *err_jprs=NULL;
    //va_list vl;
    char strtmp[MAX_STRING_LEN];
    char strtm1[MAX_STRING_LEN];
    int code=-32001; // Default code error
    //str_lst_t *str_tmp=NULL;
    //int ret=0;

    // Clear all into json list
    v2_json_free();

    json_box.ident=1;
    json_box.header=1;
    //v2_json_locale(&json_box, u_cgi.locale, 0);

    v2_json_add_str("jsonrpc", "2.0");
    v2_json_add_obj("error");

    if(!v2_is_par(in_str)) { // Internal error
	v2_json_add_int("code", (code=-32603));
	v2_json_add_str("message", "Internal error");
    } else {

	//va_start(vl, in_str);
	//vsprintf(strtmp, in_str, vl);
	//va_end(vl);

	VL_STR(strtmp, MAX_STRING_LEN, in_str);

	if(strtmp[0] == '-') { // Numbered error
	    //sprintf(strtmp, "%s", in_str); - not need already
	    v2_getword(strtm1, strtmp, ' ');

	    if((code=atoi(strtm1))) {
		v2_json_add_int("code", code);
	    } else {
		v2_json_add_int("code", (code=-32603));
	    }

	    if(!strtmp[0]) {
		if(code==-32600) {
		    v2_json_add_str("message", "Invalid request");
		    //v2_lstr.tail(&err_jprs, "Use JSON-RPC v2.0 post request");
		    //v2_lstr.tail(&err_jprs, "Specification at http://www.jsonrpc.org/specification");
		    //v2_lstr.tail(&err_jprs, "Protocol at http://topola.unity.net/");
		    //v2_add_error("Use JSON-RPC v2.0 post request");
		    //v2_add_error("Specification at http://www.jsonrpc.org/specification");
		    //v2_add_error("Protocol at http://topola.unity.net/");
		    v2_json_add_arr("data"); {
			v2_json_add_str("str1", "WARNING! Use JSON-RPC v2.0 post request");
			v2_json_add_str("str2", "Specification at http://www.jsonrpc.org/specification");
			v2_json_add_str("str3", "Protocol at http://topola.unity.net/");
			v2_json_end_arr();
		    }
		} else if(code==-32700) {
		    v2_json_add_str("message", "Parse error");
		} else if(code==-32601) {
		    v2_json_add_str("message", "Method not found");
		} else if(code==-32602) {
		    v2_json_add_str("message", "Invalid params");
		} else { // if(code==-32603) { - any other errors
		    v2_json_add_str("message", "Internal error");
		}
	    } else {
		v2_json_add_str("message", strtmp);
	    }
	} else { // Other error
	    v2_json_add_int("code", (code=-32001));
	    //v2_json_add_str("message", in_str?in_str:"Internal error");
	    v2_json_add_str("message", strtmp);
	}

    }

    //if(v2_err_lst) {
	//if(v2_err_lst->next) v2_json_add_arr("data");
	//FOR_LST(str_tmp, v2_err_lst) {

//    if(err_jprs) {
//	if(err_jprs->next) v2_json_add_arr("data");
//	FOR_LST(str_tmp, err_jprs) {
//	    ret=sprintf(strtmp, "ERROR!");
//	    if(str_tmp->num) ret+=sprintf(strtmp+ret, " [RC=%d]", str_tmp->num);
//	    ret+=sprintf(strtmp+ret, " %s%s%s", str_tmp->key, str_tmp->str?" ":"", v2_nn(str_tmp->str));
//	    v2_json_add_str("data", strtmp);
//	}
//
//	//if(v2_err_lst->next) v2_json_end_arr();
//	if(err_jprs->next) v2_json_end_arr();
//	v2_lstr.free(&err_jprs);
//    }
    v2_json_end_obj();
    v2_jrpc.end();


    return(code);
}
/* ====================================================================== */
static int v2_jrpc_beg(void) {

    v2_json_add_str("jsonrpc", "2.0");
    v2_json_add_obj("result");

    return(0);
}
/* ====================================================================== */
static int v2_jrpc_end(void) {

    v2_json_end_obj();
    if(v2_jrpc.id) {
	v2_json_add_int("id", v2_jrpc.id);
    } else if(v2_jrpc.id_str) {
	v2_json_add_str("id", v2_jrpc.id_str);
    } else {
	v2_json_add_null("id");
    }
    v2_jrpc.is_sent = 1;
    return(v2_json_print()); // Print json output
}
/* ====================================================================== */
v2_jrpc_t v2_jrpc = {

    .parse = &v2_jrpc_parse,
    .error = &v2_jrpc_error,

    .beg =   &v2_jrpc_beg,
    .end =   &v2_jrpc_end,

};
/* ====================================================================== */
