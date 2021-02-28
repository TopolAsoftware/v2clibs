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

#ifndef _V2_WRBUF_H
#define _V2_WRBUF_H 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h> // va_list

// #include "v2_util.h"

#define V2_WRBUF_BLOCK 65536

// Errors
#define V2_WRBUF_EMPTY_POST 14872

typedef struct {

    char *buf;   // Buffer itself
    char *str;   // String current
    char *pos;   // Pointer to the next value (current position)

    size_t siz; // Size of allocated buffer
    size_t cnt; // Number of written (stored) bytes into buff
    size_t yet; // Number of left "unread" bytes

    size_t sbl; // Size of block for allocation, most time == V2_WRBUF_BLOCK

} wrbuf_t;

// Check if buffer ok and has data - if OK == 0
int v2_wrbuf_ok(wrbuf_t *in_wrf);

// Set init values for the buffer and reset buffer
int v2_wrbuf_new(wrbuf_t **p_wrf);
int v2_wrbuf_init(wrbuf_t *in_wrf);
int v2_wrbuf_reset(wrbuf_t *in_wrf);
int v2_wrbuf_free(wrbuf_t **in_wrf);
int v2_wrbuf_seek(wrbuf_t *in_wrf, size_t position);

// Service function - get string from the buffer
int v2_wrbuf_nxtstr(wrbuf_t *in_wrf);

// Base buffer write function
size_t v2_wrbuf_write(wrbuf_t *in_wrf, char *in_str, size_t in_size, size_t in_num);

// CURL and other based function: usedata - pointer to existed wrbuf_t: data curl_easy_setopt(curl, CURLOPT_WRITEDATA, wrbuf);
size_t v2_wrbuf_curl(char *in_str, size_t in_size, size_t in_num, void *userdata);

// Print string to to buffer
int v2_wrbuf_printf(wrbuf_t *in_wrf, char *format, ...);

// Read file to wrbuf
int v2_wrbuf_file_read(wrbuf_t *in_wrf, char *file, ...);

// Get data block from wrbuf to *in_wrf->str + allocation size = in_size
size_t v2_wrbuf_data(wrbuf_t *in_wrf, size_t in_size);

// Read POST data for web
int v2_wrbuf_read_post(wrbuf_t *in_wrf);

// Save wrbuf to file
int v2_wrbuf_save(wrbuf_t *in_wrf, char *to_file, ...);

// Compare wrbuf data and file
int v2_wrbuf_comp(wrbuf_t *in_wrf, char *to_file, ...);

#endif // _V2_WRBUF_H
