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

#define skip_line(ptr) \
  ({ while(*ptr && *ptr++ != '\n') /*empty*/; })

#define skip_blanks(ptr) \
  ({ while(*ptr && *ptr == ' ') ptr++; })

int scan_multi(const char ** str, const char ** search, const int * lens, int nr)
{
  const char * tmp = *str;
  int i;
  for(;;) {
    skip_blanks(tmp);
    if(!*tmp) {
      *str = tmp;
      return -1;
    }
    for(i = 0; i < nr; i++) {
      if(!strncmp(tmp, search[i], lens[i])) {
	tmp += lens[i];
	skip_blanks(tmp);
	*str = tmp;
	return i;
      }
    }
    skip_line(tmp);
  }
}

const char * scan_one(const char * str, const char * search, int searchlen)
{
  for(;;) {
    skip_blanks(str);
    if(!*str) {
      return NULL;
    }
    if(!strncmp(str, search, searchlen)) {
      str += searchlen;
      skip_blanks(str);
      return str;
    }
    skip_line(str);
  }
}

/////////////////////////////////////////////////////////////////


const char * rexstr_brick = "^brick *([:/])?= *(\\w*)";

int parse_brick(char * buf, int len, char * res_op, char * res_name, int len_name)
{
#if 1
  const char * found = scan_one(buf, "brick", 5);
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
  skip_blanks(found);
  if(!*found) {
    return -1;
  }
  int res_len;
  for(res_len = 0; found[res_len] && found[res_len] != '\n'; res_len++) {
    // empty
  }
  copy_str(res_name, len_name, found, 0, res_len);
  return (long)found + res_len - (long)buf;
#else
  int start[3];
  int end[3];
  int pos = rex_search(rexbuf_brick, buf, len, 3, (long)start-(long)buf, end);
  if(pos < 0) {
     return pos;
  }
  pos = end[0];
  *res_op = 0;
  if(start[1] > 0)
    *res_op = buf[start[1]];
  copy_str(res_name, len_name, buf, start[2], end[2]);
#ifdef DEBUG
  printf("++++ brick '%s'\n", res_name);
#endif
  return pos;
#endif
}

const char * rexstr_connector = "^ *connect *(\\w+)(\\[([0-9]+)\\])? *([:/=])?= *(.*)";

int parse_connector(char * buf, int len, char * res_name, int len_name, index_t * res_index, char * res_op, char * res_other, int len_other, int * reslen_other)
{
#if 1
  const char * found = scan_one(buf, "connect", 7);
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
  skip_blanks(found);
  const char op_char = *found++;
  if(op_char == ':' || op_char == '/') {
    *res_op = op_char;
  } else {
    *res_op = 0;
  }
  if(*found == '=') {
    found++;
  }
  skip_blanks(found);
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
#else
  int start[6];
  int end[6];
  int pos = rex_search(rexbuf_connector, buf, len, 6, start, end);
  if(pos < 0) {
     return pos;
  }
  pos = end[0];
  copy_str(res_name, len_name, buf, start[1], end[1]);
  *res_index = 0;
  if(start[3] > 0) {
    sscanf(buf+start[3], "%d", res_index);
  }
  *res_op = 0;
  if(start[4] > 0)
    *res_op = buf[start[4]];
  *reslen_other = end[5] - start[5];
  if(*reslen_other >= len_other) {
    *reslen_other = len_other-1;
  }
  copy_str(res_other, len_other, buf, start[5], end[5]);
#ifdef DEBUG
  printf("++++ conn '%s' '%s'\n", res_name, res_other);
#endif
  return pos;
#endif
}

const char * rexstr_elem = "^([0-9a-f]+):(\\w+)(\\[[0-9]+\\])?,? *";

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
#if 1
  const char * ptr = buf;
  skip_blanks(ptr);
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
  skip_blanks(ptr);
  return (long)ptr - (long)buf;
#else
  int start[4];
  int end[4];
  int pos = rex_search(rexbuf_elem, buf, len, 4, start, end);
  if(pos < 0) {
     return pos;
  }
  pos = end[0];
  conn->conn_addr = -1;
  if(!start[1]) {
    sscanf(buf, "%Lx", &conn->conn_addr);
  }
  copy_str(conn->conn_name, sizeof(conn->conn_name), buf, start[2], end[2]);
  conn->conn_index = 0;
  if(start[3] > 0) {
    sscanf(buf+start[3], "%d", &conn->conn_index);
  }
#ifdef DEBUG
  printf("++++ elem '%s' index %d\n", conn->conn_name, conn->conn_index);
#endif
  return pos;
#endif
}

const char * rexstr_inout = "^ *(in|out)put *([:/=])?= *(\\w+)(\\[[0-9]+\\])?";

int parse_inout(char * buf, int len, int * res_type, char * res_op, struct conn_info * conn)
{
#if 1
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
  skip_blanks(found);
  if(!*found) {
    return -1;
  }
  found = parse_connstr(found, conn);
  return (long)found - (long)buf;
#else
  int start[5];
  int end[5];
  int pos = rex_search(rexbuf_inout, buf, len, 5, start, end);
  if(pos < 0) {
     return pos;
  }
  pos = end[0];
  *res_type = 0;
  if(start[1] >= 0 && buf[start[1]] == 'o') {
	  *res_type = 1;
  }
  *res_op = 0;
  if(start[2] > 0)
    *res_op = buf[start[2]];
  copy_str(conn->conn_name, sizeof(conn->conn_name), buf, start[3], end[3]);
  conn->conn_index = 0;
  if(start[4] > 0) {
    sscanf(buf+start[4], "%d", &conn->conn_index);
  }
#ifdef DEBUG
  printf("++++ inout '%s'\n", conn->conn_name);
#endif
  return pos;
#endif
}

