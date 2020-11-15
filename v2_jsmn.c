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

// ERROR_CODE 173XX : 17300 - 17349

#include "v2_jsmn.h"
#include "v2_iconv.h"
#include "utf8.h"

v2_jsmn_t v2_jsmn; // Read Write buffer

/* ================== Proto ================================================ */
int vj_make_array();
int vj_make_object();

/* ========================================================================= */
int v2_jsmn_init(v2_jsmn_t *in_jsmn) {
    //int rc=0;

    if(!in_jsmn) return(17300);

    jsmn_init(&in_jsmn->parser);

    if(in_jsmn->tokens) {
        free(in_jsmn->tokens);
        in_jsmn->tokens=NULL;
    }

    in_jsmn->tmax = 0;
    in_jsmn->tcnt = 0;
    in_jsmn->tcur = 0;

    in_jsmn->json=NULL;

    return(0);
}
/* ========================================================================= */
char *vj_get_string(v2_jsmn_t *in_jsmn) {
    static char strtmp[MAX_STRING_LEN];
    static char strtm1[MAX_STRING_LEN];
    char *out=NULL;
    int x=0;
    int y=0;

    for(y=in_jsmn->tokens[in_jsmn->tcur].start; y<in_jsmn->tokens[in_jsmn->tcur].end && x<MAX_STRING_LEN; y++, x++) {
	strtmp[x]=in_jsmn->b->pos[y];
    }
    strtmp[x]='\0';

    u8_unescape(strtm1, MAX_STRING_LEN, strtmp);

    if(in_jsmn->locale) {
	if((out=v2_iconv("UTF-8", in_jsmn->locale, strtm1))) {
	    snprintf(strtm1, MAX_STRING_LEN, "%s", out);
	    v2_freestr(&out);
	}
    }

    return(strtm1);
}
/* ========================================================================= */
int vj_make_value(v2_jsmn_t *in_jsmn, char *in_name) {
    char name[MAX_STRING_LEN];
    char *in_val=NULL;
    static int none=0;

    if(!in_jsmn) return(17202);

    if(!in_name || !in_name[0]) {
	sprintf(name, "_array_%04d", ++none);
    } else {
	sprintf(name, "%s", in_name);
    }

    if(in_jsmn->tokens[in_jsmn->tcur].type==JSMN_OBJECT) {
	if(in_jsmn->tcur) v2_json_add_node(in_jsmn->box, name, JS_OBJECT);
	vj_make_object(in_jsmn);
	if(in_jsmn->tcur) v2_json_end(in_jsmn->box, JS_OBJECT);
    } else if(in_jsmn->tokens[in_jsmn->tcur].type==JSMN_ARRAY) {
	v2_json_add_node(in_jsmn->box, name, JS_ARRAY);
	vj_make_array(in_jsmn);
	v2_json_end(in_jsmn->box, JS_ARRAY);
    } else if(in_jsmn->tokens[in_jsmn->tcur].type==JSMN_STRING) {
	v2_json_add_node(in_jsmn->box, name, JS_STRING);
	v2_let_var(&in_jsmn->box->tek->str, vj_get_string(in_jsmn));
    } else {
        in_val=vj_get_string(in_jsmn);
	if(!in_val) {
	    v2_json_add_node(in_jsmn->box, name, JS_NULL);
	    v2_let_var(&in_jsmn->box->tek->str, "null");
	} else if(in_val[0] == 'n') {
	    v2_json_add_node(in_jsmn->box, name, JS_NULL);
	    v2_let_var(&in_jsmn->box->tek->str, "null");
	} else if(in_val[0] == 'f') {
	    v2_json_add_node(in_jsmn->box, name, JS_BOOLEAN);
	    v2_let_var(&in_jsmn->box->tek->str, "false");
	} else if(in_val[0] == 't') {
	    v2_json_add_node(in_jsmn->box, name, JS_BOOLEAN);
	    v2_let_var(&in_jsmn->box->tek->str, "true");
            in_jsmn->box->tek->num=1; // True
	} else if(strchr(in_val, '.')) {
	    v2_json_add_node(in_jsmn->box, name, JS_DOUBLE);
	    in_jsmn->box->tek->dnum=atof(in_val);
            in_jsmn->box->tek->str=v2_string("%g", in_jsmn->box->tek->dnum);
	} else {
	    v2_json_add_node(in_jsmn->box, name, JS_LONG);
            in_jsmn->box->tek->lnum=atof(in_val);
            in_jsmn->box->tek->str=v2_string("%lld", in_jsmn->box->tek->lnum);
	}
    }

    in_jsmn->json=in_jsmn->box->lst; // Copy pointer to main value

    return(0);
}
/* ========================================================================= */
int vj_make_array(v2_jsmn_t *in_jsmn) {
    int nums=in_jsmn->tokens[in_jsmn->tcur].size;
    int rc=0;
    int x=0;

    for(x=0; x<nums && !rc; x++) {
	in_jsmn->tcur++;
	rc=vj_make_value(in_jsmn, NULL);
    }
    return(rc);
}
/* ========================================================================= */
int vj_make_object(v2_jsmn_t *in_jsmn) {
    char str_name[MAX_STRING_LEN];
    int nums=in_jsmn->tokens[in_jsmn->tcur].size;
    int x=0;
    int is_name=0;
    int rc=0;

    for(x=0; x<nums && !rc; x++) {
	in_jsmn->tcur++;

	// 1. Name
	if((is_name=1-is_name)) {
	    if(in_jsmn->tokens[in_jsmn->tcur].type != JSMN_STRING) return(17340); // This is not name
	    sprintf(str_name, "%s", vj_get_string(in_jsmn));
	    continue;
	}
	// Value
	rc=vj_make_value(in_jsmn, str_name);
    }

    return(rc);
}
/* ========================================================================= */
// Don't use it separately
static int v2_jsmn_parse_any(v2_jsmn_t *in_jsmn) {
    int t_max=0;
    int rc=0;
    int x=0;

    if(!in_jsmn) return(17300);
    if((rc=v2_wrbuf_ok(in_jsmn->b))) return(rc);

    if(!in_jsmn->b->pos[0]) return(52);
    if(!in_jsmn->b->pos[1]) return(53);

    // -vvv- not needed -vvv-
    if(in_jsmn->b->pos[0] != '{') {
	if(in_jsmn->b->pos[0] != '[') {
	    v2_add_debug(1, "This is not json: %s", in_jsmn->b->pos);
	    return(V2_NO_JSMN); // == 17312 // Looks like not json text
	}
        if(in_jsmn->b->pos[1] == ']') return(0); // Empty array
    } else {
        if(in_jsmn->b->pos[1] == '}') return(0); // Empty array
    }

    if(in_jsmn->b->pos[in_jsmn->b->yet] != '\0')  return(17313); // Non zero end of buffer - required
    // -^^^- not needed -^^^-

    // Count toekns value
    in_jsmn->tmax=0;
    for(x=0; x<in_jsmn->b->yet; x++) {
	if(in_jsmn->b->pos[x] == ':') in_jsmn->tmax++; // Tokens
	if(in_jsmn->b->pos[x] == ',') in_jsmn->tmax++; // Array members
    }

    if(in_jsmn->tmax == 0)      in_jsmn->tmax=t_max;
    if(in_jsmn->tmax == 0)      return(17314); // Not fund any json separator... Maybe wrong for 1-element array
    if(in_jsmn->tmax > 5000000) return(17315); // Look so hight size... What to do?

    in_jsmn->tmax*=2; // Make 2 times more, each ':' gives maximal 2 tokens
    in_jsmn->tmax+=1; // One extra token - ROOT element

    if((rc=v2_json_new(&in_jsmn->box))) return(rc);

    if(!(in_jsmn->tokens=(jsmntok_t *)calloc(in_jsmn->tmax+1, sizeof(jsmntok_t)))) return(17316);

    in_jsmn->tcnt=jsmn_parse(&in_jsmn->parser, in_jsmn->b->pos, in_jsmn->b->yet, in_jsmn->tokens, in_jsmn->tmax);

    if(in_jsmn->tcnt==JSMN_ERROR_NOMEM) rc=17320; // No mem
    if(in_jsmn->tcnt==JSMN_ERROR_INVAL) rc=17321; // Wrong values - invalid chars into strings
    if(in_jsmn->tcnt==JSMN_ERROR_PART)  rc=17322; // Unexpected and of the json

    in_jsmn->tcur=0;

    if(rc) return(rc);

    return(vj_make_value(in_jsmn, NULL));
    //return(0);
}
/* ========================================================================= */
int v2_jsmn_parse_file(v2_jsmn_t *in_jsmn, char *in_file) {
    //int x=0;
    int rc=0;
    int rc1=0;

    if((rc=v2_jsmn_init(in_jsmn))) return(rc);
    if(!in_jsmn->b) v2_wrbuf_new(&in_jsmn->b); // If needs - create buffer
    if((rc=v2_wrbuf_file_read(in_jsmn->b, in_file))) return(rc);
    rc=v2_jsmn_parse_any(in_jsmn);
    if((rc1=v2_wrbuf_reset(in_jsmn->b))) return(rc1); // Clear previouse value in any case
    
    return(rc);
}
/* ========================================================================= */
// Just add data to in_jsmn->b
int v2_jsmn_parse(v2_jsmn_t *in_jsmn) {
    int rc=0;

    if((rc=v2_jsmn_init(in_jsmn)))      return(rc);
    if((rc=v2_jsmn_parse_any(in_jsmn))) return(rc);

    return(0);
}
/* ========================================================================= */
// Add to debug diagnostics info
int v2_jsmn_warn(v2_jsmn_t *in_jsmn) {

    if(!in_jsmn) {
	v2_add_warn("V2_JSMN_DEBUG: jsmn == NULL");
	return(0);
    }

    v2_add_warn("V2_JSMN_DEBUG: wrbuf   = %s", in_jsmn->b?"Allocated":"NULL");
    v2_add_warn("V2_JSMN_DEBUG: box:    = %s", in_jsmn->box?"Allocated":"NULL");
    v2_add_warn("V2_JSMN_DEBUG: json:   = %s", in_jsmn->json?"Allocated":"NULL");
    v2_add_warn("V2_JSMN_DEBUG: tokens: = %s", in_jsmn->tokens?"Allocated":"NULL");
    v2_add_warn("V2_JSMN_DEBUG: locale: = %s", v2_st(in_jsmn->locale, "NULL"));

    return(0);
}
/* ========================================================================= */
