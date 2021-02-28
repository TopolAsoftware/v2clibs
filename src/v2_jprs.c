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

// ERROR_CODE 174XX : 17450 - 17499

#include "v2_jprs.h"

/* ==================================================================== */
// Retunrs 0 if name OK
static int v2_jprs_name(json_lst_t *in_el, char *in_name) {

    if(!in_el)                              return(0); // False
    if(!v2_strcmp(in_el->id, in_name))      return(1); // OK
    if(!strchr(in_name, '.'))               return(0); // False - not dotted name
    if(!v2_strcmp(in_el->full_id, in_name)) return(1); // OK - dotted name
    return(0); // False
}
/* ==================================================================== */
static int v2_jprs_str(json_lst_t *in_el, char **p_var, char *in_name) {

    if(!v2_jprs_name(in_el, in_name)) return(0); // Check name
    if(in_el->js_type == JS_STRING) {
	if(p_var) {
	    if(v2_strcmp(*p_var, in_el->str)) v2_jprs.is_upd=1;
	    v2_let_var(p_var, in_el->str); // Let var if exists, else - just skip
	}
    } else if(v2_jprs.no_strict && in_el->js_type == JS_NULL) {
	if(p_var && *p_var) {
	    v2_let_var(p_var, NULL);
	    v2_jprs.is_upd=1;
	}
    } else {
	return(0); // Not our type or just not exists
    }

    return(1); // Retur 1 - means we found our string
}
/* ==================================================================== */
static int v2_jprs_num(json_lst_t *in_el, char **p_var, char *in_name) {

    if(!v2_jprs_name(in_el, in_name)) return(0); // Check name
    if(v2_json_type(in_el) != JS_LONG) {
	if(v2_json_type(in_el) != JS_INT) return(0); // Not our type or just not exists
    }
    if(p_var) {
	if(v2_strcmp(*p_var, in_el->str)) v2_jprs.is_upd=1;
	v2_let_var(p_var, in_el->str);
    }

    return(1); // Retur 1 - means we found our string
}
/* ==================================================================== */
static int v2_jprs_bol(json_lst_t *in_el, int *p_bol, char *in_name) {

    if(!v2_jprs_name(in_el, in_name)) return(0); // Check name
    if(v2_json_type(in_el) != JS_BOOLEAN)     return(0); // Not our type or just not exists
    if(p_bol) {
	if(*p_bol != in_el->num) v2_jprs.is_upd=1;
	*p_bol = in_el->num;
    }

    return(1); // Retur 1 - means we found our string
}
/* ==================================================================== */
// Read long value from int, long or sting
static int v2_jprs_num_long(json_lst_t *in_el, long *p_long, char *in_name) {
    json_field jtype=v2_json_type(in_el);

    if(!v2_jprs_name(in_el, in_name)) return(0); // Check name
    if(jtype == JS_LONG) {
	if(p_long) {
	    if(*p_long != in_el->lnum) v2_jprs.is_upd=1;
	    *p_long = in_el->lnum;
	}
    } else if(jtype == JS_STRING) {
	if(in_el->str) {
	    in_el->lnum=atol(in_el->str);
	} else {
	    in_el->lnum=0;
	}
	if(p_long) {
	    if(*p_long != in_el->lnum) v2_jprs.is_upd=1;
	    *p_long = in_el->lnum;
	}
    } else if(v2_jprs.no_strict && jtype == JS_NULL) {
	if(p_long) {
	    if(*p_long) v2_jprs.is_upd=1;
	    *p_long=0;
	}
    } else {
	return(0);
    }

    return(1); // Retur 1 - means we found our string
}
/* ==================================================================== */
// Read int value from int, long or sting
static int v2_jprs_num_int(json_lst_t *in_el, int *p_int, char *in_name) {
    json_field jtype=v2_json_type(in_el);

    if(!v2_jprs_name(in_el, in_name)) return(0); // Check name
    if(jtype == JS_LONG) {
	if(p_int) {
	    if(*p_int != in_el->lnum) v2_jprs.is_upd=1;
	    *p_int = in_el->lnum;
	}
    } else if(jtype == JS_INT) {
	if(p_int) {
	    if(*p_int != in_el->num) v2_jprs.is_upd=1;
	    *p_int = in_el->num;
	}
    } else if(jtype == JS_STRING) {
	if(in_el->str) {
	    in_el->num=atoi(in_el->str);
	} else {
	    in_el->num=0;
	}
	if(p_int) {
	    if(*p_int != in_el->num) v2_jprs.is_upd=1;
	    *p_int = in_el->num;
	}
    } else if(v2_jprs.no_strict && jtype == JS_NULL) {
	if(p_int) {
	    if(*p_int) v2_jprs.is_upd=1;
	    *p_int=0;
	}
    } else {
	return(0);
    }

    return(1); // Retur 1 - means we found our string
}
/* ==================================================================== */
static int v2_jprs_arr_str(json_lst_t *in_el, str_lst_t **p_arr, char *in_name) {
    json_lst_t *jsn=NULL;
    //str_lst_t *str_tmp=NULL;

    if(!v2_jprs_name(in_el, in_name))           return(0); // Check name
    if(v2_json_type(in_el) != JS_ARRAY)         return(0); // Not our type or just not exists
    if(v2_json_type(in_el->child) != JS_STRING) return(0); // This is not strings array

    FOR_LST(jsn, in_el->child) {
	if(!v2_is_par(jsn->str)) {
	    v2_add_debug(1, "JSON_PRS: GOT Empty \"%s\" : \"%s\" array filed", v2_nn(in_name), jsn->id);
	    continue;
	}
	//v2_add_debug(1, "JSON_PRS: GOT Var \"%s\" : \"%s\" = \"%s\" array filed", v2_nn(in_name), jsn->id, v2_nn(jsn->str));
	v2_lstr.tail(p_arr, jsn->str);
    }

    return(1); // Retur 1 - means we found our string
}
/* ==================================================================== */
static int v2_jprs_arr_obj(json_lst_t *in_el, void *in_data, int (*in_fun)(json_lst_t*, void*), char *in_name) {

    if(!in_el)                                  return(0);
    //v2_add_debug(1, "Type[%d]: %s", v2_jprs.no_strict, v2_json_type_str(in_el->child));
    if(v2_json_type(in_el->child) != JS_OBJECT) return(0); // This is not strings array

    return(v2_jprs.arr_fun(in_el, in_data, in_fun, in_name));
}
/* ==================================================================== */
// Array of array
static int v2_jprs_arr_arr(json_lst_t *in_el, void *in_data, int (*in_fun)(json_lst_t*, void*), char *in_name) {

    if(!in_el)                                 return(0);
    //if(v2_jprs.no_strict && (v2_json_type(in_el->child) == JS_NONE)) return(0); // Empty Array
    if(v2_json_type(in_el->child) != JS_ARRAY) return(0); // This is not array array

    return(v2_jprs.arr_fun(in_el, in_data, in_fun, in_name));
}
/* ==================================================================== */
static int v2_jprs_arr_fun(json_lst_t *in_el, void *in_data, int (*fun)(json_lst_t*, void*), char *in_name) {
    json_lst_t *jsn=NULL;
    int rc=0;

    if(!fun) return(0); // Where to read it?

    if(!v2_jprs_name(in_el, in_name))           return(0); // Check name
    if(v2_json_type(in_el) != JS_ARRAY)         return(0); // Not our type or just not exists

    FOR_LST_IF(jsn, rc, in_el->child) rc=fun(jsn, in_data);

    if(rc) return(0); // Not good applet

    return(1);
}
/* ==================================================================== */
v2_jprs_t  v2_jprs = {

    .name    = &v2_jprs_name,

    .str     = &v2_jprs_str,
    .bol     = &v2_jprs_bol,
    .num     = &v2_jprs_num,

    .num_lng = &v2_jprs_num_long,
    .num_int = &v2_jprs_num_int,

    .arr_str = &v2_jprs_arr_str,
    .arr_obj = &v2_jprs_arr_obj,
    .arr_arr = &v2_jprs_arr_arr,
    .arr_fun = &v2_jprs_arr_fun,
};
/* ==================================================================== */
