/*
 *  Copyright (c) 2016-2021 Oleg Vlasenko <vop@unity.net>
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

#ifndef _V2_ERR_H
#define _V2_ERR_H 1

#include "v2_lstr.h"

extern str_lst_t *v2_err_lst;
extern int (*v2_hook_error)(str_lst_t *err_rec); // Hook for error and debug

extern int v2_debug_time; // Print time before debug message
extern char *v2_debug_pref; // Prefix for debug output

// ERROR functions
int v2_add_err(int err, int level, char *err_msg); // Main base functino

int v2_prn_error(void);                            // Print error, warning and debug
int v2_add_error(char *err_form, ...);             // Add error string to the list
int v2_ret_error(int err, char *err_form, ...);    // Add error with code "err", and return it (code).
int v2_add_debug(int level, char *err_form, ...);  // Add warning (level == 1) and debug (>1) string to the list
int v2_add_warn(char *warn_form, ...);             // Easy form of v2_add_debug(1, ...)
int v2_is_error(int in_err);                       // Check if error exists and get latest error code, if in_err == 0
int v2_del_error(void);                            // Delete all errors into list

#endif // _V2_ERR_H
