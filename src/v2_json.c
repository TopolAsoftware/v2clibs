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

#include "v2_json.h"
#include "v2_iconv.h"
#include "utf8.h" // u8escape

// ERROR_CODE 173XX : 17350 - 17399

json_box_t json_box; // Default box context

// External function
//int (*v2_json_fun)(json_lst_t *in_json)=NULL;

/* =================================================================== */
int v2_json_free_el(json_lst_t **p_json) {
    json_lst_t *jsn_tmp=NULL;

    if(!p_json) return(0);

    while((jsn_tmp=(*p_json))) {
        (*p_json)=(*p_json)->next;

	v2_json_free_el(&jsn_tmp->child);
        v2_freestr(&jsn_tmp->id);
	v2_freestr(&jsn_tmp->str);

        // Free jsn_tmp?
	free(jsn_tmp);
    }

    return(0);
}
/* =================================================================== */
int v2_json_free_box(json_box_t *in_jbox) {

    v2_json_free_el(&in_jbox->lst);
    v2_json_locale(in_jbox, NULL, 0);

    v2_wrbuf_free(&in_jbox->b);

    in_jbox->tek    = NULL;
    in_jbox->prn    = NULL; // Print point for printing
    in_jbox->chan   = NULL; // Chain of all named elements
    in_jbox->ctek   = NULL; // Current of all named elements
    in_jbox->str    = NULL;

    in_jbox->arr_no      = 0;
    in_jbox->ident       = 0;
    in_jbox->header      = 0;
    in_jbox->no_escape   = 0;
    in_jbox->no_fullid   = 0;
    in_jbox->no_clean    = 0;

    return(0);
}
/* =================================================================== */
int v2_json_free(void) {

    return(v2_json_free_box(&json_box));
}
/* =================================================================== */
int v2_json_new(json_box_t **p_box) {

    if(!p_box) return(17370);
    if(!*p_box) {
        if(!(*p_box=(json_box_t *)calloc(sizeof(json_box_t), 1))) return(17371);
    }

    v2_json_free_box(*p_box);
    return(0);
}
/* =================================================================== */
int v2_json_iconv(json_box_t *in_jbox, char *in_str) {
    char *out=NULL;

    if(!v2_is_par(in_str)) return(0);

    if(!in_jbox)         return(0);
    if(!in_jbox->locale) return(0);

    if(in_jbox->is_delocale) {
	if(!(out=v2_iconv("UTF-8", in_jbox->locale, in_str))) return(0);
    } else {
	if(!(out=v2_iconv(in_jbox->locale, "UTF-8", in_str))) return(0);
    }

    sprintf(in_str, "%s", out);

    v2_freestr(&out);
    return(0);
}
/* =================================================================== */
// Set locale and assign iconv function
int v2_json_locale(json_box_t *in_jbox, char *in_locale, int is_de) {

    if(!in_jbox) return(17374);
    v2_let_var(&in_jbox->locale, in_locale); // If in_locale == null - reset  funct it too

    if(in_jbox->locale) {
	if(!in_jbox->boxstr) in_jbox->boxstr=&v2_json_iconv;
	in_jbox->is_delocale=is_de;
    } else {
	in_jbox->boxstr=NULL; // Reset conversion
        in_jbox->is_delocale=0;
    }
    return(0);
}

/* =================================================================== */
// Marks list as none type objects
int v2_json_none_lst(json_lst_t *in_json) {
    json_lst_t *jsn_tmp=NULL;

    if(!in_json) {
	in_json=json_box.lst; // All fields
	json_box.tek=NULL;
    }

    FOR_LST(jsn_tmp, in_json) {
	jsn_tmp->js_type=JS_NONE;
	if((in_json==json_box.lst) && !jsn_tmp->next) json_box.tek=jsn_tmp;
    }

    return(0);
}
/* =================================================================== */
json_field v2_json_type(json_lst_t *in_json) {
    if(!in_json) return(JS_NONE);
    return(in_json->js_type);
}
/* =================================================================== */
char *v2_json_type_str(json_lst_t *in_json) {
    if(v2_json_type(in_json) == JS_STRING)  return(" String");
    if(v2_json_type(in_json) == JS_INT)     return("    Int");
    if(v2_json_type(in_json) == JS_LONG)    return("   Long");
    if(v2_json_type(in_json) == JS_DOUBLE)  return(" Double");
    if(v2_json_type(in_json) == JS_ARRAY)   return("  Array");
    if(v2_json_type(in_json) == JS_BOOLEAN) return("Boolean");
    if(v2_json_type(in_json) == JS_OBJECT)  return(" Object");
    if(v2_json_type(in_json) == JS_NULL)    return("   Null");
    return("None");
}
/* =================================================================== */
// This json element has parent type (obj or arr)
int v2_json_is_parent(json_lst_t *in_json) {
    if(!in_json) return(0);
    if(in_json->js_type == JS_OBJECT) return(1);
    if(in_json->js_type == JS_ARRAY)  return(1);
    return(0);
}
/* =================================================================== */
int v2_json_add_json(json_box_t *in_jbox, json_lst_t *json_inp) {

    if(!in_jbox)  return(17351);
    if(!json_inp) return(17352);

    if(v2_json_is_parent(json_inp)) json_inp->open=1;

    if(!in_jbox->lst) {
	in_jbox->lst=json_inp;
    } else {
	if(!in_jbox->tek) in_jbox->tek=in_jbox->lst; // Never should be happens

	// Add Obj + Array
	if(in_jbox->tek->open==1) { // Means already have parent element, but has not childrens yet
	    in_jbox->tek->open          = 0;
	    in_jbox->tek->child         = json_inp;
	    in_jbox->tek->child->parent = in_jbox->tek;
	} else {
	    in_jbox->tek->next          = json_inp;
	    in_jbox->tek->next->prev    = in_jbox->tek;
	    in_jbox->tek->next->parent  = in_jbox->tek->parent;
	}
    }
    in_jbox->tek = json_inp;

    if(in_jbox->no_fullid) return(0);

    if(json_inp->parent) {
	v2_let_varf(&json_inp->full_id, "%s.%s", json_inp->parent->full_id, json_inp->id);
    } else {
	v2_let_var(&json_inp->full_id ,json_inp->id);
    }

    if(json_inp->open!=1) { // Not array or obj
	if(!in_jbox->chan) {
	    in_jbox->chan=json_inp; // Set first pointer and temporal one
	} else {
	    in_jbox->ctek->list=json_inp; // Set next one
	}
	in_jbox->ctek=json_inp;
    }

    return(0);
}
/* =================================================================== */
int v2_json_add_node(json_box_t *in_jbox, char *in_id, json_field js_type) {
    json_lst_t *json_tmp=NULL;

    if(!in_jbox) return(17361);

    if(!(json_tmp=(json_lst_t *)calloc(sizeof(json_lst_t), 1))) return(17350);

    if(in_id && *in_id) {
	v2_let_var(&json_tmp->id, in_id);
    } else {
	v2_let_varf(&json_tmp->id, "_obj_%04d", ++in_jbox->arr_no); // Check if parent id array
    }

    json_tmp->js_type=js_type;

    return(v2_json_add_json(in_jbox, json_tmp));
}
/* =================================================================== */
// And child object or array
int v2_json_add_end(json_box_t *in_jbox, json_field js_type) {

    if(!in_jbox)      return(17354);
    if(!in_jbox->tek) return(17355);

    if(in_jbox->tek->open==1) { // Just close started parent
	in_jbox->tek->open=0;
	if(in_jbox->tek->js_type == JS_OBJECT) in_jbox->tek->js_type=JS_NULL; // Object can not be empty
        return(0);
    }

    if(!in_jbox->tek->parent) return(0); // Top level - root

    in_jbox->tek=in_jbox->tek->parent;

    if(!in_jbox->tek->open)              return(17356); // Try to close not opened object

    if(js_type != JS_NONE) { // Allow to close any parent objects (obj or arr)
	if(in_jbox->tek->js_type != js_type) return(17357); // And check type
    }

    return(0);
}
/* =================================================================== */
/* TrdSafe functions */
/* =================================================================== */
int v2_json_end(json_box_t *in_jbox) {

    return(v2_json_add_end(in_jbox, JS_NONE)); // Autodetect
}
/* =================================================================== */
int v2_json_arr(json_box_t *in_jbox, char *in_id) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_ARRAY))) return(rc);

    return(0);
}
/* =================================================================== */
int v2_json_obj(json_box_t *in_jbox, char *in_id) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_OBJECT))) return(rc);

    return(0);
}
/* =================================================================== */
 // Add ready object
int v2_json_jobj(json_box_t *in_jbox, char *in_id, json_lst_t *in_json) {
    int rc=0;

    if(!in_json) return(v2_json_null(in_jbox, in_id)); // Add null if object empty

    if((rc=v2_json_add_node(in_jbox, in_id, JS_OBJECT))) return(rc);
    if((rc=v2_json_add_json(in_jbox, in_json)))          return(rc);

    return(0);
}
/* =================================================================== */
int v2_json_str(json_box_t *in_jbox, char *in_id, char *in_val) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_STRING))) return(rc);
    v2_let_var(&in_jbox->tek->str, in_val);

    return(0);
}
/* =================================================================== */
int v2_json_bool(json_box_t *in_jbox, char *in_id, int is_true) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_BOOLEAN))) return(rc);
    in_jbox->tek->num=is_true;
    v2_let_var(&in_jbox->tek->str, is_true?"true":"false");

    return(0);
}
/* =================================================================== */
int v2_json_int(json_box_t *in_jbox, char *in_id, int in_num) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_INT))) return(rc);
    in_jbox->tek->num=in_num;
    in_jbox->tek->str=v2_string("%d", in_num);

    return(0);
}
/* =================================================================== */
int v2_json_lint(json_box_t *in_jbox, char *in_id, long long in_lnum) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_LONG))) return(rc);
    in_jbox->tek->lnum=in_lnum;
    in_jbox->tek->str=v2_string("%lld", in_lnum);

    return(0);
}
/* =================================================================== */
int v2_json_double(json_box_t *in_jbox, char *in_id, double in_dnum) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_DOUBLE))) return(rc);
    in_jbox->tek->dnum=in_dnum;
    in_jbox->tek->str=v2_string("%lld", in_dnum);

    return(0);
}
/* =================================================================== */
int v2_json_null(json_box_t *in_jbox, char *in_id) {
    int rc=0;

    if((rc=v2_json_add_node(in_jbox, in_id, JS_NULL))) return(rc);
    v2_let_var(&in_jbox->tek->str, "null");

    return(0);
}
/* =================================================================== */
/* Static functions */
/* =================================================================== */
int v2_json_end_arr(void) {

    return(v2_json_add_end(&json_box, JS_ARRAY));
}
/* =================================================================== */
int v2_json_end_obj(void) {

    return(v2_json_add_end(&json_box, JS_OBJECT));
}
/* =================================================================== */
int v2_json_add_arr(char *in_id) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_ARRAY))) return(rc);

    return(0);
}
/* =================================================================== */
int v2_json_add_obj(char *in_id) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_OBJECT))) return(rc);

    return(0);
}
/* =================================================================== */
 // Add ready object
int v2_json_add_jobj(char *in_id, json_lst_t *in_json) {
    int rc=0;

    if(!in_json) {
	if((rc=v2_json_add_node(&json_box, in_id, JS_NULL))) return(rc);
	v2_let_var(&json_box.tek->str, "null");
	return(0);
    }

    if((rc=v2_json_add_node(&json_box, in_id, JS_OBJECT))) return(rc);
    if((rc=v2_json_add_json(&json_box, in_json)))          return(rc);

    return(0);
}
/* =================================================================== */
int v2_json_add_str(char *in_id, char *in_val) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_STRING))) return(rc);
    v2_let_var(&json_box.tek->str, in_val);

    return(0);
}
/* =================================================================== */
int v2_json_add_bool(char *in_id, int is_true) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_BOOLEAN))) return(rc);
    json_box.tek->num=is_true;
    v2_let_var(&json_box.tek->str, is_true?"true":"false");

    return(0);
}
/* =================================================================== */
int v2_json_add_int(char *in_id, int in_num) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_INT))) return(rc);
    json_box.tek->num=in_num;
    json_box.tek->str=v2_string("%d", in_num);

    return(0);
}
/* =================================================================== */
int v2_json_add_lint(char *in_id, long long in_lnum) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_LONG))) return(rc);
    json_box.tek->lnum=in_lnum;
    json_box.tek->str=v2_string("%lld", in_lnum);

    return(0);
}
/* =================================================================== */
int v2_json_add_double(char *in_id, double in_dnum) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_DOUBLE))) return(rc);
    json_box.tek->dnum=in_dnum;
    json_box.tek->str=v2_string("%lld", in_dnum);

    return(0);
}
/* =================================================================== */
int v2_json_add_null(char *in_id) {
    int rc=0;

    if((rc=v2_json_add_node(&json_box, in_id, JS_NULL))) return(rc);
    v2_let_var(&json_box.tek->str, "null");

    return(0);
}
/* =================================================================== */
/* =================================================================== */
int v2_json_escape_quotas(char *out_str, size_t out_size, char *in_str) {
    int x, y;

    for (x=0, y=0; in_str[x] && (x < out_size); ++x) {
	if(in_str[x] == '"') out_str[y++] = '\\';
	out_str[y++] = in_str[x];
    }

    out_str[y] = '\0';
    return(0);
}
/* =================================================================== */
// Print functions
/* =================================================================== */
int v2_json_prn_field(json_box_t *in_jbox, json_lst_t *in_json) {
    //json_lst_t *in_json=NULL;
    char strout[MAX_STRING_LEN*6];
    char strtmp[MAX_STRING_LEN*6];
    char *coma="";
    char *ends="";
    //int x, y;

    if(!in_jbox) return(0);
    if(!in_json) return(0);

    if(in_json->next) coma=",";
    if(in_jbox->ident) ends="\n";

    if(in_json->js_type==JS_STRING) {
	if(!in_json->str) {
	    v2_wrbuf_printf(in_jbox->b, "\"\"%s%s", coma, ends);
	    return(0);
	}

	sprintf(strout, "%s", in_json->str);
	if(in_jbox->boxstr) in_jbox->boxstr(in_jbox, strout); // new interface - need for local locale
	if(in_jbox->str)    in_jbox->str(strout);             // for ex. koi -> utf8 old one

	if(!in_jbox->no_escape) {
	    u8_escape(strtmp, MAX_STRING_LEN*6-1, strout, 1);
            sprintf(strout, "%s", strtmp);
	} else if (in_jbox->no_escape == 2 ) { // Escape only quotas
	    v2_json_escape_quotas(strtmp, MAX_STRING_LEN*6-1, strout);
            sprintf(strout, "%s", strtmp);
	}
	v2_wrbuf_printf(in_jbox->b, "\"%s\"%s%s", strout, coma, ends);
        return(0);
    }

    if(in_json->js_type==JS_INT) {
	v2_wrbuf_printf(in_jbox->b, "%d%s%s", in_json->num, coma, ends);
        return(0);
    }

    if(in_json->js_type==JS_LONG) {
	v2_wrbuf_printf(in_jbox->b, "%lld%s%s", in_json->lnum, coma, ends);
        return(0);
    }

    if(in_json->js_type==JS_DOUBLE) {
	v2_wrbuf_printf(in_jbox->b, "%g%s%s", in_json->dnum, coma, ends);
        return(0);
    }

    if(in_json->js_type==JS_BOOLEAN) {
	v2_wrbuf_printf(in_jbox->b, "%s%s%s", in_json->num?"true":"false", coma, ends);
        return(0);
    }


    // Object
    if(in_json->js_type==JS_OBJECT) {
	if(!in_json->child) {
	    v2_wrbuf_printf(in_jbox->b, "null%s%s", coma, ends);
	    return(0);
	}
	v2_wrbuf_printf(in_jbox->b, "{%s", ends);
	v2_json_prnone(in_jbox, in_json->child);
        if(in_jbox->spaces) v2_wrbuf_printf(in_jbox->b, "%*c", in_jbox->spaces, ' ');
	v2_wrbuf_printf(in_jbox->b, "}%s%s", coma, ends);
	return(0);
    }

    if(in_json->js_type==JS_ARRAY) {
	if(!in_json->child) {
	    //printf("null%s%s", in_coma, ends);
	    v2_wrbuf_printf(in_jbox->b, "[]%s%s", coma, ends);
	    return(0);
	}
        v2_wrbuf_printf(in_jbox->b, "[%s", ends);
	v2_json_prnone(in_jbox, in_json->child);
	if(in_jbox->spaces) v2_wrbuf_printf(in_jbox->b, "%*c", in_jbox->spaces, ' ');
	v2_wrbuf_printf(in_jbox->b, "]%s%s", coma, ends);
	return(0);
    }

    // Unsupported obj
    v2_wrbuf_printf(in_jbox->b, "null%s%s", coma, ends);
    return(0);
}
/* =================================================================== */
//int v2_json_prn_one(json_lst_t *in_json) {
int v2_json_prnone(json_box_t *in_jbox, json_lst_t *in_json) {
    char strout[MAX_STRING_LEN*6];
    char strtmp[MAX_STRING_LEN*6];
    json_lst_t *jsn_tmp=NULL;

    if(!in_jbox) return(0);

    in_jbox->spaces+=in_jbox->ident;

    FOR_LST(jsn_tmp, in_json) {
	if(jsn_tmp->js_type==JS_NONE) continue;

	if(in_jbox->spaces) v2_wrbuf_printf(in_jbox->b, "%*c", in_jbox->spaces, ' ');

	if(!(jsn_tmp->parent && (jsn_tmp->parent->js_type == JS_ARRAY))) {

	    //sprintf(strout, "%s", jsn_tmp->id);
	    strncpy(strout, jsn_tmp->id, MAX_STRING_LEN*2);
	    if(in_jbox->boxstr) in_jbox->boxstr(in_jbox, strout);
	    if(in_jbox->str)    in_jbox->str(strout);

	    if(!in_jbox->no_escape) {
		u8_escape(strtmp, MAX_STRING_LEN*6, strout, 1);
		sprintf(strout, "%s", strtmp);
	    } else if (in_jbox->no_escape == 2 ) { // Escape only quotas
		v2_json_escape_quotas(strtmp, MAX_STRING_LEN*6-1, strout);
		sprintf(strout, "%s", strtmp);
	    }
	    v2_wrbuf_printf(in_jbox->b, "\"%s\": ", strout);
	}
	v2_json_prn_field(in_jbox, jsn_tmp);
    }

    in_jbox->spaces-=in_jbox->ident;

    return(0);
}
/* =================================================================== */
// Move structure to output buffer
int v2_json_text(json_box_t *in_jbox) {
    str_lst_t *str_tmp=NULL;
    int is_alloc=0;
    int rc=0;

    if(!in_jbox) return(17391); // Nothin to do....

    if(!in_jbox->b) is_alloc=1;

    if(is_alloc || !in_jbox->no_clean) {
	if((rc=v2_wrbuf_new(&in_jbox->b))) return(rc);
    }

    if(in_jbox->header) {
	v2_wrbuf_printf(in_jbox->b, "Content-Type: application/json; charset=\"utf-8\"\n");
	FOR_LST(str_tmp, in_jbox->hdr) v2_wrbuf_printf(in_jbox->b, "%s%s%s\n", str_tmp->key, str_tmp->str?": ":"", v2_nn(str_tmp->str));
	v2_wrbuf_printf(in_jbox->b, "\n");
    }

    in_jbox->tek = NULL;
    in_jbox->spaces = 0;

    if(!in_jbox->prn) in_jbox->prn=in_jbox->lst;

    if(!in_jbox->prn) {
	v2_wrbuf_printf(in_jbox->b, "[]\n"); // Empty list.
    } else {

	if((v2_json_type(in_jbox->prn) == JS_ARRAY) && !v2_strcmp(in_jbox->prn->id, "_")) { // Special case core arr "_" : [el, el, ]
	    v2_wrbuf_printf(in_jbox->b, "[%s", in_jbox->ident?"\n":"");
	    rc=v2_json_prnone(in_jbox, in_jbox->prn->child);
	    v2_wrbuf_printf(in_jbox->b, "]\n");
	} else {
	    v2_wrbuf_printf(in_jbox->b, "{%s", in_jbox->ident?"\n":"");
	    rc=v2_json_prnone(in_jbox, in_jbox->prn);
	    v2_wrbuf_printf(in_jbox->b, "}\n");
	}

        if(rc) return(rc);
    }
    // Print if asked
    if(is_alloc) {
	if(in_jbox->b->buf) printf("%s", in_jbox->b->buf); // Local printing
	v2_wrbuf_free(&in_jbox->b);
    }

    in_jbox->prn=NULL; // Reset print pointer

    return(rc);
}
/* =================================================================== */
int v2_json_print(void) {
    int rc=0;

    if((rc=v2_json_text(&json_box))) return(0);
    if(json_box.b && json_box.b->cnt) printf("%s", json_box.b->buf);

    return(0);
}
/* =================================================================== */
char *v2_json_out(json_lst_t *in_json) {
    json_box_t *jbox=NULL;
    char *out=NULL;

    if(v2_json_new(&jbox)) return(NULL);

    jbox->ident=2;
    jbox->no_escape=1;

    jbox->lst = in_json;

    if(v2_wrbuf_new(&jbox->b)) return(NULL);
    if(v2_json_text(jbox))     return(NULL);

    out=v2_move(&jbox->b->buf);
    v2_wrbuf_free(&jbox->b);
    free(jbox); // Nothing to clear - json list external (!!!)

    return(out);
}
/* =================================================================== */
// Write file
int v2_json_save(char *fname, ...) {
    char strfil[MAX_STRING_LEN];
    int rc=0;

    VL_STR(strfil, MAX_STRING_LEN, fname);

    if((rc=v2_wrbuf_new(&json_box.b)))               return(rc); // Create or clean buffer
    if((rc=v2_json_text(&json_box)))                 return(rc);
    if((rc=v2_wrbuf_save(json_box.b, "%s", strfil))) return(rc);
    //v2_wrbuf_reset(v2_jsmn.box->b); // Does it need here? - Looks like not.
    return(0);
}
/* =================================================================== */
int v2_json_store(json_lst_t *in_json, int is_esc,  char *fname, ...) {
    json_box_t *jbox=NULL;
    char file_name[MAX_STRING_LEN];
    int rc=0;

    VL_STR(file_name, MAX_STRING_LEN, fname);

    if(!*file_name) return(17375);

    if(v2_json_new(&jbox)) return(17376);

    jbox->ident=2;
    jbox->no_escape=is_esc;

    jbox->lst = in_json;

    if(v2_wrbuf_new(&jbox->b)) return(17377);
    if(v2_json_text(jbox)) return(17378);
    if((rc=v2_wrbuf_save(jbox->b, "%s", file_name))) return(v2_ret_error(17379, "Can not save file[rc=%d]: %s", rc, file_name));
    v2_wrbuf_free(&jbox->b);

    return(0);
}
/* =================================================================== */
// Debug message string by stirng
int v2_json_debug(int level, json_lst_t *in_json, char *in_msg, ...) {
    json_lst_t *json_tmp=NULL;
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, in_msg);

    if(v2_json_type(in_json) == JS_NONE) {
	v2_add_debug(level, "%s: -=empty_json=-", strtmp);
	return(0);
    }

    v2_add_debug(level, "%s: %s \"%s\" = \"%s\"", strtmp, v2_json_type_str(in_json), in_json->full_id, v2_nn(in_json->str));

    if((v2_json_type(in_json)==JS_OBJECT) || (v2_json_type(in_json)==JS_ARRAY)) {
	FOR_LST(json_tmp, in_json->child) v2_json_debug(level, json_tmp, in_msg);
    }

    return(0);
}
/* =================================================================== */
// Debuf full format message to debug log
int v2_json_debug_full(int level, json_lst_t *in_json, char *in_msg, ...) {
    char strtmp[MAX_STRING_LEN];
    char *out=NULL;

    if(level>1) { // Warning (level==1) always added. Debug - test it
	if(level>v2_debug) return(0);
    }

    VL_STR(strtmp, MAX_STRING_LEN, in_msg);

    if((out=v2_json_out(in_json))) {
	v2_add_debug(level, "%s:\n\"%s\": %s", strtmp, v2_nn(in_json->id), out);
	v2_freestr(&out);
    }
    return(0);
}
/* =================================================================== */
int v2_jbox_warn(json_box_t *in_jbox) {


    return(0);
}
/* =================================================================== */
