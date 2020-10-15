/*
 *  Copyright (c) 2005-2016 Oleg Vlasenko <vop@unity.net>
 *  All Rights Reserved.
 *
 *  This file is part of the V2-utils which is a remote
 *  agent of the TopolA managment software.
 *
 *  V2-utils is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  V2-utils is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _V2_FUN_H
#define _V2_FUN_H 1

#include "v2_util.h"

#define V2_FUN_NOT_FOUND 1780
#define V2_FUN_NOT_DEFND 1781

typedef struct {
    int (*add)(char*, char*, int (*)(char*));  // Add function (in_list, in_name, in_fun());
    int (*dsc)(char*, ...);                   // Add description of latest added function (in_desc)
    int (*run)(char*,char*,char*);            // Run function (in_list, in_name, in_arg)
    int (*runf)(char*,char*,char*, ...);      // Run function (in_list, in_name, in_format, ...)
    int (*(*get)(char*,char*))(char*);        // Return pointer to function

    int (*prn)(void);                         // Print list of fuctions (can be selected group)

    int (*prncb)(char*, int (*cb)(char*));                         // Print list of fuctions (can be selected group)

    // If in_name == NULL, returns number of functions into "in_list" list
    // If in_name defined - returnn 1 if function exists
    // in_list should be defined
    int (*cnt)(char*,char*);                  // Count functions number (in_lisr, in_name)

    // Returns list of registered functions
    // If in_list == NULL, returns all functions
    // key = name, str = description, dstr = group
    str_lst_t *(*lst)(char*); // List of registered functions

    // Filter rc result, exclude V2_FUN_NOT_FOUND and V2_FUN_NOT_DEFND errors
    int (*rc)(int);

    // External functions
    // Define next function which can print or process errors
    // Return non-zero if you want to stop process list of functions
    int (*err)(char *in_list, char *in_name, int err_no); // External function

    // Tracker/profiler functon - use with caution
    // err_no actual only for pass == 1 (post)
    int (*tr)(int pass, char *in_list, char *in_name, char *in_args, int err_no);

    int debug; // Set debug level. 0, 1 = off, 2 = on

} v2_fun_t;

extern v2_fun_t v2_fun;

/* ===================================================================== */
// adds function "in_fun" with "in_name" name at "in_list" group

int v2_add_fun(char *in_list, char *in_name, int (*in_fun)(char *));

/* ===================================================================== */
// runs function "in_name" from "in_list" group with "in_str" opts

int v2_run_fun(char *in_list, char *in_name, char *in_str);

/* ===================================================================== */
int v2_prn_fun(void);

int v2_prn_funcb(char *in_grp, int (*cb)(char*));

/* ===================================================================== */
// If in_name == NULL, returns number of functions into "in_list" list
// If in_name defined - returnn 1 if function exists
// in_list should be defined
int v2_cnt_fun(char *in_list, char *in_name);

/* ===================================================================== */
// Returns list of registered functions
// If in_list == NULL, returns all functions
// key = name, str = description, dstr = group
str_lst_t *v2_lst_fun(char *in_list);

/* ===================================================================== */
// Define next function which can print or process errors
// Return non-zero if you want to stop process list of functions
//extern int (*v2_err_fun)(char *in_list, char *in_name, int err_no); // External function
// Use: v2_fun.err()


/* ===================================================================== */
#endif // _V2_FUN_H

