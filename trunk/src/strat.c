/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#define BASEFILE "strat.c"

#include "common.h"
#include "strat.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

void copy_str(char * dst, int maxlen, const char * src, int start, int end)
{
  int len = end - start;
  if(start < 0 || len < 0) {
    *dst = '\0';
    return;
  }
  if(len >= maxlen)
    len = maxlen-1; // null-termination must always occur
  strncpy(dst, src+start, len);
  dst[len] = '\0';
}

#define _skip_line(ptr)                                                       \
  ({ while(*ptr && *ptr++ != '\n') /*empty*/; })

#define _skip_blanks(ptr)                                                     \
  ({ while(*ptr && *ptr == ' ') ptr++; })

void skip_line(const char ** str)
{
  const char * tmp = *str;
  _skip_line(tmp);
  *str = tmp;
}

void skip_blanks(const char ** str)
{
  const char * tmp = *str;
  _skip_blanks(tmp);
  *str = tmp;
}

int scan_multi(const char ** str, const char * search[], const int lens[], int nr)
{
  const char * tmp = *str;
  int i;
  for(;;) {
    _skip_blanks(tmp);
    if(!*tmp) {
      *str = tmp;
      return -1;
    }
    for(i = 0; i < nr; i++) {
      if(!strncmp(tmp, search[i], lens[i])) {
	tmp += lens[i];
	_skip_blanks(tmp);
	*str = tmp;
	return i;
      }
    }
    _skip_line(tmp);
  }
}

const char * scan_single(const char * str, const char * search, int searchlen)
{
  for(;;) {
    _skip_blanks(str);
    if(!*str) {
      return NULL;
    }
    if(!strncmp(str, search, searchlen)) {
      str += searchlen;
      _skip_blanks(str);
      return str;
    }
    _skip_line(str);
  }
}

const bool id_table[256] = {
  ['a'...'z'] = 1,
  ['A'...'Z'] = 1,
  ['0'...'9'] = 1,
  ['_'] = 1,
};

const bool num_table[256] = {
  ['0'...'9'] = 1,
};

const bool hex_table[256] = {
  ['0'...'9'] = 1,
  ['a'...'f'] = 1,
  ['A'...'F'] = 1,
  ['x'] = 1,
};

const bool ws_table[256] = {
  [' '] = 1,
  ['\t'] = 1,
  ['\n'] = 1,
};

const bool bl_table[256] = {
  [' '] = 1,
  ['\t'] = 1,
};

const bool nl_table[256] = {
  ['\n'] = 1,
};

#define SCAN_BODY(OP)                                                         \
  const char * tmp = *str;                                                    \
  int i;                                                                      \
  _skip_blanks(tmp);                                                          \
  *res = tmp;                                                                 \
  for(i = 0; tmp[i] && OP table[(unsigned)tmp[i]]; i++) {                     \
  }                                                                           \
  *str = tmp + i;                                                             \
  return i;

int scan_table(const char ** str, const bool table[], const char ** res)
{
  SCAN_BODY()
}

int scan_not_table(const char ** str, const bool table[], const char ** res)
{
  SCAN_BODY(!)
}

#define SCAN_BODY2(OP)                                                        \
  const char * tmp = *str;                                                    \
  int i;                                                                      \
  _skip_blanks(tmp);                                                          \
  for(i = 0; tmp[i] && OP table[(unsigned)tmp[i]]; i++) {                     \
  }                                                                           \
  if(i >= reslen) {                                                           \
    return -1;                                                                \
  }                                                                           \
  strncpy(res, tmp, i);                                                       \
  res[i] = '\0';                                                              \
  *str = tmp + i;                                                             \
  return i;

int scan_table_copy(const char ** str, const bool table[], char * res, int reslen)
{
  SCAN_BODY2()
}

int scan_not_table_copy(const char ** str, const bool table[], char * res, int reslen)
{
  SCAN_BODY2(!)
}

char scan_op(const char ** str)
{
  const char * tmp = *str;
  char res = *tmp++;
  while(*tmp && *tmp == '=') tmp++;
  *str = tmp;
  return res;
}

const char * strat_keywords[scod_max] = {
  "brick",
  "input",
  "output",
  "connect",
};

const int strat_keylens[scod_max] = {
  5,
  5,
  6,
  7,
};

bool scan_connstr(const char ** str, struct conn_info * conn)
{
  _skip_blanks((*str));
  int count = scan_table_copy(str, id_table, conn->conn_name, sizeof(conn->conn_name));
  if(**str == '[') {
    (*str)++;
    sscanf(*str, "%d", &conn->conn_index);
    while(**str && *(*str)++ != ']') {
      // empty
    }
  }
  if(**str == ',') {
    (*str)++;
  }
  if(!**str && count <= 0) {
    return FALSE;
  }
  return TRUE;
}

bool scan_connstraddr(const char ** str, struct conn_info * conn)
{
  _skip_blanks((*str));
  sscanf(*str, "%llx", &conn->conn_addr);
  const char * dummy;
  scan_table(str, hex_table, &dummy);
  if(**str != ':') {
    return FALSE;
  }
  (*str)++;
  return scan_connstr(str, conn);
}

/////////////////////////////////////////////////////////////////


// const char * rexstr_brick = "^brick *([:/])?= *(\\w*)";

int parse_brick(char * buf, int len, char * res_op, char * res_name, int len_name)
{
  const char * found = scan_single(buf, "brick", 5);
  if(!found) {
    return -1;
  }
  const char op_char = *found++;
  if(op_char == ':' || op_char == '/') {
    *res_op = op_char;
  } else {
    *res_op = 0;
  }
  if(*found == '=') {
    found++;
  }
  _skip_blanks(found);
  if(!*found) {
    return -1;
  }
  int res_len;
  for(res_len = 0; found[res_len] && found[res_len] != '\n'; res_len++) {
    // empty
  }
  copy_str(res_name, len_name, found, 0, res_len);
  return (long)found + res_len - (long)buf;
}

// const char * rexstr_connector = "^ *connect *(\\w+)(\\[([0-9]+)\\])? *([:/=])?= *(.*)";

int parse_connector(char * buf, int len, char * res_name, int len_name, index_t * res_index, char * res_op, char * res_other, int len_other, int * reslen_other)
{
  const char * found = scan_single(buf, "connect", 7);
  if(!found) {
    return -1;
  }
  int pos;
  for(pos = 0; found[pos] && ((found[pos] >= 'a' && found[pos] <= 'z') || found[pos] == '_'); pos++) {
    // empty
  }
  copy_str(res_name, len_name, found, 0, pos);
  found += pos;
  *res_index = 0;
  if(found[pos] == '[') {
    found++;
    *res_index = atoi(found);
    while(*found && *found++ != ']') {
      // empty
    }
  }
  _skip_blanks(found);
  const char op_char = *found++;
  if(op_char == ':' || op_char == '/') {
    *res_op = op_char;
  } else {
    *res_op = 0;
  }
  if(*found == '=') {
    found++;
  }
  _skip_blanks(found);
  if(!*found) {
    return -1;
  }
  int res_len;
  for(res_len = 0; found[res_len] && found[res_len] != '\n'; res_len++) {
    // empty
  }
  *reslen_other = res_len;
  copy_str(res_other, len_other, found, 0, res_len);
  return (long)found + res_len - (long)buf;;
}

// const char * rexstr_elem = "^([0-9a-f]+):(\\w+)(\\[[0-9]+\\])?,? *";

const char * parse_connstr(const char * ptr, struct conn_info * conn)
{
  int i;
  for(i = 0; ((ptr[i] >= 'a' && ptr[i] <= 'z') || ptr[i] == '_'); i++) {
    // empty
  }
  copy_str(conn->conn_name, sizeof(conn->conn_name), ptr, 0, i);
  ptr += i;
  if(*ptr == '[') {
    ptr++;
    sscanf(ptr, "%d", &conn->conn_index);
    while(*ptr && *ptr++ != ']') {
      // empty
    }
  }
  return ptr;
}

int parse_elem(char * buf, int len, struct conn_info * conn)
{
  const char * ptr = buf;
  _skip_blanks(ptr);
  if(!*ptr) {
    return -1;
  }
  sscanf(ptr, "%Lx", &conn->conn_addr);
  while(*ptr && *ptr++ != ':') {
    // empty
  }
  if(!*ptr) {
    return -1;
  }
  ptr = parse_connstr(ptr, conn);
  if(*ptr == ',') {
    ptr++;
  }
  _skip_blanks(ptr);
  return (long)ptr - (long)buf;
}

// const char * rexstr_inout = "^ *(in|out)put *([:/=])?= *(\\w+)(\\[[0-9]+\\])?";

int parse_inout(char * buf, int len, int * res_type, char * res_op, struct conn_info * conn)
{
  static const char * search[] = { "input", "output"};
  static const int slen[] = { 5, 6 };
  const char * found = buf;
  int code = scan_multi(&found, search, slen, 2);
  if(code < 0) {
    return -1;
  }
  *res_type = code;
  const char op_char = *found++;
  if(op_char == ':' || op_char == '/') {
    *res_op = op_char;
  } else {
    *res_op = 0;
  }
  if(*found == '=') {
    found++;
  }
  _skip_blanks(found);
  if(!*found) {
    return -1;
  }
  found = parse_connstr(found, conn);
  return (long)found - (long)buf;
}

