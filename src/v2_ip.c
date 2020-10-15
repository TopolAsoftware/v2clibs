/*
 *  Copyright (c) 2018-2019 Oleg Vlasenko <vop@unity.net>
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

#include "v2_ip.h"
// #include <assert.h>

#include <inttypes.h>
#include <sys/socket.h> // AF_INET freebsd

/* ============================================================ */
// Returns port number for IPv4 and IPv6
int v2_ip_port(struct sockaddr_storage *in_addr) {
    in_port_t ipport=0;

    if(!in_addr) return(0);

    if(in_addr->ss_family == AF_INET) {
	ipport=((struct sockaddr_in*)in_addr)->sin_port;
    } else if(in_addr->ss_family == AF_INET6) {
	ipport=((struct sockaddr_in6*)in_addr)->sin6_port;
    }

    return((int)ntohs(ipport));
}
/* ============================================================ */
// Set variable to ip textual address
char *v2_ip_var(char **pvar, struct sockaddr_storage *in_addr) {
    char strbuf[INET6_ADDRSTRLEN];
    char *out=NULL;

    strbuf[0] = '\0';

    if(in_addr->ss_family == AF_INET) {
	struct sockaddr_in *sa=(struct sockaddr_in*)in_addr;
	inet_ntop(in_addr->ss_family, (const void*)&sa->sin_addr, strbuf, INET6_ADDRSTRLEN);
    } else if(in_addr->ss_family == AF_INET6) {
	struct sockaddr_in6 *sa=(struct sockaddr_in6*)in_addr;
	inet_ntop(in_addr->ss_family, (const void*)&sa->sin6_addr, strbuf, INET6_ADDRSTRLEN);
    }
    if(strbuf[0]) out=strcpy((char*)calloc(strlen(strbuf)+1, 1), strbuf);

    if(pvar) {
	if(*pvar) free(*pvar);
	*pvar=out;
    }
    return(out);
}
/* ============================================================ */
// Returns AF_ as string
char *v2_ip_afstr(int in_af) {
    if(in_af == AF_UNSPEC) return("AF_UNSPEC");
    if(in_af == AF_INET)   return("AF_INET");
    if(in_af == AF_INET6)  return("AF_INET6");
    return("AF_OTHER");
}
/* ============================================================ */
// Get ipv4 or ipv6 from the string, returns(AF_INET, AF_INET6, AF_UNSPEC);
int v2_ip_addr(char *in_str, struct sockaddr_storage *in_ss) {
    struct sockaddr_storage ss_local;
    struct sockaddr_in  *sa;
    struct sockaddr_in6 *sa6;

    if(!in_ss) in_ss=&ss_local;

    sa  = (struct sockaddr_in*)  in_ss;
    sa6 = (struct sockaddr_in6*) in_ss;

    memset(in_ss, 0, sizeof(struct sockaddr_storage));

    if(!in_str || !*in_str) return(AF_UNSPEC); // ss_family == 0

    if(inet_pton(AF_INET,  in_str, &sa->sin_addr)   == 1) return((sa->sin_family   = AF_INET)); // IPv4
    if(inet_pton(AF_INET6, in_str, &sa6->sin6_addr) == 1) return((sa6->sin6_family = AF_INET6)); // IPv6

    return(AF_UNSPEC);
}
/* ============================================================ */
 // Compare two uint31_t values
int v2_ip_cmp4(struct in_addr *in1, struct in_addr *in2) {

    if(in1 && in2) {
	uint32_t hl1=ntohl(in1->s_addr);
	uint32_t hl2=ntohl(in2->s_addr);

	if(hl1 > hl2) return(1);
	if(hl1 < hl2) return(-1);
    }

    return(0);
}
/* ============================================================ */
// Compare two uint8_t[16] arrays
int v2_ip_cmp6(struct in6_addr *in1, struct in6_addr *in2) {
    int i;
    if(in1 && in2) {
	for(i = 0; i < 16; ++i) {
	    if (in1->s6_addr[i] < in2->s6_addr[i])      return(-1);
	    else if (in1->s6_addr[i] > in2->s6_addr[i]) return(1);
	}
    }
    return(0);
}
/* ============================================================ */
int v2_ip_cmp_ss(struct sockaddr_storage *ss1, struct sockaddr_storage *ss2) {

    if(!ss1 || !ss2) return(0); // Uncomparable NULL strings

    if(ss1->ss_family == AF_UNSPEC) {
	if(ss2->ss_family != AF_UNSPEC) return(1); // String always greater than any IP
	//return(strcoll(ip1, ip2));      // Just compare two strings
	return(0);
    }

    if(ss1->ss_family == AF_INET6) {
	struct sockaddr_in6 *in1=(struct sockaddr_in6*)ss1;
	struct sockaddr_in6 *in2=(struct sockaddr_in6*)ss2;

	if(ss2->ss_family == AF_UNSPEC) return(-1); // AF_INET6 always lower than AF_UNSPEC
	if(ss2->ss_family == AF_INET)   return(1);  // AF_INET6 always greater than AF_INET
	return(v2_ip_cmp6(&in1->sin6_addr, &in2->sin6_addr));
    }

    if(ss1->ss_family == AF_INET) {
	struct sockaddr_in *in1=(struct sockaddr_in*)ss1;
	struct sockaddr_in *in2=(struct sockaddr_in*)ss2;

	if(ss2->ss_family != AF_INET)  return(-1);  // AF_INET always lower than AF_INET6 or AF_UNSPEC
	return(v2_ip_cmp4(&in1->sin_addr, &in2->sin_addr));
    }

    // Looks like ip1 has some other types than AF_UNSPEC, AF_INET nor AF_INET6
    return(0);
}
/* ============================================================ */
int v2_ip_cmp(char *ip1, char *ip2) {
    struct sockaddr_storage ss1;
    struct sockaddr_storage ss2;

    if(!ip1 || !ip2) return(0); // Uncomparable NULL strings

    v2_ip_addr(ip1, &ss1);
    v2_ip_addr(ip2, &ss2);

    if(ss1.ss_family == ss2.ss_family && ss1.ss_family == AF_UNSPEC) return(strcoll(ip1, ip2));

    return(v2_ip_cmp_ss(&ss1, &ss2));
}
/* ============================================================ */
