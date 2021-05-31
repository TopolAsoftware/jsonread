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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <inttypes.h>
#include <locale.h>
#include <errno.h>

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "v2_iconv.h"
#include "v2_jsmn.h"

char *locale=NULL;

v2_jsmn_t jr_jsmn; // Main parser structure

/* ======================================================== */
/*
int jr2_fm_locale(char *in_str) {
    char *out_str=NULL;

    if(!in_str) return(0);

    if((out_str=v2_iconv("utf-8", locale, in_str))) {
	sprintf(in_str, "%s", out_str);
    }
    return(0);
}*/
/* ======================================================== */
int main(int argc, char *argv[], char *argp[]) {
    //json_lst_t *jsn=NULL;
    char *in_file=NULL;
    int rc=0;

    if(argc == 1) {
	if(isatty(fileno(stdin))) {
	    fprintf(stderr, "Usage:\n\t%s file.json|-\n", argv[0]);
	    return(0);
	} else if(errno != ENOTTY) {
	    fprintf(stderr, "Error: %d %s\n", errno, strerror(errno));
	    return(errno);
	}
	in_file="-";
    } else {
        in_file=argv[1];
    }

    //locale=getenv("LANG");
    //locale=getenv("LC_ALL");
    //if(v2_is_par(locale)) {
    //    if(strncmp(locale, "ru", 2)) locale=NULL;
    //}

    if((rc=v2_jsmn_parse_file(&jr_jsmn, in_file))) {
	fprintf(stderr, "ERROR Returned code = %d\n", rc);
        return(0);
    }

    if(!jr_jsmn.box) {
	printf("[]\n");
	return(0);
    }

    //FOR_LST(jsn, jr_jsmn.json) printf("JS: %s\n", jsn->id);

    jr_jsmn.box->ident=4;
    jr_jsmn.box->no_escape=1;
    //jr_jsmn.box->header=1;
    //jr_jsmn.box->str=&jr2_fm_locale; // Operate by string

    v2_json_locale(jr_jsmn.box, getenv("LC_ALL"), 1); // DeLocalize it

    v2_json_text(jr_jsmn.box);

    return(0);
}
/* ======================================================== */
