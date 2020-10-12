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

#ifndef _V2_IP_H
#define _V2_IP_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

int   v2_ip_port(struct sockaddr_storage *in_addr);             // Returns port number
char *v2_ip_var(char **pvar, struct sockaddr_storage *in_addr); // Returns allocated string with textual ip address
char *v2_ip_afstr(int in_af);
int   v2_ip_addr(char *in_str, struct sockaddr_storage *in_ss); // Parse IP addres and returns Address Family

int   v2_ip_cmp_ss(struct sockaddr_storage *ss1, struct sockaddr_storage *ss2); // Compare structures
int   v2_ip_cmp(char *ip1, char *ip2); // Compare two ip address v4 and v6 include

#endif // _V2_IP_H
