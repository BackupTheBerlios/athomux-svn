/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#define BASEFILE "strat.c"

#include "common.h"
#include "strat.h"

#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

/* PROBLEM:
 * The Gnu regex library uses malloc() and realloc(), even if we pre-allocate
 * our own space for the buffer.
 * Thus we need a modified version of regex with no dynamic memory
 * allocation at all (for a true standalone Athomux version).
 * For the prototype, we encapsulate dynamic memory for now to
 * enable easy replacement by another version later.
 *
 * The interface of rex_compile() and rex_free() may be extended later,
 * in order to allocate pages from a tmp input.
 */

const char * rexstr_brick;
struct rex_buffer * rexbuf_brick;
const char * rexstr_connector;
struct rex_buffer * rexbuf_connector;
const char * rexstr_elem;
struct rex_buffer * rexbuf_elem;
const char * rexstr_inout;
struct rex_buffer * rexbuf_inout;

struct rex_buffer {
  struct re_pattern_buffer re;
};

struct rex_buffer * rex_compile(const char * rex_str)
{
  struct rex_buffer * pat = malloc(sizeof(struct rex_buffer));
  const char * res;
  if(!pat)
    return NULL;
  memset(pat, 0, sizeof(struct rex_buffer));
  re_syntax_options = pat->re.syntax = RE_CHAR_CLASSES | RE_CONTEXT_INDEP_ANCHORS | RE_CONTEXT_INDEP_OPS | RE_INTERVALS | RE_CONTEXT_INVALID_OPS | RE_NO_BK_PARENS| RE_NO_BK_VBAR;
  res = re_compile_pattern(rex_str, strlen(rex_str), &pat->re);
  if(res) {
#ifdef DEBUG
    printf("regex compile error: %s\n", res);
    exit(-1);
#endif
    rex_free(pat);
    return NULL;
  }
  pat->re.regs_allocated = REGS_FIXED;
  pat->re.newline_anchor = 1;
  return pat;
}

void rex_free(struct rex_buffer * pat)
{
  regfree(&pat->re);
  free(pat);
}

int rex_search(struct rex_buffer * pat, const char * str, int len, int count, int start[], int end[])
{
  int res;
  struct re_registers regs = {count, start, end};
#if 1
  memset(start, 0, sizeof(int) * count);
  memset(end, 0, sizeof(int) * count);
#endif
  res = re_search(&pat->re, str, len, 0, len-1, &regs);
  return res;
}


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

/////////////////////////////////////////////////////////////////


#define REX_INIT(name) \
  if(!rexbuf_##name) \
     rexbuf_##name = rex_compile(rexstr_##name);

void rex_init(void)
{
  REX_INIT(brick);
  REX_INIT(connector);
  REX_INIT(elem);
  REX_INIT(inout);
}

const char * rexstr_brick = "^brick *([:/])?= *(\\w*)";

int parse_brick(char * buf, int len, char * res_op, char * res_name, int len_name)
{
  int start[3];
  int end[3];
  int pos = rex_search(rexbuf_brick, buf, len, 3, start, end);
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
}

const char * rexstr_connector = "^ *connect *(\\w+)(\\[([0-9]+)\\])? *([:/=])?= *(.*)";

int parse_connector(char * buf, int len, char * res_name, int len_name, index_t * res_index, char * res_op, char * res_other, int len_other, int * reslen_other)
{
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
}

const char * rexstr_elem = "^([0-9a-f]+):(\\w+)(\\[[0-9]+\\])?,? *";

int parse_elem(char * buf, int len, struct conn_info * conn)
{
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
}

const char * rexstr_inout = "^ *(in|out)put *([:/=])?= *(\\w+)(\\[[0-9]+\\])?";

int parse_inout(char * buf, int len, int * res_type, char * res_op, struct conn_info * conn)
{
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
}

