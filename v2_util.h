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


/* A basic set of various procedures and data types. */

#ifndef _V2_UTIL_H
#define _V2_UTIL_H 1

// Updated at 21 Sep 2020

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h> // va_list
#include <time.h>
#include <inttypes.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>

#define V2_MAX_LEN 2048

#ifndef MAX_STRING_LEN
#define MAX_STRING_LEN V2_MAX_LEN
#endif

#define FOR_LST(m,n) for((m)=(n); (m); (m)=(m)->next)
#define FOR_LST_IF(m,k,n) for((m)=(n); (m) && !(k); (m)=(m)->next)
// #define FOR_LST_RC(m,n) for((m)=(n); (m) && !rc; (m)=(m)->next)
#define FOR_LST_NEXT(m,n) for((m)=(n); (m) && (m)->next; (m)=(m)->next)

#define XFREAD_NOT_RUN    -743
#define XFREAD_NOT_CLOSE  -744
#define XFREAD_NOT_FOUND  -745
#define XFREAD_NOT_ACCESS -746
#define XFREAD_NOT_NAME   -747
#define XFREAD_NOT_WRITE  -748
#define XFREAD_NOT_RENAME -750
#define XFREAD_NOT_FUNC   -751
#define XFREAD_NOT_FILE   -753
#define XFREAD_NOT_OTHER  -754

#define XFREAD_NOT_RENAME_SIZE -780
#define XFREAD_NOT_RENAME_NOTF -781
#define XFREAD_NOT_READ_SIZE   -782
#define XFREAD_NOT_WRITE_SIZE  -783
#define XFREAD_NOT_READ_ERROR  -784

#define XFREAD_NOT_PARAM -789

#define V2_MALLOC_ERROR -801
#define V2_SIZE_ZERO -802

#define V2_INIT(module) \
    static void module##_start(void) __attribute__((constructor)) ; \
    static void module##_start(void)

#define V2_FIN(module) \
    static void module##_fin(void) __attribute__((destructor)) ; \
    static void module##_fin(void)


#define VL_LEN MAX_STRING_LEN

#define VL_STR(s,l,f) {\
    *(s)=(char)0; \
    if((f) && *(f)) { \
    va_list vl; \
    va_start(vl, (f)); \
    vsnprintf(s, (l)?(l):VL_LEN, (f), vl); \
    va_end(vl); }}

#define V2_TO_HEAD(n, l) { (n)->next=(l); (l)=(n); }

#define V2_BACK_LST(type_l,list) {\
    type_l *outl=NULL; \
    type_l *tmpl=NULL; \
    while((tmpl=list)) { \
        list=list->next; \
        tmpl->next=outl; \
        outl=tmpl; \
    }\
    list=outl; }


// Types....

// == 0 - hard off (can not be set on)
// == 1 - soft off (can be switched to on)
// == 2 - soft on (can be swithed off)
// == 3 - hard on (always turned on)

typedef enum sflg_enum {
    sflg_hard_off = 0,
    sflg_soft_off = 1,
    sflg_soft_on  = 2,
    sflg_hard_on  = 3
} sflg_t;

typedef enum {
    v2_off = 0,
    v2_on  = 1
} v2_flg_t;

typedef struct _str_lst_t {
    struct _str_lst_t *next;
    struct _str_lst_t *prev;
    struct _str_lst_t *link;
    char *key;
    char *str;
    time_t a_time;
    time_t b_time;
    double value;
    int num;
    v2_flg_t is_rmv;    // Removed record - ignore it
    int is_rdt;         // Is red - internal use
    int is_key_unalloc; // key was not allocated
    int is_str_unalloc; // str was not allocated
    int is_dat_unalloc; // data was not allocated
    union {
	void *data; // Just container
	char *dstr; // String type
    };
} str_lst_t;

extern str_lst_t *v2_err_lst;
extern int (*v2_hook_error)(str_lst_t *err_rec); // Hook for error and debug

extern str_lst_t *v2_css_lst;

extern str_lst_t *v2_wo_lst;
extern str_lst_t *v2_wo_tek;
extern str_lst_t *v2_wo_lst_pure;

extern char *v2_str_zero;

extern int v2_debug; // Debug level for add debug
extern int v2_debug_time; // Print time before debug message

#define v2_kaput(n) v2_abort(n)
void v2_abort(char *in_msg); // Stop programm execution
void v2_free(void **p_mem); // Free memory blick if exists

// ------------------------------------------------------------------------------------------------------------------
int v2_getword(char *word, char *line, char stop);
int v2_getnetxarg(char *word, char *line);
char *makeword(char *line, char stop);
char *fmakeword(FILE *f, char stop, int *cl);
char x2c(char *what);
void unescape_url(char *url);
char *escape_url(char *in_str);
char *escape_url_quota(char *in_str);
void plustospace(char *str);
int v2_getline(char *s, int n, FILE *f);
void v2_putline(FILE *f,char *l);
void send_fd(FILE *f, FILE *fd);

// ------------------------------------------------------------------------------------------------------------------
FILE *xfopen_read(char *file, ...);
FILE *xfopen_write(char *file, ...);
// int xfread_cfg(str_lst_t **pt_lst, char *file, ...); // Read config file - moved to v2_lstr.fcfg()
int xfread(str_lst_t **pt_lst, char *file, ...); // rc=-745 == XFREAD_NOT_FOUND - file not found
int xfwrite(str_lst_t *in_lst, int (*wr_str)(FILE*, str_lst_t *), char *file, ...); // wr_str - customed write funct

int xfread_new(str_lst_t **pt_lst, char *file, ...); // Uses sfread() as core

int sfread_print(char *in_str, void *in_data); // user as rd_str function for sfread

int sfreadf(int (*rd_str)(char*, void*), void *pass_data, char *file, ...); // New generation
int sfread(int (*rd_str)(char*, void*), void *pass_data, char *file);     // New generation

int sfsave(int (*wr_fun)(FILE*, void*), void *pass_data, char *file);
int sfsavef(int (*wr_fun)(FILE*, void*), void *pass_data, char *in_file, ...);

int v2_cat(char *in_file, ...); // Cat (print) file at stdout

int v2_dir(mode_t mode, char *in_dir, ...); // Check and create diretory, correct mode

// Read dir into in_list, or just call filter (0 = good entr, 1 == bad entr, >1 = error);
// in_data just passed to filter
int v2_readdir(str_lst_t **p_list, int (*filter)(char*,void*), void *in_data, char *in_dir, ...);


char *v2_xrc(int code); // Prints error code
// ------------------------------------------------------------------------------------------------------------------
// Strings
int v2_strshift(int pos, char *in_word); // Shift string at positions
char *v2_tok(int num, char *in_str, char dlm); // Token number num, separated by char dlm
char *v2_toks(int num, char *in_str, char *in_dlm); // Token number num, separated byt any char from in_dlm
char *v2_toke(char *in_tok, char *in_dlm); // Return pointer just after token
char *v2_tok_r(char *sbuf, int slen, int tok_num, char *in_str, char *in_dlm); // Fill input sbuf by token (arg)

char *v2_letstr(char **p_str, char *in_str); // let *p_str = in_str, old value of p_str will freed

void v2_freestr(char **p_str);        // Kust Freed string if exists
char *v2_winput(char *in_str);        // Replace ", < and >. Not NULL returned
char *v2_toupper(char *in_str);       // Turn string to upper case
char *v2_strtmp(char *format, ...);   // Returns static string [MAX_STRING_LEN]
char *v2_string(char *format, ...);   // Returns allocated formatted string
char *v2_strcpy(char *in_str);        // Allocate memory and copy in_str

char *v2_move(char **p_str); // Returns value from VAR, and let var = NULL

char *v2_s(char *in_buf, size_t buf_len, char *in_format, ...);   // Make string from format to buffer
char *v2_sup(char *in_buf, size_t buf_len, char *in_format, ...); // v2_s() + toupper
//char *v2_slo(char *in_buf, size_t buf_len, char *in_format, ...); // v2_s() + tolower

char *v2_st(char *in_str, char *in_rpl);        // Show string or replacement
char *v2_stf(char *in_str, char *in_rpl, ...);  // Show string or replacement
//char *v2_as(char *in_str);                // Shows in_str or "-=unset=-" value == v2_st(in_str, "-=unset=-")
//char *v2_nn(char *in_str);                // Returns in.str, or "", but not NULL

#define v2_as(n)      v2_st((n), "-=unset=-")
#define v2_nn(n)      v2_st((n), "")

//extern char *(*v2_add_var)(char**, char*, ...);

char *v2_let_var(char **pvar, char *in_val);      // Let (set) pvar, freed it before
char *v2_let_varf(char **pvar, char *format, ...); // Add varibale, freed it before

char *v2_let_if_var(char **pvar, char *in_val);   // Let (set) pvar, if pvar is empty
char *v2_let_if_varf(char **pvar, char *format, ...); // Add varibale, if pvar empty

char *v2_let_varc(char **pvar, char *in_val, char stop);     // Let (set) var, freed it before stop by strchr()
char *v2_let_varn(char **pvar, char *in_val, size_t in_len); // Let (set) var, freed it before in_len + '\0'

char *v2_let_tplf(char **p_var, char *in_value, char *in_tpl, ...); // Replace in_tpl by in_value inside *p_var

char *v2_is_par(char *in_par);        // Check if "in_par" string exists and it not zero leght, return in_par
char *v2_is_val(char *in_val);        // Check if "in_val" string exists and != "-", return in_val for OK

char *v2_untag(char *in_buf, int in_size, char *in_str); // UnTAG with external buffer
char *v2_filter_tags(char *in_str);   // Cut off HTML TAGs

int v2_strcmp(char *in_str1, char *in_str2);     // Copare two string counting possible NULL
int v2_strcmpf(char *in_str, char *in_fmt, ...); // Compare with format

int v2_atoir(char *in_str, int min, int max); // Returns bitween min and max

char *v2_prnhex(char *in_buf, size_t in_size, char *in_bin); // Prints binary buffer as a hex

// ------------------------------------------------------------------------------------------------------------------
// Config functions
char *v2_cf(char *cf_str, char* var_name); // If start cf_str == var_name, returns rest of string after ' ' and '\t'
char *v2_cf_var(char **pvar, char *cf_str, char *var_name); // Returns pointer to new var, if got, "varname" or "+varname"
int v2_cf_int(int *ivar, char *cf_str, char *var_name);     // Returns integer to var
int v2_cf_prc(int *ivar, char *cf_str, char *var_name);     // Retuurns percent rouded (1-100)
int v2_cf_long(long *lvar, char *cf_str, char *var_name);   // Returns long to lvar
int v2_cf_u64(uint64_t *uvar, char *cf_str, char *var_name);// Returns uint64_t to uvar
int v2_cf_flag(int *iflag, char *cf_str, char *var_name);   // Just set flag to 1 if config present

// ------------------------------------------------------------------------------------------------------------------
// sFlag
int v2_sflg_isvar(sflg_t in_sflg);
int v2_sflg_var(sflg_t *p_sflg);

int v2_sflg_isset(sflg_t in_sflg);
int v2_sflg_set(sflg_t *p_sflg);
int v2_sflg_reset(sflg_t *p_sflg);

// fFlag
int v2_fflag(char *f_name, ...);                   // Set file flag
int v2_is_fflag(char *f_name, ...);                // Check file flag.
int v2_old_fflag(time_t in_tou, char *f_name, ...); // 1 = flag exists and younger then in_tout

// date_util

int v2_get_mon_days(int month, int year); // Returns days number at  month
char *v2_get_date(time_t in_time);
char *v2_give_date_str(int in_dat);  // From "YYYYMMDD"
int givemonth(char *smonth);

// Trim strings
int v2_skip_spaces(char *in_str);
void v2_purge_spaces(char *in_str);
int v2_str_trim(char *in_str, char *in_trm);

int v2_vop_rename(off_t f_size, char *t_name, char *f_name);

// ------------------------------------------------------------------------------------------------------------------
// ListSTR functions
str_lst_t *v2_add_lstr_head(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time);
str_lst_t *v2_add_lstr_tail(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time);
str_lst_t *v2_add_lstr_sort(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time);

int v2_lstr_add_data(str_lst_t *i_str, void *in_data);

int v2_lstr_sort(str_lst_t **p_str, int (*fun_sort)(str_lst_t *, str_lst_t *)); // Sort list with user's function

str_lst_t *v2_lstr_separate(str_lst_t *i_str);                                  // Separate key to key and str
int v2_lstr_separate_by(str_lst_t *i_str, char by_ch);                          // Separate by key
int v2_lstr_separate_lst_by(str_lst_t *i_str, char by_ch);                      // Separate all list

#define v2_lstr_spr_by(n, m) v2_lstr_separate_by((n), (m))
#define v2_lstr_spr(n) v2_lstr_separate(n)

int v2_lstr_rback(str_lst_t **p_str);
int v2_lstr_free(str_lst_t **p_str);
str_lst_t *v2_lstr_rmv(str_lst_t *in_lstr); // Set is_rmv

str_lst_t *v2_lstr_add_tail(str_lst_t **in_parent, str_lst_t *in_tail); // Add in_tail at the end of in_parent list

str_lst_t *v2_lstr_find(str_lst_t *p_str, char *in_key, char *in_str); // Finds key/str pair
str_lst_t *v2_get_lstr_key(str_lst_t *p_str, char *in_key);            // Get str rec by key
char *v2_get_lstr_str(str_lst_t *p_str, char *in_key);                 // Get string content
void *v2_get_lstr_data(str_lst_t *p_str, char *in_key);                // Returns ->data if extsts
char *v2_lstr_str(str_lst_t *in_str);                                  // Returns ->str if exists, or ->key

str_lst_t *v2_str_to_lstr(char *in_str, char spr);

// ------------------------------------------------------------------------------------------------------------------
// ERROR functions
int v2_add_err(int err, int level, char *err_msg); // Main base functino


int v2_prn_error(void);                           // Print error, warning and debug
int v2_add_error(char *err_form, ...);            // Add error string to the list
int v2_ret_error(int err, char *err_form, ...);   // Add error with code "err", and return it (code).
int v2_add_debug(int level, char *err_form, ...); // Add warning (level == 1) and debug (>1) string to the list
int v2_add_warn(char *warn_form, ...);            // Easy form of v2_add_debug(1, ...)
int v2_is_error(int in_err);                      // Check if error exists and get latest error code, if in_err == 0
int v2_del_error(void);                           // Delete all errors into list

// ------------------------------------------------------------------------------------------------------------------
// WebOpts functions
char *v2_wo_key(char *key);                    // Get str of this key
char *v2_wo_keyf(char *key_format, ...);       // Get str of this key in format
char *v2_wo_var(char **pvar, char *in_key);    // Let (set) var from wo_lst, freed it before
int v2_wo_sflg(sflg_t *in_sflg, char *in_key); // Set sflg variable, if key exists
int v2_wo_post(int is_uniq);                   // Read POST options
int v2_wo_get(int is_uniq);                    // Read GET options
int v2_wo_read(char *in_str, int is_uniq);     // Read options from string
int v2_wo_print(char *end_str);                // Prints wo list, each string finalized be end_str
int v2_wo_add_var(char **p_var, char *wo_key); // Add value from wo list

// ------------------------------------------------------------------------------------------------------------------
// CSS
int v2_css_read(str_lst_t **in_list, char *in_file); // Read css list from the file
int v2_css_print(str_lst_t *in_css); // Print CSS table output
// ------------------------------------------------------------------------------------------------------------------
// V2_ARG
//v2_arg_t *v2_arg(v2_arg_t *in_arg, char *in_str);

char **v2_argv(char *in_str);
int v2_argc(char **in_argv);

// ------------------------------------------------------------------------------------------------------------------

#endif // _V2_UITL_H
