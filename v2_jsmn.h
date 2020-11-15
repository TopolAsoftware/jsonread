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

#ifndef _V2_JSMN_H
#define _V2_JSMN_H 1

// Not JSON into buffer
#define V2_NO_JSMN 17312

#include <stdlib.h>

#include "v2_json.h"
#include "jsmn.h"


typedef struct {

    wrbuf_t  *b; // Receive buffer

    json_box_t *box;
    json_lst_t *json; // Json list from jbox above

    jsmn_parser parser; // Parser
    jsmntok_t *tokens;  // Tokens array

    char *locale; // Local locate to delocale it

    int tmax; // Maximal allocated tokens
    int tcnt; // Tokens counter
    int tcur; // Current reading token

} v2_jsmn_t;


extern v2_jsmn_t v2_jsmn; // Read Write buffer

// ---------------------------------------------------------------------------------
int v2_jsmn_init(v2_jsmn_t *in_jsmn);

int v2_jsmn_parse(v2_jsmn_t *in_jsmn); // Parse buffer
int v2_jsmn_parse_file(v2_jsmn_t *in_jsmn, char *in_file); // Parse from file
// ---------------------------------------------------------------------------------

int v2_jsmn_warn(v2_jsmn_t *in_jsmn); // Add to debug status of structure

#endif // _V2_JSMN_H
