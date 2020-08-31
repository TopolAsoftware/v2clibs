/*
 *  Copyright (c) 2005-2015 Oleg Vlasenko <vop@unity.net>
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

#ifndef _V2_AMM_H
#define _V2_AMM_H 1

#include <inttypes.h>
#include "v2_util.h"

#ifndef PRIAMM
#define PRIAMM PRId64
#define AMM_FMT "%"PRIAMM
#define A_TO_AMM(m) (strtoll((m), (char **)NULL, 10))
typedef int64_t amm_t;
#endif

#define AMM_LEN 36

#define AMM_ROUND_TYPE_NRM 1
#define AMM_ROUND_TYPE_MAX 2
#define AMM_ROUND_TYPE_MIN 3
#define AMM_ROUND_TYPE_DIF 4

amm_t amm_abs(amm_t in_amm); // Returns absolute (positive) value

// Universal rounder
// in_rnd - result of amm_get_route_type()
// in_base - say 1,2,3 or so, which means round for 10,100,1000 or so
amm_t amm_rnd(amm_t in_amm, amm_t in_base, int in_rnd);

amm_t amm_round_max(amm_t in_amm); // Round 1.210 -> 1.21, 1.211 -> 1.22, -1.211 -> -1.22
amm_t amm_round_min(amm_t in_amm); // Round 1.210 -> 1.21, 1.211 -> 1.21, -1.211 -> -1.21
amm_t amm_round_dif(amm_t in_amm); // Round 1.210 -> 1.21, 1.211 -> 1.21, -1.211 -> -1.22
amm_t amm_round(amm_t in_amm);     // Round 1.211 -> 1.21, 1.219 -> 1.22, -1.219 -> -1.22

amm_t amm_fm_str(char *in_amm); // Read amm 12540 from "12.54"

char *amm_str_r(char *strbuf, size_t len, amm_t in_amm); // To buffer str
char *amm_str_a(char *strbuf, amm_t in_amm); // The same as abouve, but strbuf at least AMM_LEN length
char *amm_str_var(char **p_var, amm_t amm); // Allocate var, removing previouse one

char *amm_str(amm_t amm);    // Make string "12.54" from 12540, allocates output string
char *amm_to_str(amm_t amm); // The same as above, but old one

char *amm_str_round(amm_t in_amm);     // The same as above, but with round - betteruser amm_str(amm_rnd());
char *amm_to_str_round(amm_t in_amm);  // The same as above, old one

char *amm_ret_round_type(int in_type); // Returns test value of the type
int amm_get_round_type(char *in_type); // Returns round id type
int amm_set_round_type(char *in_type); // Set round type, "norm" or "max" - all other equial to "none"

int v2_getw_amm(amm_t *p_amm, char *line, char stop); // v2_util type function like v2_getword()

amm_t v2_wo_amm(char *in_key); // Return amm from QUERY_STRING param
int v2_cf_amm(amm_t *avar, char *cf_str, char *var_name); // Use it at config readers
char *v2_let_amm(char **avar, amm_t in_amm); // Allocate result

#endif // _V2_AMM_H
