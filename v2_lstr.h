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

#ifndef _V2_LSTR_H
#define _V2_LSTR_H 1

#include "v2_util.h"

/* Formated options for prnf() and fprnf() functions
 *
 * %Tk -> str_lst->key
 * %Ts -> str_lst->str
 * %Tv -> str_lst->str?str_lst->str:str_lst->key
 * %Ta -> str_lst->a_time
 * %Tb -> str_lst->b_time
 * %Tn -> str_lst->num
 * %Tf -> str_lst->value (float)
 * %Te -> str_lst->value (exponent)
 * %Td -> (char*)str_lst->data  (be careful!!!)
 *
 * Config options
 * %Ov -> skip output line if key, str or dstr are null
 * %Ok -> skip if key == NULL
 * %Os -> skip if str == NULL
 * %Od -> skip if data == NULL
 *
*/

#define V_L_CHAR 1
#define V_L_INT  2
#define V_L_LONG 3

#define V_L_REST 99

// For other applications
#define V_L_DIR 50

// Remove next element from the lstr list.
#define V2_NEXT_DEL(list)\
    if(list) {\
        str_lst_t *str_oom=(list)->next;\
        if(str_oom) {\
            (list)->next=str_oom->next;\
	    str_oom->next=NULL;\
            v2_lstr_free(&str_oom);\
        }\
    }


// Structure for qsort and bsearch functions
typedef struct {
    str_lst_t *list;
    str_lst_t **array;
    int num; // Elements number
} v2_qlstr_t;


typedef struct {

    int err;  // Error number

    str_lst_t *(*unew)(char *una_key);                             // Create new lstr element with unallocated key
    str_lst_t *(*ustr)(str_lst_t *in_lstr, char *una_str);         // Add unallocated str to lstr element

    str_lst_t *(*anew)(char *una_key);                             // Create new lstr element with already allocated

    str_lst_t *(*to_head)(str_lst_t **p_lstr, str_lst_t *in_lstr); // Add in_lstr to head of p_lstr list
    str_lst_t *(*to_tail)(str_lst_t **p_lstr, str_lst_t *in_lstr); // Add in_lstr to tail of p_lstr list

    str_lst_t *(*astr)(str_lst_t *in_lstr, char *in_str);          // Add pure str to lstr element
    str_lst_t *(*dstr)(str_lst_t *in_lstr, char *in_str);          // Add string to data structure

    str_lst_t *(*astrf)(str_lst_t *in_lstr, char *format, ...);    // Add formated str to lstr element
    str_lst_t *(*dstrf)(str_lst_t *in_lstr, char *format, ...);    // Add formated string to data structure

    str_lst_t *(*headf)(str_lst_t **p_lstr, char *format, ...);    // Add formated key to the head of lstr
    str_lst_t *(*tailf)(str_lst_t **p_lstr, char *format, ...);    // Add formated key to the tail of lstr

    str_lst_t *(*head)(str_lst_t **p_lstr, char *key);             // Add key to the head of lstr
    str_lst_t *(*tail)(str_lst_t **p_lstr, char *key);             // Add key to the tail of lstr

    str_lst_t *(*heada)(str_lst_t **p_lstr, char *key);            // Add ALLOCALTED key to the head of lstr
    str_lst_t *(*taila)(str_lst_t **p_lstr, char *key);            // Add ALLOCALTED key to the tail of lstr

    str_lst_t *(*heads)(str_lst_t **p_lstr, char *key, char *in_val, ...); // Add key and val to the head of lstr
    str_lst_t *(*tails)(str_lst_t **p_lstr, char *key, char *in_val, ...); // Add key and val to the tail of lstr

    //str_lst_t *(*headu)(str_lst_t **p_lstr, char *key);            // Update record (if exists - reset is_rmv)
    //str_lst_t *(*tailu)(str_lst_t **p_lstr, char *key);            // Update record (if exists - reset is_rmv)

    str_lst_t *(*if_head)(str_lst_t **p_lstr, char *key);          // Find key, if no - add to the head of lstr !!! Better to use .headu()

    str_lst_t *(*link)(str_lst_t *in_lstr, char *in_key);          // Add element to in_lstr->link list

    str_lst_t *(*find)(str_lst_t *in_lstr, char *in_key, char *in_str); // Find pair of key/str into list

    str_lst_t **(*pend)(str_lst_t **);          // Find address last element

    int (*sort)(str_lst_t **p_lstr, int (*fun_sort)(str_lst_t *, str_lst_t *)); // Sort lstr list,
                                                                                // fun_sort == NULL means strcmp

    int (*back)(str_lst_t **p_lstr);            // Return list back order
    int (*free)(str_lst_t **p_lstr);            // Free all lstr list

    int (*to_str)(str_lst_t*, char**, char);    // Make char** str from str_lst* with char delimiter
    str_lst_t *(*fm_str)(char*, char);          // Make str_lst str from char* with char delimiter
    //str_lst_t *(*fm_str_old)(char*, char);          // Make str_lst str from char* with char delimiter

    // Search functions
    str_lst_t *(*keyf)(str_lst_t*, char*, ...); // Get/find lstr rec by name, exclude is_rmv elements
    char *(*strf)(str_lst_t*, char*, ...);      // Get/find str value by name

    str_lst_t *(*key)(str_lst_t*, char*);       // Get/find lstr rec by name, exclude is_rmv elements
    char *(*str)(str_lst_t*, char*);            // Get/find str value by name
    void *(*data)(str_lst_t*, char*);           // Get/find data value by name

    str_lst_t *(*num)(str_lst_t*,int);          // Get/find rec by number, if num != 0
    str_lst_t *(*keyn)(str_lst_t*,int,char*);   // Get/find rec where key == part of the word.
    str_lst_t *(*keyr)(str_lst_t*, char*);      // Get/find lstr rec by name, include is_rmv elements

    // Remove records
    str_lst_t *(*del)(str_lst_t*, char*);       // Find and mark record for removing
    str_lst_t *(*rmv)(str_lst_t*);              // Mark record for removing
    str_lst_t *(*rdt)(str_lst_t*);              // Increment is_rdt element each time

    int (*is_nodel)(str_lst_t*);                // List contains active (no rmv) elements

    // Output functions
    int (*prn)(str_lst_t*);                     // prints (at stdout) list from str_lst
    int (*prnf)(str_lst_t*, char*);             // prints (at stdout) list from str_lst with format
    int (*fprnf)(FILE*,str_lst_t*, char*);      // prints (at FILE) list from str_lst with format

    // File function
    int (*read)(int(*)(char*, str_lst_t**), str_lst_t**, char*, ...); // Read file to str_lst
    int (*xread)(str_lst_t**, char*, ...);       // Read file to str_lst - replacment for xfread

    // Config functions
    int (*fcfg)(str_lst_t**, char*, ...);       // Read config file
    char *(*cf_str)(str_lst_t*, char**, char*); // Return string variable value
    char *(*cf_int)(str_lst_t*, int*, char*);   // Return int variable value
    char *(*cf_lng)(str_lst_t*, long*, char*);  // Return long variable value
    char *(*cf_flg)(str_lst_t*, int*, char*);   // Return int variable value
    //char *(*cf_dir)(str_lst_t*, char**, char*); // Return string as relative or absolute dir/file variable value

    //char *var_cf_dir; // Directory for cf_dir function
    //int (*cf_set_dir)(char*); // Set base directory for cf_dir() function

    // Exec and argv
    int (*len)(str_lst_t*);                     // Count number elements at the list
    char **(*argv)(str_lst_t*);                 // Creates argv[] massive from str_lst_t
    int (*exec)(char*, ...);                    // Exec external command

} lstr_t;

extern lstr_t v2_lstr;

/* ====================================================================== */
// Separated functions - last way

str_lst_t *v2_lstr_headu(str_lst_t **p_lstr, char *in_key); // Add element, if not exists, or restore from is_rmv
str_lst_t *v2_lstr_tailu(str_lst_t **p_lstr, char *in_key); // Add element, if not exists, or restore from is_rmv
str_lst_t *v2_lstr_sortu(str_lst_t **p_lstr, char *in_key); // Add element, if not exists, or restore from is_rmv

str_lst_t *v2_lstr_qsort(v2_qlstr_t *qbase); // Sort with qsort fun. Use also v2_qlstr.[ch]
str_lst_t *v2_lstr_qkey(v2_qlstr_t qbase, char *in_key); // Search after qsort

str_lst_t *v2_lstr_keyn(str_lst_t *in_lstr, int in_len, char *in_key); // Find/get by part of input key

char *v2_lstr_pull(str_lst_t *in_lstr, char *key); // withdraw element from the list

str_lst_t *v2_lstr_copy(str_lst_t **p_dst, str_lst_t *in_src); // Make full copy of list
str_lst_t *v2_lstr_move(str_lst_t **p_src); // Move list to output

str_lst_t *v2_lstr_purge(str_lst_t **pt_lstr); // Purge rmv'ed elements

int v2_lstr_read(int(*rd_fun)(char*, str_lst_t**), str_lst_t **p_lstr, char *file, ...); // Read file to str_lst
int v2_lstr_xread(str_lst_t **p_lstr, char *file, ...); // Read file to lstr list

int v2_lstr_save(str_lst_t *in_lstr, int (*wr_fun)(FILE*, str_lst_t *), char *file);
int v2_lstr_savef(str_lst_t *in_lstr, int (*wr_fun)(FILE*, str_lst_t *), char *file, ...);

/* ====================================================================== */
str_lst_t *v2_lstr_add_str(str_lst_t *in_lstr, char *format, ...); // Add formatted str to lstr element

//str_lst_t *v2_lstr_headf(str_lst_t **p_lstr, char *format, ...); // Add formated key to the head of lstr
//str_lst_t *v2_lstr_tailf(str_lst_t **p_lstr, char *format, ...); // Add formated key to the tail of lstr
str_lst_t *v2_lstr_link(str_lst_t *in_lstr, char *in_key); // Add element to in_lstr->link list

void *v2_lstr_link_data(str_lst_t *in_lstr, char *in_key); // Returns data from linked element

// int v2_lstr_init_print(void); // Init (register) print format %R

int v2_lstr_cmpfile(str_lst_t *in_lst, char *in_file, ...); // Compare lstr list with file
/* ====================================================================== */
// ListSTR functions (imported from v2_tools)
str_lst_t *v2_add_lstr_head(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time);
str_lst_t *v2_add_lstr_tail(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time);
str_lst_t *v2_add_lstr_sort(str_lst_t **p_str, char *in_key, char *in_str, time_t a_time, time_t b_time);

int v2_lstr_add_data(str_lst_t *i_str, void *in_data);

int v2_lstr_sort(str_lst_t **p_str, int (*fun_sort)(str_lst_t *, str_lst_t *)); // Sort list with user's function

str_lst_t *v2_lstr_separate(str_lst_t *i_str);                                  // Separate key to key and str
int v2_lstr_separate_by(str_lst_t *i_str, char by_ch);                          // Separate by key
int v2_lstr_separate_lst_by(str_lst_t *i_str, char by_ch);                      // Separate all list

#define v2_lstr_spr_lst(n, m) v2_lstr_separate_lst_by((n), (m))
#define v2_lstr_spr_by(n, m) v2_lstr_separate_by((n), (m))
#define v2_lstr_spr(n) v2_lstr_separate(n)

int v2_lstr_rback(str_lst_t **p_str);
int v2_lstr_free(str_lst_t **p_str);
str_lst_t *v2_lstr_rmv(str_lst_t *in_lstr); // Set is_rmv

str_lst_t *v2_lstr_add_tail(str_lst_t **in_parent, str_lst_t *in_tail); // Add in_tail at the end of in_parent list

str_lst_t *v2_lstr_find(str_lst_t *p_str, char *in_key, char *in_str); // Finds key/str pair
str_lst_t *v2_lstr_key(str_lst_t *p_str, char *in_key);            // Get str rec by key
char *v2_get_lstr_str(str_lst_t *p_str, char *in_key);                 // Get string content
void *v2_get_lstr_data(str_lst_t *p_str, char *in_key);                // Returns ->data if extsts
char *v2_lstr_str(str_lst_t *in_str);                                  // Returns ->str if exists, or ->key

// Moved to v2_lstr.fm_str()
// str_lst_t *v2_str_to_lstr(char *in_str, char spr);

int v2_lstr_rdir(str_lst_t **p_list, int (*filter)(char*,void*), void *in_data, char *in_dir, ...);

/* ====================================================================== */

#endif // _V2_LSTR_H
