/*
 *  Copyright (c) 2005-2016 Oleg Vlasenko <vop@unity.net>
 *  All Rights Reserved.
 *
 *  This file is part of the V2-utils which is a remote
 *  agent of the TopolA managment software.
 *
 *  V2-utils is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  V2-utils is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "v2_fun.h"

// -- ERROR _ CODE 17XX

typedef struct fun_lst_s {
    struct fun_lst_s *next;
    char *group;       // Group of the funstions - for multyply lists
    char *name;        // Name of config option
    char *desc;        // Description of the function
    int (*fun)(char*); // Init func for this variable or post functions
    //v2fun_t fun;
    int rc;
} fun_lst_t;

fun_lst_t *fun_lst=NULL; // List of all functions
fun_lst_t *fun_cur=NULL; // List of cur functions

/* ===================================================== */
int v2_fun_compare(fun_lst_t *in_fun1, fun_lst_t *in_fun2) {
    int ret=0;

    if(!in_fun1 || !in_fun2) return(0); // Hmmm, a bit strange things

    if(!in_fun1->group) {
        if(in_fun2->group) return(-1); // <
    } else {
	if(!in_fun2->group) return(1); // >
	if((ret=strcmp(in_fun1->group, in_fun2->group))) return(ret);
    }
    return(strcmp(in_fun1->name, in_fun2->name));
}
/* ===================================================== */
int v2_add_fun(char *in_list, char *in_name, int (*in_fun)(char *)) {
    //fun_lst_t *fun_tmp=NULL;
    fun_lst_t *fun_ttt=NULL;

    if(!v2_is_par(in_name)) return(0);
    if(!v2_is_val(in_list)) in_list="__main"; // Default value

    fun_cur=NULL;
    FOR_LST_IF(fun_ttt, fun_cur, fun_lst) {
	if(v2_strcmp(fun_ttt->group, in_list)) continue;
	if(v2_strcmp(fun_ttt->name, in_name)) continue;
	fun_cur=fun_ttt;
	fun_cur->fun=in_fun; // Redefine (UnDefine) function
    }

    if(fun_cur) return(0);

    if(!(fun_cur=(fun_lst_t *)calloc(sizeof(fun_lst_t), 1))) return(1);

    v2_let_var(&fun_cur->group, in_list);
    v2_let_var(&fun_cur->name, in_name);
    fun_cur->fun=in_fun;

    if(!fun_lst) {
	fun_lst=fun_cur;
    } else if(v2_fun_compare(fun_cur, fun_lst) < 0) {
	fun_cur->next=fun_lst;
	fun_lst=fun_cur;
    } else {
	for(fun_ttt=fun_lst; fun_ttt->next && (v2_fun_compare(fun_cur, fun_ttt->next) > 0); fun_ttt=fun_ttt->next)
	    ;
        fun_cur->next=fun_ttt->next;
	fun_ttt->next=fun_cur;
        fun_ttt=NULL;
    }

    //fun_cur=fun_tmp;
    //fun_tmp=NULL;

    return(0);
}
/* ===================================================== */
int v2_fun_dsc(char *in_desc, ...) {
    char strtmp[MAX_STRING_LEN];

    if(!fun_cur)              return(0);
    if(!in_desc || !*in_desc) return(0);

    VL_STR(strtmp, 0, in_desc);

    v2_let_var(&fun_cur->desc, strtmp);
    return(0);
}
/* ===================================================== */
// If in_name==NULL, run all functions in the list
int v2_run_fun(char *in_list, char *in_name, char *in_str) {
    fun_lst_t *fun_tmp=NULL;
    fun_lst_t *fun_ttt=NULL;
    int erc=0;
    int rc=0;

    //if(!v2_is_par(in_list)) return(0); // Always should be group/list
    if(!v2_is_val(in_list)) in_list="__main"; // Default value

    for(fun_tmp=fun_lst; fun_tmp && !fun_ttt && !erc; fun_tmp=fun_tmp->next) {
	if(v2_strcmp(fun_tmp->group, in_list)) continue; // Not our group

        // Named function
	if(v2_is_par(in_name)) {
	    if(v2_strcmp(fun_tmp->name, in_name)) continue; // We search only named function. Else - all names ok
	    fun_ttt=fun_tmp; // Just to detect Function not found
	}

	if(v2_fun.tr) v2_fun.tr(0, in_list, fun_tmp->name, in_str, 0); // Pass 0 - pre-run

        // List of functions
	if(!fun_tmp->fun) {
	    fun_tmp->rc=V2_FUN_NOT_DEFND; // Function now defined
	    if(v2_fun.debug>1) v2_add_debug(v2_fun.debug, "FUN %s:%s(\"%s\") not defined", fun_tmp->group, fun_tmp->name, v2_as(in_str));
	    continue;
	}

	fun_cur=fun_tmp;

	if(v2_fun.debug>1) v2_add_debug(v2_fun.debug, "FUN %s:%s(\"%s\")", fun_tmp->group, fun_tmp->name, v2_as(in_str));

	fun_tmp->rc=fun_tmp->fun(in_str);
	if(v2_fun.tr) v2_fun.tr(1, in_list, fun_tmp->name, in_str, fun_tmp->rc); // Pass 1 - post-run
	if(fun_tmp->rc == 0) continue;

	rc=fun_tmp->rc; // Add error from list functiopns if exists
	if(v2_fun.debug>0) v2_add_debug(v2_fun.debug, "WARNING FUN %s:%s(\"%s\") returns %d",
					    v2_st(fun_tmp->group, "-any-"), fun_tmp->name, v2_as(in_str), fun_tmp->rc);

	if(v2_fun.err) erc=v2_fun.err(fun_tmp->group, fun_tmp->name, fun_tmp->rc); // If erc != 0 - stop process list
    }

    //if(rc) return(rc);

    if(fun_ttt)            return(fun_ttt->rc);      // Single function result
    if(v2_is_par(in_name)) return(V2_FUN_NOT_FOUND); // We did not find single function == 1780

    return(rc);
}
/* ===================================================== */
// Formatted argument
// If in_name==NULL, run all functions in the list
int v2_run_funf(char *in_list, char *in_name, char *in_fmt, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, 0, in_fmt);

    return(v2_fun.run(in_list, in_name, strtmp));
}
/* ===================================================== */
int v2_prn_funcb(char *in_grp, int (*cb)(char*)) {
    fun_lst_t *fun_tmp=NULL;
    char strtmp[MAX_STRING_LEN];
    int res=0;

    FOR_LST(fun_tmp, fun_lst) {
	res=0;
	if(in_grp && in_grp[0]) {
	    if(v2_strcmp(fun_tmp->group, in_grp)) continue;
	    res+=sprintf(strtmp+res, "%s", fun_tmp->name);
	} else {
	    res+=sprintf(strtmp+res, "Fun: ");
	    if(fun_tmp->group) res+=sprintf(strtmp+res, "%s:", fun_tmp->group);
	    res+=sprintf(strtmp+res, "%s()", fun_tmp->name);
	}
	if(fun_tmp->desc) res+=sprintf(strtmp+res, " [%s]", fun_tmp->desc);
	if(fun_tmp->rc) res+=sprintf(strtmp+res, " = %d", fun_tmp->rc);

	if(cb) {
	    cb(strtmp);
	} else {
	    printf("%s\n", strtmp);
	}
    }

    return(0);
}
/* ===================================================== */
int v2_prn_fun(void) {
    return(v2_prn_funcb(NULL, NULL));
}
/* ===================================================== */
// v2fun_t v2_get_fun(char *in_list, char *in_name) {
int (*v2_get_fun(char *in_list, char *in_name))(char*) {
    fun_lst_t *fun_tmp=NULL;
    fun_lst_t *fun_out=NULL;

    if(!in_list || !in_list[0]) return(0);

    FOR_LST_IF(fun_tmp, fun_out, fun_lst) {
	if(strcmp(fun_tmp->group, in_list)) continue;
	if(v2_strcmp(fun_tmp->name, in_name)) continue;
	fun_out=fun_tmp;
    }

    return(fun_out?fun_out->fun:NULL);
}
/* ===================================================== */
int v2_cnt_fun(char *in_list, char *in_name) {
    fun_lst_t *fun_tmp=NULL;
    int num=0;

    if(!in_list || !in_list[0]) return(0);

    FOR_LST(fun_tmp, fun_lst) {
	if(strcmp(fun_tmp->group, in_list)) continue;
	if(in_name && in_name[0]) {
	    if(strcmp(fun_tmp->name, in_name)) continue;
	}
        num++;
    }

    return(num);
}
/* ===================================================== */
str_lst_t *v2_lst_fun(char *in_list) {
    str_lst_t *lst_out=NULL;
    fun_lst_t *fun_tmp=NULL;

    FOR_LST(fun_tmp, fun_lst) {
	if(v2_is_par(in_list) && strcmp(fun_tmp->group, in_list)) continue;
	v2_add_lstr_head(&lst_out, fun_tmp->name, fun_tmp->desc, 0, 0);
	v2_let_var(&lst_out->dstr, fun_tmp->group);
    }

    v2_lstr_rback(&lst_out);

    return(lst_out);
}
/* ===================================================== */
int v2_rc_fun(int in_rc) {
    if(in_rc==V2_FUN_NOT_FOUND) return(0);
    if(in_rc==V2_FUN_NOT_DEFND) return(0);
    return(in_rc);
}
/* ===================================================== */
v2_fun_t v2_fun = {
    .add = &v2_add_fun,
    .dsc = &v2_fun_dsc,
    .run = &v2_run_fun,
    .prn = &v2_prn_fun,
    .cnt = &v2_cnt_fun,
    .lst = &v2_lst_fun,

    .prncb = &v2_prn_funcb,

    .get = &v2_get_fun,

    .rc  = &v2_rc_fun,

    .runf = &v2_run_funf,
};
/* ===================================================== */
