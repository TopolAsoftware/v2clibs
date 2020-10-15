/*
 *  Copyright (c) 2017-2018 Oleg Vlasenko <vop@unity.net>
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

// ERROR_CODE 129XX

#include "v2_disk.h"
#include <limits.h>
#include <errno.h>

// Just magik number
#define V2_DISK_NO_ID 7712981

/* =================================================================================== */
static int lo_disk_find(v2_disk_t *in_disk, char *in_id) {
    v2_disk_t *ltmp=NULL;
    char *id=in_id;

    if(!in_id || !*in_id) id="main";

    //if(v2_dsk.lcur && !v2_strcmp(v2_dsk.lcur->id, id)) return(0); // Found

    v2_dsk.lcur=NULL;
 
    FOR_LST_IF(ltmp, v2_dsk.lcur, in_disk) {
	if(!v2_strcmp(ltmp->id, id)) v2_dsk.lcur=ltmp;
    }

    if(v2_dsk.lcur) return(0);
    return(V2_DISK_NO_ID); // Not found
}
/* =================================================================================== */
// Returns 1, if disk Ok, not delted and not empty
static int v2_disk_ok(v2_disk_t *in_disk) {

    if(!in_disk)        return(0);
    if(in_disk->is_del) return(0);

    if(!in_disk->id || !*in_disk->id) return(0);

    return(1); // OK
}
/* =================================================================================== */
/* =================================================================================== */
static int v2_disk_del(v2_disk_t *in_disk) {

    if(!in_disk) return(12997);

    in_disk->size=0;
    in_disk->used=0;
    in_disk->free=0;

    in_disk->used_pc=0;
    in_disk->free_pc=0;

    in_disk->is_del=1;

    return(0);
}
/* =================================================================================== */
static int v2_disk_stat(char *in_path) {
    char pthstr[PATH_MAX];

    v2_let_var(&v2_dsk.fname, realpath(v2_st(in_path, "./"), pthstr));

    v2_dsk.size = 0;
    v2_dsk.free = 0;

    memset(&v2_dsk.svfs, 0, sizeof(v2_dsk.svfs));

    if (statvfs(v2_dsk.fname, &v2_dsk.svfs) != 0) {
	return(v2_ret_error(12901, "v2_disk.stat: %s [ %s ]", v2_dsk.fname, strerror(errno)));
    }

    v2_dsk.size = v2_dsk.svfs.f_frsize/1024 * v2_dsk.svfs.f_blocks;
    v2_dsk.free = v2_dsk.svfs.f_frsize/1024 * v2_dsk.svfs.f_bavail;
    v2_dsk.used = v2_dsk.size - v2_dsk.svfs.f_frsize/1024 * v2_dsk.svfs.f_bfree;

    return(0);
}
/* =================================================================================== */
static int lo_disk_set_pc(v2_disk_t *in_disk) {

    if (in_disk) {
	in_disk->used_pc = ((in_disk->size - in_disk->free) * 1000 / in_disk->size + 5) / 10;
	in_disk->free_pc = 100 - in_disk->used_pc;
    }

    return(0);
}
/* =================================================================================== */
// Make disk into list and make it's stat
static int v2_disk_load(v2_disk_t **h_disk, char *in_id, char *in_path) {
    v2_disk_t **p_disk=h_disk?h_disk:&v2_dsk.list; // Head of the list
    int rc=0;

    if ((rc=v2_dsk.add(p_disk, in_id))) return(rc);

    v2_let_var(&v2_dsk.lcur->path, v2_st(in_path, "./"));

    if ((rc=v2_dsk.stat(v2_dsk.lcur->path))) return(rc);

    v2_let_var(&v2_dsk.lcur->fname, v2_dsk.fname);

    v2_dsk.lcur->size = v2_dsk.size;
    v2_dsk.lcur->used = v2_dsk.used;
    v2_dsk.lcur->free = v2_dsk.free;

    v2_let_varf(&v2_dsk.lcur->size_str, "%"PRIu64" kB", v2_dsk.lcur->size);
    v2_let_varf(&v2_dsk.lcur->used_str, "%"PRIu64" kB", v2_dsk.lcur->used);
    v2_let_varf(&v2_dsk.lcur->free_str, "%"PRIu64" kB", v2_dsk.lcur->free);

    lo_disk_set_pc(v2_dsk.lcur);

    return(0);
}
/* =================================================================================== */
static int v2_disk_add(v2_disk_t **h_disk, char *in_id) {
    v2_disk_t **p_disk=h_disk?h_disk:&v2_dsk.list;
    char *id=in_id;
    int rc=0;

    if (!in_id || !*in_id) id="main";

    if (0 == (rc=lo_disk_find(*p_disk, id))) { // Found
	//if((*p_diks)->is_del) v2_dsk.clean();
	v2_dsk.lcur->is_del=0;
	return(0);
    }

    if (rc!=V2_DISK_NO_ID) return(rc); // Not id found

    if (!(v2_dsk.lcur=(v2_disk_t*)calloc(sizeof(v2_disk_t), 1))) return(12999);
    v2_let_var(&v2_dsk.lcur->id, id);

    if (!*p_disk) {
	*p_disk = v2_dsk.lcur;
    } else {
	v2_disk_t *ltmp;
	FOR_LST_NEXT(ltmp, *p_disk);
	ltmp->next=v2_dsk.lcur;
    }

    return(0);
}
/* =================================================================================== */
static int v2_disk_add_size_str(v2_disk_t **h_disk, char *in_size) {
    v2_disk_t **p_disk=h_disk?h_disk:&v2_dsk.list;

    v2_let_var(&(*p_disk)->size_str, in_size);

    (*p_disk)->size=strtoll(v2_nn(in_size), (char **)NULL, 10);

    if ((*p_disk)->size && (*p_disk)->free) lo_disk_set_pc(*p_disk);

    return(0);
}
/* =================================================================================== */
static int v2_disk_add_free_str(v2_disk_t **h_disk, char *in_free) {
    v2_disk_t **p_disk=h_disk?h_disk:&v2_dsk.list;

    v2_let_var(&(*p_disk)->free_str, in_free);

    (*p_disk)->free=strtoll(v2_nn(in_free), (char **)NULL, 10);

    if ((*p_disk)->size && (*p_disk)->free) lo_disk_set_pc(*p_disk);

    return(0);
}
/* =================================================================================== */
static int v2_disk_add_used_str(v2_disk_t **h_disk, char *in_used) {
    v2_disk_t **p_disk=h_disk?h_disk:&v2_dsk.list;

    v2_let_var(&(*p_disk)->used_str, in_used);

    (*p_disk)->used=strtoll(v2_nn(in_used), (char **)NULL, 10);

    return(0);
}
/* =================================================================================== */
/* =================================================================================== */
v2_dsk_t v2_dsk = {
    .stat = &v2_disk_stat,
    .load = &v2_disk_load,

    .del  = &v2_disk_del,

    .add          = &v2_disk_add,
    .add_size_str = &v2_disk_add_size_str,
    .add_free_str = &v2_disk_add_free_str,
    .add_used_str = &v2_disk_add_used_str,

    .ok   = &v2_disk_ok,

};
/* =================================================================================== */
