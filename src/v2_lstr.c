/*
 *  Copyright (c) 2016-2020 Oleg Vlasenko <vop@unity.net>
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

// Family LSTR functions

#define _GNU_SOURCE

#include "v2_lstr.h"

#include <errno.h>
#include <assert.h>

// ERROR_CODE 175XX

// Need for internal intermediacalls
typedef struct {
    str_lst_t **plstr;
    int (*rdfun)(char*, str_lst_t**);
} _vstr_rdt_t;


/* ============================================================= */
// Add in_lstr to the head of p_lstr list
str_lst_t *v2_lstr_to_head(str_lst_t **p_lstr, str_lst_t *in_lstr) {

    assert(p_lstr);
    assert(in_lstr);

    in_lstr->next = *p_lstr;
    *p_lstr=in_lstr;

    return(*p_lstr);
}
/* ============================================================= */
// Make new lstr element with unallocated key
str_lst_t *v2_lstr_unew(char *una_key) {
    str_lst_t *str_tmp=NULL;

    str_tmp=v2_lstr.anew(una_key);
    str_tmp->is_key_unalloc = 1;

    return(str_tmp);
}
/* ============================================================= */
// Make new lstr element with already allocated key
str_lst_t *v2_lstr_anew(char *una_key) {
    str_lst_t *str_tmp=(str_lst_t *)calloc(sizeof(str_lst_t), 1);

    assert(una_key); // Not create
    assert(str_tmp);

    str_tmp->key            = una_key;

    return(str_tmp);
}
/* ============================================================= */
// Make new lstr and allocate key
str_lst_t *v2_lstr_new(char *in_frmt, ...) {
    va_list vl;
    char *tempa=NULL;
    int rc=0;

    if(!in_frmt || !*in_frmt) return(NULL);

    va_start(vl, in_frmt);
    rc=vasprintf(&tempa, in_frmt, vl);
    va_end(vl);

    if(rc==-1) return(0);

    return(v2_lstr.anew(tempa));
}
/* ============================================================= */
str_lst_t *v2_lstr_free_str(str_lst_t *in_lstr) {
    if(in_lstr) { // Free it, if exists
	if(in_lstr->is_str_unalloc) { // Unallocated value
	    in_lstr->is_str_unalloc=0;
	    in_lstr->str=NULL;
	} else {
	    v2_freestr(&in_lstr->str); // Clean previouse value
	}
    }
    return(in_lstr);
}
/* ============================================================= */
// Add unallocated str
str_lst_t *v2_lstr_ustr(str_lst_t *in_lstr, char *una_str) {

    if(v2_lstr_free_str(in_lstr)) {
	in_lstr->str=una_str;
	in_lstr->is_str_unalloc=1;
    }

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_astr(str_lst_t *in_lstr, char *in_str) {

    if(!v2_lstr_free_str(in_lstr)) return(NULL);

    if(in_str && *in_str) v2_let_var(&in_lstr->str, in_str);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_astrf(str_lst_t *in_lstr, char *format, ...) {
    va_list vl;
    int rc=0;

    if(!v2_lstr_free_str(in_lstr)) return(NULL);

    if(!format || !format[0]) return(in_lstr);

    va_start(vl, format);
    rc=vasprintf(&in_lstr->str, format, vl);
    va_end(vl);
    if(rc==-1) return(NULL);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_free_dstr(str_lst_t *in_lstr) {
    if(in_lstr) { // Free it, if exists
	if(in_lstr->is_dat_unalloc) { // Unallocated value
	    in_lstr->is_dat_unalloc=0;
	    in_lstr->dstr=NULL;
	} else {
	    v2_freestr(&in_lstr->dstr); // Clean previouse value
	}
    }
    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_dstr(str_lst_t *in_lstr, char *in_str) {

    if(!v2_lstr_free_dstr(in_lstr)) return(NULL);

    if(in_str && *in_str) v2_let_var(&in_lstr->dstr, in_str);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_dstrf(str_lst_t *in_lstr, char *format, ...) {
    va_list vl;
    int rc=0;

    if(!v2_lstr_free_dstr(in_lstr)) return(NULL);

    if(!format || !format[0]) return(in_lstr);

    va_start(vl, format);
    rc=vasprintf(&in_lstr->dstr, format, vl);
    va_end(vl);

    if(rc==-1) return(NULL);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t **v2_lstr_pend(str_lst_t **p_lstr) {
    str_lst_t *str_tmp=NULL;

    if(!p_lstr || !*p_lstr) return(p_lstr);

    FOR_LST_NEXT(str_tmp, *p_lstr);

    return(&str_tmp->next);
}
/* ============================================================= */
str_lst_t *v2_lstr_heads(str_lst_t **p_lstr, char *in_key, char *in_val, ...) {
    char strtmp[MAX_STRING_LEN];
    str_lst_t *str_tmp=NULL;
    if(!(str_tmp=v2_lstr.head(p_lstr, in_key))) return(NULL);
    if(in_val && *in_val) {
	VL_STR(strtmp, MAX_STRING_LEN, in_val);
	v2_lstr.astr(str_tmp, strtmp);
    }
    return(str_tmp);
}
/* ============================================================= */
str_lst_t *v2_lstr_tails(str_lst_t **p_lstr, char *in_key, char *in_val, ...) {
    char strtmp[MAX_STRING_LEN];
    str_lst_t *str_tmp=NULL;
    if(!(str_tmp=v2_lstr.tail(p_lstr, in_key))) return(NULL);
    if(in_val && *in_val) {
	VL_STR(strtmp, MAX_STRING_LEN, in_val);
	v2_lstr.astr(str_tmp, strtmp);
    }
    return(str_tmp);
}
/* ============================================================= */
// Find, if not add to head - do not replace is_rmv records
str_lst_t *v2_lstr_if_head(str_lst_t **p_lstr, char *in_key) {
    str_lst_t *str_tmp=NULL;

    if(!in_key || !*in_key || !p_lstr) return(NULL);

    if((str_tmp=v2_lstr.key(*p_lstr, in_key))) return(str_tmp);
    return(v2_lstr.head(p_lstr, in_key));
}
/* ============================================================= */
str_lst_t *v2_lstr_headu(str_lst_t **p_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!p_lstr || !in_key || !*in_key) return(NULL);

    if((str_out=v2_lstr.keyr(*p_lstr, in_key))) { // Find record include removed one
	str_out->is_rmv=v2_off;
    } else {
	str_out=v2_lstr.head(p_lstr, in_key);
    }

    return(str_out);
}
/* ============================================================= */
str_lst_t *v2_lstr_tailu(str_lst_t **p_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!p_lstr || !in_key || !*in_key) return(NULL);

    if((str_out=v2_lstr.keyr(*p_lstr, in_key))) { // Find record include removed one
	str_out->is_rmv=v2_off;
    } else {
	str_out=v2_lstr.tail(p_lstr, in_key);
    }

    return(str_out);
}
/* ============================================================= */
str_lst_t *v2_lstr_headf(str_lst_t **p_lstr, char *format, ...) {
    va_list vl;
    str_lst_t *str_tmp=NULL;
    char *tempa=NULL;
    int rc=0;

    if(!p_lstr)               return(NULL);
    if(!format || !format[0]) return(NULL);

    va_start(vl, format);
    rc=vasprintf(&tempa, format, vl);
    va_end(vl);

    if(rc==-1) return(NULL);

    str_tmp                 = v2_lstr.unew(tempa);
    str_tmp->is_key_unalloc = 0; // We allocated this key already

    return(v2_lstr.to_head(p_lstr, str_tmp));
}
/* ============================================================= */
str_lst_t *v2_lstr_tailf(str_lst_t **p_lstr, char *format, ...) {
    va_list vl;
    str_lst_t *str_tmp=NULL;
    char *tempa=NULL;
    int rc=0;

    if(!p_lstr)               return(NULL);
    if(!format || !format[0]) return(NULL);

    va_start(vl, format);
    rc=vasprintf(&tempa, format, vl);
    va_end(vl);

    if(rc==-1) return(NULL);

    str_tmp                 = v2_lstr.unew(tempa);
    str_tmp->is_key_unalloc = 0; // We allocated this key already

    return(v2_lstr.to_tail(p_lstr, str_tmp));
    //v2_lstr_add_tail(p_lstr, str_tmp);

    //return(str_tmp);
}
/* ============================================================= */
str_lst_t *v2_lstr_head(str_lst_t **p_lstr, char *in_key) {

    if(!in_key || !*in_key) return(NULL);

    return(v2_add_lstr_head(p_lstr, in_key, NULL, 0, 0));
}
/* ============================================================= */
str_lst_t *v2_lstr_tail(str_lst_t **p_lstr, char *in_key) {

    if(!in_key || !*in_key) return(NULL);

    return(v2_add_lstr_tail(p_lstr, in_key, NULL, 0, 0));
}
/* ============================================================= */
// Add already allocated key
str_lst_t *v2_lstr_heada(str_lst_t **p_lstr, char *in_key) {
    return(v2_lstr.to_head(p_lstr, v2_lstr.anew(in_key)));
}
/* ============================================================= */
// Add already allocated key
str_lst_t *v2_lstr_taila(str_lst_t **p_lstr, char *in_key) {
    return(v2_lstr.to_tail(p_lstr, v2_lstr.anew(in_key)));
}
/* ============================================================= */
// Get data for linked element
void *v2_lstr_link_data(str_lst_t *in_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!in_lstr)                                      return(NULL);
    if(!(str_out=v2_lstr.key(in_lstr->link, in_key))) return(NULL);

    return(str_out->data);
}
/* ============================================================= */
// Finds and adds if needed to in_lstr->link element with key
str_lst_t *v2_lstr_link(str_lst_t *in_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!in_lstr)              return(NULL);
    if(!in_key || !in_key[0]) return(NULL);

    if((str_out=v2_lstr.keyr(in_lstr->link, in_key))) {
        str_out->is_rmv=v2_off;      // Reset remiving
    } else {
        str_out=v2_lstr.head(&in_lstr->link, in_key);
    }

    return(str_out);
}
/* ============================================================= */
str_lst_t *v2_lstr_find_num(str_lst_t *in_lstr, int in_num) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_out=NULL;

    if(!in_num) return(0); // Not zerro found

    FOR_LST_IF(str_tmp, str_out, in_lstr) {
	if(str_tmp->num==in_num) str_out=str_tmp;
    }

    return(str_out);
}
/* ============================================== */
// Get key element include is_rmv attribute
str_lst_t *v2_lstr_get_keyr(str_lst_t *p_str, char *in_key) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_rrr=NULL;

    if(!p_str || !in_key || !*in_key) return(NULL);

    FOR_LST_IF(str_tmp, str_rrr,  p_str) {
	if(!v2_strcmp(str_tmp->key, in_key)) str_rrr=str_tmp;
    }

    return(str_rrr);
}
/* ============================================================= */
str_lst_t *v2_lstr_del_key(str_lst_t *in_lstr, char *in_key) {
    //str_lst_t *str_out=NULL;

    //str_out=v2_lstr.key(in_lstr, in_key);
    //if(str_out) str_out->is_rmv=v2_on;
    //return(str_out);
    return(v2_lstr.rmv(v2_lstr.key(in_lstr, in_key)));
}
/* ============================================================= */
// Check if list contain alive records with noempty keys
int v2_lstr_is_nodel(str_lst_t *in_lstr) {
    str_lst_t *str_tmp=NULL;
    int out=0;

    FOR_LST_IF(str_tmp, out, in_lstr) {
	if(!str_tmp->is_rmv && v2_is_par(str_tmp->key)) out=1; // Alive record
    }

    return(out);
}
/* ============================================================= */
 // Make full copy of list
str_lst_t *v2_lstr_copy(str_lst_t **p_dst, str_lst_t *in_src) {
    str_lst_t *str_tmp=NULL;

    if(!p_dst) return(NULL);

    FOR_LST(str_tmp, in_src) {
	if(v2_add_lstr_head(p_dst, str_tmp->key, str_tmp->str, str_tmp->a_time, str_tmp->b_time)) return(0);
	(*p_dst)->value  = str_tmp->value;
	(*p_dst)->num    = str_tmp->num;
	(*p_dst)->is_rdt = str_tmp->is_rdt;
	(*p_dst)->is_rmv = str_tmp->is_rmv;

	(*p_dst)->link   = str_tmp->link; // ???

	v2_lstr_add_data(*p_dst, str_tmp->data); // Non allocated adding
    }

    v2_lstr.back(p_dst);
    return(*p_dst);
}
/* ============================================================= */
/*
static int print_str_lst(FILE *stream, const struct printf_info *info, const void *const *args) {
  const str_lst_t *s;
  char *buffer;
  int len;

  // Format the output into a string.
  s = *((const str_lst_t **) (args[0]));
  if(s->str) {
      len=asprintf (&buffer, "%s %s", s->key, s->str);
  } else {
      len=asprintf (&buffer, "%s", s->key);
  }
  if (len == -1) return(-1);

  // Pad to the minimum field width and print to the stream.
  len = fprintf (stream, "%*s", (info->left ? -info->width : info->width), buffer);

  // Clean up and return
  free(buffer);
  return(len);
}*/
/* ============================================================= */
/*
static int print_str_lst_arginfo(const struct printf_info *info, size_t n, int *argtypes, int *size) {
    if (n > 0) argtypes[0] = PA_POINTER;
    return(1);
}*/
/* ============================================================= */
/*
int v2_lstr_init_print(void) {
    register_printf_specifier('R', &print_str_lst, &print_str_lst_arginfo);
    return(0);
}*/
/* ============================================================= */
static int v2_lstr_to_str(str_lst_t *in_lstr, char **p_str, char in_dlm) {
    str_lst_t *str_tmp=NULL;
    char dlm=in_dlm;
    int cnt=0;

    FOR_LST(str_tmp, in_lstr) {
        if(str_tmp->is_rmv) continue;
	cnt += strlen(str_tmp->key) + 1;
    }

    if(p_str) {
	v2_freestr(p_str);
        if(!cnt) return(0);

	*p_str=(char*)calloc(cnt, 1);
	cnt=0;
        if(!dlm) dlm=',';
	FOR_LST(str_tmp, in_lstr) {
	    if(str_tmp->is_rmv) continue;
	    cnt += sprintf((*p_str)+cnt, "%s", str_tmp->key);
            if(str_tmp->next) cnt += sprintf((*p_str)+cnt, "%c", dlm);
	}
    }
    return(cnt);
}
/* ========================================================= */
static str_lst_t *v2_lstr_fm_str(char *in_str, char spr) {
    str_lst_t *str_out=NULL;
    char *beg, *end;

    if(!in_str || !*in_str) return(NULL);

    if(!spr) return(NULL); // ?? MAybe replace to white space?

    beg=in_str;

    while(beg) {

	while(*beg == spr) beg++; // Skip empty spaces
	//if(*beg=='\0') continue; // Exit

	if((end=strchr(beg, spr))) { // Last point
	    v2_lstr.heada(&str_out, v2_let_varn(NULL, beg, end-beg));
	    beg=end+1;
	} else {
	    v2_lstr.head(&str_out, beg);
	    beg=NULL; // End
	}
    }

    v2_lstr.back(&str_out);

    return(str_out);
}
/* ============================================================= */
// Output functions
/* ============================================================= */
// Format details at "v2_lstr.h" file
static int v2_lstr_fprnf(FILE *in_cf, str_lst_t *in_lstr, char *in_frmt, ...) {
    char strout[MAX_STRING_LEN];
    char format[MAX_STRING_LEN];
    str_lst_t *str_tmp=NULL;
    int is_skip=0;
    int is_skip_key=0;
    int is_skip_str=0;
    int is_skip_data=0;
    int no_show=0;
    int cnt=0;
    int out=0;
    int x;

    if(!in_lstr) return(17561);

    if(!in_cf) in_cf=stdout;

    VL_STR(format, 0, in_frmt);

    //no_show      = 0;
    //is_skip      = 0;
    //is_skip_key  = 0;
    //is_skip_str  = 0;
    //is_skip_data = 0;

    // Just read options
    for(x=0; format[x]; x++) {
	if(format[x] != '%')   continue;
	if(format[x+1] != 'O') continue;
	if(format[x+2] == 'v') is_skip=1;
	if(format[x+2] == 'k') is_skip_key=1;
	if(format[x+2] == 's') is_skip_str=1;
	if(format[x+2] == 'd') is_skip_data=1;
    }



    FOR_LST(str_tmp, in_lstr) {

	if(str_tmp->is_rmv) continue;
	if(!str_tmp->key)   continue;

	if(format[0]=='\0') {
	    cnt+=fprintf(in_cf, "%s%s%s\n", str_tmp->key, str_tmp->str?" ":"", str_tmp->str?str_tmp->str:"");
	    continue;
	}

	// Here we need to make format %Tk %Ts %Ta
	for(x=0, cnt=0; format[x]; x++) {
	    if(format[x] != '%') {
		strout[cnt++]=format[x];
		continue;
	    }
	    x++;

	    if(format[x] == 'T') { // Format output
		x++;
		if(format[x] == 'k') {
		    if(!str_tmp->key && (is_skip || is_skip_key)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_nn(str_tmp->key));
		} else if(format[x] == 's') {
		    if(!str_tmp->str && (is_skip || is_skip_str)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_nn(str_tmp->str));
		} else if(format[x] == 'v') {
		    if(!str_tmp->key && !str_tmp->str && is_skip) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_lstr_str(str_tmp));
		} else if(format[x] == 'd') {
		    if(!str_tmp->dstr && (is_skip || is_skip_data)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_nn(str_tmp->dstr));
		} else if(format[x] == 'a') {
		    cnt+=sprintf(strout+cnt, "%ld", (long)str_tmp->a_time);
		} else if(format[x] == 'b') {
		    cnt+=sprintf(strout+cnt, "%ld", (long)str_tmp->b_time);
		} else if(format[x] == 'n') {
		    cnt+=sprintf(strout+cnt, "%d", str_tmp->num);
		} else if(format[x] == 'r') {
		    cnt+=sprintf(strout+cnt, "%d", str_tmp->is_rdt);
		} else if(format[x] == 'f') {
		    cnt+=sprintf(strout+cnt, "%f", str_tmp->value);
		} else if(format[x] == 'e') {
		    cnt+=sprintf(strout+cnt, "%e", str_tmp->value);
		} else if(format[x] == 'l') { // All line
		    if(!str_tmp->key && (is_skip || is_skip_key)) no_show=1;
		    if(!str_tmp->str && (is_skip || is_skip_str)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s%s%s", str_tmp->key, str_tmp->str?" ":"", str_tmp->str?str_tmp->str:"");
		} else {
		    strout[cnt++]=format[x-2]; // '%'
		    strout[cnt++]=format[--x]; // 'T' & set pointer to prev pos
		}
		continue;
	    }

	    if(format[x] == 'O') { // Skip config options
		x++; // 's'
		if(format[x] == 'v') continue; // Skip if empty key or str
		if(format[x] == 'k') continue; // Skip if empty key or str
		if(format[x] == 's') continue; // Skip if empty key or str
		if(format[x] == 'd') continue; // Skip if empty key or str

		strout[cnt++]=format[x-2]; // '%'
		strout[cnt++]=format[--x]; // 'O' and set pointer to prev pos
		continue;
	    }

	    strout[cnt++]=format[--x]; // %
	}

	strout[cnt]='\0';
	if(!no_show) out+=fprintf(in_cf, "%s", strout);
    }

    return(cnt);
}
/* ============================================================= */
static int v2_lstr_prnf(str_lst_t *in_lstr, char *in_frmt, ...) {
    char strtmp[MAX_STRING_LEN];

    if(in_frmt && *in_frmt) {
	VL_STR(strtmp, 0, in_frmt);
    } else {
	strtmp[0]='\0';
    }
    return(v2_lstr.fprnf(NULL, in_lstr, "%s", strtmp)); // Print it w/ format to stdout
}
/* ============================================================= */
static int v2_lstr_prn(str_lst_t *in_lstr) {
    return(v2_lstr.prnf(in_lstr, NULL)); // Print it w/o format
}
/* ============================================================= */
// File functions
/* ============================================================= */
// !!! This is prototype of external function
//static int v2_lstr_read_rdfun(char *in_str, str_lst_t **p_lstr) {
//
//    v2_add_lstr_head(p_lstr, in_str, NULL, 0, 0);
//
//    return(0);
//}
/* ============================================================= */
static int v2_lstr_read_str(char *in_str, void *in_data) {
    _vstr_rdt_t *_rdt=(_vstr_rdt_t*)in_data;

    if(!_rdt)       return(0); // ???? - return error?
    if(_rdt->rdfun) return(_rdt->rdfun(in_str, _rdt->plstr));

    if(!in_str || !*in_str) return(0); // Empty sting can not be added to lstr

    if(!v2_lstr.head(_rdt->plstr, in_str)) return(17575);

    return(0);
}
/* ============================================================= */
static int v2_lstr_read(int(*rd_fun)(char*, str_lst_t**), str_lst_t **p_lstr, char *file, ...) {       // Read file to str_lst
    _vstr_rdt_t rdt;
    va_list vl;
    char *tfile=NULL;
    int rc=0;

    if(!file || !*file)       return(17570);
    if(!(rdt.plstr = p_lstr)) return(17571);

    v2_lstr.free(p_lstr);

    rdt.rdfun = rd_fun;

    va_start(vl, file);
    rc=vasprintf(&tfile, file, vl);
    va_end(vl);

    if(rc==-1) return(XFREAD_NOT_NAME); // Suspect filename is NULL

    rc=sfread(&v2_lstr_read_str, (void *)&rdt, tfile);

    v2_lstr.back(p_lstr);

    free(tfile);

    return(rc);
}
/* ============================================================= */
static int v2_lstr_xread(str_lst_t **p_lstr, char *file, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, file);
    return(v2_lstr.read(NULL, p_lstr, "%s", strtmp));
}
/* ============================================================= */
// Config functions
/* ============================================================= */
static char *v2_lstr_cf_str(str_lst_t *cf_lstr, char **pvar, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);
    if(!pvar)                           return(cf_lstr->str);

    cf_lstr->is_rdt=V_L_CHAR; // 1 - char**
    v2_lstr_add_data(cf_lstr, (void*)pvar);
    return(v2_let_var(pvar, cf_lstr->str));
}
/* ============================================================= */
static char *v2_lstr_cf_int(str_lst_t *cf_lstr, int *pint, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);

    if(!cf_lstr->str)                   return(NULL);
    if(!pint)                           return(cf_lstr->str);

    cf_lstr->num=atoi(cf_lstr->str);
    cf_lstr->is_rdt=V_L_INT; // 2 - int*
    v2_lstr_add_data(cf_lstr, (void*)pint);
    *pint=cf_lstr->num;

    return(cf_lstr->str);
}
/* ============================================================= */
static char *v2_lstr_cf_lng(str_lst_t *cf_lstr, long *plng, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);

    if(!cf_lstr->str)                   return(NULL);
    if(!plng)                           return(cf_lstr->str);

    cf_lstr->a_time=strtol(cf_lstr->str, NULL, 0);
    cf_lstr->is_rdt=V_L_LONG; // 2 - long*
    v2_lstr_add_data(cf_lstr, (void*)plng);
    *plng=cf_lstr->a_time;

    return(cf_lstr->str);
}
/* ============================================================= */
static char *v2_lstr_cf_flg(str_lst_t *cf_lstr, int *pflg, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);

    if(!cf_lstr->str)                   return(NULL);
    if(!pflg)                           return(cf_lstr->str);

    cf_lstr->num=1;
    cf_lstr->is_rdt=2; // 2 - int*
    v2_lstr_add_data(cf_lstr, (void*)pflg);
    *pflg=cf_lstr->num;

    return(cf_lstr->str?cf_lstr->str:"1");
}
/* ============================================================= */
// File: [#|;][:|!]file_name
// ; - skip comment strings
// # - skip comment strings, but keep special comment starting from "#=#..."
// : - not open file, but run script
// ! - not error if file not found
// File: [#|;]-
// Read stdin

static int v2_lstr_fcfg(str_lst_t **pt_lst, char *file, ...) {
    FILE *cf=NULL;
    char strtmp[MAX_STRING_LEN];
    int is_exec=0;
    int r_pos=0; // Remark position
    int no_spc=0; // Special mode
    int no_found=0;

    if(!pt_lst) return(-742);
    v2_lstr_free(pt_lst);

    VL_STR(strtmp, MAX_STRING_LEN, file);

    if(!strtmp[0]) return(-741);

    if(strtmp[0] == '#') r_pos=1; // Filter comments, but not special config lines "#=#..."
    if(strtmp[0] == ';') {r_pos=1; no_spc=1;} // Filter all comments

    if(strtmp[r_pos] == ':') is_exec=1;  // Exec script
    if(strtmp[r_pos] == '!') no_found=1; // Ignore "file not found"


    errno=0;
    if(strtmp[r_pos+no_found] == '-') {
	cf=stdin;
    } else if(is_exec) {
	if(!(cf=popen(strtmp+1+r_pos, "r"))) return(XFREAD_NOT_RUN);
    } else if(!(cf=fopen(strtmp+r_pos+no_found, "r"))) {
	if(errno != ENOENT) return(XFREAD_NOT_ACCESS); // Exists, but unreadble
	if(no_found) return(0); // Ignore not founded file.
	return(XFREAD_NOT_FOUND);
    }

    while(!v2_getline(strtmp, MAX_STRING_LEN, cf)) {
	if(!strtmp[0]) continue;
	if(r_pos) {
	    if(strtmp[0]==';') continue;
	    if(strtmp[0]=='#') {
		if(no_spc)     continue; // Not keep special lines "#=#..."
		if(strtmp[1]!='=') continue;
		if(strtmp[2]!='#') continue; // Special config parameter
	    }
	}
	v2_lstr_separate(v2_lstr.head(pt_lst, strtmp));
    }
    v2_lstr_rback(pt_lst);

    if(cf==stdin) return(0); // Standart input still to be opened

    if(is_exec) return(pclose(cf));

    if(fclose(cf)) return(XFREAD_NOT_CLOSE); // EBADF - Descriptor is not valid...

    return(0);
}
/* ============================================================= */
str_lst_t *v2_lstr_keyf(str_lst_t *in_list, char *in_key, ...) {
    char strtmp[MAX_STRING_LEN];

    if(!v2_is_par(in_key)) return(NULL);

    VL_STR(strtmp, MAX_STRING_LEN, in_key);

    return(v2_get_lstr_key(in_list, strtmp));
}
/* ============================================================= */
char *v2_lstr_strf(str_lst_t *in_list, char *in_key, ...) {
    char strtmp[MAX_STRING_LEN];

    if(!v2_is_par(in_key)) return(NULL);

    VL_STR(strtmp, MAX_STRING_LEN, in_key);

    return(v2_get_lstr_str(in_list, strtmp));
}
/* ============================================================= */
// If in_len == 0 just find by key
str_lst_t *v2_lstr_keyn(str_lst_t *in_lstr, int in_len, char *in_key) { // Find/get bypart of input word
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_out=NULL;
    int len=0;

    if(!in_lstr)           return(NULL);
    if(!v2_is_par(in_key)) return(NULL);

    if(!in_len) return(v2_get_lstr_key(in_lstr, in_key));

    len=strlen(in_key);

    if(len < in_len) return(NULL);

    FOR_LST_IF(str_tmp, str_out, in_lstr) {
	if(str_tmp->is_rmv || !str_tmp->key || !str_tmp->key[0]) continue;
	if(strncmp(str_tmp->key, in_key, in_len)) continue;
	str_out=str_tmp;
    }

    return(str_out);
}
/* ============================================================= */
// Argv functions
/* ============================================================= */
int v2_lstr_len(str_lst_t *in_lstr) {
    str_lst_t *str_tmp=NULL;
    int out=0;

    FOR_LST(str_tmp, in_lstr) out++;
    return(out);
}
/* ============================================================= */
char **v2_lstr_argv(str_lst_t *in_lstr) {
    str_lst_t *str_tmp=NULL;
    char **argv_out=NULL;
    int cnt=0;

    argv_out=(char**)calloc(sizeof(char*), v2_lstr_len(in_lstr)+1);
    assert(argv_out);

    FOR_LST(str_tmp, in_lstr) argv_out[cnt++]=v2_let_var(NULL, str_tmp->key);

    return(argv_out);
}
/* ============================================================= */
int v2_lstr_exec(char *in_args, ...) {
    str_lst_t *arg_lst=NULL;
    char strarg[MAX_STRING_LEN]; // can not be longer ARG_MAX bytes
    char **argvv;

    VL_STR(strarg, MAX_STRING_LEN, in_args);

    if(!strarg[0]) return(16540);

    arg_lst=v2_lstr.fm_str(strarg, ' '); // argv[0] ... argv[n]
    argvv=v2_lstr.argv(arg_lst);
    v2_lstr.free(&arg_lst);

    if(execvp(argvv[0], argvv)) return(16541);
    return(0); // This never happensÂ because function does not returns at success
}


/* ================================================================== */
int lstr_compare_r(const void *in_one, const void *in_two, void *in_data) {
    int (*fun)(str_lst_t *, str_lst_t *)=in_data;
    str_lst_t *str_one=*(str_lst_t * const *)in_one;
    str_lst_t *str_two=*(str_lst_t * const *)in_two;

    if(!str_one || !str_one->key) return(0);
    if(!str_two || !str_two->key) return(0);

    if(fun) return(fun(str_one, str_two));
    return(strcoll(str_one->key, str_two->key));
}
/* ================================================================== */
// FreeBSD version
#ifndef __linux__
int lstr_compare_f(void *in_data, const void *in_one, const void *in_two) {
    return(lstr_compare_r(in_one, in_two, in_data));
}
#endif
/* ================================================================== */
int lstr_compare(const void *in_one, const void *in_two) {
    return(lstr_compare_r(in_one, in_two, NULL));
}
/* ============================================================= */
 // Sort with qsort fun
str_lst_t *v2_lstr_qsort(v2_qlstr_t *qbase, int (*fun_sort)(str_lst_t *, str_lst_t *)) {
    str_lst_t *str_tmp=NULL;
    int x=0;

    if(!qbase) return(NULL);
    if(!qbase->list) return(NULL);

    qbase->num=0;
    FOR_LST(str_tmp, qbase->list) qbase->num++;
    if(qbase->num==0)  return(NULL); // Notnig to sort

    // Prepare Array for QSort
    if(!(qbase->array=(str_lst_t **)malloc(qbase->num*sizeof(str_lst_t*)))) v2_abort("qsort: Memory allocation error.");

    FOR_LST(str_tmp, qbase->list) qbase->array[x++]=str_tmp;

#ifdef __linux__
    qsort_r(&(qbase->array[0]), qbase->num, sizeof(str_lst_t*), lstr_compare_r, (void*)fun_sort);
#else
    qsort_r(&(qbase->array[0]), qbase->num, sizeof(str_lst_t*), (void*)fun_sort, lstr_compare_f);
#endif

    // Link sort
    for(x=0; x<qbase->num; x++) {
	if(x == (qbase->num-1)) { // Last element
	    qbase->array[x]->next=NULL;
	} else {
	    qbase->array[x]->next=qbase->array[x+1];
	}
    }
    qbase->list=qbase->array[0];
    return(qbase->list);
}
/* ============================================================= */
// Search element after v2_lstr_qsort sorting
str_lst_t *v2_lstr_qkey(v2_qlstr_t *qbase, char *in_key) {
    str_lst_t *str_hint=NULL; // Hint
    str_lst_t **pnt_ret=NULL;

    if(!qbase || !qbase->list || !in_key || !*in_key) return(NULL); // Nothing to search

    if(!qbase->num) qbase->list=v2_lstr_qsort(qbase, NULL); // Sort it

    v2_lstr.head(&str_hint, in_key);
    pnt_ret=(str_lst_t **)bsearch(&str_hint, qbase->array, qbase->num, sizeof(str_lst_t*), lstr_compare);
    v2_lstr.free(&str_hint);

    if(pnt_ret) return(*pnt_ret);
    return(NULL);
}
/* ============================================================= */


lstr_t v2_lstr = {

    .unew    = &v2_lstr_unew,     // Create new element with unallocated key
    .ustr    = &v2_lstr_ustr,     // Add unallocated string

    .anew    = &v2_lstr_anew,     // Create new element with already allocated key

    .to_head = &v2_lstr_to_head,  // Add in_lstr to head of p_lstr list
    .to_tail = &v2_lstr_add_tail, // Add in_lstr to tail of p_lstr list // v2_util

    .astr    = &v2_lstr_astr, // Add string to str
    .dstr    = &v2_lstr_dstr, // Add string to data - dstr

    .astrf   = &v2_lstr_astrf, // The same
    .dstrf   = &v2_lstr_dstrf,    // Add string to data - dstr

    .heads = &v2_lstr_heads,
    .tails = &v2_lstr_tails,

    //.headu = &v2_lstr_headu, // Update (undel or add)
    //.tailu = &v2_lstr_tailu,

    .headf = &v2_lstr_headf, // Add to head with format
    .tailf = &v2_lstr_tailf,

    .head  = &v2_lstr_head,
    .tail  = &v2_lstr_tail,

    .heada = &v2_lstr_heada, // Add allocated key
    .taila = &v2_lstr_taila, // Add allocated key

    .pend  = &v2_lstr_pend,

    .if_head  = &v2_lstr_if_head, // !!! Better to use .headu()

    .link = &v2_lstr_link,
    .sort = &v2_lstr_sort,   // v2_util
    .back = &v2_lstr_rback,  // v2_util
    .free = &v2_lstr_free,   // v2_util

    .find = &v2_lstr_find,

    .to_str = &v2_lstr_to_str,
    .fm_str = &v2_lstr_fm_str, // New test version
    .fm_str_old = &v2_str_to_lstr,

    .keyf   = &v2_lstr_keyf,
    .strf   = &v2_lstr_strf,

    .key    = &v2_get_lstr_key, // v2_util
    .str    = &v2_get_lstr_str, // v2_util
    .num    = &v2_lstr_find_num, // Find by number, if it non-zero
    .data   = &v2_get_lstr_data, // v2_util

    .keyr   = &v2_lstr_get_keyr, // Tha same, as key, but include is_rmv attirbute

    .keyn   = &v2_lstr_keyn,     // Find rec by key as part of word

    .del    = &v2_lstr_del_key,
    .rmv    = &v2_lstr_rmv, // v2_util

    .is_nodel = &v2_lstr_is_nodel, // List contains alive elements

    // Output functions
    .prn    = &v2_lstr_prn,
    .prnf   = &v2_lstr_prnf,
    .fprnf  = &v2_lstr_fprnf,

    // File functions
    .read   = &v2_lstr_read,
    .xread  = &v2_lstr_xread,

    // Conf functions
    .fcfg   = &v2_lstr_fcfg,   // Read config file to str_lst
    .cf_str = &v2_lstr_cf_str, // Get config string
    .cf_int = &v2_lstr_cf_int, // Get config integer
    .cf_lng = &v2_lstr_cf_lng, // Get config long integer
    .cf_flg = &v2_lstr_cf_flg, // Get config flag (=1)

    // Argv
    .len    = &v2_lstr_len,    // Count number of elemts into list (like arg)
    .argv   = &v2_lstr_argv,   // Creates arg[] massive from str_lst_t
    .exec   = &v2_lstr_exec,   // Exec external command

};
/* ============================================================= */

