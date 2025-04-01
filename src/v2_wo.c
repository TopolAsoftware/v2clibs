/*
 *  Copyright (c) 1997-2020 Oleg Vlasenko <vop@unity.net>
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

#include "v2_wo.h"

str_lst_t *v2_wo_lst=NULL;
str_lst_t *v2_wo_tek=NULL;
//str_lst_t *v2_wo_lst_pure=NULL; // Not actually needed

/* ========================================================= */
//      Web Opts functions - former v2_webopts.c
/* ========================================================= */
// str_lst a_time==is_post, b_time==is_used
int v2_add_wo_lstr(char *key, char *str, int is_uniq, int is_post) {
    str_lst_t *str_tmp=NULL;

    if(!key || !key[0]) return(1);

    if(is_uniq) {
        if(v2_lstr_key(v2_wo_lst, key)) return(1);
    }

    str_tmp=v2_add_lstr_tail(&v2_wo_lst, key, str, is_post, 0);
    str_tmp->num=strlen(key);
    str_tmp=NULL;

    return(0);
}
/* ============================================================ */
char *v2_wo_key(char *key) {
    str_lst_t *str_tmp=NULL;

    v2_wo_tek=NULL;
    if(!key || !key[0]) return(NULL);

    for(str_tmp=v2_wo_lst; str_tmp && !v2_wo_tek; str_tmp=str_tmp->next) {
	if(strcmp(key, str_tmp->key)) continue;
	v2_wo_tek=str_tmp;
	v2_wo_tek->b_time=1;
    }

    if(!v2_wo_tek) return(NULL);
    return(v2_wo_tek->str);
}
/* ============================================================== */
char *v2_wo_keyf(char *key_format, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, key_format);
    return(v2_wo_key(strtmp));
}
/* ============================================================== */
// Return allocated copy from wo list
char *v2_wo_var(char **pvar, char *in_key) { // Let (set) var from wo_lst, freed ir before
    return(v2_let_var(pvar, v2_wo_key(in_key)));
}
/* ============================================================ */
int v2_wo_sflg(sflg_t *in_sflg, char *in_key) {

    if(!in_sflg)                 return(1); // Have not variable
    if(!v2_sflg_isvar(*in_sflg)) return(1); // Does not allowed set variable

    if(v2_wo_key(in_key)) {
	v2_sflg_set(in_sflg);
    } else {
	v2_sflg_reset(in_sflg);
    }

    return(0);
}
/* ============================================================ */
// Need to be escape URL
char *v2_wo_prn_get(char *init_str, ...) {
    str_lst_t *str_tmp=NULL;
    char strtmp[MAX_STRING_LEN];
    va_list vl;
    int res=0;

    if(init_str && init_str[0]) {
	va_start(vl, init_str);
	res=vsprintf(strtmp, init_str, vl);
	va_end(vl);
    }

    if(v2_wo_lst) {
	res+=sprintf(strtmp+res, "?");
	FOR_LST(str_tmp, v2_wo_lst) {
	    res+=sprintf(strtmp+res, "%s=%s%s", str_tmp->key, v2_nn(str_tmp->str), str_tmp->next?"&":"");
	}
    }

    return(v2_strcpy(strtmp));
}
/* ============================================================ */
int v2_wo_post(int is_uniq) {
    register int x;
    char *t1=NULL;
    char *t2=NULL;
    int cl;
    int rc=0;

    if(!(t1 = getenv("CONTENT_LENGTH"))) return(0);
    cl = atoi(t1);
    
    for(x=0; cl && (!feof(stdin)); x++) {
	t1 = fmakeword(stdin,'&',&cl);

	// Add pure sting
	//if(t1) v2_add_lstr_tail(&v2_wo_lst_pure, t1, NULL, 0, 0);

        t2 = makeword(t1,'=');
        plustospace(t1);
        unescape_url(t1);
        unescape_url(t2);
        rc+=v2_add_wo_lstr(t2, t1, is_uniq, 1);
	//free(t1); -- used at prev func
        //free(t2);
    }

    if(rc) rc=1;
    return(rc);

}
/* ============================================================ */
int v2_wo_get(int is_uniq) {
    char *querstr=NULL;
    char *t1=NULL;
    char *t2=NULL;
    int rc=0;

    if(!(t1=getenv("QUERY_STRING"))) return(1);
    if(!v2_let_var(&querstr, t1)) return(0); // OK, but empty

    while(querstr[0]) {
	//t1 = makeword(querstr,'&');
	if(!(t1 = makeword(querstr,'&'))) continue;
	if(!t1[0]) {
	    free(t1);
            continue;
	}
        t2 = makeword(t1,'=');
        plustospace(t1);
	unescape_url(t1);
        unescape_url(t2);
        rc+=v2_add_wo_lstr(t2, t1, is_uniq, 0);
	//free(t1); -- used at prev func
        //free(t2);
    }

    free(querstr);

    if(rc) rc=1;
    return(rc);

}
/* ============================================================ */
int v2_wo_read(char *in_str, int is_uniq) {
    char *querstr=NULL;
    char *t1=NULL;
    char *t2=NULL;
    int rc=0;

    if(!in_str) return(1);
    if(!in_str[0]) return(0); // Empty

    if(!v2_let_var(&querstr, in_str)) return(0); // OK, but empty

    while(querstr[0]) {
	if(!(t1 = makeword(querstr,'&'))) continue;
	if(!t1[0]) {
	    free(t1);
            continue;
	}
        t2 = makeword(t1,'=');
        plustospace(t1);
        unescape_url(t1);
        unescape_url(t2);
        rc+=v2_add_wo_lstr(t2, t1, is_uniq, 0);
	//free(t1); -- used at prev func
        //free(t2);
    }

    free(querstr);

    if(rc) rc=1;
    return(rc);

}
/* ========================================================= */
int v2_wo_print(char *end_str) { // Prints wo list, each string finalized be end_str
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_ttt=NULL;

    if(!v2_wo_lst) {
	printf("-=empty_list=-%s\n", end_str?end_str:"");
	return(0);
    }

    FOR_LST(str_tmp, v2_wo_lst) {
	if(str_tmp->is_rmv) continue;
	printf("%s = ", str_tmp->key);
	if(str_tmp->num) printf("[%d] ", str_tmp->num);
	if(str_tmp->str) {
            printf("%s%s\n", str_tmp->str, end_str?end_str:"");
	} else {
            printf("-=none=-%s\n", end_str?end_str:"");
	}
	if(str_tmp->data && str_tmp->a_time) {
	    printf("| \"data\", size = %ld%s\n", (long)str_tmp->a_time, end_str?end_str:"");
	}
	FOR_LST(str_ttt, str_tmp->link) {
	    if(str_ttt->is_rmv) continue;
	    printf("| %s = ", str_ttt->key);
	    if(str_ttt->num) printf("[%d] ", str_ttt->num);
	    if(str_ttt->str) {
		printf("%s%s\n", str_ttt->str, end_str?end_str:"");
	    } else {
		printf("-=none=-%s\n", end_str?end_str:"");
	    }
	}
    }

    return(0);
}
/* ========================================================= */
// Return string len or 0
int v2_wo_add_var(char **p_var, char *wo_key) {

    if(!p_var) return(0);

    v2_let_var(p_var, v2_wo_key(wo_key));
    if(!*p_var) return(0);

    return(strlen(*p_var));
}
