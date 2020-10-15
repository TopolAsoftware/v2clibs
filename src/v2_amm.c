/*
 *  Copyright (c) 2005-2016 Oleg Vlasenko <vop@unity.net>
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

#include "v2_amm.h"

static int amm_round_type=0; // Round type

/* ============================================================ */
// Returns only absolute (positive_ amount
amm_t amm_abs(amm_t in_amm) {
    if(in_amm < 0) return(-in_amm);
    return(in_amm);
}
/* ============================================================ */
amm_t amm_rnd(amm_t in_amm, amm_t in_base, int in_rnd) {
    int base=1;
    int rnd=in_rnd;
    int x=0;

    if(in_base<1) return(in_amm); // Not rounded

    if(in_base>8) in_base=8;

    //in_base = 10^in_base;

    for(x=0; x<in_base; x++) base *= 10;

    if(rnd==-1) rnd=amm_round_type;

    if(rnd==0)          return(in_amm);
    if(in_amm==0)       return(in_amm);
    if(!in_amm%in_base) return(in_amm);

    if(in_amm>0) {
	if(rnd == AMM_ROUND_TYPE_NRM) {
	    in_amm += (base/2);
	}
	if(rnd == AMM_ROUND_TYPE_MAX) {
	    in_amm += (base-1);
	} 
    } else {
	if(rnd == AMM_ROUND_TYPE_NRM) {
	    in_amm -= (base/2);
	}
	if((rnd == AMM_ROUND_TYPE_MAX) || (rnd == AMM_ROUND_TYPE_DIF)) {
	    in_amm -= (base-1);
	} 
    }
    return((in_amm/base)*base);
}
/* ============================================================ */
// max
// 1.200 -> 1.20
// 1.201 -> 1.21
// -1.250 -> -1.25
// -1.251 -> -1.26
amm_t amm_round_max(amm_t in_amm) {

    if(!in_amm) return(0);
    if(!in_amm%10) return(in_amm);

    if(in_amm < 0) return(((in_amm-9)/10)*10);
    return(((in_amm+9)/10)*10);
}
/* ============================================================ */
// min
// 1.200 -> 1.20
// 1.201 -> 1.20
// -1.250 -> -1.25
// -1.251 -> -1.25
amm_t amm_round_min(amm_t in_amm) {

    if(!in_amm) return(0);
    return((in_amm/10)*10);
}

/* ============================================================ */
// min
// 1.200 -> 1.20
// 1.201 -> 1.20
// -1.250 -> -1.25
// -1.251 -> -1.26
amm_t amm_round_dif(amm_t in_amm) {
    if(in_amm>0) {
	in_amm=amm_round_min(in_amm);
    } else if(in_amm<0) {
	in_amm=amm_round_max(in_amm);
    }
    return(in_amm);
}
/* ============================================================ */
// norm
// 1.244 -> 1.24
// 1.245 -> 1.25
amm_t amm_round(amm_t in_amm) {

    if(!in_amm) return(0);
    if(!in_amm%10) return(in_amm);

    if(in_amm < 0) return(((in_amm-5)/10)*10);
    return(((in_amm+5)/10)*10);
}
/* ============================================================ */
amm_t amm_fm_str(char *in_amm) {
    double summ=0;

    if(!in_amm || !in_amm[0]) return(0);

    if(strlen(in_amm) >21)    return(0); // strlen(-1U) == 20

    summ=atof(in_amm);

    if(summ<0) return((amm_t)((summ-0.0005)*1000));
    return((amm_t)((summ+0.0005)*1000));
}
/* ============================================================ */
// Make ronud to buffer
char *amm_str_r(char *strbuf, size_t len, amm_t in_amm) {
    char *neg="";

    if(!strbuf) return("0.00");
    if(!len) len=MAX_STRING_LEN; // Default value

    if(in_amm<0) {
	in_amm = -in_amm;
	neg="-";
    }

    if(in_amm==0) {
        snprintf(strbuf, len, "0.00");
    } else if(in_amm%10) {
        snprintf(strbuf, len, "%s%"PRIAMM".%03"PRIAMM"", neg, in_amm/1000, in_amm%1000);
    } else {
	snprintf(strbuf, len, "%s%"PRIAMM".%02"PRIAMM"", neg, in_amm/1000, (in_amm%1000)/10);
    }

    return(strbuf);
}
/* ============================================================ */
// strbuf at least AMM_LEN length
char *amm_str_a(char *strbuf, amm_t in_amm) {
    return(amm_str_r(strbuf, AMM_LEN, in_amm));
}
/* ============================================================ */
char *amm_str(amm_t amm) {
    char s[AMM_LEN];

    return(v2_let_var(NULL, amm_str_a(s, amm)));
}
/* ============================================================ */
// Replace it to v2_let_var()
char *amm_str_var(char **p_var, amm_t amm) {
    char s[AMM_LEN];

    return(v2_let_var(p_var, amm_str_a(s, amm)));
}
/* ============================================================ */
// Old one
char *amm_to_str(amm_t amm) {
    return(amm_str(amm));
}
/* ============================================================ */
// Output with counting round type
char *amm_str_round(amm_t in_amm) {

    if(amm_round_type == AMM_ROUND_TYPE_NRM) in_amm=amm_round(in_amm);
    if(amm_round_type == AMM_ROUND_TYPE_MAX) in_amm=amm_round_max(in_amm);
    if(amm_round_type == AMM_ROUND_TYPE_MIN) in_amm=amm_round_min(in_amm);
    if(amm_round_type == AMM_ROUND_TYPE_DIF) {
	if(in_amm>0) {
	    in_amm=amm_round_min(in_amm);
	} else {
	    in_amm=amm_round_max(in_amm);
	}
    }

    return(amm_str(in_amm));

}
/* ============================================================ */
// Old one
char *amm_to_str_round(amm_t in_amm) {
    return(amm_str_round(in_amm));
}
/* ============================================================ */
char *amm_ret_round_type(int id_type) {

    if(id_type == -1) id_type=amm_round_type;

    //if(in_type == AMM_ROUND_TYPE_NRM) return("nrm");
    if(id_type == AMM_ROUND_TYPE_MAX) return("max");
    if(id_type == AMM_ROUND_TYPE_MIN) return("min");
    if(id_type == AMM_ROUND_TYPE_DIF) return("dif");
    return("nrm");
}
/* ============================================================ */
int amm_get_round_type(char *in_type) {

    if(!v2_strcmp(in_type, "none")) return(-1);
    if(!v2_strcmp(in_type, "norm")) return(AMM_ROUND_TYPE_NRM);
    if(!v2_strcmp(in_type, "mid"))  return(AMM_ROUND_TYPE_NRM);
    if(!v2_strcmp(in_type, "max"))  return(AMM_ROUND_TYPE_MAX);
    if(!v2_strcmp(in_type, "min"))  return(AMM_ROUND_TYPE_MIN);
    if(!v2_strcmp(in_type, "dif"))  return(AMM_ROUND_TYPE_DIF);

    return(0);
}
/* ============================================================ */
// Might be used as template function
int amm_set_round_type(char *in_type) {
    int type=amm_get_round_type(in_type);

    if(!type) return(0);
    amm_round_type=type;
    if(type<0) amm_round_type=0;
    return(type); // Maybe exit with error?
}
/* ========================================================== */
int v2_getw_amm(amm_t *p_amm, char *line, char stop) {
    char strtmp[MAX_STRING_LEN];
    int len=0;

    if((len=v2_getword(strtmp, line, stop)));

    if(len && p_amm) *p_amm=amm_fm_str(strtmp);

    return(len);
}
/* ========================================================== */
amm_t v2_wo_amm(char *in_key) {
    return(amm_fm_str(v2_wo_key(in_key)));
}
/* ============================================================ */
int v2_cf_amm(amm_t *avar, char *cf_str, char *var_name) {
    char *tmp=NULL;

    if(!avar)                          return(0);

    if(!(tmp=v2_cf(cf_str, var_name)) || !*tmp) return(0); // Not var found
    *avar=A_TO_AMM(tmp);

    return(1); // OK
}
/* ============================================================ */
char *v2_let_amm(char **avar, amm_t in_amm) {
    char stramm[AMM_LEN];
    return(v2_let_var(avar, amm_str_a(stramm, in_amm)));
}
/* ============================================================ */

