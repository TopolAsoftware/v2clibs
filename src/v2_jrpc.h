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

#ifndef _V2_JSONRPC_H
#define _V2_JSONRPC_H 1

#include "u_cgi.h"
#include "v2_json.h"
#include "v2_iconv.h"
// #include "parson.h"

#define V2_JRPC_OK 156342
#define V2_JRPC_NOT_FOUND 156346
#define V2_JRPC_ERROR     156348

// extern int (*v2_jsonrpc_parse_method)(char*, JSON_Object*); // Main method + params parser

// int v2_jsonrpc_error(char *in_str);
// int v2_jsonrpc_parse(void);

// int v2_jrpc_beg(void);
// int v2_jrpc_end(void);

typedef struct v2_jrpc_s {

    int    is_sent; // Is answer already sent?

    int    id;
    char   *id_str;
    char   *method; // Request methos
    char   *jsonrpc; // Vrsion "2.0"

    json_lst_t *json_par; // Json out object

    int    (*parse)(json_box_t*); // Parse info from box
    int    (*error)(char*, ...);       // Make error message

    int    (*beg)(void); // Start RPC anser
    int    (*end)(void); // Finish RPC anser

    int    (*to_method)(struct v2_jrpc_s*); // Call external method

} v2_jrpc_t;

extern v2_jrpc_t v2_jrpc;

#endif // _V2_JSONRPC_H
