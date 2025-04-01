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

#ifndef _V2_WO_H
#define _V2_WO_H 1

#include "v2_lstr.h"

extern str_lst_t *v2_wo_lst;
extern str_lst_t *v2_wo_tek;
// extern str_lst_t *v2_wo_lst_pure;

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



#endif // _V2_WO_H
