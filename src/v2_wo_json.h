/*
 *  Copyright (c) 2014-2015 Oleg Vlasenko <vop@unity.net>
 *  All Rights Reserved.
 *
 *  This file is part of the the TopolA managment software.
 *
 *  TaRemote is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  TaRemote is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TopolA; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307 USA.
 */

// Converts json params to v2_wo_lst.

#ifndef _V2_WO_JSON_H
#define _V2_WO_JSON_H 1

#include "v2_wo.h"
#include "parson.h"

int v2_wo_json(char *in_json);

#endif // _V2_WO_JSON_H
