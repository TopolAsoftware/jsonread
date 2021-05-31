/*
 *  Copyright (c) 2016-2020 Oleg Vlasenko <vop@unity.net>
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

// Family LSTR functions

#define _GNU_SOURCE

#include "v2_lstr.h"

#include <errno.h>
#include <assert.h>
#include <dirent.h>

// ERROR_CODE 175XX

// Need for internal intermediacalls
// Read type
typedef struct {
    str_lst_t **plstr;
    int (*fun)(char*, str_lst_t**);
} _vstr_rdt_t;

// Save type
typedef struct {
    str_lst_t *inlstr;
    int (*fun)(FILE*, str_lst_t*);
} _vstr_sdt_t;


/* ============================================================= */
// Add in_lstr to the head of p_lstr list
str_lst_t *v2_lstr_to_head(str_lst_t **p_lstr, str_lst_t *in_lstr) {

    assert(p_lstr);
    assert(in_lstr);

    in_lstr->next = *p_lstr;
    *p_lstr=in_lstr;

    return(*p_lstr);
}
/* ============================================================= */
// Make new lstr element with unallocated key
str_lst_t *v2_lstr_unew(char *una_key) {
    str_lst_t *str_tmp=NULL;

    str_tmp=v2_lstr.anew(una_key);
    str_tmp->is_key_unalloc = 1;

    return(str_tmp);
}
/* ============================================================= */
// Make new lstr element with already allocated key
str_lst_t *v2_lstr_anew(char *una_key) {
    str_lst_t *str_tmp=(str_lst_t *)calloc(sizeof(str_lst_t), 1);

    assert(una_key); // Not create
    assert(str_tmp);

    str_tmp->key            = una_key;

    return(str_tmp);
}
/* ============================================================= */
// Make new lstr and allocate key
str_lst_t *v2_lstr_new(char *in_frmt, ...) {
    va_list vl;
    char *tempa=NULL;
    int rc=0;

    if(!in_frmt || !*in_frmt) return(NULL);

    va_start(vl, in_frmt);
    rc=vasprintf(&tempa, in_frmt, vl);
    va_end(vl);

    if(rc==-1) return(0);

    return(v2_lstr.anew(tempa));
}
/* ============================================================= */
str_lst_t *v2_lstr_free_str(str_lst_t *in_lstr) {
    if(in_lstr) { // Free it, if exists
	if(in_lstr->is_str_unalloc) { // Unallocated value
	    in_lstr->is_str_unalloc=0;
	    in_lstr->str=NULL;
	} else {
	    v2_freestr(&in_lstr->str); // Clean previouse value
	}
    }
    return(in_lstr);
}
/* ============================================================= */
// Add unallocated str
str_lst_t *v2_lstr_ustr(str_lst_t *in_lstr, char *una_str) {

    if(v2_lstr_free_str(in_lstr)) {
	in_lstr->str=una_str;
	in_lstr->is_str_unalloc=1;
    }

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_astr(str_lst_t *in_lstr, char *in_str) {

    if(!v2_lstr_free_str(in_lstr)) return(NULL);

    if(in_str && *in_str) v2_let_var(&in_lstr->str, in_str);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_astrf(str_lst_t *in_lstr, char *format, ...) {
    va_list vl;
    int rc=0;

    if(!v2_lstr_free_str(in_lstr)) return(NULL);

    if(!format || !format[0]) return(in_lstr);

    va_start(vl, format);
    rc=vasprintf(&in_lstr->str, format, vl);
    va_end(vl);
    if(rc==-1) return(NULL);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_free_dstr(str_lst_t *in_lstr) {
    if(in_lstr) { // Free it, if exists
	if(in_lstr->is_dat_unalloc) { // Unallocated value
	    in_lstr->is_dat_unalloc=0;
	    in_lstr->dstr=NULL;
	} else {
	    v2_freestr(&in_lstr->dstr); // Clean previouse value
	}
    }
    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_dstr(str_lst_t *in_lstr, char *in_str) {

    if(!v2_lstr_free_dstr(in_lstr)) return(NULL);

    //if(in_str && *in_str) // Not needed
    v2_let_var(&in_lstr->dstr, in_str);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_dstrf(str_lst_t *in_lstr, char *format, ...) {
    va_list vl;
    int rc=0;

    if(!v2_lstr_free_dstr(in_lstr)) return(NULL);

    if(!format || !format[0]) return(in_lstr);

    va_start(vl, format);
    rc=vasprintf(&in_lstr->dstr, format, vl);
    va_end(vl);

    if(rc==-1) return(NULL);

    return(in_lstr);
}
/* ============================================================= */
str_lst_t **v2_lstr_pend(str_lst_t **p_lstr) {
    str_lst_t *str_tmp=NULL;

    if(!p_lstr || !*p_lstr) return(p_lstr);

    FOR_LST_NEXT(str_tmp, *p_lstr);

    return(&str_tmp->next);
}
/* ============================================================= */
str_lst_t *v2_lstr_heads(str_lst_t **p_lstr, char *in_key, char *in_val, ...) {
    char strtmp[MAX_STRING_LEN];
    str_lst_t *str_tmp=NULL;
    if(!(str_tmp=v2_lstr.head(p_lstr, in_key))) return(NULL);
    if(in_val && *in_val) {
	VL_STR(strtmp, MAX_STRING_LEN, in_val);
	v2_lstr.astr(str_tmp, strtmp);
    }
    return(str_tmp);
}
/* ============================================================= */
str_lst_t *v2_lstr_tails(str_lst_t **p_lstr, char *in_key, char *in_val, ...) {
    char strtmp[MAX_STRING_LEN];
    str_lst_t *str_tmp=NULL;
    if(!(str_tmp=v2_lstr.tail(p_lstr, in_key))) return(NULL);
    if(in_val && *in_val) {
	VL_STR(strtmp, MAX_STRING_LEN, in_val);
	v2_lstr.astr(str_tmp, strtmp);
    }
    return(str_tmp);
}
/* ============================================================= */
// Find, if not add to head - do not replace is_rmv records
str_lst_t *v2_lstr_if_head(str_lst_t **p_lstr, char *in_key) {
    str_lst_t *str_tmp=NULL;

    if(!in_key || !*in_key || !p_lstr) return(NULL);

    if((str_tmp=v2_lstr.key(*p_lstr, in_key))) return(str_tmp);
    return(v2_lstr.head(p_lstr, in_key));
}
/* ============================================================= */
str_lst_t *v2_lstr_headu(str_lst_t **p_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!p_lstr || !in_key || !*in_key) return(NULL);

    if((str_out=v2_lstr.keyr(*p_lstr, in_key))) { // Find record include removed one
	str_out->is_rmv=v2_off;
    } else {
	str_out=v2_lstr.head(p_lstr, in_key);
    }

    return(str_out);
}
/* ============================================================= */
str_lst_t *v2_lstr_tailu(str_lst_t **p_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!p_lstr || !in_key || !*in_key) return(NULL);

    if((str_out=v2_lstr.keyr(*p_lstr, in_key))) { // Find record include removed one
	str_out->is_rmv=v2_off;
    } else {
	str_out=v2_lstr.tail(p_lstr, in_key);
    }

    return(str_out);
}
/* ============================================================= */
str_lst_t *v2_lstr_sortu(str_lst_t **p_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!p_lstr || !in_key || !*in_key) return(NULL);

    if((str_out=v2_lstr.keyr(*p_lstr, in_key))) { // Find record include removed one
	str_out->is_rmv=v2_off;
    } else {
	str_out=v2_add_lstr_sort(p_lstr, in_key, NULL, 0, 0);
    }

    return(str_out);
}
/* ============================================================= */
str_lst_t *v2_lstr_headf(str_lst_t **p_lstr, char *format, ...) {
    va_list vl;
    str_lst_t *str_tmp=NULL;
    char *tempa=NULL;
    int rc=0;

    if(!p_lstr)               return(NULL);
    if(!format || !format[0]) return(NULL);

    va_start(vl, format);
    rc=vasprintf(&tempa, format, vl);
    va_end(vl);

    if(rc==-1) return(NULL);

    str_tmp                 = v2_lstr.unew(tempa);
    str_tmp->is_key_unalloc = 0; // We allocated this key already

    return(v2_lstr.to_head(p_lstr, str_tmp));
}
/* ============================================================= */
str_lst_t *v2_lstr_tailf(str_lst_t **p_lstr, char *format, ...) {
    va_list vl;
    str_lst_t *str_tmp=NULL;
    char *tempa=NULL;
    int rc=0;

    if(!p_lstr)               return(NULL);
    if(!format || !format[0]) return(NULL);

    va_start(vl, format);
    rc=vasprintf(&tempa, format, vl);
    va_end(vl);

    if(rc==-1) return(NULL);

    str_tmp                 = v2_lstr.unew(tempa);
    str_tmp->is_key_unalloc = 0; // We allocated this key already

    return(v2_lstr.to_tail(p_lstr, str_tmp));
    //v2_lstr_add_tail(p_lstr, str_tmp);

    //return(str_tmp);
}
/* ============================================================= */
str_lst_t *v2_lstr_head(str_lst_t **p_lstr, char *in_key) {

    if(!in_key || !*in_key) return(NULL);

    return(v2_add_lstr_head(p_lstr, in_key, NULL, 0, 0));
}
/* ============================================================= */
str_lst_t *v2_lstr_tail(str_lst_t **p_lstr, char *in_key) {

    if(!in_key || !*in_key) return(NULL);

    return(v2_add_lstr_tail(p_lstr, in_key, NULL, 0, 0));
}
/* ============================================================= */
// Add already allocated key
str_lst_t *v2_lstr_heada(str_lst_t **p_lstr, char *in_key) {
    return(v2_lstr.to_head(p_lstr, v2_lstr.anew(in_key)));
}
/* ============================================================= */
// Add already allocated key
str_lst_t *v2_lstr_taila(str_lst_t **p_lstr, char *in_key) {
    return(v2_lstr.to_tail(p_lstr, v2_lstr.anew(in_key)));
}
/* ============================================================= */
// Get data for linked element
void *v2_lstr_link_data(str_lst_t *in_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!in_lstr)                                      return(NULL);
    if(!(str_out=v2_lstr.key(in_lstr->link, in_key))) return(NULL);

    return(str_out->data);
}
/* ============================================================= */
// Finds and adds if needed to in_lstr->link element with key
str_lst_t *v2_lstr_link(str_lst_t *in_lstr, char *in_key) {
    str_lst_t *str_out=NULL;

    if(!in_lstr)              return(NULL);
    if(!in_key || !in_key[0]) return(NULL);

    if((str_out=v2_lstr.keyr(in_lstr->link, in_key))) {
        str_out->is_rmv=v2_off;      // Reset remiving
    } else {
        str_out=v2_lstr.head(&in_lstr->link, in_key);
    }

    return(str_out);
}
/* ============================================================= */
str_lst_t *v2_lstr_find_num(str_lst_t *in_lstr, int in_num) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_out=NULL;

    if(!in_num) return(0); // Not zerro found

    FOR_LST_IF(str_tmp, str_out, in_lstr) {
	if(str_tmp->num==in_num) str_out=str_tmp;
    }

    return(str_out);
}
/* ============================================== */
// Get key element include is_rmv attribute
str_lst_t *v2_lstr_get_keyr(str_lst_t *p_str, char *in_key) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_rrr=NULL;

    if(!p_str || !in_key || !*in_key) return(NULL);

    FOR_LST_IF(str_tmp, str_rrr,  p_str) {
	if(!v2_strcmp(str_tmp->key, in_key)) str_rrr=str_tmp;
    }

    return(str_rrr);
}
/* ============================================================= */
str_lst_t *v2_lstr_del_key(str_lst_t *in_lstr, char *in_key) {
    //str_lst_t *str_out=NULL;

    //str_out=v2_lstr.key(in_lstr, in_key);
    //if(str_out) str_out->is_rmv=v2_on;
    //return(str_out);
    return(v2_lstr.rmv(v2_lstr.key(in_lstr, in_key)));
}
/* ============================================================= */
str_lst_t *v2_lstr_purge(str_lst_t **pt_lstr) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_out=NULL;

    while(*pt_lstr) {
	str_tmp  = *pt_lstr;
	*pt_lstr = (*pt_lstr)->next;

	if(!str_tmp->is_rmv) {
	    str_tmp->next = str_out;
	    str_out       = str_tmp;
	} else {
	    str_tmp->next=NULL;
	    v2_lstr.free(&str_tmp);
	}
    }

    *pt_lstr=str_out;
    v2_lstr.back(pt_lstr);

    return(*pt_lstr);
}
/* ============================================================= */
str_lst_t *v2_lstr_rdt(str_lst_t *in_lstr) {
    if(in_lstr) ++in_lstr->is_rdt;
    return(in_lstr);
}
/* ============================================================= */
// Check if list contain alive records with noempty keys
int v2_lstr_is_nodel(str_lst_t *in_lstr) {
    str_lst_t *str_tmp=NULL;
    int out=0;

    FOR_LST_IF(str_tmp, out, in_lstr) {
	if(!str_tmp->is_rmv && v2_is_par(str_tmp->key)) out=1; // Alive record
    }

    return(out);
}
/* ============================================================= */
 // Make full copy of list
str_lst_t *v2_lstr_copy(str_lst_t **p_dst, str_lst_t *in_src) {
    str_lst_t *str_tmp=NULL;

    if(!p_dst) return(NULL);

    FOR_LST(str_tmp, in_src) {
	if(v2_add_lstr_head(p_dst, str_tmp->key, str_tmp->str, str_tmp->a_time, str_tmp->b_time)) return(0);
	(*p_dst)->value  = str_tmp->value;
	(*p_dst)->num    = str_tmp->num;
	(*p_dst)->is_rdt = str_tmp->is_rdt;
	(*p_dst)->is_rmv = str_tmp->is_rmv;

	(*p_dst)->link   = str_tmp->link; // ???

	v2_lstr_add_data(*p_dst, str_tmp->data); // Non allocated adding
    }

    v2_lstr.back(p_dst);
    return(*p_dst);
}
/* ============================================================= */
// Just move list from p_src to output
str_lst_t *v2_lstr_move(str_lst_t **p_src) {
    str_lst_t *outl=NULL;

    if(p_src) {
	outl=*p_src;
	*p_src=NULL;
    }
    return(outl);
}
/* ============================================================= */
/*
static int print_str_lst(FILE *stream, const struct printf_info *info, const void *const *args) {
  const str_lst_t *s;
  char *buffer;
  int len;

  // Format the output into a string.
  s = *((const str_lst_t **) (args[0]));
  if(s->str) {
      len=asprintf (&buffer, "%s %s", s->key, s->str);
  } else {
      len=asprintf (&buffer, "%s", s->key);
  }
  if (len == -1) return(-1);

  // Pad to the minimum field width and print to the stream.
  len = fprintf (stream, "%*s", (info->left ? -info->width : info->width), buffer);

  // Clean up and return
  free(buffer);
  return(len);
}*/
/* ============================================================= */
/*
static int print_str_lst_arginfo(const struct printf_info *info, size_t n, int *argtypes, int *size) {
    if (n > 0) argtypes[0] = PA_POINTER;
    return(1);
}*/
/* ============================================================= */
/*
int v2_lstr_init_print(void) {
    register_printf_specifier('R', &print_str_lst, &print_str_lst_arginfo);
    return(0);
}*/
/* ============================================================= */
static int v2_lstr_to_str(str_lst_t *in_lstr, char **p_str, char in_dlm) {
    str_lst_t *str_tmp=NULL;
    char dlm=in_dlm;
    int cnt=0;

    FOR_LST(str_tmp, in_lstr) {
        if(str_tmp->is_rmv) continue;
	cnt += strlen(str_tmp->key) + 1;
    }

    if(p_str) {
	v2_freestr(p_str);
        if(!cnt) return(0);

	*p_str=(char*)calloc(cnt, 1);
	cnt=0;
        if(!dlm) dlm=',';
	FOR_LST(str_tmp, in_lstr) {
	    if(str_tmp->is_rmv) continue;
	    cnt += sprintf((*p_str)+cnt, "%s", str_tmp->key);
            if(str_tmp->next) cnt += sprintf((*p_str)+cnt, "%c", dlm);
	}
    }
    return(cnt);
}
/* ========================================================= */
// If spr == 0, separate by all space symbols
static str_lst_t *v2_lstr_fm_str(char *in_str, char spr) {
    str_lst_t *str_out=NULL;
    char *beg, *end;

    if(!in_str || !*in_str) return(NULL);

    //if(!spr) return(NULL); // ?? Maybe replace to white space? (isspace())

    beg=in_str;

    while(beg) {

	if(spr) {
	    while(*beg == spr) beg++; // Skip delimitors
	    end=strchr(beg, spr);
	} else {
	    while(isspace(*beg)) beg++; // Skip empty spaces
	    end=beg;
	    while(end && !isspace(*end)) end++;
	}

	if(end) {
	    v2_lstr.heada(&str_out, v2_let_varn(NULL, beg, end-beg));
	    beg=end+1;
	} else {
	    v2_lstr.head(&str_out, beg);
	    beg=NULL; // End
	}
    }

    v2_lstr.back(&str_out);

    return(str_out);
}
/* ============================================================= */
// Output functions
/* ============================================================= */
// Format details at "v2_lstr.h" file
//static int v2_lstr_fprnf(FILE *in_cf, str_lst_t *in_lstr, char *in_frmt, ...) {
static int v2_lstr_fprnf(FILE *in_cf, str_lst_t *in_lstr, char *in_format) {
    char strout[MAX_STRING_LEN];
    char format[MAX_STRING_LEN];
    str_lst_t *str_tmp=NULL;
    int is_skip=0;
    int is_skip_key=0;
    int is_skip_str=0;
    int is_skip_data=0;
    int no_show=0;
    int cnt=0;
    int out=0;
    int x;

    if(!in_lstr) return(17561);

    if(!in_cf) in_cf=stdout;

    //VL_STR(format, 0, in_frmt);

    snprintf(format, MAX_STRING_LEN, "%s", v2_nn(in_format));

    // Just read options
    for(x=0; format[x]; x++) {
	if(format[x] != '%')   continue;
	if(format[x+1] != 'O') continue;
	if(format[x+2] == 'v') is_skip=1;
	if(format[x+2] == 'k') is_skip_key=1;
	if(format[x+2] == 's') is_skip_str=1;
	if(format[x+2] == 'd') is_skip_data=1;
    }



    FOR_LST(str_tmp, in_lstr) {

	if(str_tmp->is_rmv) continue;
	if(!str_tmp->key)   continue;

	if(format[0]=='\0') {
	    cnt+=fprintf(in_cf, "%s%s%s\n", v2_nn(str_tmp->key), str_tmp->str?" ":"", v2_nn(str_tmp->str));
	    continue;
	}

	// Here we need to make format %Tk %Ts %Ta
	for(x=0, cnt=0; format[x]; x++) {
	    if(format[x] != '%') {
		strout[cnt++]=format[x];
		continue;
	    }
	    x++;

	    if(format[x] == 'T') { // Format output
		x++;
		if(format[x] == 'k') {
		    if(!str_tmp->key && (is_skip || is_skip_key)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_nn(str_tmp->key));
		} else if(format[x] == 's') {
		    if(!str_tmp->str && (is_skip || is_skip_str)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_nn(str_tmp->str));
		} else if(format[x] == 'v') {
		    if(!str_tmp->key && !str_tmp->str && is_skip) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_lstr_str(str_tmp));
		} else if(format[x] == 'd') {
		    if(!str_tmp->dstr && (is_skip || is_skip_data)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s", v2_nn(str_tmp->dstr));
		} else if(format[x] == 'a') {
		    cnt+=sprintf(strout+cnt, "%ld", (long)str_tmp->a_time);
		} else if(format[x] == 'b') {
		    cnt+=sprintf(strout+cnt, "%ld", (long)str_tmp->b_time);
		} else if(format[x] == 'n') {
		    cnt+=sprintf(strout+cnt, "%d", str_tmp->num);
		} else if(format[x] == 'r') {
		    cnt+=sprintf(strout+cnt, "%d", str_tmp->is_rdt);
		} else if(format[x] == 'f') {
		    cnt+=sprintf(strout+cnt, "%f", str_tmp->value);
		} else if(format[x] == 'e') {
		    cnt+=sprintf(strout+cnt, "%e", str_tmp->value);
		} else if(format[x] == 'l') { // All line
		    if(!str_tmp->key && (is_skip || is_skip_key)) no_show=1;
		    if(!str_tmp->str && (is_skip || is_skip_str)) no_show=1;
		    cnt+=sprintf(strout+cnt, "%s%s%s", v2_nn(str_tmp->key), str_tmp->str?" ":"", v2_nn(str_tmp->str));
		} else {
		    strout[cnt++]=format[x-2]; // '%'
		    strout[cnt++]=format[--x]; // 'T' & set pointer to prev pos
		}
		continue;
	    }

	    if(format[x] == 'O') { // Skip config options
		x++; // 's'
		if(format[x] == 'v') continue; // Skip if empty key or str
		if(format[x] == 'k') continue; // Skip if empty key or str
		if(format[x] == 's') continue; // Skip if empty key or str
		if(format[x] == 'd') continue; // Skip if empty key or str

		strout[cnt++]=format[x-2]; // '%'
		strout[cnt++]=format[--x]; // 'O' and set pointer to prev pos
		continue;
	    }

	    strout[cnt++]=format[--x]; // %
	}

	strout[cnt]='\0';
	if(!no_show) out+=fprintf(in_cf, "%s", strout);
    }

    return(cnt);
}
/* ============================================================= */
static int v2_lstr_prnf(str_lst_t *in_lstr, char *in_format) {
    return(v2_lstr.fprnf(NULL, in_lstr, in_format)); // Print it w/ format to stdout
}
/* ============================================================= */
static int v2_lstr_prn(str_lst_t *in_lstr) {
    return(v2_lstr.prnf(in_lstr, NULL)); // Print it w/o format
}
/* ============================================================= */
// File functions
/* ============================================================= */
// !!! This is prototype of external function
//static int v2_lstr_read_fun(char *in_str, str_lst_t **p_lstr) {
//
//    v2_add_lstr_head(p_lstr, in_str, NULL, 0, 0);
//
//    return(0);
//}
/* ============================================================= */
static int loc_lstr_read_data(char *in_str, void *in_data) {
    _vstr_rdt_t *_rdt=(_vstr_rdt_t*)in_data;

    if(!_rdt)     return(0); // ???? - return error?
    if(_rdt->fun) return(_rdt->fun(in_str, _rdt->plstr));

    if(!in_str || !*in_str) return(0); // Empty sting can not be added to lstr

    if(!v2_lstr.head(_rdt->plstr, in_str)) return(17575);

    return(0);
}
/* ============================================================= */
static int v2_lstr_nread(int(*rd_fun)(char*, str_lst_t**), str_lst_t **p_lstr, char *file) {       // Read file to str_lst
    _vstr_rdt_t rdt;
    int rc=0;

    if(!file || !*file)       return(17570);
    if(!(rdt.plstr = p_lstr)) return(17571); // Realy needed?

    rdt.fun = rd_fun;

    v2_lstr_free(p_lstr);

    rc=sfread(&loc_lstr_read_data, (void*)&rdt, file);

    v2_lstr.back(p_lstr);

    return(rc);
}
/* ============================================================= */
int v2_lstr_read(int(*rd_fun)(char*, str_lst_t**), str_lst_t **p_lstr, char *file, ...) {       // Read file to str_lst
    va_list vl;
    char *tfile=NULL;
    int rc=0;

    va_start(vl, file);
    rc=vasprintf(&tfile, file, vl);
    va_end(vl);

    if(rc==-1) return(XFREAD_NOT_NAME); // Suspect filename is NULL

    rc=v2_lstr_nread(rd_fun, p_lstr, tfile);

    free(tfile);

    return(rc);
}
/* ============================================================= */
int v2_lstr_xread(str_lst_t **p_lstr, char *file, ...) {
    char strtmp[MAX_STRING_LEN];

    VL_STR(strtmp, MAX_STRING_LEN, file);
    return(v2_lstr_nread(NULL, p_lstr, strtmp));
}
/* ============================================================= */
/* ============================================================= */
static int loc_lstr_save_data(FILE *in_cf, void *in_data) {
    _vstr_sdt_t *_sdt=(_vstr_sdt_t*)in_data;
    str_lst_t *str_tmp=NULL;
    int cnt=0;

    if(!_sdt)     return(0); // ???? - should not happen

    if(_sdt->fun) cnt+=_sdt->fun(in_cf, NULL); // Write header if needed

    FOR_LST(str_tmp, _sdt->inlstr) {
	if(_sdt->fun) {
	    cnt+=_sdt->fun(in_cf, str_tmp);
	    continue;
	}

	if(str_tmp->is_rmv) continue;
	cnt+=fprintf(in_cf, "%s%s%s\n", v2_nn(str_tmp->key), str_tmp->str?" ":"", v2_nn(str_tmp->str));
    }

    return(cnt);
}
/* ============================================================= */
int v2_lstr_save(str_lst_t *in_lstr, int (*wr_fun)(FILE*, str_lst_t *), char *file) {
    _vstr_sdt_t sdt;
    int rc=0;

    if(!(sdt.inlstr = in_lstr)) return(17576);

    sdt.fun = wr_fun;

    rc=sfsave(&loc_lstr_save_data, (void*)&sdt, file);

    return(rc);
}
/* ============================================================= */
int v2_lstr_savef(str_lst_t *in_lstr, int (*wr_fun)(FILE*, str_lst_t *), char *file, ...) {
    va_list vl;
    char *tfile=NULL;
    int rc=0;

    va_start(vl, file);
    rc=vasprintf(&tfile, file, vl);
    va_end(vl);

    if(rc==-1) return(XFREAD_NOT_NAME); // Suspect filename is NULL

    rc=v2_lstr_save(in_lstr, wr_fun, tfile);

    free(tfile);

    return(rc);
}
/* ============================================================= */
// Config functions
/* ============================================================= */
static char *v2_lstr_cf_str(str_lst_t *cf_lstr, char **pvar, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);
    if(!pvar)                           return(cf_lstr->str);

    cf_lstr->is_rdt=V_L_CHAR; // 1 - char**
    v2_lstr_add_data(cf_lstr, (void*)pvar);
    return(v2_let_var(pvar, cf_lstr->str));
}
/* ============================================================= */
static char *v2_lstr_cf_int(str_lst_t *cf_lstr, int *pint, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);

    if(!cf_lstr->str)                   return(NULL);
    if(!pint)                           return(cf_lstr->str);

    cf_lstr->num=atoi(cf_lstr->str);
    cf_lstr->is_rdt=V_L_INT; // 2 - int*
    v2_lstr_add_data(cf_lstr, (void*)pint);
    *pint=cf_lstr->num;

    return(cf_lstr->str);
}
/* ============================================================= */
static char *v2_lstr_cf_lng(str_lst_t *cf_lstr, long *plng, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);

    if(!cf_lstr->str)                   return(NULL);
    if(!plng)                           return(cf_lstr->str);

    cf_lstr->a_time=strtol(cf_lstr->str, NULL, 0);
    cf_lstr->is_rdt=V_L_LONG; // 2 - long*
    v2_lstr_add_data(cf_lstr, (void*)plng);
    *plng=cf_lstr->a_time;

    return(cf_lstr->str);
}
/* ============================================================= */
static char *v2_lstr_cf_flg(str_lst_t *cf_lstr, int *pflg, char *in_key) {

    if(!cf_lstr)                        return(NULL);
    if(v2_strcmp(cf_lstr->key, in_key)) return(NULL);

    if(!cf_lstr->str)                   return(NULL);
    if(!pflg)                           return(cf_lstr->str);

    cf_lstr->num=1;
    cf_lstr->is_rdt=2; // 2 - int*
    v2_lstr_add_data(cf_lstr, (void*)pflg);
    *pflg=cf_lstr->num;

    return(cf_lstr->str?cf_lstr->str:"1");
}
/* ============================================================= */
// File: [#|;][:|!]file_name
// ; - skip comment strings
// # - skip comment strings, but keep special comment starting from "#=#..."
// : - not open file, but run script
// ! - not error if file not found
// File: [#|;]-
// Read stdin

static int v2_lstr_fcfg(str_lst_t **pt_lst, char *file, ...) {
    FILE *cf=NULL;
    char strtmp[MAX_STRING_LEN];
    int is_exec=0;
    int r_pos=0; // Remark position
    int no_spc=0; // Special mode
    int no_found=0;

    if(!pt_lst) return(-742);
    v2_lstr_free(pt_lst);

    VL_STR(strtmp, MAX_STRING_LEN, file);

    if(!strtmp[0]) return(-741);

    if(strtmp[0] == '#') r_pos=1; // Filter comments, but not special config lines "#=#..."
    if(strtmp[0] == ';') {r_pos=1; no_spc=1;} // Filter all comments

    if(strtmp[r_pos] == ':') is_exec=1;  // Exec script
    if(strtmp[r_pos] == '!') no_found=1; // Ignore "file not found"


    errno=0;
    if(strtmp[r_pos+no_found] == '-') {
	cf=stdin;
    } else if(is_exec) {
	if(!(cf=popen(strtmp+1+r_pos, "r"))) return(XFREAD_NOT_RUN);
    } else if(!(cf=fopen(strtmp+r_pos+no_found, "r"))) {
	if(errno != ENOENT) return(XFREAD_NOT_ACCESS); // Exists, but unreadble
	if(no_found) return(0); // Ignore not founded file.
	return(XFREAD_NOT_FOUND);
    }

    while(!v2_getline(strtmp, MAX_STRING_LEN, cf)) {
	if(!strtmp[0]) continue;
	if(r_pos) {
	    if(strtmp[0]==';') continue;
	    if(strtmp[0]=='#') {
		if(no_spc)     continue; // Not keep special lines "#=#..."
		if(strtmp[1]!='=') continue;
		if(strtmp[2]!='#') continue; // Special config parameter
	    }
	}
	v2_lstr_separate(v2_lstr.head(pt_lst, strtmp));
    }
    v2_lstr_rback(pt_lst);

    if(cf==stdin) return(0); // Standart input still to be opened

    if(is_exec) return(pclose(cf));

    if(fclose(cf)) return(XFREAD_NOT_CLOSE); // EBADF - Descriptor is not valid...

    return(0);
}
/* ============================================================= */
str_lst_t *v2_lstr_keyf(str_lst_t *in_list, char *in_key, ...) {
    char strtmp[MAX_STRING_LEN];

    if(!v2_is_par(in_key)) return(NULL);

    VL_STR(strtmp, MAX_STRING_LEN, in_key);

    return(v2_lstr_key(in_list, strtmp));
}
/* ============================================================= */
char *v2_lstr_strf(str_lst_t *in_list, char *in_key, ...) {
    char strtmp[MAX_STRING_LEN];

    if(!v2_is_par(in_key)) return(NULL);

    VL_STR(strtmp, MAX_STRING_LEN, in_key);

    return(v2_get_lstr_str(in_list, strtmp));
}
/* ============================================================= */
// If in_len == 0 just find by key
str_lst_t *v2_lstr_keyn(str_lst_t *in_lstr, int in_len, char *in_key) { // Find/get bypart of input word
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_out=NULL;
    int len=0;

    if(!in_lstr)           return(NULL);
    if(!v2_is_par(in_key)) return(NULL);

    if(!in_len) return(v2_lstr.key(in_lstr, in_key));

    len=strlen(in_key);

    if(len < in_len) return(NULL);

    FOR_LST_IF(str_tmp, str_out, in_lstr) {
	if(str_tmp->is_rmv || !str_tmp->key || !str_tmp->key[0]) continue;
	if(strncmp(str_tmp->key, in_key, in_len)) continue;
	str_out=str_tmp;
    }

    return(str_out);
}
/* ============================================================= */
// Pull out string value from Lstr and mark it as removed
char *v2_lstr_pull(str_lst_t *in_lstr, char *key) {
    str_lst_t *str_tmp=NULL;

    if(!(str_tmp=v2_lstr.del(in_lstr, key))) return(NULL);
    return(str_tmp->str);
}
/* ============================================================= */
// Argv functions
/* ============================================================= */
int v2_lstr_len(str_lst_t *in_lstr) {
    str_lst_t *str_tmp=NULL;
    int out=0;

    FOR_LST(str_tmp, in_lstr) out++;
    return(out);
}
/* ============================================================= */
char **v2_lstr_argv(str_lst_t *in_lstr) {
    str_lst_t *str_tmp=NULL;
    char **argv_out=NULL;
    int cnt=0;

    argv_out=(char**)calloc(sizeof(char*), v2_lstr_len(in_lstr)+1);
    assert(argv_out);

    FOR_LST(str_tmp, in_lstr) argv_out[cnt++]=v2_let_var(NULL, str_tmp->key);

    return(argv_out);
}
/* ============================================================= */
int v2_lstr_exec(char *in_args, ...) {
    str_lst_t *arg_lst=NULL;
    char strarg[MAX_STRING_LEN]; // can not be longer ARG_MAX bytes
    char **argvv;

    VL_STR(strarg, MAX_STRING_LEN, in_args);

    if(!strarg[0]) return(16540);

    arg_lst=v2_lstr.fm_str(strarg, ' '); // argv[0] ... argv[n]
    argvv=v2_lstr.argv(arg_lst);
    v2_lstr.free(&arg_lst);

    if(execvp(argvv[0], argvv)) return(16541);
    return(0); // This never happens because function does not returns at success
}

/* ================================================================== */
// quick sort/find functions
/* ================================================================== */
int lstr_compare(const void *in_one, const void *in_two) {
    str_lst_t *str_one=*(str_lst_t * const *)in_one;
    str_lst_t *str_two=*(str_lst_t * const *)in_two;
    int rc=0;

    if(!str_one || !str_one->key) return(0);
    if(!str_two || !str_two->key) return(0);

    if((rc=strcoll(str_one->key, str_two->key))) return(rc);

    // Keys are equial - compare stings
    if(!str_one->str || !str_two->str) return(0);
    return(strcoll(str_one->str, str_two->str));
}
/* ============================================================= */

// v2_lstr_qsort_r() Moved to GNU_SOURCEd v2_qlstr.[ch]

/* ============================================================= */
 // Sort with qsort fun
str_lst_t *v2_lstr_qsort(v2_qlstr_t *qbase) {
    str_lst_t *str_tmp=NULL;
    int x=0;

    if(!qbase || !qbase->list) return(NULL);

    qbase->num=0;
    FOR_LST(str_tmp, qbase->list) qbase->num++;
    if(qbase->num==0) return(NULL); // Notnig to sort

    // Prepare Array for QSort
    if(!(qbase->array=(str_lst_t **)calloc(qbase->num, sizeof(str_lst_t*)))) v2_abort("qsort: Memory allocation error.");

    FOR_LST(str_tmp, qbase->list) qbase->array[x++]=str_tmp;

    //qsort(&(qbase->array[0]), qbase->num, sizeof(str_lst_t*), &lstr_compare);
    qsort(qbase->array, qbase->num, sizeof(str_lst_t*), &lstr_compare);

    // Link sort
    for(x=0; x<qbase->num; x++) {
	if(x == (qbase->num-1)) { // Last element
	    qbase->array[x]->next=NULL;
	} else {
	    qbase->array[x]->next=qbase->array[x+1];
	}
    }
    qbase->list=qbase->array[0];
    return(qbase->list);
}
/* ============================================================= */
// Search element after v2_lstr_qsort sorting
str_lst_t *v2_lstr_qkey(v2_qlstr_t qbase, char *in_key) {
    str_lst_t str_sts;
    str_lst_t *str_hint=&str_sts; // Hint
    str_lst_t **pnt_ret=NULL;

    if(!in_key || !*in_key) return(NULL); // Nothing to search

    //if(!qbase)       return(NULL);

    if(!qbase.list) return(NULL);

    if(!qbase.num) qbase.list=v2_lstr_qsort(&qbase); // Sort it

    str_sts.key = in_key;
    str_sts.str = NULL; // Becaue we try to compare str too

    pnt_ret=(str_lst_t **)bsearch(&str_hint, qbase.array, qbase.num, sizeof(str_lst_t*), &lstr_compare);

    if(pnt_ret) return(*pnt_ret);
    return(NULL);
}
/* ============================================================= */
// Other
/* ============================================================= */
static int _fun_cmpfile(char *in_str, void *in_data) {
    str_lst_t **pstr=in_data;

    if(!v2_is_par(in_str))              return(17587);
    //if(!v2_is_par(in_str))              return(0);
    if(!*pstr || !(*pstr)->key)         return(17588);
    //printf("!!! %s = %s\n", in_str, (*pstr)->key);
    //printf("!!! %s - %s\n", in_str, (*pstr)->next->key);
    if(v2_strcmp(in_str, (*pstr)->key)) {
	return(17589);
    }

    *pstr=(*pstr)->next;

    return(0);
}
/* ============================================================= */
// Compare two lstr chains
int v2_lstr_cmpfile(str_lst_t *in_lst, char *in_file, ...) {
    char file_name[MAX_STRING_LEN];
    str_lst_t *str_tmp=in_lst;
    int rc=0;

    VL_STR(file_name, MAX_STRING_LEN, in_file);

    if(!file_name[0]) return(17586);

    rc=sfread(&_fun_cmpfile, &str_tmp, file_name);

    return(rc);
}
/* ============================================== */
// LSTR functions (exported from v2_tools)
/* ============================================== */
str_lst_t *v2_add_lstr_head(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time) {
    str_lst_t *str_tmp=NULL;

    if(!p_str) return(NULL);

    if(!(str_tmp=(str_lst_t *)calloc(sizeof(str_lst_t), 1))) return(NULL);
    v2_let_var(&str_tmp->key, in_key);
    v2_let_var(&str_tmp->str, in_str);
    str_tmp->a_time=a_time;
    str_tmp->b_time=b_time;
    str_tmp->next=(*p_str);
    (*p_str)=str_tmp;
    return(str_tmp);
}
/* ============================================== */
str_lst_t *v2_add_lstr_tail(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_ttt=NULL;

    if(!p_str) return(NULL);

    if(!(str_tmp=(str_lst_t *)calloc(sizeof(str_lst_t), 1))) return(NULL);
    v2_let_var(&str_tmp->key, in_key);
    v2_let_var(&str_tmp->str, in_str);
    str_tmp->a_time=a_time;
    str_tmp->b_time=b_time;

    if(!(*p_str)) {
        (*p_str)=str_tmp;
    } else {
        for(str_ttt=(*p_str); str_ttt->next; str_ttt=str_ttt->next);
        str_ttt->next=str_tmp;
        str_ttt=NULL;
    }
    return(str_tmp);
}
/* ============================================== */
str_lst_t *v2_add_lstr_sort(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_ttt=NULL;

    if(!p_str) return(NULL);

    if(!in_key || !*in_key) return(NULL);

    if(!(str_tmp=(str_lst_t *)calloc(sizeof(str_lst_t), 1))) return(NULL);
    v2_let_var(&str_tmp->key, in_key); // Always should be
    v2_let_var(&str_tmp->str, in_str);
    str_tmp->a_time=a_time;
    str_tmp->b_time=b_time;

    if(!(*p_str)) {
        (*p_str)=str_tmp;
    } else if(strcmp(str_tmp->key, (*p_str)->key) < 0) {
        str_tmp->next=(*p_str);
        (*p_str)=str_tmp;
    } else {
        for(str_ttt=(*p_str); str_ttt->next && (strcmp(str_tmp->key, str_ttt->next->key) > 0); str_ttt=str_ttt->next);
        str_tmp->next=str_ttt->next;
        str_ttt->next=str_tmp;
        str_ttt=NULL;
    }
    return(str_tmp);
}
/* ============================================== */
int v2_lstr_def_sort(str_lst_t *a_str, str_lst_t *b_str) {
    if(!a_str || !b_str) return(0); // Hmm what to do?
    if(!a_str->key || !a_str->key[0]) return(0);
    if(!b_str->key || !b_str->key[0]) return(0);
    return(strcmp(a_str->key, b_str->key));
}
/* ============================================== */
int v2_lstr_sort(str_lst_t **p_str, int (*fun_sort)(str_lst_t *, str_lst_t *)) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_ttt=NULL;
    str_lst_t *str_out=NULL;

    if(!p_str) return(0);
    //if(!(*p_str)) return(0);
    if(!fun_sort) fun_sort=&v2_lstr_def_sort;

    while((str_tmp=(*p_str))) {
	(*p_str)=(*p_str)->next;
	str_tmp->next=NULL;

	if(!str_out) {
	    str_out=str_tmp;
	} else if(fun_sort(str_tmp, str_out) < 0) {
	    str_tmp->next=str_out;
	    str_out=str_tmp;
	} else {
	    for(str_ttt=str_out; str_ttt->next && (fun_sort(str_tmp, str_ttt->next) > 0); str_ttt=str_ttt->next);
	    str_tmp->next=str_ttt->next;
	    str_ttt->next=str_tmp;
	    str_ttt=NULL;
	}
    }
    (*p_str)=str_out;
    str_out=NULL;

    return(0);
}
/* ============================================== */
str_lst_t *v2_lstr_key(str_lst_t *p_str, char *in_key) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_rrr=NULL;

    if(!p_str || !in_key || !*in_key) return(NULL);

    for(str_tmp=p_str; str_tmp && !str_rrr; str_tmp=str_tmp->next) {
	if(str_tmp->is_rmv) continue;
	if(!str_tmp->key || !str_tmp->key[0]) continue;
	if(strcmp(str_tmp->key, in_key)) continue;
        str_rrr=str_tmp;
    }

    return(str_rrr);
}
/* ============================================== */
char *v2_get_lstr_str(str_lst_t *p_str, char *in_key) {
    str_lst_t *str_tmp=NULL;

    if(!(str_tmp=v2_lstr_key(p_str, in_key))) return(NULL);
    return(str_tmp->str); // If NULL, then returns NULL
}
/* ============================================== */
void *v2_get_lstr_data(str_lst_t *p_str, char *in_key) {
    str_lst_t *str_tmp=NULL;

    if(!(str_tmp=v2_lstr_key(p_str, in_key))) return(NULL);
    return(str_tmp->data);
}
/* ============================================== */
// Finds pair of key/str into lstr list
str_lst_t *v2_lstr_find(str_lst_t *p_str, char *in_key, char *in_str) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_out=NULL;

    if(!p_str) return(NULL);
    if(!in_key) return(NULL);
    if(!in_key[0]) return(NULL);

    for(str_tmp=p_str; str_tmp && !str_out; str_tmp=str_tmp->next) {
	if(!str_tmp->key || !str_tmp->key[0]) continue;
	if(strcmp(str_tmp->key, in_key)) continue;
	if(!in_str || !in_str[0]) {
	    if(!str_tmp->str || !str_tmp->str[0]) str_out=str_tmp;
            continue;
	}
	if(!str_tmp->str || !str_tmp->str[0]) continue;
	if(!strcmp(in_str, str_tmp->str)) str_out=str_tmp;
    }
    return(str_out); // If NULL, then returns NULL
}
/* ============================================== */
int v2_lstr_rback(str_lst_t **p_str) {
    str_lst_t *str_tmp=NULL;
    str_lst_t *str_out=NULL;

    if(!p_str) return(1);

    while((*p_str)) {
        str_tmp=(*p_str);
        (*p_str)=(*p_str)->next;

        str_tmp->next=str_out;
        str_out=str_tmp;
    }
    str_tmp=NULL;
    (*p_str)=str_out;
    str_out=NULL;

    return(0);
}
/* ========================================================= */
int v2_lstr_separate_by(str_lst_t *i_str, char by_ch) {
    char *s=NULL;

    if(!i_str)      return(0);
    if(!i_str->key) return(0);
    if(i_str->str)  return(0);
    if(by_ch=='\0') return(0); // nothing to separate after end of sting

    if(!(s=strchr(i_str->key, by_ch))) return(0);
    if(!*s) return(0);
    *s='\0';
    v2_let_var(&i_str->str, ++s); // should to add while(*(++s)==' ');

    return(0);
}
/* ========================================================= */
str_lst_t *v2_lstr_separate(str_lst_t *i_str) {
    v2_lstr_separate_by(i_str, ' ');
    return(i_str);
}
/* ========================================================= */
// If by_ch == '\0', Separate by ' ', but skip '#' and ';' - as config file
int v2_lstr_separate_lst_by(str_lst_t *i_str, char by_ch) {
    str_lst_t *str_tmp=NULL;

    FOR_LST(str_tmp, i_str) {
	if(by_ch == '\0') {
	    if(str_tmp->key[0] == '#') continue;
	    if(str_tmp->key[0] == ';') continue;
	}
	v2_lstr_separate_by(str_tmp, by_ch?by_ch:' ');
    }

    return(0);
}
/* ============================================== */
// Safe add date - for correct lstr_free function
int v2_lstr_add_data(str_lst_t *i_str, void *in_data) {

    if(!i_str)      return(14);
    if(i_str->data) return(15);
    if(!in_data)    return(16);

    i_str->data=in_data;
    i_str->is_dat_unalloc=1; // Unallocated data (external)
    return(0);
}
/* ============================================== */
// in_tail not changed !!!
str_lst_t *v2_lstr_add_tail(str_lst_t **p_parent, str_lst_t *in_tail) {
    str_lst_t *str_tmp=NULL;

    if(!p_parent) return(NULL);
    if(!in_tail)  return(NULL); // Nothnig to add

    if(!(*p_parent)) {
	*p_parent=in_tail;
        return(in_tail);
    }

    FOR_LST_NEXT(str_tmp, *p_parent);

    if(!str_tmp) return(NULL);

    return((str_tmp->next=in_tail));
}
/* ============================================== */
int v2_lstr_free(str_lst_t **p_str) {
    str_lst_t *str_tmp=NULL;

    if(!p_str) return(1);

    while((str_tmp=(*p_str))) {
        (*p_str)=(*p_str)->next;

	//v2_lstr_free(&str_tmp->link); // Check Link!!!

	if(str_tmp->key  && !str_tmp->is_key_unalloc) free(str_tmp->key);
	if(str_tmp->str  && !str_tmp->is_str_unalloc) free(str_tmp->str);
	if(str_tmp->data && !str_tmp->is_dat_unalloc) free(str_tmp->data);

        free(str_tmp);
        str_tmp=NULL;
    }

    return(0);
}
/* ============================================== */
// Set in_lstr->is_rmv
str_lst_t *v2_lstr_rmv(str_lst_t *in_lstr) {
    if(in_lstr) in_lstr->is_rmv=v2_on;
    return(in_lstr);
}
/* ============================================== */
char *v2_lstr_str(str_lst_t *in_str) {

    if(!in_str) return("");
    if(in_str->str) return(in_str->str);
    if(in_str->key) return(in_str->key);
    return("");
}
/* ========================================================= */
/* Moved to v2_lstr.fm_str()
str_lst_t *v2_str_to_lstr(char *in_str, char spr) {
    str_lst_t *str_out=NULL;
    char *str=NULL;
    //char strtmp[MAX_STRING_LEN];
    char strtm1[MAX_STRING_LEN];

    //if(!in_str || !in_str[0]) return(NULL); // Nothing to separate
    if(!spr) return(0); // Have not criteria

    if(!v2_let_var(&str, in_str)) return(NULL);
    //strcpy(str, in_str);

    while(str[0]) {
	if(!v2_getword(strtm1, str, spr)) continue;
        v2_add_lstr_tail(&str_out, strtm1, NULL, 0, 0);
    }

    free(str);

    return(str_out);
}*/
/* ============================================================== */
// Read dir into list or just call function filter()
int v2_lstr_rdir(str_lst_t **p_list, int (*filter)(char*,void*), void *in_data, char *in_dir, ...) {
    DIR *tdir=NULL;
    struct dirent *tent=NULL;
    char strdir[MAX_STRING_LEN];
    int len=0; // Len of base dir
    int rc=0;

    if(!p_list && !filter) return(1);

    if(!in_dir || !in_dir[0]) {
	if(!getcwd(strdir, MAX_STRING_LEN)) return(1); // -Wunused-result
    } else {
	VL_STR(strdir, 0, in_dir);
    }

    if((len=strlen(strdir)) && strdir[len-1]=='/') len--; // Remove trailer slash

    if(!(tdir=opendir(strdir))) return(1);

    while((tent=readdir(tdir)) && !rc) {

	if(filter) {
	    if((rc=filter(tent->d_name, in_data))) {
		if(rc==1) rc=0; // Reset good value
		continue;
	    }
	} else {
	    if(tent->d_name[0] == '.') {
		if(tent->d_name[1] == '\0') continue;
		if(tent->d_name[1] == '.') {
		    if(tent->d_name[2] == '\0') continue;
		}
	    }
	}
	if(!p_list) continue;

	snprintf(strdir+len, MAX_STRING_LEN-len, "/%s", tent->d_name);

	v2_add_lstr_head(p_list, tent->d_name, strdir, strlen(tent->d_name), 0);
    }

    closedir(tdir);

    if(p_list) v2_lstr_rback(p_list);

    return(rc);
}
/* ============================================================= */
lstr_t v2_lstr = {

    .unew    = &v2_lstr_unew,     // Create new element with unallocated key
    .ustr    = &v2_lstr_ustr,     // Add unallocated string

    .anew    = &v2_lstr_anew,     // Create new element with already allocated key

    .to_head = &v2_lstr_to_head,  // Add in_lstr to head of p_lstr list
    .to_tail = &v2_lstr_add_tail, // Add in_lstr to tail of p_lstr list // v2_util

    .astr    = &v2_lstr_astr, // Add string to str
    .dstr    = &v2_lstr_dstr, // Add string to data - dstr

    .astrf   = &v2_lstr_astrf, // The same
    .dstrf   = &v2_lstr_dstrf,    // Add string to data - dstr

    .heads = &v2_lstr_heads,
    .tails = &v2_lstr_tails,

    //.headu = &v2_lstr_headu, // Update (undel or add)
    //.tailu = &v2_lstr_tailu,

    .headf = &v2_lstr_headf, // Add to head with format
    .tailf = &v2_lstr_tailf,

    .head  = &v2_lstr_head,
    .tail  = &v2_lstr_tail,

    .heada = &v2_lstr_heada, // Add allocated key
    .taila = &v2_lstr_taila, // Add allocated key

    .pend  = &v2_lstr_pend,

    .if_head  = &v2_lstr_if_head, // !!! Better to use .headu()

    .link = &v2_lstr_link,
    .sort = &v2_lstr_sort,   // v2_util
    .back = &v2_lstr_rback,  // v2_util
    .free = &v2_lstr_free,   // v2_util

    .find = &v2_lstr_find,

    .to_str = &v2_lstr_to_str,
    .fm_str = &v2_lstr_fm_str, // New test version
    //.fm_str_old = &v2_str_to_lstr,

    .keyf   = &v2_lstr_keyf,
    .strf   = &v2_lstr_strf,

    .key    = &v2_lstr_key, // v2_util
    .str    = &v2_get_lstr_str, // v2_util
    .num    = &v2_lstr_find_num, // Find by number, if it non-zero
    .data   = &v2_get_lstr_data, // v2_util

    .keyr   = &v2_lstr_get_keyr, // Tha same, as key, but include is_rmv attirbute
    .keyn   = &v2_lstr_keyn,     // Find rec by key as part of word

    .del    = &v2_lstr_del_key,
    .rmv    = &v2_lstr_rmv, // v2_util
    .rdt    = &v2_lstr_rdt, // Increment rdt each time

    .is_nodel = &v2_lstr_is_nodel, // List contains alive elements

    // Output functions
    .prn    = &v2_lstr_prn,
    .prnf   = &v2_lstr_prnf,
    .fprnf  = &v2_lstr_fprnf,

    // File functions
    .read   = &v2_lstr_read,
    .xread  = &v2_lstr_xread,

    // Conf functions
    .fcfg   = &v2_lstr_fcfg,   // Read config file to str_lst
    .cf_str = &v2_lstr_cf_str, // Get config string
    .cf_int = &v2_lstr_cf_int, // Get config integer
    .cf_lng = &v2_lstr_cf_lng, // Get config long integer
    .cf_flg = &v2_lstr_cf_flg, // Get config flag (=1)

    // Argv
    .len    = &v2_lstr_len,    // Count number of elemts into list (like arg)
    .argv   = &v2_lstr_argv,   // Creates arg[] massive from str_lst_t
    .exec   = &v2_lstr_exec,   // Exec external command

};
/* ============================================================= */

