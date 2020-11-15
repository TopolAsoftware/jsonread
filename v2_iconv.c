/*
 *  Copyright (c) 2005-2020 Oleg Vlasenko <vop@unity.net>
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
#include <iconv.h>
#include <stdio.h>
#include <string.h>
// #include <errno.h>

/* ========================================================= */
char *v2_iconv(char *fm_locale, char *to_locale, char *in_str) {
    iconv_t i_conv;
    char s[50];
    char *fm_code=NULL;
    char *to_code=NULL;
    char *o_buf=NULL;
#ifdef __linux__
    char *i_buf=NULL;
#else
    const char *i_buf=NULL;
#endif
    char *t_buf=NULL; // Transfer buffer
    size_t i_len=0;
    size_t o_len=0; // Size of left output symbols
    size_t t_len=0; // Transfer size

    if(!in_str    || !*in_str)    return((char*)calloc(1, 1)); // == allocaled empty string ""
    if(!fm_locale || !*fm_locale) return(strcpy((char *)malloc(strlen(in_str)+1), in_str));
    if(!to_locale || !*to_locale) return(strcpy((char *)malloc(strlen(in_str)+1), in_str));

    if((fm_code=strrchr(fm_locale, '.'))) {
        fm_code++;
    } else {
	fm_code=fm_locale;
    }

    if((to_code=strrchr(to_locale, '.'))) {
        to_code++;
    } else {
	to_code=to_locale;
    }

    i_buf=in_str;
    i_len=strlen(in_str);

    o_len=i_len*2+2;
    o_buf=(char*)malloc(o_len+1);

    t_buf=o_buf;
    t_len=o_len;

    snprintf(s, 50, "%s//IGNORE", to_code);
    if((i_conv=iconv_open(s, fm_code)) == (iconv_t)-1) {
	return(strncpy(o_buf, in_str, o_len));
    }

    if(iconv(i_conv, &i_buf, &i_len, &t_buf, &t_len) == (size_t)-1) {
	return(strncpy(o_buf, in_str, o_len));
    }

    iconv_close(i_conv);

    *t_buf='\0'; // End of string

    return(o_buf);
}
/* ========================================================= */
