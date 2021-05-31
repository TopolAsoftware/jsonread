/*
 *  Copyright (c) 1997-2021 Oleg Vlasenko <vop@unity.net>
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


/*
 * Important note! This file contains code snippets by another authors.
 * Unfortunately, I have no information about all of them.
 * I believe these snippets are licensed for free use or public domain.
 * If not so, let me know.
 */


/*
 * The x2c(), unescape_url() and few more routines were lifted directly
 * from NCSA's sample program util.c, packaged with their HTTPD.
 */

/* ============================================== */


#define _GNU_SOURCE
#include "v2_util.h"
#include <errno.h>
#include <dirent.h>

// Special case
// ERROR_CODE 110XX
// ERROR_CODE 111XX
// ERROR_CODE 112XX
// ERROR_CODE 113XX
// ERROR_CODE 114XX
// ERROR_CODE 115XX

/* A basic set of various procedures and data types. */

// #define LF 10
// #define CR 13

char *v2_str_zero="";

int v2_debug=0; // Debug level for add debug
int v2_debug_time=0; // Print time before debug message
char *v2_debug_pref=NULL; // Prefix for debud strings. It better than previouse

int v_mon_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char nonc='\0';

/* ============================================== */
// Abort programm with error message
void v2_abort(char *in_msg) {
    fprintf(stderr, "%s\n", v2_st(in_msg, "V2_UTIL: Memory allocation error."));
    _exit(119);
}
/* ============================================== */
// Freed void pointer with checking and setting it to null
void v2_free(void **p_mem) {
    if(!p_mem || !*p_mem) return;
    free(*p_mem);
    *p_mem=NULL;
    return;
}
/* ============================================== */
// Freed string with checking and reseting
void v2_freestr(char **p_str) {
    if(!p_str || !*p_str) return;
    free(*p_str);
    *p_str=NULL;
    return;
}
/* ============================================== */
// Replace old allocated variable by alreasy allocated one
char *v2_letstr(char **p_str, char *in_str) { // Not allocated

    if(!p_str) return(in_str);
    if(*p_str) free(*p_str);
    return((*p_str=in_str));
}
/* ============================================== */
// Shift string to start on number positions
int v2_strshift(int pos, char *in_word) {
    int len=0;
    int x=0;

    if(in_word==NULL) return(0);

    len=strlen(in_word);

    if(pos<0) return(len);

    if(pos >= len) {
	in_word[0] = '\0';
	return(0);
    }

    len-=pos;

    while((in_word[x++] = in_word[pos++]));

    return(len); // Return letf chars
}

/* ============================================== */
// Return pointer to numbered token separated by dilemiter
char *v2_tok(int num, char *in_str, char dlm) {
    char *pnt=NULL;
    if(!dlm) dlm=' ';
    for(pnt=in_str; *pnt && num; pnt++) {
	if(*pnt==dlm) num--;
    }
    return(pnt);
}
/* ============================================== */
// Token starts from v2_toks(), and len is v2_toke() - v2_toks()
char *v2_toks(int num, char *in_str, char *in_dlm) {
    char *pnt=in_str;
    int is_dlm=1; // Means delimmiter already was

    if(!in_str) return(NULL);

    while(*pnt && num) {
	if(strchr(v2_st(in_dlm, " "), *pnt)) { // Delimiter
	    is_dlm=1;
	} else { // Argument
	    if(is_dlm) num--;
	    is_dlm=0;
	}
	if(num) pnt++;
    }
    return(*pnt?pnt:NULL);
}
/* ============================================== */
// Returns pointer to the first drlim char after token
char *v2_toke(char *in_tok, char *in_dlm) {
    char *pnt=in_tok;

    if(!in_tok) return(NULL);

    while(*pnt && !strchr(v2_st(in_dlm, " "), *pnt++));

    return(*pnt?pnt:NULL);
}
/* ============================================== */
// Returns signle token from string. Not detects too long tokens
char *v2_tok_r(char *sbuf, int slen, int tok_num, char *in_str, char *in_dlm) {
    char *p, *e;
    int len=0;

    if(!sbuf) return(NULL);

    sbuf[0] = '\0';

    if(!slen || !tok_num || !in_str || !in_str[0]) return(sbuf);

    if(!(p=v2_toks(tok_num, in_str, in_dlm)))      return(sbuf);

    if((e=v2_toke(p, in_dlm))) {
	len=e-p; // strlen + 1 symbol
    } else {
	len=strlen(p)+1; // Include \n
    }

    if(len>slen) len=slen; // Restrict string len to buffer slen

    snprintf(sbuf, len, "%s", p);

    return(sbuf);
}
/* ============================================== */
// Get next word from line. Looks like rework from Apache.
int v2_getword(char *word, char *line, char stop) {
    int x = 0,y,l;

    if(line==NULL) return(0);

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    l=x;

    if(line[x]) ++x;
    y=0;

    while((line[y++] = line[x++]));
    return(l); // Word len
}

/* ============================================== */
// Like getword, but with string markers
int v2_getnetxarg(char *word, char *line) {
    int x, y, l;
    char stop=' ';

    if(line[0] == '"') stop='"';
    if(line[0] == '[') stop=']';
    l=v2_getword(word, line, stop);

    if(line[0] == ' ') {
	y=0; x=1;
	while((line[y++] = line[x++]));
    }
    return(l);
}

/* ============================================== */
// Allocate word from line. Guess from Apache.
char *makeword(char *line, char stop) {
    int x = 0,y;
    char *word = (char *) malloc(sizeof(char) * (strlen(line) + 1));

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while((line[y++] = line[x++]));
    return word;
}

/* ============================================== */
// Make word from input stream. Guess from Apache.
char *fmakeword(FILE *f, char stop, int *cl) {
    int wsize;
    char *word;
    int ll;

    wsize = 102400;
    ll=0;
    word = (char *) malloc(sizeof(char) * (wsize + 1));

    while(1) {
        word[ll] = (char)fgetc(f);
        if(ll==wsize) {
            word[ll+1] = '\0';
            wsize+=102400;
            word = (char *)realloc(word,sizeof(char)*(wsize+1));
        }
        --(*cl);
        if((word[ll] == stop) || (feof(f)) || (!(*cl))) {
            if(word[ll] != stop) ll++;
            word[ll] = '\0';
	    word = (char *) realloc(word, ll+1);
            return word;
        }
        ++ll;
    }
}

/* ============================================== */
// hex to char
char x2c(char *inps) {
    register char outc;

    outc = (inps[0] >= 'A' ? ((inps[0] & 0xdf) - 'A')+10 : (inps[0] - '0'));
    outc *= 16;
    outc += (inps[1] >= 'A' ? ((inps[1] & 0xdf) - 'A')+10 : (inps[1] - '0'));
    return(outc);
}
/* ============================================== */
// Unescape URL symbols
void unescape_url(char *url) {
    register int x,y;

    if(!url || !url[0]) return;

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
	}
    }
    url[x] = '\0';
}

/* ============================================== */
// Unescape URL space
void plustospace(char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
    return;
}

/* ============================================== */
// Code to hex
char to_hex(char code) {
  static char hex[] = "0123456789ABCDEF";
  return hex[code & 0x0f];
}
/* ============================================== */
// Escape URL symbols. Guess from Apache.
char *escape_url(char *in_str) {
    static char strtmp[MAX_STRING_LEN];
    int x=0;
    int y=0;

    while(in_str[x]) {
	if(isalnum(in_str[x]) || in_str[x] == '-' || in_str[x] == '_' || in_str[x] == '.' || in_str[x] == '~') {
            strtmp[y++] = in_str[x++];
	} else if(in_str[x] == ' ') {
            strtmp[y++] = '+'; x++;
	} else {
	    strtmp[y++]='%'; strtmp[y++]=to_hex(in_str[x] >> 4), strtmp[y++]=to_hex(in_str[x] & 15); x++;
	}
    }
    strtmp[y]='\0';
    return(strtmp);
}
/* ============================================== */
// Escape quotas symbols and spaces
char *escape_url_quota(char *in_str) {
    char strtmp[strlen(in_str)*2];
    int res=0;
    int x;

    if(in_str == NULL) return("");

    for(x=0; in_str[x]; x++) {
	if(in_str[x]=='"') {
	    res+=sprintf(strtmp+res, "&quot;");
	} else if(in_str[x]==' ') {
	    res+=sprintf(strtmp+res, "%s", "%20");
	} else {
	    strtmp[res++]=in_str[x];
	}
    }
    strtmp[res]='\0';
    return(res?v2_strcpy(strtmp):"");
}

/* ============================================== */
// Get line from the line. Rework.
int v2_getline(char *s, int n, FILE *f) {
    register int i=0;
    char rt='\0';

    s[0]='\0';
    if(!n) return(0);
    while(1) {
	s[i] = '\0';
	if(i == (n-1)) return(0);
	rt = (char)fgetc(f);
	if(feof(f)) return(i?0:1);
	if(rt == 0x0d) {
	    rt = (char)fgetc(f);
	    if(feof(f)) return(0);
	}
	if((rt == 0x4) || (rt == 0x0a) || (i == (n-1))) {
            return (0);
	}
	s[i] = rt;
	++i;
    }
    return(0);
}

/* ============================================== */
// Put line to file. Opposite to printf, this one puts w/o formating
void v2_putline(FILE *f,char *l) {
    int x;

    for(x=0;l[x];x++) fputc(l[x],f);
    fputc('\n',f);
}

/* ============================================== */
// Copy chars from stream to stream.
void send_fd(FILE *f, FILE *fd) {
    char c;

    while (1) {
        c = fgetc(f);
        if(feof(f)) return;
        fputc(c,fd);
    }
     return;
}

/* ================================================================= */
// Purge taggs from the string.
char *v2_untag(char *in_buf, int in_size, char *in_str) {
    int x=0;
    int y=0;
    int is_tag=0;

    if(!in_buf) {nonc='\0'; return(&nonc);} // Do not return constant string "", but variable

    in_buf[0]='\0';
    if(!in_str || !*in_str) return(in_buf);

    while(in_str[y] && (y < MAX_STRING_LEN)) {
	if(is_tag) {
	    if(in_str[y]=='>') is_tag=0;
	    y++;
	    continue;
	}
	if(in_str[y]=='<') {
	    is_tag=1;
	    y++;
	    continue;
	}
	in_buf[x++]=in_str[y++];
    }
    in_buf[x]='\0';

    if(is_tag) return(in_str); // Not closed tag - return original string

    return(in_buf);
}
/* ================================================================= */
// The same as above, but inside of the string - not good function, rewrite it
char *v2_filter_tags(char *in_str) {
    char strtmp[MAX_STRING_LEN];
    int x=0;
    int y=0;
    int is_tag=0;

    if(!in_str || !in_str[0]) return("");

    while(in_str[y]) {
	if(is_tag) {
	    if(in_str[y]=='>') is_tag=0;
            y++;
            continue;
	}
	if(in_str[y]=='<') {
	    is_tag=1;
	    y++;
	    continue;
	}
	strtmp[x++]=in_str[y++];

    }
    strtmp[x]='\0';

    if(!strtmp[0]) return("");
    return(v2_strcpy(strtmp));
}
/* ========================== xutil ========================= */
// Formatted open for read
FILE *xfopen_read(char *file, ...) {
    //va_list vl;
    char strtmp[MAX_STRING_LEN];

    if(!file || !file[0]) return(NULL);

    //va_start(vl, file);
    //vsprintf(strtmp, file, vl);
    //va_end(vl);

    VL_STR(strtmp, MAX_STRING_LEN, file);

    errno=0;
    return(fopen(strtmp, "r"));
}

/* ============================================== */
// Formatted open for write
FILE *xfopen_write(char *file, ...) {
    //va_list vl;
    char strtmp[MAX_STRING_LEN];

    if(!file || !file[0]) return(NULL);

    //va_start(vl, file);
    //vsprintf(strtmp, file, vl);
    //va_end(vl);

    VL_STR(strtmp, MAX_STRING_LEN, file);

    errno=0;
    return(fopen(strtmp, "w"));
}

/* ============================================== */
/* ============================================== */
// Use to print/cat file
// sfread(&sfread_strprn, NULL, filename
int sfread_print(char *in_str, void *in_data) {
    if(in_str) printf("%s\n", in_str);
    return(0);
}
/* ============================================== */
// Universal file reading function
// [#][!][:|-](file_name)
int sfread(int (*rd_str)(char*, void*), void *pass_data, char *file) {
    FILE *cf=NULL;
    char strtmp[MAX_STRING_LEN];
    int no_comm=0; // No comments
    int no_found=0; // No found is not error
    //int is_fp=0; // Is file == or pipe == 2
    int is_exec=0;
    int rc=0;

    if(!rd_str)         return(XFREAD_NOT_FUNC);
    if(!file || !*file) return(XFREAD_NOT_NAME);

    //if(*file == ';') { ++file; no_comm=2;  }
    if(*file == '#') { ++file; no_comm=1;  }
    if(*file == '!') { ++file; no_found=1; } // Current variant

    if(!strcmp(file, "-")) {
	cf=stdin;
    } else if(*file==':') {
	if(!(cf=popen(++file, "r"))) return(XFREAD_NOT_RUN);
	is_exec=1;
    } else if(!(cf=fopen(file, "r"))) {
	if(errno != ENOENT)	return(XFREAD_NOT_ACCESS);
	if(no_found) return(0); // Not found is not error!
	return(XFREAD_NOT_FOUND);
    }

    while(!v2_getline(strtmp, MAX_STRING_LEN, cf) && !rc) {
	if(no_comm) {
	    if(strtmp[0] == '\0') continue;
	    if(strtmp[0] == ';')  continue;
	    if(strtmp[0] == '#') continue;
	}
	rc=rd_str(strtmp, pass_data);
    }

    if(cf == stdin) return(rc);

    if(is_exec) { // File
	int rc1=0;
	int status=pclose(cf);
	if(WIFEXITED(status)) rc1=WEXITSTATUS(status);
	return(rc1?rc1:rc);
    }

    if(fclose(cf)) return(XFREAD_NOT_CLOSE);

    return(rc);
}
/* ============================================== */
// Formated variant of sfread()
int sfreadf(int (*rd_str)(char*, void*), void *pass_data, char *file, ...) {
    char *tfile=NULL;
    va_list vl;
    int rc=0;

    va_start(vl, file);
    rc=vasprintf(&tfile, file, vl);
    va_end(vl);

    if(rc==-1) return(XFREAD_NOT_NAME); // Suspect filename is NULL

    rc=sfread(rd_str, pass_data, tfile);

    free(tfile);

    return(rc);
}
/* ============================================== */
// Universal file write function
int sfsave(int (*wr_fun)(FILE*, void*), void *pass_data, char *file) {
    FILE *cf=NULL;
    char *t_name=NULL;
    int res=0;
    int rc=0;

    if(!file || !file[0]) return(XFREAD_NOT_NAME);
    if(!wr_fun)           return(XFREAD_NOT_FUNC);

    t_name=v2_string("%s.tmp_%d", file, (int)getpid());

    errno=0;
    if(!(cf=fopen(t_name, "w"))) {
	v2_freestr(&t_name);
	return(XFREAD_NOT_WRITE);
    }

    res=wr_fun(cf, pass_data);

    errno=0;
    if(fclose(cf)) {
	rc=XFREAD_NOT_CLOSE;
    } else {
	rc=v2_vop_rename(res, t_name, file);
    }

    v2_freestr(&t_name);

    return(rc);

}
/* ============================================== */
// Formatted variant of previouse one
int sfsavef(int (*wr_fun)(FILE*, void*), void *pass_data, char *in_file, ...) {
    va_list vl;
    char *nfile=NULL;
    int rc=0;

    if(!in_file || !*in_file) return(XFREAD_NOT_NAME);
    if(!wr_fun)               return(XFREAD_NOT_FUNC);

    va_start(vl, in_file);
    rc=vasprintf(&nfile, in_file, vl);
    va_end(vl);

    if(rc == -1) return(XFREAD_NOT_OTHER);

    rc=sfsave(wr_fun, pass_data, nfile);
    v2_freestr(&nfile);
    return(rc);
}
/* ============================================== */
// Returns most common errors of read/save functions. Use also errno as additional one
char *v2_xrc(int code) {
    static char strerr[30];

    if(code == XFREAD_NOT_RUN)    return("Can run funct");
    if(code == XFREAD_NOT_CLOSE)  return("Unable to close");
    if(code == XFREAD_NOT_FOUND)  return("File not found");
    if(code == XFREAD_NOT_ACCESS) return("Access denied");
    if(code == XFREAD_NOT_NAME)   return("Empty file name");
    if(code == XFREAD_NOT_WRITE)  return("Can not write");
    if(code == XFREAD_NOT_RENAME) return("Can not rename");
    if(code == XFREAD_NOT_FUNC)   return("Ext funct empty");
    if(code == XFREAD_NOT_FILE)   return("This is not readable file");

    if(code == XFREAD_NOT_RENAME_SIZE) return("Can not rename, wrong size");
    if(code == XFREAD_NOT_RENAME_NOTF) return("Can not rename, temp file absent");
    if(code == XFREAD_NOT_READ_SIZE)   return("Red size don't match file size");
    if(code == XFREAD_NOT_WRITE_SIZE)  return("Wrote data size don't match input size");

    if(code == V2_MALLOC_ERROR)   return("Malloc error");

    return(v2_s(strerr, 30, "xrc=%d", code));
}
/* ============================================================== */
// Print file
int v2_cat(char *in_file, ...) {
    struct stat st;
    FILE *cf=NULL;
    char strtmp[MAX_STRING_LEN];
    void *rbuf;
    size_t result;

    VL_STR(strtmp, 0, in_file);

    if(stat(strtmp, &st))          return(XFREAD_NOT_FOUND);
    if(st.st_size==0)              return(V2_SIZE_ZERO);
    if(!(rbuf=malloc(st.st_size))) return(V2_MALLOC_ERROR);
    if(!(cf=fopen(strtmp, "r")))   return(XFREAD_NOT_ACCESS);

    result=fread(rbuf, 1, st.st_size, cf);
    if(result == -1)               return(XFREAD_NOT_READ_ERROR);
    if(result != st.st_size)       return(XFREAD_NOT_READ_SIZE);

    if(fclose(cf))                 return(XFREAD_NOT_CLOSE);

    result=fwrite((const void *)rbuf, 1, st.st_size, stdout);
    if(result != st.st_size)       return(XFREAD_NOT_WRITE_SIZE);

    free(rbuf);

    return(0);
}
/* ============================================================== */
// Check and create directory, change mode for existed
// if in_dir = "+/something..." Only Create directory, but not change, if exists
int v2_dir(mode_t mode, char *in_dir, ...) {
    va_list vl;
    struct stat st;
    char tmp[MAX_STRING_LEN];
    int is_cre=0; // 1 - means only new direcory needs

    if(!v2_is_par(in_dir)) return(1);

    if(in_dir[0] == '+') is_cre=1;

    va_start(vl, in_dir);
    vsnprintf(tmp, MAX_STRING_LEN, in_dir+is_cre, vl);
    va_end(vl);

    if(!stat(tmp, &st)) { // OK! Found
	//printf("<br><br>Dir: %s<br>time = %ld<br>%s\n", tmp, st.st_mtime, strerror(errno));
	if(is_cre)                         return(0); // Already exists - do nothing
	if(!S_ISDIR(st.st_mode))           return(2); // Not directory
        if(!mode)                          return(0); // OK, not need check mode
	if(mode == (st.st_mode | ~S_IFMT)) return(0); // Mode OK - ??
	if(chmod(tmp, mode))               return(3); // Mode ready
    } else { // Not found
	if(mkdir(tmp, mode?mode:0755))     return(4);
    }

    return(0);
}
/* ============================================================== */
// Returns value and restrict it bitween min and max
// If not number at in_str - returns 0
int v2_atoir(char *in_str, int min, int max) {
    int out=0;

    if(!in_str || !*in_str) return(0);

    out=atoi(in_str);

    if(min && (out < min)) return(min);
    if(max && (out > max)) return(max);
    return(out);
}
/* ============================================================== */
// Prints in_bin with size in_size to in_buf. In_buf shoulf be at least 2*in_size+1
char *v2_prnhex(char *in_buf, size_t in_size, char *in_bin) { // Prints binary buffer as a hex
    int x=0;

    if(!in_buf || !in_size || !in_bin) return("");

    for(x=0; x<in_size; x++) sprintf(in_buf+x*2, "%02x", in_bin[x]);

    return(in_buf);
}
/* ============================================================== */
char *v2_st(char *in_str, char *in_rpl) { // Show string or replacement
    if(in_str && *in_str) return(in_str);
    return(in_rpl);
}
/* ============================================================== */
char *v2_stf(char *in_str, char *in_rpl, ...) { // Show string or replacement
    static char strtmp[MAX_STRING_LEN];
    if(in_str && *in_str) return(in_str);
    VL_STR(strtmp, MAX_STRING_LEN, in_rpl);
    return(strtmp);
}
/* ============================================================== */
// For web input options
char *v2_winput(char *in_str) {
    static char strtmp[MAX_STRING_LEN];
    int res=0;
    int x;

    if(in_str==NULL) return(""); // NULL not returned.

    for(x=0; in_str[x] && (x<MAX_STRING_LEN); x++) {
	if(in_str[x] =='"') {
            res+=sprintf(strtmp+res, "%s", "&quot;");
	} else if(in_str[x] =='\'') {
	    res+=sprintf(strtmp+res, "%s", "&#39;");
	} else if(in_str[x] =='<') {
            res+=sprintf(strtmp+res, "%s", "&lt;");
	} else if(in_str[x] =='>') {
            res+=sprintf(strtmp+res, "%s", "&gt;");
	} else {
	    strtmp[res++]=in_str[x];
	}
    }

    strtmp[res]='\0';
    return(strtmp);
}
/* ============================================================== */
char *v2_toupper(char *in_str) {
    int x;

    if(in_str) {
	for(x=0; in_str[x]; x++) in_str[x]=toupper(in_str[x]);
    }

    return(in_str);
}
/* ===================================================================== */
// Compare ctrings counting NULL
int v2_strcmp(char *in_str1, char *in_str2) {
    if(!in_str1) {
	if(!in_str2) return(0); // Equal
	return(-1); // NULL less than str
    } else if(!in_str2) {
	return(1); // str greater than NULL
    }
    return(strcmp(in_str1, in_str2));
}
/* ============================================================== */
int v2_strcmpf(char *in_str, char *in_fmt, ...) {
    char tmp[MAX_STRING_LEN];

    VL_STR(tmp, MAX_STRING_LEN, in_fmt);
    return(v2_strcmp(in_str, tmp));
}
/* ============================================================== */
// Create Temporal String
char *v2_strtmp(char *in_format, ...) {
    static char tmp[MAX_STRING_LEN];

    VL_STR(tmp, MAX_STRING_LEN, in_format);

    return(tmp);
}
/* ============================================================== */
char *v2_s(char *in_buf, size_t buf_len, char *in_format, ...) {

    if(!in_buf) {nonc='\0'; return(&nonc);}

    VL_STR(in_buf, buf_len, in_format);

    return(in_buf);
}
/* ============================================================== */
// strtmp toupper
char *v2_sup(char *in_buf, size_t buf_len, char *in_format, ...) {

    VL_STR(in_buf, buf_len, in_format);

    return(v2_toupper(in_buf));
}
/* ============================================================== */
char *v2_string(char *format, ...) {
    va_list vl;
    char *tmp=NULL;
    int rc=0;

    if(!format || !format[0]) return(NULL);

    va_start(vl, format);
    rc=vasprintf(&tmp, format, vl);
    va_end(vl);

    if(rc == -1) return(NULL);

    return(tmp);
}
/* ============================================================== */
char *v2_strcpy(char *in_str) {
    return(v2_let_var(NULL, in_str));
}
/* ============================================================== */
// Returns value from VAR, and let var = NULL
char *v2_move(char **p_str) {
    char *p=NULL;
    if(p_str) {
	p=*p_str;
	*p_str=NULL;
    }
    return(p);
}
/* ============================================================== */
// Return allocated copy of in_var + write it at pvar pointer
char *v2_let_var(char **pvar, char *in_val) { // Let (set) var, freed ir before
    char *tmp=NULL;

    if((in_val == NULL) || (*in_val == '\0')) {
	v2_freestr(pvar);
	return(NULL);
    }

    if(!(tmp=strdup(in_val))) v2_abort(NULL);

    return(v2_letstr(pvar, tmp));
}
/* ============================================================== */
// Returns allocated string not longer than in_len + '\0'
char *v2_let_varn(char **pvar, char *in_val, size_t in_len) { // Let (set) var, freed it before
    char *tmp=NULL;

    if(!in_val || !*in_val)       return(v2_let_var(pvar, NULL)); // Clean var if exists and return

    if(!(tmp = calloc(in_len+1, 1))) v2_abort("No memory for v2_util."); // Have to zerro it
	
    strncpy(tmp, in_val, in_len);

    return(v2_letstr(pvar, tmp));
}
/* ============================================================== */
// Returns allocated string upto stop char exclude it + \0 at the end
char *v2_let_varc(char **pvar, char *in_val, char stop) { // Let (set) var, freed it before
    char *p=NULL;
    
    if(!in_val || !*in_val)       return(v2_let_var(pvar, NULL)); // Clean var if exists and return
    if(!(p=strchr(in_val, stop))) return(v2_let_var(pvar, in_val)); // Return string w/o stop char

    return(v2_let_varn(pvar, in_val, p-in_val));
}
/* ============================================================== */
// Formatted addon to var
char *v2_let_varf(char **pvar, char *format, ...) {
    va_list vl;
    char *tmp=NULL;
    int rc=0;

    if(format && format[0]) {
	va_start(vl, format);
	rc=vasprintf(&tmp, format, vl);
	va_end(vl);
    }

    if(rc == -1) tmp=NULL;

    if(pvar) {
	v2_freestr(pvar);
	(*pvar)=tmp;
    }
    return(tmp);
}
/* ============================================================== */
// Return allocated copy of in_var + write it at pvar pointer if var empty
char *v2_let_if_var(char **pvar, char *in_val) { // Let (set) var, freed ir before

    if(!pvar) return(NULL);
    if(*pvar) return(NULL);

    return(v2_let_var(pvar, in_val));
}
/* ============================================================== */
// Return allocated copy of in_var + write it at pvar pointer if var empty
char *v2_let_if_varf(char **pvar, char *format, ...) { // Let (set) var, freed ir before
    va_list vl;
    int rc=0;

    if(!pvar) return(NULL);
    if(*pvar) return(NULL);

    if(format && format[0]) {
	va_start(vl, format);
	rc=vasprintf(pvar, format, vl);
	va_end(vl);
    }
    if(rc == -1) *pvar=NULL;
    return(*pvar);
}
/* ============================================================== */
// Replace templated string by value
// v2_let_tplf(&my_var, "value", "{subid%d}", 3);
char *v2_let_tplf(char **p_var, char *in_value, char *in_tpl, ...) {
    char out[MAX_STRING_LEN*3];
    char tpl[MAX_STRING_LEN];
    char *str=NULL;
    char *pnt=NULL;
    int cnt=0;
    int len=0;

    if(!p_var || !*p_var) return(NULL);
    str=*p_var;

    VL_STR(tpl, MAX_STRING_LEN, in_tpl);

    if(!(len=strlen(tpl))) return(*p_var);

    while((pnt=strstr(str, tpl))) {
	snprintf(out+cnt, MAX_STRING_LEN-cnt, "%s", str);

	cnt+=pnt-str;
	out[cnt]='\0';

	cnt+=snprintf(out+cnt, MAX_STRING_LEN-cnt, "%s", v2_nn(in_value));
	str=pnt+len;
    }

    return(v2_let_varf(p_var, "%s%s", out, str));
}
/* ==================================================================== */
// Config functions

/* ==================================================================== */
char *v2_cf(char *cf_str, char* var_name) {
    int nam_len=0;
    char *tmp=NULL;

    if(!cf_str || !*cf_str || !var_name)   return(NULL);

    if(!(nam_len=strlen(var_name)))        return(NULL);
    //if(!(tmp=strchr(cf_str, ' '))) {
    if(!(tmp=strpbrk(cf_str, " \t"))) {
	if(!strcmp(cf_str, var_name))      return(""); // Cfg str w/o argument
	return(NULL);
    }
    if(nam_len != (tmp-cf_str))            return(NULL);
    if(strncmp(cf_str, var_name, nam_len)) return(NULL);

    tmp+=strspn(tmp, " \t");

    //while((*tmp==' ') || (*tmp=='\t')) tmp++;

    return(tmp);
}
/* ==================================================================== */
// Returns pointer to new var value, if OK, else 0
// If varname like "+vaname", do not replace variable value if exists
char *v2_cf_var(char **pvar, char *cf_str, char *var_name) {
    char *tmp=NULL;
    int no_repl=0; // No replace if exists

    if(!pvar)      return(NULL); // Nothing to read
    if(!var_name)  return(NULL); // Nothing to select

    if(*var_name == '+') no_repl=1;

    if(!(tmp=v2_cf(cf_str, var_name+no_repl)) || !*tmp) return(NULL);

    if(no_repl && *pvar) return(v2_strcpy(tmp));
    return(v2_let_var(pvar, tmp));
}
/* ==================================================================== */
int v2_cf_int(int *ivar, char *cf_str, char *var_name) {
    char *tmp=NULL;

    if(!ivar)                           return(0);

    if(!(tmp=v2_cf(cf_str, var_name)) || !*tmp) return(0); // Not var found
    *ivar=atoi(tmp);

    return(1); // OK
}
/* ==================================================================== */
// Have to be rebuild for  range
int v2_cf_prc(int *ivar, char *cf_str, char *var_name) {

    if(!v2_cf_int(ivar, cf_str, var_name)) return(0); // Not our variable

    if(*ivar < 0 )   *ivar = 0;
    if(*ivar > 100 ) *ivar = 100;

    return(1); // OK
}
/* ==================================================================== */
int v2_cf_long(long *lvar, char *cf_str, char *var_name) {
    char *pvar=NULL;

    if(!lvar)                           return(0);

    if(!(pvar=v2_cf(cf_str, var_name)) || !*pvar) return(0); // Not var found

    *lvar=strtol(pvar, (char**)NULL, 10);

    return(1); // OK
}
/* ==================================================================== */
int v2_cf_u64(uint64_t *uvar, char *cf_str, char *var_name) {
    char *pvar=NULL;

    if(!uvar)                           return(0);

    if(!(pvar=v2_cf(cf_str, var_name)) || !*pvar) return(0); // Not var found

    *uvar=strtoull(pvar, (char**)NULL, 10);

    return(1); // OK
}
/* ==================================================================== */
int v2_cf_flag(int *iflag, char *cf_str, char *var_name) { // Just set flag to 1 if config present
    //char *pvar=NULL;

    if(!v2_cf(cf_str, var_name)) return(0);

    //if(!(pvar=v2_cf(cf_str, var_name))) {
    //    if(v2_strcmp(cf_str, var_name)) return(0);
    //}
    if(iflag) return((*iflag=1));
    return(1);
}
/* ==================================================================== */
/* ==================================================================== */
// Easy function to check string param = 1 if param OK
char *v2_is_par(char *in_par) {
    if(!in_par)    return(NULL);
    if(!*in_par)   return(NULL);
    return(in_par);
}
/* ==================================================================== */
// Easy function to check string value, OK if exists and doen not eq "-"
char *v2_is_val(char *in_val) {
    if(!in_val)                      return(NULL);
    if(!*in_val)                     return(NULL);
    if((*in_val=='-') && !in_val[1]) return(NULL); // == "-"
    return(in_val);
}
/* ==================================================================== */
// sflg - special flag value:
// sflg_hard_off == 0 - hard off (can not be set on)
// sflg_soft_off == 1 - soft off (can be switched to on)
// sflg_soft_on  == 2 - soft on (can be swithed off)
// sflg_hard_on  == 3 - hard on (always turned on)

/* ==================================================================== */
// Returns 1 if sflg can be switched
int v2_sflg_isvar(sflg_t in_sflg) {
    if(in_sflg == sflg_soft_off) return(1);
    if(in_sflg == sflg_soft_on)  return(1);
    return(0);
}
/* ==================================================================== */
// Make sflg soft what can be switched
int v2_sflg_var(sflg_t *p_sflg) {
    if(!p_sflg) return(17800);
    if(*p_sflg == sflg_hard_off) *p_sflg=sflg_soft_off;
    if(*p_sflg == sflg_hard_on)  *p_sflg=sflg_soft_on;
    return(0);
}
/* ==================================================================== */
// Returns 1 if sflg set on
int v2_sflg_isset(sflg_t in_sflg) {
    if(in_sflg==sflg_soft_on) return(1);
    if(in_sflg==sflg_hard_on) return(1);
    return(0);
}
/* ==================================================================== */
// Set soft flag
int v2_sflg_set(sflg_t *p_sflg) {
    if(!p_sflg)                 return(17801);
    if(!v2_sflg_isvar(*p_sflg)) return(17802); // Can not be switched
    *p_sflg=sflg_soft_on;
    return(0);
}
/* ==================================================================== */
// ReSet soft flag
int v2_sflg_reset(sflg_t *p_sflg) {
    if(!p_sflg)                 return(17805);
    if(!v2_sflg_isvar(*p_sflg)) return(17806); // Can not be switched
    *p_sflg=sflg_soft_off;
    return(0);
}

/* ==================================================================== */
// fFalg
/* ==================================================================== */
static int fun_fflag(FILE *in_cf, void *no_data) {
    return(fprintf(in_cf, "AutoCreated Flag file at %s", v2_get_date(0)));
}
/* ==================================================================== */
int v2_fflag(char *f_name, ...) { // Set file flag
    char strtmp[MAX_STRING_LEN];
    va_list vl;
    int rc=0;

    if(!f_name || !f_name[0]) return(0);
    va_start(vl, f_name);
    vsnprintf(strtmp, MAX_STRING_LEN, f_name, vl);
    va_end(vl);

    rc=sfsave(&fun_fflag, NULL, strtmp);

    return(rc);
}
/* ==================================================================== */
int v2_is_fflag(char *f_name, ...) { // Check file flag.
    struct stat st;
    va_list vl;
    char strtmp[MAX_STRING_LEN];

    if(!f_name || !f_name[0]) return(0);
    va_start(vl, f_name);
    vsnprintf(strtmp, MAX_STRING_LEN, f_name, vl);
    va_end(vl);

    if(stat(strtmp, &st)) return(0); // Not found - not flag

    unlink(strtmp);

    return(1);
}
/* ==================================================================== */
// Returns 0 - if flag not exists, or too old. and 1 - if flag exists and good
int v2_old_fflag(time_t in_tout, char *f_name, ...) { // Check if flag already older than in_tou
    struct stat st;
    va_list vl;
    char strtmp[MAX_STRING_LEN];

    if(!f_name || !f_name[0]) return(0);
    va_start(vl, f_name);
    vsnprintf(strtmp, MAX_STRING_LEN, f_name, vl);
    va_end(vl);

    if(stat(strtmp, &st)) return(0); // Not found - not flag

    if(st.st_mtime+in_tout < time(NULL)) return(0); // To old == not found

    return(1);
}
/* ==================================================================== */
// Dates
/* ======================================================== */
int v2_get_mon_days(int month, int year) {
    int days=0;

    if(month<1)  return(31); // The same as Jan
    if(month>12) return(31); // The same as Dec

    days=v_mon_days[month-1];

    if(year != 0) {
	if((month == 2) && (year%4 == 0) && ((year%100 != 0) || (year%400 == 0))) return(29);
    }
    return(days);
}
/* ============================================================== */
char *v2_get_date(time_t in_time) {
    struct tm *ptm;
    char strtmp[MAX_STRING_LEN];

    if(!in_time) in_time=time(NULL);
    if((ptm=localtime(&in_time))) {
	strftime(strtmp, MAX_STRING_LEN, "%d-%b-%Y %H:%M:%S", ptm);
    } else {
	snprintf(strtmp, MAX_STRING_LEN, "[date_error]");
    }
    return(v2_strcpy(strtmp));
}
/* ===================== date_util ================ */
// Gives datestr from YYYYMMDD
char *v2_give_date_str(int in_dat) {
    //char strtmp[MAX_STRING_LEN];
    int i_year=0;
    int i_month=0;
    int i_day=0;

    char *mont[]={
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


    if(!in_dat) return("zerro");

    i_year=in_dat/10000;
    if((i_year<1900) || (i_year>2100))  return("unkn_year");
    i_month=(in_dat%10000)/100;
    if((i_month<1) || (i_month>12))  return("unkn_month");
    i_day=(in_dat%100);
    if((i_day<1) || (i_day>31))  return("unkn_day");

    //sprintf(strtmp, "%02d-%s-%04d", i_day, mont[i_month-1], i_year);
    return(v2_string("%02d-%s-%04d", i_day, mont[i_month-1], i_year));
}
/* ===================== date_util ================ */
int givemonth(char *smonth) {

    if(!strcmp(smonth, "Jan")) { return 1; }
    else if(!strcmp(smonth, "Feb")) { return 2; }
    else if(!strcmp(smonth, "Mar")) { return 3; }
    else if(!strcmp(smonth, "Apr")) { return 4; }
    else if(!strcmp(smonth, "May")) { return 5; }
    else if(!strcmp(smonth, "Jun")) { return 6; }
    else if(!strcmp(smonth, "Jul")) { return 7; }
    else if(!strcmp(smonth, "Aug")) { return 8; }
    else if(!strcmp(smonth, "Sep")) { return 9; }
    else if(!strcmp(smonth, "Oct")) { return 10; }
    else if(!strcmp(smonth, "Nov")) { return 11; }
    else if(!strcmp(smonth, "Dec")) { return 12; }
    return 0;

}

/* ======================================================= */
void v2_purge_spaces(char *in_str) {
    register int x;
    register int y;
    int is_sp=1;

    if(!in_str) return;
    if(!in_str[0]) return;

    for(x=0, y=0; (in_str[x]) && x<MAX_STRING_LEN; x++) {
        if((is_sp) && ((in_str[x] == ' ') || (in_str[x] == '\t'))) continue;
        if(in_str[x] == '\t') in_str[x] = ' ';
        in_str[y++] = in_str[x];
        if(in_str[x] == ' ') {is_sp=1;}
        else {is_sp=0;}
    }
    in_str[y] = '\0';

    while(y && in_str[y-1]==' ') in_str[--y]='\0';

    return;
}
/* ======================================================= */
int v2_skip_spaces(char *in_str) {
    int s_len=0;
    int x, y;

    if(!in_str || !in_str[0]) return(0);

    s_len=strlen(in_str);
    y=strspn(in_str, " \t");

    if(y==s_len) { // str contains only trim symbols
	*in_str='\0';
        return(0);
    }

    s_len-=y;
    x=0;
    while((in_str[x++]=in_str[y++]));

    return(s_len); // Returns lengh of sting
}
/* ======================================================= */
int v2_str_trim(char *in_str, char *in_trm) {
    int s_len=0;
    int x, y;

    if(!in_str || !*in_str) return(0);
    if(!in_trm || !*in_trm) return(0);

    s_len=strlen(in_str);
    y=strspn(in_str, in_trm);

    if(y==s_len) { // str contains only trim symbols
	*in_str='\0';
        return(0);
    }

    s_len-=y;
    x=0;
    while((in_str[x++]=in_str[y++]));

    y=s_len;
    while(strchr(in_trm, in_str[y-1]) && (y>=0)) y--;

    in_str[y]='\0';
    return(0);
}
/* ======================================================= */
int v2_vop_rename(off_t f_size, char *t_name, char *f_name) {
    struct stat st;

    if(!t_name || !*t_name || !f_name || !*f_name) return(XFREAD_NOT_NAME);

    if(stat(t_name, &st)) { // errno
        unlink(t_name); // ???? File not found!
        return(XFREAD_NOT_RENAME_NOTF);
    }

    if(st.st_size != f_size) {
        unlink(t_name); // errno
        return(XFREAD_NOT_RENAME_SIZE);
    }

    if(rename(t_name, f_name)) {
        unlink(t_name); // errno
	return(XFREAD_NOT_RENAME);
    }
    return(0);
}
/* ========================================================= */
/* ========================================================= */
char **v2_argv(char *in_str) {
    char **outv=NULL;
    char *buf=NULL;
    int cnt=2; // + Argv[0] + NULL =  size of vector
    int len=0;
    int x, y;

    if((len=strlen(v2_nn(in_str))) == 0) return(NULL);

    for(x=0; in_str[x]; ++x) {
	if(in_str[x] == ' ') ++cnt;
    }

    cnt *= sizeof(char**);

    if(!(outv=(char**)calloc(cnt+len+1, 1))) return(NULL);

    buf=(char*)(outv+cnt);

    outv[0] = buf; // Argv[0]

    strcpy(buf, in_str);

    for(x=0, y=0; buf[x]; ++x) {
	if(buf[x] != ' ') continue;
	buf[x] ='\0';
	outv[++y] = buf+x+1;
    }

    return(outv);
}
/* ========================================================= */
// Returns number of arguments
int v2_argc(char **in_argv) {
    int out=0;

    if(!in_argv) return(0); // Not any argv

    while(*(in_argv+out)) ++out;

    return(out);
}
/* ========================================================= */
