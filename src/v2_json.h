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

#ifndef _V2_JSON_H
#define _V2_JSON_H 1

#include "v2_wrbuf.h"
#include "v2_util.h"

typedef enum {
    JS_NONE,
    JS_STRING,
    JS_INT,
    JS_LONG,
    JS_DOUBLE,
    JS_ARRAY,
    JS_BOOLEAN,
    JS_OBJECT,
    JS_NULL
} json_field;

typedef struct _json_lst_t {
    struct _json_lst_t *next;
    struct _json_lst_t *prev;

    struct _json_lst_t *child;
    struct _json_lst_t *parent;

    struct _json_lst_t *array;

    struct _json_lst_t *list; // Long list lnk thru all list => json_box.chan

    json_field js_type;

    char *id;
    char *full_id; // Full ID like "group.admin.name.first"
    char *str;

    int num;
    long long lnum;
    double dnum;

    //int bl; // Boolean 0=none, 1=true, 2=false - depricated - remove it and use "num" instead

    int open; // Marks open array or object
} json_lst_t;


typedef struct json_box_s {
    json_lst_t *lst; // Root of json structure
    json_lst_t *tek; // Current pointer
    json_lst_t *prn; // Print point for printing

    json_lst_t *chan; // Chain of all named elements
    json_lst_t *ctek; // Current of all named elements

    wrbuf_t *b;    // OutPut Buffer for printing or parsing json

    int ident;     // Ident - number of spaces - good choice - 4 - set 0 to one strings output
    int header;    // 1 = add Content-Type header
    int no_escape; // 1 = do not escape UTF8 symbols (visual output), 2 = escape only quotas (raw UTF mode)
    int no_fullid; // 1 = Do not make full ID and chain list
    int no_clean;  // 1 = Do not clean output buff - just add text

    int arr_no; // Array element number

    str_lst_t *hdr;  // Extra headers line if ->header != 0

    // New interface for external box
    char *locale;   // Set local locale
    int (*boxstr)(struct json_box_s*, char *); // Operate by string - ex. locale to UTF (Send) or UTF to locale (Receive)
    int is_delocale; // Have to delocale it UTF -> locale

    // Easy way for default box
    int (*str)(char *); // Operate by string - ex. utf to locale

    // Internal use
    int spaces;
} json_box_t; // Box with all json things


extern json_box_t json_box; // Default box

// External function
//extern int (*v2_json_fun)(json_lst_t *in_json);

int v2_json_none_lst(json_lst_t *in_json); // Marks all nodes as NONE

json_field v2_json_type(json_lst_t *in_json);
char *v2_json_type_str(json_lst_t *in_json); // Returns type as string

// -----------------------------------------------------------------------------------------
// Box functions
int v2_json_new(json_box_t **p_box);
int v2_json_free_box(json_box_t *in_jbox);
int v2_json_add_node(json_box_t *in_jbox, char *in_id, json_field js_type);
int v2_json_add_end(json_box_t *in_jbox, json_field js_type);
int v2_json_text(json_box_t *in_jbox);

// -----------------------------------------------------------------------------------------
int v2_json_locale(json_box_t *in_jbox, char *in_locale, int is_de); // Set locale (or de_locale) and assign iconv function
// is_de == 0 - send from in_locale to UTF
// is_de == 1 - receive from UTF to in_locale

// -----------------------------------------------------------------------------------------
// Defaultox functions
int v2_json_free(void); // Frees all structure

int v2_json_add_arr(char *in_id);
int v2_json_end_arr(void);

int v2_json_add_jobj(char *in_id, json_lst_t *in_json); // Add ready object

int v2_json_add_obj(char *in_id);
int v2_json_end_obj(void);

int v2_json_add_str(char *in_id,    char *in_val);
int v2_json_add_bool(char *in_id,   int is_true);
int v2_json_add_int(char *in_id,    int in_num);
int v2_json_add_lint(char *in_id  , long long in_lnum);
int v2_json_add_double(char *in_id, double in_dnum);
int v2_json_add_null(char *in_id);

// -----------------------------------------------------------------------------------------
// Thread Safe functions

int v2_json_end(json_box_t *in_jbox);

int v2_json_jobj(json_box_t *in_jbox, char *in_id, json_lst_t *in_json);

int v2_json_arr(json_box_t *in_jbox, char *in_id);
int v2_json_obj(json_box_t *in_jbox, char *in_id);

int v2_json_str(json_box_t *in_jbox, char *in_id, char *in_val);
int v2_json_bool(json_box_t *in_jbox, char *in_id, int is_true);
int v2_json_int(json_box_t *in_jbox, char *in_id, int in_num);
int v2_json_lint(json_box_t *in_jbox, char *in_id, long long in_lnum);
int v2_json_double(json_box_t *in_jbox, char *in_id, double in_dnum);
int v2_json_null(json_box_t *in_jbox, char *in_id);

// -----------------------------------------------------------------------------------------

int v2_json_prnone(json_box_t *in_jbox, json_lst_t *in_json); // Print one element

int v2_json_print(void);            // Print json list

char *v2_json_out(json_lst_t *in_json); // Returns pointer to allocated buffer with text of json

int v2_json_save(char *fname, ...); // Write json_box to a file
int v2_json_store(json_lst_t *in_json, int is_esc,  char *fname, ...); // Write json list to file

int v2_json_debug(int level, json_lst_t *in_json, char *in_msg, ...);      // Add json values at debug list
int v2_json_debug_full(int level, json_lst_t *in_json, char *in_msg, ...); // Add full json segment to debug

#endif // _V2_JSON_H
