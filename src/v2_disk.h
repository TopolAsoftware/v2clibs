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

#ifndef _V2_DISK_H
#define _V2_DISK_H 1

#include "v2_util.h"
#include "inttypes.h"
#include <sys/statvfs.h>

/* ========================================================================== */
typedef struct _v2_disk_t {
    struct _v2_disk_t *next;

    char *id;
    char *path;
    char *fname;

    char *size_str;
    char *used_str;
    char *free_str;

    uint64_t size;
    uint64_t used;
    uint64_t free;

    unsigned int used_pc; // Busy percent
    unsigned int free_pc; // Free percent

    int is_del; // Skip disk

} v2_disk_t;

/* ========================================================================== */
typedef struct v2_dsk_s {

    struct statvfs svfs;

    uint64_t size; // In kBytes - Disk size
    uint64_t used; // In kBytes - Disk busy
    uint64_t free; // In kBytes - Disk avaliable

    char *fname;

    v2_disk_t *list; // List of disks
    v2_disk_t *lcur; // Current pointer to disk

    int (*stat)(char*);                     // (path) Get status of disk by patch
    int (*load)(v2_disk_t**, char*, char*); // (id, path) Get status of disk by patch and register it with id
    int (*del)(v2_disk_t*); // Mark disk as deleted

    // Adding functions
    int (*add)(v2_disk_t**, char*);         // (id) Just register disk into list
    int (*add_size_str)(v2_disk_t**, char*);
    int (*add_free_str)(v2_disk_t**, char*);
    int (*add_used_str)(v2_disk_t**, char*);

    //int (*next)(struct v2_dsk_s*);               // Set next disk to lcur;

    int (*ok)(v2_disk_t*);                  // =1 Disk enitry is OK! not deletedand not empty

    // Returned values for current disk
    //char* (*s_id)(struct v2_dsk_s*);
    //char* (*s_path)(struct v2_dsk_s*);
    //char* (*s_fname)(struct v2_dsk_s*);

    //char* (*s_size_str)(struct v2_dsk_s*);
    //char* (*s_used_str)(struct v2_dsk_s*);
    //char* (*s_free_str)(struct v2_dsk_s*);

    //uint64_t (*u_size)(struct v2_dsk_s*);
    //uint64_t (*u_used)(struct v2_dsk_s*);
    //uint64_t (*u_free)(struct v2_dsk_s*);

    //unsigned int (*u_used_pc)(struct v2_dsk_s*);
    //unsigned int (*u_free_pc)(struct v2_dsk_s*);



} v2_dsk_t;

extern v2_dsk_t v2_dsk;
/* ========================================================================== */
#endif // _V2_DISK_H
