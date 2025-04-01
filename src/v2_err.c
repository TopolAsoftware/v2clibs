/*
 *  Copyright (c) 2016-2021 Oleg Vlasenko <vop@unity.net>
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

#include "v2_err.h"

str_lst_t *v2_err_lst=NULL;
int (*v2_hook_error)(str_lst_t *err_rec)=NULL;

int v2_debug_time=0; // Print time before debug message
char *v2_debug_pref=NULL; // Prefix for debud strings. It better than previouse

/* ======================================================================= */
// Error functions
/* ======================================================================= */
int v2_prn_error(void) {
    str_lst_t *str_tmp=NULL;
    int rc=0;

    FOR_LST(str_tmp, v2_err_lst) {
	if(str_tmp->is_rmv) continue;
	if(v2_debug_time==1)         printf("%s : ", v2_get_date(str_tmp->a_time));
	if(v2_debug_time==77)        printf("# "); // Just comment
	if(v2_debug_time==771)       printf("### "); // Just long comment
	if(v2_is_par(v2_debug_pref)) printf("%s", v2_debug_pref);

	if(str_tmp->b_time==0) {
	    printf("ERROR");
	} else if(str_tmp->b_time==1) {
	    printf("WARN!");
	} else {
	    printf("DEBUG[%ld]", (long)str_tmp->b_time);
	}
	if(str_tmp->num) {
	    printf(" [RC=%d]", str_tmp->num);
            rc=str_tmp->num;
	}

	printf(" %s%s%s\n", v2_st(str_tmp->key, "-"), str_tmp->str?" ":"", v2_nn(str_tmp->str));
    }

    return(rc); // Latest error number
}
/* ======================================================================= */
int v2_add_err(int err, int level, char *err_msg) {
    str_lst_t *err_tmp=NULL;

    if(!err_msg || !*err_msg) return(0); // Nothing to do

    if(!v2_strcmp(err_msg, "-")) return(v2_del_error()); // Reset all error list

    err_tmp=v2_add_lstr_tail(&v2_err_lst, err_msg, NULL, time(NULL), level);
    err_tmp->num=err;

    if(v2_hook_error) v2_hook_error(err_tmp);

    return(err_tmp->num);
}
/* ======================================================================= */
int v2_add_error(char *err_form, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, err_form);

    return(v2_add_err(0, 0, strtmp));
}
/* ======================================================================= */
// Returns error code
int v2_ret_error(int err, char *err_form, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, err_form);

    return(v2_add_err(err, 0, strtmp));
}
/* ======================================================================= */
int v2_add_debug(int level, char *err_form, ...) {
    char strtmp[MAX_STRING_LEN];

    if(level>1) { // Warning (level==1) always added. Debug - test it
	if(level>v2_debug) return(0);
    }

    VL_STR(strtmp, MAX_STRING_LEN, err_form);

    return(v2_add_err(0, level, strtmp));
}
/* ======================================================================= */
// Delete all errors into list
int v2_del_error(void) {
    v2_lstr_free(&v2_err_lst);
    return(0);
}
/* ======================================================================= */
int v2_add_warn(char *warn_form, ...) {
   char strtmp[MAX_STRING_LEN];

   VL_STR(strtmp, MAX_STRING_LEN, warn_form);

   return(v2_add_err(0, 1, strtmp));
}
/* ======================================================================= */
// Re-check error
int v2_is_error(int in_err) {
    str_lst_t *str_tmp=NULL;
    int err=0;

    if(in_err) return(in_err);

    FOR_LST(str_tmp, v2_err_lst) {
        if(str_tmp->b_time) continue; // Skip debug records!!!
        if(!err) err++; // At least one error string exists
	if(str_tmp->num) err=str_tmp->num; // Remember latest error
    }

    return(err);
}
/* ======================================================================= */
