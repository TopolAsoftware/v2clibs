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

// ERROR_CODE 149XX

#include "v2_wrbuf.h"
#include "v2_util.h"
#include <sys/types.h>
#include <unistd.h>

/* ================================================================ */
// Check if buffer ok and has data
int v2_wrbuf_ok(wrbuf_t *in_wrf) {

    if(!in_wrf)      return(14901);
    if(!in_wrf->yet) return(14902);
    if(!in_wrf->pos) return(14903);

    return(0);
}
/* ================================================================ */
int v2_wrbuf_init(wrbuf_t *in_wrf) {

    if(in_wrf) memset((void*)in_wrf, 0, sizeof(wrbuf_t));

    return(0);
}
/* ================================================================ */
int v2_wrbuf_reset(wrbuf_t *in_wrf) {

    if(!in_wrf) return(0);

    if(in_wrf->buf) free(in_wrf->buf);
    if(in_wrf->str) free(in_wrf->str);
    return(v2_wrbuf_init(in_wrf));
}
/* ================================================================ */
int v2_wrbuf_new(wrbuf_t **p_wrf) {

    if(!p_wrf) return(14970);
    if(!(*p_wrf)) {
	if(!(*p_wrf=(wrbuf_t *)calloc(sizeof(wrbuf_t), 1))) return(14971);
        return(0);
    }

    return(v2_wrbuf_reset(*p_wrf));
}
/* ================================================================ */
int v2_wrbuf_free(wrbuf_t **p_wrf) {

    if(!p_wrf)  return(0);
    if(!*p_wrf) return(0);

    v2_wrbuf_reset(*p_wrf);
    free(*p_wrf);
    *p_wrf=NULL;

    return(0);
}
/* ================================================================ */
int v2_wrbuf_seek(wrbuf_t *in_wrf, size_t in_position) {

    if(!in_wrf) return(0);
    if(!in_wrf->buf) return(0); // Nothing to seek

    if(in_position>in_wrf->cnt) in_position=in_wrf->cnt;
    if(in_position == -1)       in_position=in_wrf->cnt;

    in_wrf->pos=in_wrf->buf+in_position;
    in_wrf->yet=in_wrf->cnt-in_position;

    return(0);
}
/* ================================================================ */
// Sets next string
int v2_wrbuf_nxtstr(wrbuf_t *in_wrf) {
    char *pnt=NULL;
    size_t str_siz=0;
    int excode=0;

    if(!in_wrf) return(14909);

    // First off all clean previouse string if exists
    if(in_wrf->str) {
	free(in_wrf->str);
        in_wrf->str=NULL;
    }

    // Check buffer
    if(!in_wrf->buf) return(1); // No data at buffer
    if(!in_wrf->cnt) return(1); // No data at buffer

    // Set values
    if(!in_wrf->pos) in_wrf->pos=in_wrf->buf;

    pnt=in_wrf->pos;
    while((*(in_wrf->pos) != '\0') && (*(in_wrf->pos) != '\n') && (*(in_wrf->pos) != '\r') && (in_wrf->pos < (in_wrf->buf+in_wrf->cnt))) (in_wrf->pos)++;

    str_siz=in_wrf->pos-pnt;

    if(str_siz != 0) { // Non-Zerro string
	if(!(in_wrf->str=(char *)malloc(str_siz+1))) return(14910);

	memcpy(in_wrf->str, pnt, str_siz); // better use strncpy
	in_wrf->str[str_siz]='\0';
    }

    if(*(in_wrf->pos) == '\n') {
	(in_wrf->pos)++;
        if(*(in_wrf->pos) == '\r') (in_wrf->pos)++;
    } else if(*in_wrf->pos == '\r') {
        (in_wrf->pos)++;
        if(*(in_wrf->pos) == '\n') (in_wrf->pos)++;
    } else {
	if(!str_siz) excode = 1; // Means not more data into buffer
    }

    in_wrf->yet=in_wrf->cnt-(in_wrf->pos-in_wrf->buf); // Count letf biyites

    return(excode);
}
/* ================================================================ */
size_t v2_wrbuf_write(wrbuf_t *in_wrf, char *in_str, size_t in_size, size_t in_num) {
    int cnt=in_size*in_num;

    if(in_num==-1) return(-1);
    if(cnt<0)      return(-1);
    if(cnt==0)     return(0);

    if(!in_wrf)  return(-1);
    if(!in_str)  return(-1);

    if(!in_wrf->sbl) in_wrf->sbl=V2_WRBUF_BLOCK;

    if((in_wrf->cnt+cnt) >= in_wrf->siz) { // Have to rellocate buffer + place for '\0'
	in_wrf->siz += cnt+in_wrf->sbl;
	if(!(in_wrf->buf=(char *)realloc(in_wrf->buf, in_wrf->siz))) return(-1);
    }

    if(!memcpy(&in_wrf->buf[in_wrf->cnt], in_str, cnt)) return(-1);

    in_wrf->cnt += cnt;
    in_wrf->buf[in_wrf->cnt] = '\0';

    in_wrf->pos = in_wrf->buf; // Have to set every time because buf can change
    in_wrf->yet = in_wrf->cnt;

    return(cnt); // Return number of stored bytes
}
/* ================================================================ */
size_t v2_wrbuf_curl(char *in_str, size_t in_size, size_t in_num, void *userdata) {
    wrbuf_t *in_wrb=(wrbuf_t *)userdata;

    if(!in_wrb) return(-1);
    if(in_size * in_num == 0) return(0); // This is OK for curl to call "write" zero number of bytes
    return(v2_wrbuf_write(in_wrb, in_str, in_size, in_num));
}
/* ================================================================ */
int v2_wrbuf_printf(wrbuf_t *in_wrf, char *format, ...) {
    va_list vl;
    //char *tmp=NULL;
    char tmp[20000];
    int len=0;
    int out=0;

    if(!in_wrf) return(-1); // ???
    if(!format || !format[0]) return(0);

    va_start(vl, format);
    len=vsnprintf(tmp, 20000-1, format, vl);
    va_end(vl);

    if(len<1) return(len); // 0 or -1

    out=v2_wrbuf_write(in_wrf, tmp, 1, len);

    //free(tmp);

    return(out);
}
/* ================================================================ */
int v2_wrbuf_file_read(wrbuf_t *in_wrf, char *file, ...) {
    va_list vl;
    FILE *cf=stdin;
    char buffer[V2_WRBUF_BLOCK];
    size_t reds=0;
    //size_t wrot=0;

    if(!in_wrf) return(14961);
    if(!file || !file[0]) return(0);

    if(strcmp(file, "-")) {
	va_start(vl, file);
	if(vsprintf(buffer, file, vl) < 1) return(14962);
	va_end(vl);

	if(!(cf=fopen(buffer, "r"))) return(14963);
    }

    while((reds=fread(buffer, 1, V2_WRBUF_BLOCK, cf)) && (reds != -1)) {
	//printf("Reds: %d\n", (int)reds);
	//if(reds == -1) break;
	reds=v2_wrbuf_write(in_wrf, buffer, 1, reds);
    }

    v2_wrbuf_seek(in_wrf, 0);

    if(cf!=stdin) {
	if(fclose(cf)) return(14964);
    }

    if(reds==-1) return(14965);
    return(0);
}
/* ================================================================ */
// Get data block form wrbuf->buf to wrbuf->str in_size len
size_t v2_wrbuf_data(wrbuf_t *in_wrf, size_t in_size) {
    size_t outs=in_size;

    if(!in_wrf)  return(0);

    if(in_wrf->str) {
	free(in_wrf->str);
        in_wrf->str=NULL;
    }

    if(outs > in_wrf->yet) outs=in_wrf->yet;
    if(outs==0) return(0); // Nothing left or to read

    if(!(in_wrf->str=(char *)malloc(outs+1))) return(0); // Need to be diagnisted or asserted

    memcpy(in_wrf->str, in_wrf->pos, outs);
    in_wrf->str[outs] = '\0';

    in_wrf->pos+=outs;
    in_wrf->yet-=outs;

    return(outs);
}
/* ================================================================ */
// This function can be added to u_cgi.read_post = &some funct... = read this one
int v2_wrbuf_read_post(wrbuf_t *in_wrf) {
    char *par=NULL;
    //size_t result=9;
    //int clen=0;

    if(!in_wrf)                                  return(14980);
    v2_wrbuf_reset(in_wrf);

    if(!(par=getenv("CONTENT_LENGTH")))          return(14981);
    if(!(in_wrf->siz=atoi(par)))                 return(V2_WRBUF_EMPTY_POST); // == 14872

    if(!(in_wrf->buf=(char *)calloc(in_wrf->siz+1, 1))) return(14983);

    if((in_wrf->cnt=fread(in_wrf->buf, 1, in_wrf->siz, stdin)) == -1) return(14984);

    if(in_wrf->siz != in_wrf->cnt) return(14985);
    in_wrf->buf[in_wrf->cnt]='\0';

    v2_wrbuf_seek(in_wrf, 0); // Reset to start

    return(0);
}
/* ================================================================ */
// Save wrbuf to file
int v2_wrbuf_save(wrbuf_t *in_wrf, char *to_file, ...) {
    FILE *cf=NULL;
    char f_name[MAX_STRING_LEN];
    char t_name[MAX_STRING_LEN+40];
    size_t out=0;
    va_list vl;
    int rc=0;

    if((rc=v2_wrbuf_ok(in_wrf))) return(rc);
    if(!to_file || !to_file[0])  return(14930);

    va_start(vl, to_file);
    vsnprintf(f_name, MAX_STRING_LEN-16, to_file, vl);
    va_end(vl);

    snprintf(t_name, MAX_STRING_LEN+39, "%s.tmp_%d", f_name, (int)getpid());

    if(!(cf=fopen(t_name, "w"))) return(14932);

    out=fwrite(in_wrf->pos, 1, in_wrf->yet, cf);
    if(out == -1) return(14933);

    if(fclose(cf)) return(14934);

    if(v2_vop_rename(out, t_name, f_name)) return(14935);

    return(0);
}
/* ================================================================ */
