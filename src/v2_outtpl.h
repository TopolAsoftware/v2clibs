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

#ifndef _V_OUTTPL_H
#define _V_OUTTPL_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h> // va_list
#include <inttypes.h>

// #include "v2_lstr.h"
#include "v2_wrbuf.h"
#include "v2_err.h"
#include "v2_wo.h"

// End of all templates code
#define V2_TPL_END 108

#define V2_TPL_VARLEN 64

/* ================= external vars ================== */
typedef struct {
    // Set by v2_tpl_locale();
    char *locale;
    char *charset;
    char *lang;

    char *lext; // Extention of linked dir. For example ru/ -> ../ru.koi8-r/ = "koi8-r"

    str_lst_t *skn_lst; // List of skin directories to find files
} v2_tpl_t;


extern v2_tpl_t v2_tpl;

extern FILE *v2_out_file;
//extern char v2_start1_char;
//extern char v2_start2_char;
//extern char v2_finish_char;
extern int v2_emtstr_noshow; // Don't show empty strings

//extern char *v2_lang_name;
extern char *v2_skin_name;

extern char *web_root_dir;
extern char *src_tpl_dir;

extern int all_lst_defs;

extern int v2_tpl_webout; // if == 1 print web error messages

extern char *v2_tpl_fun; // Name of current virtual function (not great hack)

extern char **environ;

extern int (*v2_tpl_out_err)(int); // Function called if exec error found

extern int (*v2_out_str)(char*); // External func to transfer strings, default v2_trans_str()

// ----------- ext interface ------------------
extern int (*v2_ot_str)(char*);
extern int (*v2_ot_fun)(int (*)(char *), char*, ...);

// --------------------------------------------
int v2_ot_stra(char *in_str);      // Allocated call
int v2_ot_strf(char *in_fmt, ...); // Call function above

/* ================= new interface ================== */
int v2_def_function(int (*str_fun)(char *), char *key_format, ...);

/* ================= prototypes ===================== */
void v2_check_lang(void);

int v2_tpl_setbuff(void); // Set internal buffer
int v2_tpl_resbuff(void); // Reset (free) buffer
char *v2_tpl_getbuff(void);   // Return buffer pointer and reset buffer itself
int v2_tpl_prnbuff(void); // Print and clean local buffer
int v2_tpl_printf(char *format, ...); // Print into file, stdout or local buffer
int v2_tpl_prnerr(char *format, ...); // Print error string if allowed
int v2_tpl_savebuff(char *in_file, ...); // Save buffer into file

int v2_tpl_trn(char *in_str, char **p_var); // Only trans variables, not functions (new generation)
int v2_trans_str(char *); // Main function ---------------------------------------------

int v2_check_module(char *modname); // 0 = mod OK, 1 = no module
int v2_out_module(char *modname);

int v2_out_merge(char*);
int v2_out_mergef(char*, ...);
int v2_out_include(char*);
int v2_out_includef(char*, ...);
int v2_out_template(char*);
int v2_out_templatef(char*, ...);

int v2_out_strlst(str_lst_t *); // Output template from str_lst list

int v2_add_def_lst(char *def_format, ...);
int v2_del_def_lst(char *def_format, ...);
int v2_if_def(char *indef); // If defined

char **v2_set_var(char *str_key, char *str_val);       // New formated version
char **v2_set_varf(char *str_key, char *str_val, ...); // Former v2_set_var()

void v2_add_var_lst(char *, char **);
char *v2_get_var(char *strkey);
char **v2_get_var_adr(char *strkey);

int v2_fun_null(char *in_str); // NULL Function

int v2_tpl_init(char *in_key, int (*in_fun)(char *)); // Add init fun to add build-in variables

void v2_add_fun_lst(char *str_key, int (*str_fun)(char *));

char *v2_tpl_locale(char *in_locale); // Set or reset locale and charset (not checked in system)
void v2_set_web_dir(char *);
void v2_set_tpl_dir(char *);

int v2_show_date(char *datefmt);

int v2_print_var(char *fibr);
int v2_print_def(char *fibr);

int v2_print_all(char *fibr);
int v2_show_all(char *in_str);

int v2_show_query(char *in_str);

int v2_print_env(char *fibr);
int v2_show_env(char *in_str);
int v2_show_skn(char *in_str);

int v2_tpl_set_env(char *in_str);

int v2_p_o_s(char *in_str, char *fibr); // Prints string as hlp

/* ================================================== */
#endif //  _V_OUTTPL_H
