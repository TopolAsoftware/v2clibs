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

// Parse JOSN Elemet

#ifndef _V2_JPRS_H
#define _V2_JPRS_H 1

#include "v2_json.h"
#include "v2_lstr.h"

typedef struct {

    int is_upd; // If some values are updated

    int no_strict; // if set, str and other types can be null

    int (*name)(json_lst_t *in_el, char *in_name); // =1 - name OK, =0 - not our name

    int (*str)(json_lst_t*,char**,char*);          // Read string from josn by name
    int (*num)(json_lst_t*,char**,char*);          // Read int or long from josn by name to string
    int (*bol)(json_lst_t*,int*,char*);            // Read boolean from josn by name

    int (*num_lng)(json_lst_t*,long*,char*);       // Read long from josn by name
    int (*num_int)(json_lst_t*,int*,char*);        // Read int from josn by name

    int (*arr_str)(json_lst_t*,str_lst_t**,char*); // Read array of strings from josn by name
    int (*arr_obj)(json_lst_t*,void*,int(*fun)(json_lst_t*,void*),char*); // Read array of objects from josn by name
    int (*arr_arr)(json_lst_t*,void*,int(*fun)(json_lst_t*,void*),char*); // Read array of arrays from josn by name
    int (*arr_fun)(json_lst_t*,void*,int(*fun)(json_lst_t*,void*),char*); // Read array of any elements from josn by name

} v2_jprs_t;

extern v2_jprs_t v2_jprs;

#endif // _V2_JPRS_H
