/*
 *  Copyright (c) 2005-2019 Oleg Vlasenko <vop@unity.net>
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

// ERROR_CODE 124XX

#define _GNU_SOURCE
#include "v2_outtpl.h"
#include <errno.h>

#ifndef MAX_STRING_LEN
#define MAX_STRING_LEN 1024
#endif

#define V2_TPL_UNFOUND 734524

// #define V2_YES(r) (r)?1:0
#define V2_NOT(r) (r)?0:1

// ----------- ext interface ------------------
int (*v2_ot_str)(char*)                       = &v2_ot_stra;
int (*v2_ot_fun)(int (*)(char *), char*, ...) = &v2_def_function;
// --------------------------------------------

// New definition !!!!
typedef struct _v2_tpl_lst {
    struct _v2_tpl_lst *next;
    struct _v2_tpl_lst *prev;
    char *key; // Key code, name of var or fun, or define
    char *help; // Help string
    char *chap; // Chapter, for var and so
    // Var
    char *int_var; // Internal var allocated
    char **pstr;   // Link to internal or external var
    // Functions
    int (*fun)(char *); // Function link
    //int is_hlp; // Function can uderstand Help
    int is_app; // External functions
    // Mod
    // str_lst_t ???
} v2_tpl_lst_t;


// Old one
typedef struct _v2_key_lst {
    struct _v2_key_lst *next;
    struct _v2_key_lst *prev;
    char *key;
    char *int_var;
    char *help;
    char *chap; // New chapter of difinitions
    char **pstr;
} v2_key_lst_t;

typedef struct _v2_fun_lst {
    struct _v2_fun_lst *next;
    char *key;
    char *help;
    int (*fun)(char *);
    //int is_hlp; // Function can uderstand Help
    int is_app; // External functions
} v2_fun_lst_t;

typedef struct _v2_def_lst {
    struct _v2_def_lst *next;
    struct _v2_def_lst *prev;
    char *str; // Key
    char *help;
} v2_def_lst_t;

typedef struct _v2_mod_lst {
    struct _v2_mod_lst *next;
    v2_def_lst_t *pstr;
    v2_def_lst_t *pstr_last;
    char *key;
    char *help;
    //v2_fun_lst_t *exec; // #modexec function
} v2_mod_lst_t;

v2_tpl_t v2_tpl;

char *web_root_dir=NULL;
char *src_tpl_dir=NULL;

int all_lst_defs=0;

FILE *v2_out_file=NULL;
char v2_finish_char=';';
char v2_start1_char='%';
char v2_start2_char='%';
int v2_emtstr_noshow=1;

int v2_tpl_webout=0; // if == 1 print web error messages

//char *v2_lang_name=NULL; // Lang name
char *v2_skin_name=NULL; // Skin name
char *v2_tpl_file=NULL;
//char *v2_tpl_locale=NULL; // Locale of files in currecnt directory

char *v2_tpl_fun=NULL; // Name of current virtual function (not great hack)

int is_init_ifun=0;  // Do internal functions active?
int no_show_del=0; // Don't print deleted vars/defs

int (*v2_tpl_out_err)(int)=NULL; // Function called if exec error found

int (*v2_out_str)(char*)=NULL; // External func to transfer strings, default v2_trans_str()

/* ======================= local vars ==================== */
char *var_rep=NULL;
int includes_num=0; // Replage it to inc_lst
str_lst_t *inc_lst=NULL; // key = template file, str = -not read-

typedef struct _if_par_t {
    struct _if_par_t *next;
    int is_hide;
    int inc; // Include level
} if_par_t;

if_par_t *if_par=NULL;
//int is_hide=0;

//typedef struct _int_vars_t {
//    struct _int_vars_t *next;
//    char *str;
//} int_vars_t;

//int_vars_t *int_vars=NULL;

v2_def_lst_t *v2_def_lst=NULL;
v2_key_lst_t *v2_key_lst=NULL;
v2_fun_lst_t *v2_fun_lst=NULL;

v2_mod_lst_t *v2_mod_lst=NULL;
v2_mod_lst_t *v2_mod_ptr=NULL; // Pointer if block active
v2_mod_lst_t *v2_mod_tek=NULL; // Pointer after check module

v2_tpl_lst_t *v2_ini_fun=NULL; // List of init functions

// Print to local buffer
#define LOC_BUFF_BLOCK_SIZE 4096
size_t loc_buff_cnt=0;
size_t loc_buff_siz=0;
uint8_t *loc_buff=NULL; // Print out at local buffer

/* ================== Prototypes ========================= */
char **add_int_var(char *in_par);
void v2_init_ifun(void);
int v2_debug_var(char *no_str);

/* ============ Short call =============================== */
// Allocated call
int v2_ot_stra(char *in_str) {
    char *tstr=NULL;
    int rc=0;

    if(!in_str || !*in_str) return(0);

    tstr=v2_strcpy(in_str);

    rc=v2_trans_str(tstr);

    v2_freestr(&tstr);

    return(rc);
}
/* ======================================================= */
int v2_ot_strf(char *in_fmt, ...) {
    va_list vl;
    char *tstr=NULL;
    int rc=0;

    if(!in_fmt || !*in_fmt) return(1); // Problem!!!

    va_start(vl, in_fmt);
    rc=vasprintf(&tstr, in_fmt, vl);
    va_end(vl);

    if(rc==-1) return(2);

    if(v2_ot_str) {
	rc=v2_ot_str(tstr);
    } else {
	rc=v2_trans_str(tstr);
    }
    free(tstr);
    return(rc);
}
/* ======================================================= */
int v2_add_def_lst_tpl(char *str_def) {
    return(v2_add_def_lst(str_def));
}
/* ======================================================= */
int v2_del_def_lst_tpl(char *str_def) {
    return(v2_del_def_lst(str_def));
}
/* ======================================================= */
//char *i_get_next_par(char ch, char *in_str) {
//    char *p=NULL;
//
//    if(!(p=in_str)) return(NULL);
//    if((ch==' ') && (*p==' ')) while(*p++==' ');
//    while((*p!='\0') && (*p!=ch)) p++;
//    if(!*p) return(NULL);
//    *p++='\0';
//    if((ch==' ') && (*p==' ')) while(*p++==' ');
//    return(p);
//}
/* ======================================================= */
// Set local buffer - can be called multiply times
int v2_tpl_setbuff(void) {

    if(loc_buff != NULL) return(0); // Already set

    if(!(loc_buff=(uint8_t *)malloc(LOC_BUFF_BLOCK_SIZE))) return(1025); // Allocation error

    *loc_buff='\0';

    loc_buff_cnt=0;
    loc_buff_siz=LOC_BUFF_BLOCK_SIZE;

    return(0);
}
/* ======================================================= */
// ReSet local buffer
int v2_tpl_resbuff(void) {

    loc_buff_cnt=0;
    loc_buff_siz=0;

    if(loc_buff) free(loc_buff);
    loc_buff=NULL;

    return(0);
}
/* ======================================================= */
char *v2_tpl_getbuff(void) {
    char *out=(char*)loc_buff;
    loc_buff=NULL;
    v2_tpl_resbuff();
    return(out);
}
/* ======================================================= */
// Add to local buffer
int v2_tpl_addbuff(char *format, va_list in_vl) {
    char *outstr=NULL;
    int cnt=0;

    if(!format || !format[0]) return(0); // Nothing to print

    if((cnt=vasprintf(&outstr, format, in_vl)) == -1) return(-1);
	   
    if((loc_buff_cnt+cnt+1) > loc_buff_siz) { // +1 - to tailer '\0'
	loc_buff=(uint8_t *)realloc(loc_buff, loc_buff_siz+LOC_BUFF_BLOCK_SIZE+cnt);
	loc_buff_siz += LOC_BUFF_BLOCK_SIZE+cnt;
    }

    memcpy(loc_buff+loc_buff_cnt, outstr, cnt);
    loc_buff_cnt += cnt;
    loc_buff[loc_buff_cnt]=0; // '\0';
    free(outstr);
    return(cnt);
}
/* ======================================================= */
// Print local buffer
int v2_tpl_prnbuff(void) {
    FILE *lf=v2_out_file?v2_out_file:stdout;
    size_t cnt=0;

    if(!loc_buff)     return(0);
    if(!loc_buff_cnt) return(0); // Nothing to print

    cnt=fwrite(loc_buff, 1, loc_buff_cnt, lf);

    v2_tpl_resbuff();

    return(cnt);
}
/* ======================================================= */
int v2_tpl_printf(char *format, ...) {
    va_list vl;
    int cnt=0;

    if(!format || !format[0]) return(0); // Nothing to print

    va_start(vl, format);
    if(loc_buff != NULL) {
        cnt=v2_tpl_addbuff(format, vl);
    } else if(v2_out_file) {
        cnt=vfprintf(v2_out_file, format, vl);
    } else {
        cnt=vprintf(format, vl);
    }
    va_end(vl);

    return(cnt);
}
/* ======================================================= */
int v2_tpl_prnerr(char *format, ...) {
    va_list vl;
    char strtmp[MAX_STRING_LEN];

    if(!v2_tpl_webout || (loc_buff != NULL)) return(0);
    if(!format || !format[0]) return(0); // Nothing to print

    va_start(vl, format);
    vsnprintf(strtmp, MAX_STRING_LEN, format, vl);
    va_end(vl);

    return(v2_tpl_printf("%s", strtmp));
}
/* ======================================================= */
int v2_tpl_savebuff(char *in_file, ...) {
    va_list vl;
    char f_name[MAX_STRING_LEN];
    char t_name[MAX_STRING_LEN+40];

    if(!in_file || !*in_file) return(0); // Nothing to print

    va_start(vl, in_file);
    vsnprintf(f_name, MAX_STRING_LEN, in_file, vl);
    va_end(vl);

    snprintf(t_name, MAX_STRING_LEN+40, "%s.tmp_%d", f_name, (int)getpid());

    if(!(v2_out_file=fopen(t_name, "w"))) return(1028);
    v2_tpl_prnbuff();
    fclose(v2_out_file);
    rename(t_name, f_name);

    return(0);
}
/* ======================================================= */
char **add_int_var(char *in_par) {
    char strtmp[MAX_STRING_LEN];
    char varname[MAX_STRING_LEN];
    v2_key_lst_t *key_tmp=NULL;
    //v2_key_lst_t *key_tek=NULL;
    char **p_out=NULL;
    int len=0;
    int res=0;

    if(!in_par || !in_par[0]) {
	v2_tpl_prnerr("<!-- Empty SET parametr -->\n");
        return(NULL);
    }

    if(strlen(in_par) > (MAX_STRING_LEN-1)) {
        v2_tpl_prnerr("<!-- Too long sting %s -->\n", in_par);
        return(NULL);
    }
    if((in_par[0] == v2_start1_char) && (in_par[1] == v2_start2_char)) {
        sprintf(strtmp, "%s", in_par+2);
    } else {
        sprintf(strtmp, "%s", in_par);
    }
    len=v2_getword(varname, strtmp, ' ');
    if(varname[len-1] != v2_finish_char) {
        sprintf(varname+len,"%c", v2_finish_char);
    }

    if((len < 2) || (len > V2_TPL_VARLEN)) {
        v2_tpl_prnerr("<!-- Wrong lenght of variable name %s -->\n", varname);
        return(NULL);
    }

    for(key_tmp=v2_key_lst; key_tmp && !res; key_tmp=key_tmp->next) {
	if(strcmp(key_tmp->key, varname)) continue;
	if(*(key_tmp->pstr)) free(*(key_tmp->pstr));
	if(strtmp[0]) {
	    *(key_tmp->pstr)=strcpy((char *)malloc(strlen(strtmp)+1), strtmp);
	} else {
	    *(key_tmp->pstr)=NULL;
	}
	p_out=key_tmp->pstr;
	res++;
    }

    if(res) return(p_out);

    if(!(key_tmp=(v2_key_lst_t *)calloc(sizeof(v2_key_lst_t), 1))) return(NULL);
    key_tmp->key=strcpy((char *)malloc(strlen(varname)+1), varname);
    key_tmp->pstr=&(key_tmp->int_var);
    if(strtmp[0]) {
        key_tmp->int_var=strcpy((char *)malloc(strlen(strtmp)+1), strtmp);
    }

    if((key_tmp->next=v2_key_lst)) {
	if(key_tmp->next->pstr != &(key_tmp->next->int_var)) {
	    key_tmp->chap=strcpy((char *)malloc(17), ".Other variables");
	}
	key_tmp->next->prev=key_tmp;
    }

    p_out=key_tmp->pstr;

    v2_key_lst=key_tmp;
    key_tmp=NULL;
    return(p_out);
}
/* ======================================================= */
int add_fun_var(char *in_par) {
    add_int_var(in_par);
    return(0);
}
/* ======================================================= */
// Set var from another var
int fun_varcpy(char *in_var) {
    char strtmp[MAX_STRING_LEN];
    char *value=NULL;

    if(!in_var || !in_var[0]) {
	v2_tpl_prnerr("<!-- Empty VARCPY parametr -->\n");
        return(0);
    }

    v2_getword(strtmp, in_var, ' ');

    // 1. Try to find var
    if((value=v2_get_var(in_var))) {
	add_int_var(v2_strtmp("%s %s", strtmp, value));
    } else {
	add_int_var(strtmp); // Reset variable
    }

    return(0);
}
/* ======================================================= */
// Returns 1 if function not found
// else result in *p_res
int run_fun_lst(char *fun_par, int *p_res) {
    v2_fun_lst_t *fun_tmp=NULL;
    v2_fun_lst_t *fun_run=NULL;
    char strtmp[MAX_STRING_LEN];
    char *p=NULL;
    char *tt_fun_name=NULL;
    //int is_exec=0;
    //int is_help=0;
    int if_not=0;
    int rc=0;

    sprintf(strtmp, "%s", fun_par);

    if((p=strchr(strtmp, ' '))) *p++='\0';

    FOR_LST_IF(fun_tmp, fun_run, v2_fun_lst) { // Try to find direct funcion
	if(strcasecmp(strtmp, fun_tmp->key)) continue;
	fun_run=fun_tmp;
    }

    // Try to find #ifn... function if input #if...
    if(!fun_run && !strncasecmp(strtmp, "ifn", 3)) {
	FOR_LST_IF(fun_tmp, fun_run, v2_fun_lst) {
	    if(strncasecmp(fun_tmp->key, "if", 2)) continue;
	    if(strcasecmp(strtmp+3, fun_tmp->key+2)) continue;
	    fun_run=fun_tmp;
	    if_not=1;
	}
    }

    if(!fun_run) {
	if(strtmp[0]) v2_tpl_prnerr("<!-- Function \"#%s\" not found |%s| -->\n", strtmp);
	return(1);
    }

    // Run function
    tt_fun_name=v2_tpl_fun; // Remeber for reentrability
    v2_tpl_fun=fun_run->key;

    rc=(*fun_run->fun)(p); // p - args or NULL

    v2_tpl_fun=tt_fun_name; // Return prev value

    if(if_not && (rc != V2_TPL_END)) { // Other rc does not change
	rc=V2_NOT(rc);
    }

    if(p_res) *p_res=rc;

    return(0);
}
/* ======================================================= */
int v2_if_file(char *in_str) {
    struct stat st;

    if(!in_str || !in_str[0]) {
	v2_tpl_prnerr("<!-- IF_FILE: Argument required -->\n");
        return(1);
    }

    if(stat(in_str, &st)) return(1); // File not found

    return(0);
}
/* ======================================================= */
// Returns 0 if defined, 1 - if not defined
int v2_check_def(char *str_def) {
    v2_def_lst_t *def_tmp=NULL;
    int res=1;

    if(all_lst_defs) return(0);
    if(!str_def || !str_def[0]) return(1);

    for(def_tmp=v2_def_lst; def_tmp && res; def_tmp=def_tmp->next) {
	if(!strcmp(str_def, def_tmp->str)) res=0;
    }

    return(res);
}
/* ======================================================= */
// If any (or) of defs exists - returns OK (0)
int v2_ifodef(char *str_odef) {
    char strtmp[MAX_STRING_LEN];
    int no_ok=1;

    if(!str_odef) return(1); // Not ok

    while(str_odef[0] && no_ok) {
	if(!v2_getword(strtmp, str_odef, ' ')) continue;

	if(strtmp[0] == '!') {
	    if(v2_check_def(strtmp+1)) no_ok=0; // Not Found... OK
	} else {
	    if(!v2_check_def(strtmp))   no_ok=0; // Found... OK
	}
    }
    //printf("OK: %d<br>\n", no_ok);
    return(no_ok); // Returns 0 if not all defs OK

}
/* ======================================================= */
// If all dfs are exists  - returns OK (0)
int v2_ifmdef(char *str_mdef) {
    char strtmp[MAX_STRING_LEN];
    int no_ok=0;

    if(!str_mdef) return(1);

    while(str_mdef[0]) {
	if(!v2_getword(strtmp, str_mdef, ' ')) continue;
	if(strtmp[0] == '!') {
	    if(v2_check_def(strtmp+1)) continue; // Not found... OK
	} else {
	    if(!v2_check_def(strtmp))   continue; // Found... OK
	}
	no_ok++;
    }
    //printf("OK: %d<br>\n", no_ok);
    return(no_ok); // Returns 0 if not all defs OK
}
/* ======================================================= */
v2_key_lst_t *find_key_lst(char *in_key) {
    v2_key_lst_t *key_tmp=NULL;
    v2_key_lst_t *key_out=NULL;
    char strtmp[MAX_STRING_LEN];
    int res=0;

    if(!in_key || !in_key[0]) return(NULL);

    if((in_key[0] == v2_start1_char) && (in_key[1] == v2_start2_char)) {
	res=snprintf(strtmp, MAX_STRING_LEN, "%s", in_key+2);
    } else {
	res=snprintf(strtmp, MAX_STRING_LEN, "%s", in_key);
    }
    if(strtmp[res-1] != v2_finish_char) res+=sprintf(strtmp+res, "%c", v2_finish_char);

    FOR_LST_IF(key_tmp, key_out, v2_key_lst) {
	if(strcasecmp(key_tmp->key, strtmp)) continue;
	key_out=key_tmp;
    }

    return(key_out);
}
/* ======================================================= */
int show_var(char *in_var) {
    v2_key_lst_t *key_tmp=NULL;
    //register int x;

    //x=0;
    for(key_tmp=v2_key_lst; key_tmp; key_tmp=key_tmp->next) {
        if(!(strcasecmp(key_tmp->key, in_var))) {
            if(*(key_tmp->pstr)) {
                var_rep = *(key_tmp->pstr);
                return(0);
            } else {
                var_rep=NULL;
                return(1);
            }
        }
    }
    var_rep=NULL;
    return(2);
}
/* ======================================================= */
int v2_check_var(char *in_var) {
    v2_key_lst_t *key_tmp=NULL;

    if(!(key_tmp=find_key_lst(in_var))) {
	v2_tpl_prnerr("<!-- Variable %c%c%s is not defined -->\n", v2_start1_char, v2_start2_char, in_var);
	return(2); // Variable not exists
    }
    if(*(key_tmp->pstr)) return(0);
    return(1);
}
/* ======================================================= */
int v2_check_var_set(char *in_var) {
    v2_key_lst_t *key_tmp=NULL;

    if(!(key_tmp=find_key_lst(in_var))) return(2);
    if(*(key_tmp->pstr)) return(0);
    return(1);
}
/* ======================================================= */
int v2_check_not_var_set(char *in_var) {
    v2_key_lst_t *key_tmp=NULL;

    if(!(key_tmp=find_key_lst(in_var))) return(0);
    if(*(key_tmp->pstr)) return(1);
    return(0);
}
/* ==================================================================== */
int v2_cmp_var(char *in_var_str) {
    v2_key_lst_t *key_tmp=NULL;
    //v2_key_lst_t *key_out=NULL;
    char strtmp[MAX_STRING_LEN];
    char strtm1[MAX_STRING_LEN];
    //int res=0;

    if(!in_var_str || !in_var_str[0]) return(2); // Empty var not equ
    sprintf(strtmp, "%s", in_var_str);
    v2_getword(strtm1, strtmp, ' ');

    if(!(key_tmp=find_key_lst(strtm1))) return(3); // Variable not exists

    if(strtmp[0]) {
	if((*(key_tmp->pstr)) && !strcmp((*(key_tmp->pstr)), strtmp)) return(0); // Zero == Zerro
    } else {
	if(!(*(key_tmp->pstr))) return(0); // Zero == Zero
    }

    return(1); // Not equal
}
/* ==================================================================== */
v2_mod_lst_t *v2_mod_find(char *in_id) {
    v2_mod_lst_t *mod_tmp=NULL;
    v2_mod_lst_t *mod_out=NULL;

    if(!in_id || !in_id[0]) return(NULL);

    FOR_LST_IF(mod_tmp, mod_out, v2_mod_lst) {
	if(!v2_strcmp(mod_tmp->key, in_id)) mod_out=mod_tmp;
    }
    return(mod_out);
}
/* ==================================================================== */
void v2_start_mod(char *in_mod, int is_add) {
    v2_mod_lst_t *mod_tmp=NULL;
    v2_def_lst_t *def_tmp=NULL;
    char modnam[MAX_STRING_LEN];
    char *ihelp=NULL;
    register int x;


    if(v2_mod_ptr) {
	v2_tpl_prnerr("<!-- Recursive modules doesn't allowed -->\n");
        return;
    }

    if(!in_mod || !in_mod[0]) {
        v2_tpl_prnerr("<!-- Undefined name of module -->\n");
        return;
    }
    sprintf(modnam, "%s", in_mod);
    for(x=0; modnam[x] && !ihelp; x++) {
        if((modnam[x] != ' ') && (modnam[x] != '\t')) continue;
        modnam[x] = '\0';
        ihelp=modnam+x+1;
    }

    if(strlen(modnam) > V2_TPL_VARLEN) {
	v2_tpl_prnerr("<!-- Module name too long (&gt;%d ch.) %s -->\n", V2_TPL_VARLEN, modnam);
        return;
    }

    //printf("Mod: %s - %s<BR>\n", modnam, ihelp?ihelp:"none");

    v2_mod_ptr=NULL;
    for(mod_tmp=v2_mod_lst; mod_tmp && !v2_mod_ptr; mod_tmp=mod_tmp->next) {
        if(strcmp(mod_tmp->key, modnam)) continue;
        v2_mod_ptr=mod_tmp;
    }
    mod_tmp=NULL;

    if(v2_mod_ptr) {
        // So, we need to clear strings before
        if(!is_add) {
            v2_mod_ptr->pstr_last=NULL;
            while(v2_mod_ptr->pstr) {
                def_tmp=v2_mod_ptr->pstr;
                v2_mod_ptr->pstr=v2_mod_ptr->pstr->next;
                if(def_tmp->str) free(def_tmp->str);
                if(def_tmp->help)  free(def_tmp->help);
                free(def_tmp);
                def_tmp=NULL;
            }
        }
        return;
    }

    if(!(mod_tmp=(v2_mod_lst_t *)calloc(sizeof(v2_mod_lst_t), 1))) return;
    mod_tmp->key=strcpy((char *)malloc(strlen(modnam)+1), modnam);
    if(ihelp) {
        mod_tmp->help=calloc(strlen(ihelp)+4, 1);
        sprintf(mod_tmp->help, " - %s", ihelp);
    }

    mod_tmp->next=v2_mod_lst;
    v2_mod_lst=mod_tmp;
    mod_tmp=NULL;

    v2_mod_ptr=v2_mod_lst;


    return;
}
/* ==================================================================== */
void v2_add_mod_str(char *in_mod) {
    v2_def_lst_t *def_tmp=NULL;

    if(!(def_tmp=(v2_def_lst_t *)calloc(sizeof(v2_def_lst_t), 1))) return;
    def_tmp->str=strcpy((char *)malloc(strlen(in_mod)+1), in_mod);

    if(v2_mod_ptr->pstr_last) {
        v2_mod_ptr->pstr_last->next=def_tmp;
        v2_mod_ptr->pstr_last=def_tmp;
        def_tmp=NULL;
        return;
    }

    if(!v2_mod_ptr->pstr) {
        v2_mod_ptr->pstr=def_tmp;
        v2_mod_ptr->pstr_last=v2_mod_ptr->pstr;
    } else {
        for(v2_mod_ptr->pstr_last=v2_mod_ptr->pstr; v2_mod_ptr->pstr_last->next; v2_mod_ptr->pstr_last=v2_mod_ptr->pstr_last->next);
        v2_mod_ptr->pstr_last->next=def_tmp;
        v2_mod_ptr->pstr_last=def_tmp;
    }

    def_tmp=NULL;
    return;
}
/* ==================================================================== */
void start_if(int in_hid) {
    if_par_t *if_tmp=NULL;

    if(!in_hid && if_par && if_par->is_hide) in_hid=1; // Filtered one can not be set unfilter

    if((if_tmp=(if_par_t *)calloc(sizeof(if_par_t), 1))) {
        if_tmp->next=if_par;
        if_par=if_tmp;
	if_par->is_hide=in_hid?1:0; // Filter values 1+
	if_par->inc=includes_num; // Keep info to clear after #end
        if_tmp=NULL;
    }

    return;
}
/* ==================================================================== */
void start_ifn(int in_hid) {
    start_if(in_hid?0:1);
    return;
}
/* ==================================================================== */
int else_if(char *in_str) {

    if(if_par) {
	if(if_par->next && (if_par->next->is_hide == 1)) return(0); // Hidden level up

	if(if_par->is_hide == 0) {
	    if_par->is_hide=1;
	} else {
	    if_par->is_hide--; // Can be 2 after elif
	}
    } else {
	v2_tpl_prnerr("<!-- ERROR! Unexpected else %s directive -->\n", v2_nn(in_str));
    }
    return(0);
}
/* ==================================================================== */
int end_if(char *in_str) {
    if_par_t *if_tmp=NULL;

    if(!if_par) {
	v2_tpl_prnerr("<!-- ERROR! Unexpected endif %s directive -->\n", v2_nn(in_str));
	return(0);
    }

    if_tmp=if_par;
    if_par=if_tmp->next;
    free(if_tmp);

    return(0);
}
/* ======================================================================== */
int elif_if(char *in_str) {
    int par_res=0;

    if(!if_par) { // We have not any #if blick
	v2_tpl_prnerr("<!-- ERROR! Unexpected el%s directive -->\n", v2_nn(in_str));
	return(0);
    }

    // We have already open #if block - next one it hidden
    if(if_par->is_hide == 0) {
	if_par->is_hide = 2;
	return(0); // The same as current is_hide
    }

    if(if_par->is_hide == 2)                     return(0); // Already closed
    if(if_par->next && if_par->next->is_hide==1) return(0); // All closed nothing to check

    // Next we have hidden #if block

    // Run any "if..." functions - if false - just skeep
    if(run_fun_lst(in_str, &par_res)) return(1); // Looks like not found
    if(par_res==V2_TPL_END) return(V2_TPL_END);

    if(!par_res) if_par->is_hide = 0;// Open it  // else_if("");

    //if(if_par->is_hide) if_par->is_hide = 2; // Change any to 2 - filter next else

    return(0); // Other value not important
}
/* ======================================================================== */
// New generation (Project only!!!)
// Returns variable array with value
int v2_tpl_trn(char *in_str, char **p_var) {
    wrbuf_t *wrb=NULL;
    char varname[V2_TPL_VARLEN+1];
    char **pvar=NULL; // Var address
    char *sbeg=NULL;
    char *scur=NULL;
    char *send=NULL;
    //int beg=0; // String start position
    //int cur=0; // Current position
    long int len=0;
    int rc=0;

    if(!p_var)  return(0);

    // Not variables string - just copy it
    if(!in_str || !strchr(in_str, v2_finish_char)) { // Just return
	if(!v2_let_var(p_var, in_str)) *p_var=(char*)calloc(1, 1); // Allocate zero length string
	return(0);
    }

    // Prepare buffer to write
    if((rc=v2_wrbuf_new(&wrb))) return(rc);

    for(scur=in_str, sbeg=in_str; *scur; ++scur) {
	if(*scur     != v2_start1_char) continue; // It is not start 1 of the var
	if(*(scur+1) != v2_start2_char) continue; // It is not start 2 of the var
	if(!(send=strchr(scur+2, v2_finish_char))) continue; // No more variables - better make break

	len=++send-scur;
	if(len > V2_TPL_VARLEN) continue;

	snprintf(varname, len+1, "%s", scur);

	if(!(pvar=v2_get_var_adr(varname))) continue; // Not a var

	// 1. Print lead part of string
	v2_wrbuf_write(wrb, sbeg, scur-sbeg, 1); // Need check error?

	// 2. Print found variable
	if(*pvar) v2_wrbuf_write(wrb, *pvar, strlen(*pvar), 1);

	// 3. Set pointer to the end of variable
	scur += len;
	sbeg=scur;
    }

    v2_wrbuf_write(wrb, sbeg, strlen(sbeg), 1); // Rest of the string

    //printf("%s<br>\n", wrb->buf);

    if(wrb->cnt && wrb->buf) *p_var=v2_move(&wrb->buf);

    v2_wrbuf_free(&wrb);

    return(0);
}
/* ======================================================= */
/*
int v2_fun_sp(char *in_str) {
    char *p=in_str;

    if(!p) return(0);

    while(*p == ' ') p++;

    if(*p == '#') {
	return(v2_trans_str(p));
    } else {
	return(v2_trans_str(in_str));
    }

    return(0);
}*/
/* ==================================================================== */
static int v2_sp(char *in_str) { // is_str always exists
    char *p=in_str;

    if(!*in_str) {
	v2_tpl_printf("\n");
	return(0);
    }

    while(*p==' ') p++;
    if(*p != '#') p=in_str; // Reset string pointer
    return(v2_trans_str(p));
}
/* ==================================================================== */
static int v2_plus(char *in_str) {
    char *pvar=NULL;
    char *p=in_str;
    int rc=0;

    while(*p==' ') p++;
    if(*p != '#')  return(v2_trans_str(in_str)); // Reset string pointer

    v2_tpl_trn(p, &pvar);
    rc=v2_trans_str(pvar);
    v2_freestr(&pvar);

    return(rc);
}
/* ==================================================================== */
int v2_trans_str(char *in_str) {
    register int x;
    int len_s=0;
    int len_o=0;
    int len_t=0;
    int is_var=0;
    int par_res=0;
    char out_str[MAX_STRING_LEN];
    char var_str[MAX_STRING_LEN];

    //char *p=NULL;

    v2_init_ifun();

    if(!in_str) return(0);
    if(!in_str[0]) {
        if(v2_emtstr_noshow) return(0); // Nothing to show
	if(!if_par || !if_par->is_hide) v2_tpl_printf("\n");
        return(0);
    }

    len_s=strlen(in_str);

    // Function include comments
    //if(in_str[0] == '#') return(v2_fun_exec(in_str));
    //if(if_par && if_par->is_hide) return(0);

    if(!strcmp(in_str, "#"))      return(0); // Single comment symbol
    if(!strncmp(in_str, "##", 2)) return(0); // Comment string

    // Next one should be before checking module of hidden - do not lose #endif and #nendmod
    if(!strncmp(in_str, "#.",  2))  return(v2_sp(in_str+2)); // The same as #sp, but <space> do not require - good one
    if(!strncmp(in_str, "#+", 2))   return(v2_plus(in_str+2)); // Run with translation of string

    // Do not use it anymore
    if(!strcmp(in_str, "#sp"))      return(v2_sp(in_str+3)); // Not good enough, use #. better
    if(!strncmp(in_str, "#sp ", 4)) return(v2_sp(in_str+3)); // Not good enough, use #. better


    // Here we need to activate module
    if(!if_par || !if_par->is_hide) {
        if(!(strncmp(in_str, "#begmod ", 8))) {
            v2_start_mod(in_str+8, 0);
            return(0);
        }

        if(!(strncmp(in_str, "#addmod ", 8))) {
            v2_start_mod(in_str+8, 1);
            return(0);
        }

        if(!(strncmp(in_str, "#endmod", 7))) {
            if(!v2_mod_ptr) {
		v2_tpl_prnerr("<!-- Unexpected #endmod directive -->\n");
                return(0);
            }
            if(in_str[7] == ' ') {
                if(in_str[8]) {
                    if(strcmp(in_str+8, v2_mod_ptr->key)) {
			v2_tpl_prnerr("<!-- Probably wrong module names start:%s != end:%s  -->\n",
                                                v2_mod_ptr->key, in_str+8);
                    }
                }
            }
            v2_mod_ptr=NULL;
            return(0);
        }

        if(v2_mod_ptr) {
            v2_add_mod_str(in_str);
            return(0);
        }
    }

    if(!strncmp(in_str, "#else", 5))  return(else_if(in_str+6));
    if(!strncmp(in_str, "#endif", 6)) return(end_if(in_str+6));
    if(!strncmp(in_str, "#fi", 3))    return(end_if(in_str+3)); // The same as endif

    if(!strncmp(in_str, "#elif", 5))  {
	int rc=0;
	if(!(rc=elif_if(in_str+3))) return(0); // Function found and OK
	if(rc == V2_TPL_END)        return(rc); // End of template
	// So, if_fun not found - continue;
    }

    // IF directives
    if(!strncmp(in_str, "#if", 3)) {

	   // Try to run funct
	if(!strncmp(in_str, "#ifexec ", 8)) {
	    if(!run_fun_lst(in_str+8, &par_res)) {
		if(par_res==V2_TPL_END) return(V2_TPL_END);
		start_if(par_res);
	    }
	    return(0);
	}
	if(!strncmp(in_str, "#ifnexec ", 9)) {
	    if(!run_fun_lst(in_str+9, &par_res)) {
		if(par_res==V2_TPL_END) return(V2_TPL_END);
		start_ifn(par_res);
	    }
	    return(0);
	}
        // Run any "if..." functions
        if(!run_fun_lst(in_str+1, &par_res)) {
	    if(par_res==V2_TPL_END) return(V2_TPL_END);
	    start_if(par_res);
	    return(0);
	}
    }

    if(if_par && if_par->is_hide) return(0);

    if(!(strncmp(in_str, "#stop", 5))) { // Stop show current template
	while(if_par) end_if("");
        return(1);
    }

    if(!(strncmp(in_str, "#return", 5))) { // The same as #stop
	while(if_par) end_if("");
        return(1);
    }

    if(!(strncmp(in_str, "#end", 4))) { // Stop to show all templates!!!
	// Close all if params in current level
	while(if_par && (if_par->inc==includes_num)) end_if("");
        return(V2_TPL_END);
    }

    if(!(strncmp(in_str, "#exit", 5))) { // Stop to show all templates!!! The same as end
	// Close all if params in current level
	while(if_par && (if_par->inc==includes_num)) end_if("");
        return(V2_TPL_END);
    }

    if(!(strncmp(in_str, "#exec ", 6))) {
	run_fun_lst(in_str+6, &par_res);
	if(par_res==V2_TPL_END) return(V2_TPL_END);
	if(par_res && v2_tpl_out_err) v2_tpl_out_err(par_res);
        return(0);
    }

    if(in_str[0] ==  '#') {
	run_fun_lst(in_str+1, &par_res);
        if(par_res==V2_TPL_END) return(V2_TPL_END);
	if(par_res && v2_out_file)    v2_tpl_printf("%s\n", in_str); // Non web version output just input string
	if(par_res && v2_tpl_out_err) v2_tpl_out_err(par_res);
	return(0);
    }



    // So, replace variables to its values
    out_str[0]='\0';
    for(x=0; x < len_s; x++) {
        switch(is_var) {
        case 0: {
            if(in_str[x] == v2_start1_char) { is_var++; }
            else {
                out_str[len_o++] = in_str[x];
            }
            break;
        }
        case 1: {
            if(in_str[x] == v2_start2_char) { is_var++; }
            else {
                out_str[len_o++] = v2_start1_char;
                out_str[len_o++] = in_str[x];
                is_var=0;
            }
            break;
        }
        case 2: {
            var_str[len_t++] = in_str[x];
            if((len_t > 1) && (len_t <= V2_TPL_VARLEN) && (in_str[x] == v2_finish_char)) {
                var_str[len_t] = '\0';
                if(show_var(var_str) != 2){
                    if(var_rep) {
                        // sprintf(out_str+len_o, "%s", var_rep);
                        out_str[len_o] = '\0';
			v2_tpl_printf("%s%s", out_str, var_rep);
                        out_str[0] = '\0';
                        len_o=0;
                    } else {
                        // sprintf(out_str+len_o, "none");
                        out_str[len_o] = '\0';
                    }
                } else {
                    out_str[len_o++] = v2_start1_char;
                    out_str[len_o++] = v2_start2_char;
                    sprintf(out_str+len_o, "%s", var_str);
                }
                len_o=strlen(out_str);
                len_t=0;
                is_var=0;
            } else if((len_t > V2_TPL_VARLEN) || (in_str[x] == v2_start1_char)) {
                if(in_str[x] == v2_start1_char) {
                    is_var=1;
                    len_t--;
                } else {
                    is_var=0;
                }

                if(len_t) {
                    var_str[len_t] = '\0'; /* After possible len_t--; */
                    out_str[len_o++] = v2_start1_char;
                    out_str[len_o++] = v2_start2_char;
		    len_o+=sprintf(out_str+len_o, "%s", var_str);
		    //len_o=strlen(out_str);
                    len_t=0;
                }
            }
            break;
        }
        }
    }
    if(is_var==2) {
        var_str[len_t] = '\0'; /* After possible len_t--; */
        out_str[len_o++] = v2_start1_char;
        out_str[len_o++] = v2_start2_char;
	len_o+=sprintf(out_str+len_o, "%s", var_str);
        //len_o=strlen(out_str);
    } else out_str[len_o] = '\0';
    
    v2_tpl_printf("%s\n", out_str);
    return(0);
}

/* ======================================================= */
void v2_check_lang(void) {
    int register x;
    char *accept_lang=NULL;

    if(v2_tpl.lang) return;

    accept_lang=getenv("HTTP_ACCEPT_LANGUAGE");
    if(accept_lang) {
	for(x=0; accept_lang[x]; x++) {
	    accept_lang[x]=tolower(accept_lang[x]);
	    if((accept_lang[x] == '-') || (accept_lang[x] == ',') || (accept_lang[x] == ';')) accept_lang[x]='\0';
	}
	v2_tpl.lang=strcpy((char *)malloc(strlen(accept_lang)+1), accept_lang);
    } else {
	v2_tpl.lang=(char *)calloc(1, 1);
    }
    return;
}
/* ======================================================= */
int v2_check_module(char *modname) {
    v2_mod_lst_t *mod_tmp=NULL;

    v2_mod_tek=NULL;
    if(!modname)    return(1);
    if(!modname[0]) return(1);

    for(mod_tmp=v2_mod_lst; mod_tmp && !v2_mod_tek; mod_tmp=mod_tmp->next) {
        if(strcmp(mod_tmp->key, modname)) continue;
        v2_mod_tek=mod_tmp;
    }
    if(!v2_mod_tek) return(1);

    return(0);
}
/* ======================================================= */
int v2_inc_add(char *in_tpl) {
    char *p=NULL;

    if(includes_num > 16) return(includes_num); // Mark to deep includes

    if((p=strrchr(in_tpl, '/')) && *(++p)) {
	v2_lstr.head(&inc_lst, p);
    } else {
	v2_lstr.head(&inc_lst, in_tpl); // A bit strange
    }

    inc_lst->num = ++includes_num;

    v2_tpl_file=inc_lst->key;

    return(0);
}
/* ======================================================= */
int v2_inc_del(void) {
    str_lst_t *inc_tmp=inc_lst;

    if(!inc_lst) return(0); // Hmmm

    inc_lst = inc_lst->next;

    inc_tmp->next=NULL;
    v2_lstr.free(&inc_tmp);

    if(inc_lst) {
	includes_num = inc_lst->num;
	v2_tpl_file=inc_lst->key;
    } else {
	includes_num = 0;
	v2_tpl_file=NULL;
    }

    return(0);
}
/* ======================================================= */
int v2_out_module(char *modname) {
    v2_def_lst_t *def_tmp=NULL;
    int is_stop=0;
    int rc=0;

    if(v2_check_module(modname)) {
	v2_tpl_prnerr("<!-- Module without name cann't be printed  -->\n");
        return(0);
    }

    includes_num++;
    if(includes_num>16) {
        includes_num--;
	v2_tpl_prnerr("<!-- Module directive rang expired (%d) [%s]-->\n", includes_num, modname);
        return(0);
    }

    for(def_tmp=v2_mod_tek->pstr; def_tmp && !rc; def_tmp=def_tmp->next) {
	if(is_stop) continue;
	if(!strcmp(def_tmp->str, "#stopmod")) {
	    is_stop=1;
            continue;
	}

	rc=v2_trans_str(def_tmp->str);
    }
    includes_num--;

    if(includes_num) { // this is inside template include
	if(rc==V2_TPL_END) return(V2_TPL_END); // Try make #end option
    }

    return(0);
}
/* ======================================================= */
int v2_out_merge(char *filename) {
    FILE *ftt;
    char strtmp[MAX_STRING_LEN];
    int rc=0;

    if(!filename || !filename[0]) return(2);

    if(v2_inc_add(filename)) {
	v2_tpl_prnerr("<!-- Merge directive rang epired (%d) [%s]-->\n", includes_num, filename);
        return(0);
    }
    errno=0;
    if(filename[0]==':') {
	ftt=popen(filename+1, "r");
    } else {
	ftt=fopen(filename, "r");
    }

    //v2_ot_strf("Run: %s<br>\n", filename);

    if(!ftt) {
	v2_add_warn("Can not open file: %s [ %s ]", filename, strerror(errno));
	//includes_num--;
	v2_inc_del();
	return(1);
    }

    while((!(v2_getline(strtmp, MAX_STRING_LEN, ftt))) && !rc) {
	if(v2_out_str) {
	    rc=v2_out_str(strtmp);
	} else {
	    rc=v2_trans_str(strtmp);
	}
    }
    if(filename[0]==':') {
	if((rc=pclose(ftt))) {
	    v2_add_warn("Can not close file[rc=%d, ret=%d]: %s [ %s ]", rc, rc/256, filename, strerror(errno));
	    v2_add_warn("Current Dir: %s", getcwd(strtmp, MAX_STRING_LEN));
	}
	//v2_tpl_printf("TEst: %s [ %s ]<br>\n", filename, strerror(errno));
   } else {
	fclose(ftt);
    }
    //includes_num--;
    v2_inc_del();

    if(includes_num) { // this is inside template include
	if(rc==V2_TPL_END) return(V2_TPL_END); // Try make #end option
    }

    //v2_tpl_prnbuff();

    return(0);
}

/* ======================================================= */
int v2_out_mergef(char *in_fmt, ...) {
    char out[MAX_STRING_LEN];

    VL_STR(out, MAX_STRING_LEN, in_fmt);

    return(v2_out_merge(out));
}
/* ======================================================= */
int v2_out_include(char *filename) {
    //char ou_fl[MAX_STRING_LEN];

    if(!filename || !filename[0]) return(2);

    if(web_root_dir) {
	return(v2_out_mergef("%s%s%s", web_root_dir, (filename[0]=='/')?"":"/", filename));
    }

    return(v2_out_merge(filename));
}
/* ======================================================= */
int v2_out_includef(char *in_fmt, ...) {
    char out[MAX_STRING_LEN];

    VL_STR(out, MAX_STRING_LEN, in_fmt);

    return(v2_out_include(out));
}
/* ============================================================ */
int v2_tpl_is_dir(char *in_dir) {
    struct stat st;

    if(!v2_is_par(in_dir))   return(0); // Not input param
    if(stat(in_dir, &st))    return(0); // Not found
    if(!S_ISDIR(st.st_mode)) return(0); // Not dir

    return(1);
}
/* ============================================================ */
// Check file and set link's extention if exists
int v2_tpl_is_file(char *in_dir, char *in_file) {
    struct stat st;
    char s[MAX_STRING_LEN];
    char *p=NULL;
    ssize_t ln=0;

    if(stat(v2_s(s, MAX_STRING_LEN, "%s/%s", in_dir, in_file), &st)) return(0); // Not found
    if(!S_ISREG(st.st_mode))                                         return(0); // Not file

    if(!v2_out_str) return(1); // Not needed check lang if not transfer function

    if((ln=readlink(in_dir, s, MAX_STRING_LEN)) > 0) {
	s[ln] = '\0'; // Make end of the string
	if((p=strrchr(s, '.'))) p++;
    }

    v2_let_var(&v2_tpl.lext, p); // Set (or reset) link extention variable

    return(1); // OK
}
/* ============================================================ */
// {tpl}/{skin}/{lang}/{page}.html
// {tpl}/{skin}/{page}.html
// {tpl}/{lang}/{page}.html
// {tpl}/{page}.html
//
int v2_tpl_set_sknl(void) {
    char s[MAX_STRING_LEN];

    if(v2_tpl.skn_lst) return(0); // Already generated

    // v2_tpl_is_dirf(char *in_dir, ...) - in todo

    if(v2_is_par(v2_skin_name) && v2_is_par(v2_tpl.lang) && v2_tpl_is_dir(v2_s(s, MAX_STRING_LEN, "%s/%s/%s", src_tpl_dir, v2_skin_name, v2_tpl.lang)))
	v2_add_lstr_tail(&v2_tpl.skn_lst, s, NULL, 0, 0);

    if(v2_is_par(v2_skin_name) && v2_tpl_is_dir(v2_s(s, MAX_STRING_LEN, "%s/%s", src_tpl_dir, v2_skin_name)))
	v2_add_lstr_tail(&v2_tpl.skn_lst, s, NULL, 0, 0);

    if(v2_is_par(v2_tpl.lang) && v2_tpl_is_dir(v2_s(s, MAX_STRING_LEN, "%s/%s", src_tpl_dir, v2_tpl.lang)))
	v2_add_lstr_tail(&v2_tpl.skn_lst, s, NULL, 0, 0);

    v2_add_lstr_tail(&v2_tpl.skn_lst, src_tpl_dir, NULL, 0, 0); // Default path


    return(0);
}

/* ============================================================ */
int v2_out_template(char *filename) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *fnd_tpl=NULL;
    //char s[MAX_STRING_LEN];
    char *prev_lext=NULL;
    //int fnd=0;
    int rc=0;

    v2_check_lang();

    if(!v2_is_par(filename)) filename="index.html";

    //v2_add_debug(8, "TPL File: %s/%s/%s", v2_nn(v2_skin_name), v2_nn(v2_tpl.lang), filename);
    //v2_tpl_prnerr("<!-- Output template[%d]: %s-->\n", includes_num, filename);
    //v2_add_warn("TPL File: %s", filename);

    if(!src_tpl_dir || !*src_tpl_dir) return(v2_out_merge(filename));

    // Detect presented directories - needed for tpld.s too
    v2_tpl_set_sknl();

    prev_lext=v2_tpl.lext;
    v2_tpl.lext=NULL;

    FOR_LST(str_tmp, v2_tpl.skn_lst) {
	v2_lstr.astrf(str_tmp, "%s/%s", str_tmp->key, filename);

	if(!fnd_tpl) {
	    str_tmp->num=v2_tpl_is_file(str_tmp->key, filename);
	    if(str_tmp->num) fnd_tpl=str_tmp;
	}
    }

    if(fnd_tpl) rc=v2_out_merge(fnd_tpl->str);

    //rc=1; // We have not show it yet...
    //FOR_LST_IF(str_tmp, fnd, v2_tpl.skn_lst) {
    //    fnd=v2_tpl_is_file(str_tmp->key, filename);
    //    //v2_add_warn("%s TPL attempt: %s/%s", fnd?"YES":"not", str_tmp->key, filename);
    //    if(!fnd) continue;
    //
    //    //rc=v2_out_merge(v2_s(s, MAX_STRING_LEN-fnd, "%s/%s", str_tmp->key, filename));
    //    rc=v2_out_mergef("%s/%s", str_tmp->key, filename);
    //}

    v2_freestr(&v2_tpl.lext);
    v2_tpl.lext=prev_lext;

    //v2_add_warn("TPL Code: %d", rc);

    return(rc);
}
/* ======================================================================== */
int v2_out_templatef(char *in_file, ...) {
    char strtpl[MAX_STRING_LEN];

    VL_STR(strtpl, MAX_STRING_LEN, in_file);
    return(v2_out_template(strtpl));
}
/* ======================================================================== */
static int v2_tpl_d_filter(char *in_file, void *in_data) {
    str_lst_t *exts=(str_lst_t*)in_data;
    char *p=NULL;

    // Select file
    if(!in_file || !*in_file || *in_file == '.') return(1);
    if(!(p=strrchr(in_file, '.')))               return(1);
    if(!v2_lstr_key(exts, p))                    return(1);
    if(strspn(in_file, "0123456789qwertyuiopasdfghjklzxcvbnm_") != p-in_file) return(1); // Forbidden symbols

    if(v2_lstr_key(exts->link, in_file))     return(1); // Exclue not needed files

    return(0); // Liiks like OK
}
/* ======================================================================== */
// template directory list
static int v2_tpls_d_fun(char *in_str) {
    str_lst_t *dirs=NULL; // Directories
    str_lst_t *exts=NULL; // Extentios
    str_lst_t *fils=NULL; // List of files
    str_lst_t *noit=NULL; // List of files
    str_lst_t *str_tmp=NULL;
    str_lst_t *tpt_tmp=NULL;
    char strtmp[MAX_STRING_LEN];
    char s[MAX_STRING_LEN];
    int len=0;
    int rc=0;

    if(!in_str || !*in_str) return(0); // Nothing to read

    //v2_add_warn("Filets: %s", in_str);

    while(in_str[0]) {
	if(!(len=v2_getword(strtmp, in_str, ' '))) continue; // skip extra-spaces

	if(strtmp[0] == '.') { // Extention
	    v2_add_lstr_head(&exts, strtmp, NULL, 0, 0);
	    continue;
	}

	if(strtmp[0] == '-') { // No show it
	    v2_add_lstr_head(&noit, strtmp+1, NULL, 0, 0);
	    continue;
	}

	if(strtmp[len-1] == '/') {strtmp[len-1] = '\0'; len--;}

	if(strtmp[0] == '/') {
	    v2_add_lstr_head(&dirs, strtmp, strtmp, len, 0); // Full path
	    continue;
	}

	tpt_tmp=NULL;

	//v2_add_warn("Filets: %s", strtmp);

	//FOR_LST_IF(str_tmp, tpt_tmp, v2_tpl.skn_lst) {
	//    if(!v2_tpl_is_dir(v2_s(s, MAX_STRING_LEN, "%s/%s", str_tmp->key, strtmp))) continue;
	//    tpt_tmp=v2_add_lstr_tail(&dirs, strtmp, s, len, 0);
	//    //v2_add_warn("TPL File: %s/%s", str_tmp->key, strtmp);
	//}
	v2_add_lstr_tail(&dirs, strtmp, v2_s(s, MAX_STRING_LEN, "%s/%s", v2_st(src_tpl_dir, "/"), strtmp), len, 0);
    }

    if(!dirs) return(0); // Nothing to show
    if(!exts) v2_add_lstr_head(&exts, ".html", NULL, 0, 0); // Default value for extention
    if(!noit) {
	v2_add_lstr_head(&noit, "index.html", NULL, 0, 0); // Default exclusion
	v2_add_lstr_head(&noit, "index.htm", NULL, 0, 0); // Default exclusion
    }

    exts->link=noit;

    FOR_LST(str_tmp, dirs) {
	//if(str_tmp->key[0] == '/') {
	//    rc=v2_readdir(&fils, &v2_tpl_d_filter, (void*)exts, "%s", str_tmp->key);
	//} else {
	//    if(!src_tpl_dir) continue; // Hmm, where is to get tihs file
	//    rc=v2_readdir(&fils, &v2_tpl_d_filter, (void*)exts, "%s/%s", src_tpl_dir, str_tmp->key);
	//}

	rc=v2_lstr_rdir(&fils, &v2_tpl_d_filter, (void*)exts, "%s", str_tmp->str);

	//v2_add_warn("%s (%s)", str_tmp->key, str_tmp->str);

	if(rc) {
	    v2_add_debug(8, "TPLS.D: Can not read dir: %s [%s]", str_tmp->key, strerror(errno));
	    continue;
	}

	FOR_LST(tpt_tmp, fils) {
	    if(!tpt_tmp->dstr) v2_let_varf(&tpt_tmp->dstr, "%s/%s", str_tmp->key, tpt_tmp->key);
	    //v2_add_warn("%s %s (%s)", str_tmp->key, tpt_tmp->key, str_tmp->str);
	}
    }
    v2_lstr_sort(&fils, NULL);

    rc=0;
    FOR_LST_IF(str_tmp, rc, fils) {
	if(!str_tmp->dstr) continue;
	if(str_tmp->dstr[0] == '/') {
	    rc=v2_out_merge(str_tmp->dstr); // RC?
	} else {
	    rc=v2_out_template(str_tmp->dstr); // RC?
	}
    }

    v2_lstr_free(&dirs);
    v2_lstr_free(&exts);
    v2_lstr_free(&noit);
    v2_lstr_free(&fils);

    return(rc);
}
/* ============================================================ */
int v2_out_strlst(str_lst_t *in_lst) {
    str_lst_t *str_tmp=NULL;
    int rc=0;

    FOR_LST_IF(str_tmp, rc, in_lst) {
	if(str_tmp->is_rmv) continue;
        if(!str_tmp->key) continue;
	rc=v2_trans_str(str_tmp->key);
    }

    return(rc);
}
/* ======================================================= */
// tpl_lst_func
/* ======================================================= */
v2_tpl_lst_t *v2_fnd_tpl_lst(v2_tpl_lst_t *tpl_head, char *in_key) {
    v2_tpl_lst_t *tpl_tmp=NULL;
    v2_tpl_lst_t *tpl_out=NULL;

    if(!in_key || !in_key[0]) return(NULL);

    FOR_LST_IF(tpl_tmp, tpl_out, tpl_head) {
        if(!strcmp(tpl_tmp->key, in_key)) tpl_out=tpl_tmp;
    }

    return(tpl_out);
}
/* ======================================================= */
v2_tpl_lst_t *v2_add_tpl_lst(v2_tpl_lst_t **tpl_ptr, char *in_key) {
    v2_tpl_lst_t *tpl_tmp=NULL;
    v2_tpl_lst_t *tpl_ttt=NULL;

    if(!in_key || !in_key[0]) return(NULL);
    if(!tpl_ptr)              return(NULL); // Rare case

    if((tpl_tmp=v2_fnd_tpl_lst(*tpl_ptr, in_key))) return(tpl_tmp);

    if(!(tpl_tmp=(v2_tpl_lst_t *)calloc(sizeof(v2_tpl_lst_t), 1))) return(NULL);
    v2_let_var(&tpl_tmp->key, in_key);

    if(!*tpl_ptr) {
	*tpl_ptr=tpl_tmp;
    } else {
	FOR_LST_NEXT(tpl_ttt, *tpl_ptr);
	tpl_ttt->next=tpl_tmp;
	tpl_ttt->next->prev=tpl_ttt;
        tpl_ttt=NULL;
    }

    return(tpl_tmp);
}
/* ======================================================= */
int v2_del_tpl_lst(v2_tpl_lst_t **tpl_ptr, char *in_key) {
    v2_tpl_lst_t *tpl_tmp=NULL;

    if(!in_key || !in_key[0]) return(0);
    if(!tpl_ptr)              return(0); // Rare case

    if(!(tpl_tmp=v2_fnd_tpl_lst(*tpl_ptr, in_key))) return(0); // Not found

    if(tpl_tmp->next) tpl_tmp->next->prev=tpl_tmp->prev;
    if(tpl_tmp->prev) {
	tpl_tmp->prev->next=tpl_tmp->next;
    } else {
        *tpl_ptr=tpl_tmp->next;
    }
    //if(tpl_tmp==(*tpl_ptr)) *tpl_ptr=(*tpl_ptr)->next;

    v2_freestr(&tpl_tmp->key);
    v2_freestr(&tpl_tmp->help);
    v2_freestr(&tpl_tmp->chap);

    if(tpl_tmp->pstr && (*tpl_tmp->pstr)==tpl_tmp->int_var)  v2_freestr(&tpl_tmp->int_var);

    free(tpl_tmp);
    tpl_tmp=NULL;

    return(0);
}
/* ======================================================= */
// Add Init fun
int v2_tpl_init(char *in_key, int (*in_fun)(char *)) {
    v2_tpl_lst_t *lst_tmp=NULL;

    if(!in_key || !*in_key || !in_fun) return(1); // Something not found

    if(!(lst_tmp=v2_add_tpl_lst(&v2_ini_fun, in_key))) return(1); // Not added
    lst_tmp->fun=in_fun;

    return(0);
}
/* ======================================================= */
/* ======================================================= */
int v2_del_def_lst(char *def_format, ...) {
    va_list vl;
    v2_def_lst_t *def_tmp=NULL;
    v2_def_lst_t *def_ttt=NULL;
    char strtmp[MAX_STRING_LEN];
    char *hlp=NULL;
    //register int x, y;

    if(!def_format) { // == NULL Clear define list
	while(v2_def_lst) {
	    def_tmp=v2_def_lst;
	    v2_def_lst=v2_def_lst->next;
            if(def_tmp->str)  free(def_tmp->str);
	    if(def_tmp->help) free(def_tmp->help);
	    free(def_tmp);
	}
	def_tmp=NULL;
	return(0);
    }
    if(!def_format[0]) return(1);

    va_start(vl, def_format);
    vsprintf(strtmp, def_format, vl);
    va_end(vl);

    //for(x=0, y=0; strtmp[x] && !y; x++) {
    //    if((strtmp[x] != ' ') && (strtmp[x] != '\t')) continue;
    //    strtmp[x] = '\0';
    //    y++;
    //}

    if((hlp=strchr(strtmp, ' ')))  (*hlp)='\0';
    if((hlp=strchr(strtmp, '\t'))) (*hlp)='\0';

    if(strlen(strtmp) > V2_TPL_VARLEN) return(1);

    for(def_tmp=v2_def_lst; def_tmp && !def_ttt; def_tmp=def_tmp->next) {
        if(strcmp(def_tmp->str, strtmp)) continue;
        def_ttt=def_tmp;
    }
    if(!def_ttt) return(0);

    //def_ttt->is_del=1;

    if(def_ttt->next) def_ttt->next->prev=def_ttt->prev;
    if(def_ttt->prev) {
	def_ttt->prev->next=def_ttt->next;
    } else {
        v2_def_lst=def_ttt->next;
    }

    if(def_ttt->str)  free(def_ttt->str);
    if(def_ttt->help) free(def_ttt->help);
    free(def_ttt);
    def_ttt=NULL;
    return(0);
}
/* ======================================================= */
int v2_add_def_lst(char *def_format, ...) {
    va_list vl;
    v2_def_lst_t *def_tmp=NULL;
    v2_def_lst_t *def_ttt=NULL;
    char strtmp[MAX_STRING_LEN];
    char *ihelp=NULL;
    register int x;

    if(!def_format) return(1);
    if(!def_format[0]) return(1);

    va_start(vl, def_format);
    vsprintf(strtmp, def_format, vl);
    va_end(vl);

    for(x=0; strtmp[x] && !ihelp; x++) {
        if((strtmp[x] != ' ') && (strtmp[x] != '\t')) continue;
        strtmp[x] = '\0';
        ihelp=strtmp+x+1;
    }

    if(strlen(strtmp) > V2_TPL_VARLEN) return(1);

    for(def_tmp=v2_def_lst; def_tmp && !def_ttt; def_tmp=def_tmp->next) {
        if(strcmp(def_tmp->str, strtmp)) continue;
        def_ttt=def_tmp;
    }

    if(def_ttt) { // We have it already
	//def_ttt->is_del=0;
        def_ttt=NULL;
        return(0);
    }

    if(!(def_tmp=(v2_def_lst_t *)calloc(sizeof(v2_def_lst_t), 1))) return(1);
    def_tmp->str=strcpy((char *)calloc(strlen(strtmp)+1, 1), strtmp);
    if(ihelp) {
	def_tmp->help=(char *)calloc(strlen(ihelp)+4, 1);
        sprintf(def_tmp->help, " - %s", ihelp);
    }

    if(v2_def_lst) {
	def_tmp->next=v2_def_lst;
        def_tmp->next->prev=def_tmp;
    }
    v2_def_lst=def_tmp;
    def_tmp=NULL;
    return(0);
}
/* ======================================================= */
// External function call
int v2_if_def(char *indef) {
    v2_def_lst_t *def_tmp=NULL;
    int rc=0;

    if(!indef || !indef[0]) return(0);

    for(def_tmp=v2_def_lst; def_tmp && !rc; def_tmp=def_tmp->next) {
        if(strcmp(def_tmp->str, indef)) continue;
        rc=1;
    }

    return(rc);
}
/* ======================================================= */
char *v2_get_var(char *strkey) {
    v2_key_lst_t *key_tmp=NULL;
    char *outs=NULL;

    if((key_tmp=find_key_lst(strkey))) outs=*key_tmp->pstr;

    return(v2_nn(outs));
}
/* ======================================================= */
char **v2_get_var_adr(char *strkey) {
    v2_key_lst_t *key_tmp=NULL;
    char **auts=NULL;

    if((key_tmp=find_key_lst(strkey))) auts=key_tmp->pstr;

    return(auts);
}
/* ======================================================= */
// If str_key == "^someword" -> "SOMEWORD" (upper case)
char **v2_set_var(char *str_key, char *str_val) {
    char strtmp[MAX_STRING_LEN];
    char *key=str_key;
    int res=0;

    if(!key || !*key) return(NULL);

    if(*key == '^') { // Transfer Variable name to Upper case
	key++;
	v2_toupper(key);
    }

    res=snprintf(strtmp, MAX_STRING_LEN, "%s ", key);

    if(str_val && *str_val) {
	res+=snprintf(strtmp+res, MAX_STRING_LEN-res, "%s", str_val);
    }

    return(add_int_var(strtmp));
}
/* ======================================================= */
char **v2_set_varf(char *str_key, char *str_val, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, str_val);
    return(v2_set_var(str_key, strtmp));
}
/* ======================================================= */
void v2_add_var_lst(char *str_key, char **str_var) {
    v2_key_lst_t *key_tmp=NULL;
    //v2_key_lst_t *key_tek=NULL;
    char strtmp[MAX_STRING_LEN];
    char strtm1[MAX_STRING_LEN+5];
    char *ihelp=NULL;
    char *ichap=NULL;
    char *key=str_key;
    register int x;
    //int is_end=0;

    if(!key || !*key) return;

    if(*key == '^') { // Transfer Variable name to Upper case
	key++;
	for(x=0; *(key+x) && (*(key+x)!=' '); x++) *(key+x)=toupper(*(key+x));
    }

    sprintf(strtmp, "%s", key);

    //for(x=0; !is_end; x++) {
    //    if(!strtmp[x]) {
    //        is_end=1;
    //    } else if(!ihelp) {

    for(x=0; strtmp[x]; x++) {
	if(!ihelp) {
            if((strtmp[x] != ' ') && (strtmp[x] != '\t')) continue;
            strtmp[x] = '\0';
            ihelp=strtmp+x+1;
        } else if(!ichap) {
            if(strtmp[x] != '|') continue;
            strtmp[x] = '\0';
            ichap=strtmp+x+1;
        }
    }

    if(strlen(strtmp) < 2) return;
    if(strlen(strtmp) > V2_TPL_VARLEN) return;

    //if(!str_var) return;

    sprintf(strtm1,"%s%c", strtmp, v2_finish_char);

    if(!(key_tmp=find_key_lst(strtm1))) {

	if(!(key_tmp=(v2_key_lst_t *)calloc(sizeof(v2_key_lst_t), 1))) return;
	//key_tmp->key=strcpy((char *)malloc(strlen(strtm1)+1), strtm1);
	key_tmp->key=v2_strcpy(strtm1);

	if((key_tmp->next=v2_key_lst)) key_tmp->next->prev=key_tmp;
	v2_key_lst=key_tmp;

	if(ihelp) {
	    //key_tmp->help=calloc(strlen(ihelp)+4, 1);
	    //sprintf(key_tmp->help, " - %s", ihelp);
	    key_tmp->help=v2_string(" - %s", ihelp);
	}
	if(ichap && *ichap) key_tmp->chap=v2_strcpy(ichap);
	    //strcpy((char *)malloc(strlen(ichap)+1), ichap);
    }

    if(str_var) {
        key_tmp->pstr=str_var;
    } else {
        key_tmp->pstr=&(key_tmp->int_var);
    }

    key_tmp=NULL;
    return;
}

/* ======================================================= */
// NUILL function
int v2_fun_null(char *in_str) {return(0);}
/* ======================================================= */
// Just echo function
static int v2_fun_echo(char *in_str) {v2_tpl_printf("%s\n", v2_nn(in_str)); return(0);}
/* ======================================================= */
void v2_add_fun_lst(char *str_key, int (*str_fun)(char *)) {
    v2_fun_lst_t *fun_tmp=NULL;
    v2_fun_lst_t *fun_tek=NULL;
    char strtmp[MAX_STRING_LEN];
    char *ihelp=NULL;
    register int x;

    if(!str_key) return;
    if(!str_key[0]) return;

    sprintf(strtmp, "%s", str_key);

    for(x=0; strtmp[x] && !ihelp; x++) {
        if((strtmp[x] != ' ') && (strtmp[x] != '\t')) continue;
        strtmp[x] = '\0';
        ihelp=strtmp+x+1;
    }

    if(strlen(strtmp) > V2_TPL_VARLEN) return;

    if(!(fun_tmp=(v2_fun_lst_t *)calloc(sizeof(v2_fun_lst_t), 1))) return;
    fun_tmp->key=strcpy((char *)malloc(strlen(strtmp)+1), strtmp);
    if(ihelp) {
	if(ihelp[0] == '.') {
            fun_tmp->help=(char *)calloc(strlen(ihelp)+3, 1);
            sprintf(fun_tmp->help, " - %s", ihelp+1);
	    //fun_tmp->is_hlp=1;
        } else {
            fun_tmp->help=(char *)calloc(strlen(ihelp)+4, 1);
            sprintf(fun_tmp->help, " - %s", ihelp);
        }
    }
    fun_tmp->is_app=is_init_ifun;
    fun_tmp->fun=str_fun?str_fun:&v2_fun_null;

    if(!v2_fun_lst) {
        v2_fun_lst=fun_tmp;
    } else {
        for(fun_tek=v2_fun_lst; fun_tek->next; fun_tek=fun_tek->next);
        fun_tek->next=fun_tmp;
        fun_tek=NULL;
    }
    fun_tmp=NULL;
    return;
}

/* ======================================================= */
int v2_def_function(int (*str_fun)(char *), char *key_format, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, key_format);
    if(!strtmp[0]) return(0);

    v2_add_fun_lst(strtmp, str_fun);
    return(0);
}
/* ======================================================= */
// Set or reset locale and charset
char *v2_tpl_locale(char *in_locale) {
    char *p=NULL;

    if(v2_let_var(&v2_tpl.locale, in_locale)) {
	if((p=strrchr(v2_tpl.locale, '.'))) p++;
    }

    v2_let_var(&v2_tpl.charset, p);

    v2_add_var_lst("V2_TPL_LOCALE Original locale",     &v2_tpl.locale);
    v2_add_var_lst("V2_TPL_CHARSET Original CharSet",   &v2_tpl.charset);
    v2_add_var_lst("V2_TPL_FROMSET Transcoded charset", &v2_tpl.lext);

    return(v2_tpl.locale);
}
/* ======================================================= */
/* ======================================================= */
void v2_set_web_dir(char *web_root) {
    v2_init_ifun();
    if(web_root) {
	v2_let_var(&web_root_dir, web_root);
    }
    v2_tpl_webout=1; // Default value, can be changed
    return;
}
/* ======================================================= */
void v2_set_tpl_dir(char *tpl_root) {

    v2_init_ifun();
    if(!tpl_root) return;

    v2_let_var(&src_tpl_dir, tpl_root);
    return;
}
/* ======================================================= */
int v2_p_o_s(char *in_str, char *fibr) {
    char strtmp[MAX_STRING_LEN+256];
    register int x, y;

    if(v2_out_file) {
	v2_tpl_printf("%s%s\n", in_str, v2_nn(fibr));
        return(0);
    }

    strtmp[0]='\0';
    y=0;
    for(x=0; in_str[x]; x++) {
        if(in_str[x] == '<') {
            strtmp[y++] = '&';
            strtmp[y++] = 'l';
            strtmp[y++] = 't';
            strtmp[y++] = ';';
        } else if(in_str[x] == '>') {
            strtmp[y++] = '&';
            strtmp[y++] = 'g';
            strtmp[y++] = 't';
            strtmp[y++] = ';';
        } else {
            strtmp[y++] = in_str[x];
        }
    }
    strtmp[y]='\0';

    v2_tpl_printf("%s%s\n", strtmp, v2_nn(fibr));
    return(0);
}
/* ======================================================= */
int v2_print_def(char *fibr) {
    v2_def_lst_t *def_tmp=NULL;
    char strtmp[MAX_STRING_LEN];

    if(!v2_def_lst) return(v2_p_o_s("Def: no set", fibr));

    for(def_tmp=v2_def_lst; def_tmp && def_tmp->next; def_tmp=def_tmp->next);
    for(;def_tmp; def_tmp=def_tmp->prev) {
	sprintf(strtmp, "Def: \"%s\"%s", def_tmp->str,
		def_tmp->help?def_tmp->help:"");
	v2_p_o_s(strtmp, fibr);
    }
    return(0);
}
/* ======================================================= */
int v2_print_var(char *fibr) {
    v2_key_lst_t *key_tmp=NULL;
    char strtmp[MAX_STRING_LEN];
    int is_shown=0;
    int res=0;

    if(!v2_key_lst) return(v2_p_o_s("Var: no set", fibr));

    is_shown=0;
    for(key_tmp=v2_key_lst; key_tmp && key_tmp->next; key_tmp=key_tmp->next);

    for(; key_tmp; key_tmp=key_tmp->prev) {

    //for(key_tmp=v2_key_lst; key_tmp; key_tmp=key_tmp->next) {
	if(key_tmp->chap) {
	    if(key_tmp!=v2_key_lst) v2_p_o_s("", fibr);
	    if(key_tmp->chap[0] == '.') {
		sprintf(strtmp, "==== %s", key_tmp->chap+1);
	    } else {
		sprintf(strtmp, "==== Variables group: %s", key_tmp->chap);
	    }
	    v2_p_o_s(strtmp, fibr);
	} else if(is_shown) {
	    is_shown=0;
	    v2_p_o_s("", fibr);
	}
	res=sprintf(strtmp, "Var: \"%c%c %s\" = ", v2_start1_char, v2_start2_char, key_tmp->key);

	if(*key_tmp->pstr) {
	    if(strlen(*key_tmp->pstr) > MAX_STRING_LEN - 400) {
		res+=sprintf(strtmp+res, "\"... too long ... (strlen = %ld) ...\"", (long int)strlen(*key_tmp->pstr));
	    } else {
		res+=sprintf(strtmp+res, "\"%s\"", *key_tmp->pstr);
	    }
	} else {
	    res+=sprintf(strtmp+res, "NULL");
	}
	is_shown=0;
	if(key_tmp->help) {
	    if(key_tmp->help[0]) {
		if(key_tmp->help[strlen(key_tmp->help)-1] == '.') {
		    key_tmp->help[strlen(key_tmp->help)-1] = '\0';
		    is_shown=1;
		}
	    }
	    res+=sprintf(strtmp+res, "%s", key_tmp->help);
	}
	v2_p_o_s(strtmp, fibr);
    }

    return(0);
}
/* ======================================================= */
int v2_tpl_warning(char *in_str) {

    v2_add_warn("TPL : %s", v2_st(in_str, "--warning--"));

    return(0);
}
/* ======================================================= */
int v2_tpl_error(char *in_str) {

    v2_add_error("TPL : %s", v2_st(in_str, "--error--"));

    return(0);
}
/* ======================================================= */
int v2_fun_crlf(char *in_str) {

    //if(in_str && in_str[0]) {
    //    v2_tpl_printf("%s\n", in_str);
    //} else {
    //    v2_tpl_printf("\n");
    //}

    v2_tpl_printf("%s\n", v2_nn(in_str));

    return(0);
}
/* ======================================================= */
// Escape characterd of variable
int v2_tpl_varesc(char *in_var) {
    char **pvar=NULL;
    char *out=NULL;
    char *p=NULL;
    size_t cnt=0;
    size_t crr=0;

    if(!(pvar=v2_get_var_adr(in_var))) return(0);

    for(p=*pvar; *p; p++) {
	cnt++;
	if(*p == '\n') crr++;
    }

    if(!crr) return(0); // Nothing escape

    out=(char*)calloc(cnt+crr+1, 1);

    for(p=*pvar, cnt=0; *p; p++) {
	if(*p == '\n') {
	    out[cnt++] = '\\';
	    out[cnt++] = 'n';
	} else {
	    out[cnt++] = *p;
	}
    }

    out[cnt]='\0';

    v2_freestr(pvar);

    *pvar=out;

    return(0);
}
/* ======================================================= */
// Mod/Var functoions
/* ======================================================= */
// Add module content as variuable
int v2_tpl_varmod(char *in_str) {
    char **pvar=NULL;
    char *out=NULL;
    char id[MAX_STRING_LEN];
    size_t spos=loc_buff_cnt; // Start position at buffer

    if(!loc_buff) return(12490); // Works only with allocated buffer

    if(!v2_getword(id, in_str, ' ')) return(12490); // Nothing to show...
    if(!in_str[0])                   return(12492);
    if(!(pvar=v2_set_var(id, "")))   return(12492);
    v2_freestr(pvar); // Clean variable if exists


    if(!v2_getword(id, in_str, ' ')) return(0); // Nothing to show...
    v2_out_module(id);

    out=(char*)loc_buff+spos;
    loc_buff_cnt=spos;

    if(!*out) return(0); // Nothing outputted

    v2_let_var(pvar, out);

    loc_buff[spos] = '\0'; // *out = '\0';

    return(0);
}
/* ======================================================= */
void v2_init_ifun(void) {
    v2_tpl_lst_t *ini_tmp=NULL;

    if(is_init_ifun) return;

    v2_def_function(&v2_print_all,         "printall Prints this info (if \"arg\" given - adds it to the end of each string)");
    v2_def_function(&v2_print_def,         "printdef Prints Defines");
    v2_def_function(&v2_print_var,         "printvar Prints Variables");
    v2_def_function(&v2_show_all,          "showall The same as #printall, but HTML version (if \"arg\" absent - adds <BR> instead)");
    v2_def_function(&v2_print_env,         "printenv Prints System Environment list");
    v2_def_function(&v2_show_env,          "showenv The same as #printenv, but HTML version (if \"arg\" absent - adds <BR> instead)");
    v2_def_function(&v2_show_query,        "showquery Shows query list (if \"arg\" absent - adds <BR> instead)");
    v2_def_function(&v2_show_skn,          "showskn Shows skin/lang templates list");

    v2_def_function(&v2_fun_echo,           "echo String - Just output string as function resilt");

    v2_def_function(&add_fun_var,          "set Var Arg\" - Set variable (Var=Arg)");
    v2_def_function(&fun_varcpy,           "varcpy VarDst VarSrc - Copy value from VarSrc to VarDst");
    v2_def_function(&v2_tpl_varesc,        "varesc Escape some chars at variable content");

    v2_def_function(&v2_tpl_set_env,       "setenv Set environment variable from Var or Const");

    v2_def_function(&v2_add_def_lst_tpl,   "define Defines \"arg\"");
    v2_def_function(&v2_del_def_lst_tpl,   "undef UnDefines \"arg\"");
    v2_def_function(&v2_show_date,         "date Prints current date, arg = format (man strftime)");
    v2_def_function(&v2_out_merge,         "merge Merges file into template from FileSystem");
    v2_def_function(&v2_out_include,       "include Includes file into template from WebRoot");
    v2_def_function(&v2_out_template,      "template Includes file into template from Template Directory");
    v2_def_function(&v2_tpls_d_fun,        "tpls.d - Show all templates from directories");
    v2_def_function(&v2_out_module,        "prnmod Prints module as template");

    v2_def_function(&v2_tpl_varmod,        "varmod Exec module and copy result at variable");

    v2_def_function(&v2_fun_crlf,          "crlf Prints empy string");
    // v2_def_function(&v2_fun_sp,             "sp Skip spaced before function");

    v2_def_function(&v2_check_var,         "if True, if variable value exists");
    v2_def_function(&v2_check_var,         "ifvar True, if variable value exists (w/o diagnostic)");

    v2_def_function(&v2_check_var_set,     "ifvarset True, if variable exists and not empty");
    v2_def_function(&v2_check_not_var_set, "ifnvarset True, if variable does not exists or empty"); // Not needed

    v2_def_function(&v2_cmp_var,           "ifequ True, if (string) VAR_NAME == value");
    v2_def_function(&v2_check_module,      "ifmod True, if Module exists");

    //v2_def_function(&v2_check_def,         "ifdef True, if Def defined");

    v2_def_function(&v2_ifmdef,            "ifdef True, if all Def in the string defined == ifmdef");
    v2_def_function(&v2_ifmdef,            "ifmdef True, if all defines in the string are ok (AND)");
    v2_def_function(&v2_ifodef,            "ifordef True, if any defines in the string are ok (OR)");
    v2_def_function(&v2_ifodef,            "ifodef == ifordef (depricated)");

    v2_def_function(&v2_if_file,           "iffile True, if file exists (any type of file)");
    //v2_def_function(&v2_if_not_file,       "ifnfile True, if file does not exists (any type of file)");

    v2_def_function(&v2_tpl_warning,       "warning Wrine warning message to log file, if exists");
    v2_def_function(&v2_tpl_error,         "error Wrine error message to log file, if exists");

    v2_def_function(&v2_debug_var, "debugvar - list all vars into debug log");

    v2_add_var_lst("V2_TPL_WEB Web server ROOT dir",         &web_root_dir);
    v2_add_var_lst("V2_TPL_DIR Program templaes dir",        &src_tpl_dir);
    v2_add_var_lst("V2_TPL_SKINID current design id",        &v2_skin_name);
    v2_add_var_lst("V2_TPL_FILE current template file name", &v2_tpl_file);
    v2_add_var_lst("V2_TPL_LANG current language code",      &v2_tpl.lang);

    // Init other functions
    FOR_LST(ini_tmp, v2_ini_fun) {
	if(ini_tmp->key && (*ini_tmp->key == '.')) is_init_ifun=1; // Mark applications functions
	if(ini_tmp->fun) ini_tmp->fun("init");
	is_init_ifun=0; // Unmark application functions
    }

    is_init_ifun=1;

    return;
}
/* ======================================================= */
int v2_print_all(char *fibr) {
    //v2_key_lst_t *key_tmp=NULL;
    v2_def_lst_t *def_tmp=NULL;
    v2_fun_lst_t *fun_tmp=NULL;
    v2_mod_lst_t *mod_tmp=NULL;
    char strtmp[MAX_STRING_LEN];
    int is_app=0;
    //int is_shown=0;
    //int res=0;

    no_show_del=1; // Do not show deleted things

    v2_p_o_s("===================== V2 Template settings ========================", fibr);

    if(web_root_dir) {
        sprintf(strtmp, "WebRoot: %s", web_root_dir);
        v2_p_o_s(strtmp, fibr);
    } else {
        v2_p_o_s("WebRoot: no set", fibr);
    }

    if(src_tpl_dir) {
        sprintf(strtmp, "Templates: %s", src_tpl_dir);
        v2_p_o_s(strtmp, fibr);
    } else {
        v2_p_o_s("Templates: no set", fibr);
    }

    sprintf(strtmp, "Vars format: %c%cVARIABLE%c", v2_start1_char, v2_start2_char, v2_finish_char);
    v2_p_o_s(strtmp, fibr);

    v2_p_o_s("", fibr);

    for(is_app=0; is_app<2; is_app++) {
        if(is_app==0) {
            v2_p_o_s("Build-in functions:", fibr);
            v2_p_o_s("Fun: \"##...any string...\" - Comment string - rest ignored", fibr);
	    //v2_p_o_s("Fun: \"#set Var Arg\" - Set variable (Var=Arg)", fibr);
        } else {
            v2_p_o_s("Application functions:", fibr);
        }

        for(fun_tmp=v2_fun_lst; fun_tmp; fun_tmp=fun_tmp->next) {
	    if(is_app != fun_tmp->is_app) continue;
            if(!is_app && !strncmp(fun_tmp->key, "if", 2)) continue;
	    //sprintf(strtmp, "Fun: \"#%s [arg%s]\"%s", fun_tmp->key, fun_tmp->is_hlp?" | help":"", fun_tmp->help?fun_tmp->help:"");
	    sprintf(strtmp, "Fun: \"#%s [arg]\"%s", fun_tmp->key, fun_tmp->help?fun_tmp->help:"");
            v2_p_o_s(strtmp, fibr);
        }
        if(!is_app) {
            v2_p_o_s("Fun: \"#begmod arg\" - Starts text module with name \"arg\"", fibr);
            v2_p_o_s("Fun: \"#addmod arg\" - Add text to module with name \"arg\"", fibr);
            v2_p_o_s("Fun: \"#endmod\" - Finish text module (Modules can not be re-enterable", fibr);

	    v2_p_o_s("Note: ---- all #if.. functions also have forms #ifn... and #elif... ----", fibr);

	    for(fun_tmp=v2_fun_lst; fun_tmp; fun_tmp=fun_tmp->next) {
		if(fun_tmp->is_app) continue;
		if(strncmp(fun_tmp->key, "if", 2)) continue;
		//sprintf(strtmp, "Fun: \"#%s [arg%s]\"%s", fun_tmp->key, fun_tmp->is_hlp?" | help":"", fun_tmp->help?fun_tmp->help:"");
		sprintf(strtmp, "Fun: \"#%s [arg]\"%s", fun_tmp->key, fun_tmp->help?fun_tmp->help:"");
		v2_p_o_s(strtmp, fibr);
	    }

            v2_p_o_s("Fun: \"#else\" - Alters any of \"#if..\" conditions", fibr);
            v2_p_o_s("Fun: \"#endif\" - Finish any of \"#if..\" conditions", fibr);
	    v2_p_o_s("Fun: \"#return\" (\"#stop\") - Stop process template/include and return to parent doc", fibr);
	    v2_p_o_s("Fun: \"#exit\" (\"#end\") - End process all templates", fibr);

	    v2_p_o_s("Fun: \"#+\" - Transfer string (with function) before it starts", fibr);
	    v2_p_o_s("Fun: \"#.\" - Skip spaces before functions", fibr);
        }
        v2_p_o_s("", fibr);
    }

    v2_print_var(fibr);

    v2_p_o_s("", fibr);

    v2_print_def(fibr);


    if(v2_mod_lst) {
        v2_p_o_s("==================== Modules ======================================", fibr);

        for(mod_tmp=v2_mod_lst; mod_tmp; mod_tmp=mod_tmp->next) {
            sprintf(strtmp, "==== Beg Module \"%s\"%s", mod_tmp->key, mod_tmp->help?mod_tmp->help:"");
            v2_p_o_s(strtmp, fibr);
            for(def_tmp=mod_tmp->pstr; def_tmp; def_tmp=def_tmp->next) {
                v2_p_o_s(def_tmp->str, fibr);
            }
            sprintf(strtmp, "==== End Module \"%s\"", mod_tmp->key);
            v2_p_o_s(strtmp, fibr);
            if(mod_tmp->next) v2_p_o_s("", fibr);

        }
    }

    v2_p_o_s("===================================================================", fibr);
    return(0);
}
/* ======================================================= */
int v2_show_all(char *in_str) {

    if(!in_str || !in_str[0]) {
        v2_print_all("<BR>");
    } else {
        v2_print_all(in_str);
    }
    return(0);
}
/* ======================================================= */
int v2_print_env(char *fibr) {
    int x;

    for(x=0; environ[x]; x++) {
	if(!strncmp(environ[x], "SSL_SERVER_CERT=", 16) || !strncmp(environ[x], "SSL_CLIENT_CERT=", 16)) {
	    v2_tpl_printf("<PRE>%s</PRE>", environ[x]);
	} else {
	    v2_tpl_printf("%s", environ[x]);
	}
	v2_tpl_printf("%s\n", fibr?fibr:"");
    }
    return(0);
}
/* ======================================================= */
int v2_show_env(char *in_str) {

    if(!in_str || !in_str[0]) {
        v2_print_env("<BR>");
    } else {
        v2_print_env(in_str);
    }
    return(0);
}
/* ======================================================= */
int v2_show_skn(char *in_str) {
    str_lst_t *str_tmp=NULL;

    v2_tpl_set_sknl();

    //printf("t");

    FOR_LST(str_tmp, v2_tpl.skn_lst) {
	//printf("tutu");
	v2_tpl_printf("%s (%s = %d)%s\n", v2_nn(str_tmp->key), v2_nn(str_tmp->str), str_tmp->num, v2_st(in_str, "<br>"));
    }
    return(0);
}
/* ======================================================= */
int v2_show_query(char *in_str) {
    str_lst_t *str_tmp=NULL;

    FOR_LST(str_tmp, v2_wo_lst) {
	v2_tpl_printf("%s=%s%s\n", str_tmp->key, v2_st(str_tmp->str, "-=unset=-"), v2_st(in_str, "<br>"));
    }
    return(0);
}
/* ======================================================= */
int v2_tpl_set_env(char *in_str) {
    char strtmp[MAX_STRING_LEN];
    char envnam[MAX_STRING_LEN];
    v2_key_lst_t *key_tmp=NULL;

    if(!in_str || !in_str[0]) return(0); // Nothing to return. Maybe show ERROR?

    snprintf(strtmp, MAX_STRING_LEN, "%s", in_str);
    v2_getword(envnam, strtmp, ' ');

    if(strtmp[0]) {
	key_tmp=find_key_lst(strtmp);
    } else {
	key_tmp=find_key_lst(envnam);
    }

    if(key_tmp) {
	if(*(key_tmp->pstr)) {
	    setenv(envnam, *(key_tmp->pstr), 1);
	    return(0);
	}
    } else if(strtmp[0]) {
	setenv(envnam, strtmp, 1);
        return(0);
    }
    // Unset var
    unsetenv(envnam);

    return(0);
}
/* ======================================================= */
int v2_show_date(char *datefmt) {
    struct tm *ptm=NULL;
    char tstr[MAX_STRING_LEN];
    time_t ltime=0;

    ltime=time(NULL);
    ptm=localtime(&ltime);

    if(!datefmt || !datefmt[0]) {
	strftime(tstr, MAX_STRING_LEN, "%d %B %Y %H:%M:%S", ptm);
    } else {
	strftime(tstr, MAX_STRING_LEN, datefmt, ptm);
    }

    v2_tpl_printf("%s", tstr);

    return(0);
}
/* ======================================================= */

/* ======================================================================= */
// Debug functinos
/* ======================================================================= */
int v2_debug_var(char *no_str) {
    v2_key_lst_t *key_tmp=NULL;
    char strtmp[MAX_STRING_LEN];
    int is_shown=0;
    int res=0;

    if(!v2_key_lst) return(v2_add_warn("Var: no set"));

    is_shown=0;
    for(key_tmp=v2_key_lst; key_tmp && key_tmp->next; key_tmp=key_tmp->next);

    for(; key_tmp; key_tmp=key_tmp->prev) {

    //for(key_tmp=v2_key_lst; key_tmp; key_tmp=key_tmp->next) {
	if(key_tmp->chap) {
	    if(key_tmp!=v2_key_lst) v2_add_warn("");
	    if(key_tmp->chap[0] == '.') {
		v2_add_warn("==== %s", key_tmp->chap+1);
	    } else {
		v2_add_warn("==== Variables group: %s", key_tmp->chap);
	    }
	} else if(is_shown) {
	    is_shown=0;
	    v2_add_warn("");
	}
	res=sprintf(strtmp, "Var: \"%c%c %s\" = ", v2_start1_char, v2_start2_char, key_tmp->key);

	if(*key_tmp->pstr) {
	    res+=sprintf(strtmp+res, "\"%s\"", *key_tmp->pstr);
	} else {
	    res+=sprintf(strtmp+res, "NULL");
	}
	is_shown=0;
	if(key_tmp->help) {
	    if(key_tmp->help[0]) {
		if(key_tmp->help[strlen(key_tmp->help)-1] == '.') {
		    key_tmp->help[strlen(key_tmp->help)-1] = '\0';
		    is_shown=1;
		}
	    }
	    res+=sprintf(strtmp+res, "%s", key_tmp->help);
	}
	v2_add_warn(strtmp);
    }

    return(0);
}
/* ======================================================================= */
