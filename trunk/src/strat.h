/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#ifndef _STRAT_H
#define _STRAT_H

/* Syntax on the strategy level:

brick==<bname>{\n
  param <bname>==<string1>\n
  attr <bname>.<aname>==<value1>\n
  ...
  input==<bname>.<iname>{\n
    param <bname>.<iname>==<string2>\n
    attr <bname>.<iname>.<aname>==<value2>\n
    ...
    connect <bname>.<iname>==<addr2>\n
  }\n
  ...
  output==<bname>.<oname>{\n
    param <bname>.<oname>==<string3>\n
    attr <bname>.<oname>.<aname>==<value3>\n
    ...
    connect <bname>.<oname>==<addr_list>\n
  }\n
  ...
}\n

When initializing or connecting something, "==" is replaced by ":=".
When destructing or disconnecting something, "==" is replaced by "/=".

When something is immutable, "=" is used instead of "==" (i.e. you
cannot replace ":=" or "/=" for "=").

However note that a brick is created with ":=", but afterwards the
brick type is immutable and thus denoted using "=".
*/


struct rex_buffer; // opaque

// compile a regular expression into internal representaition
struct rex_buffer * rex_compile(const char * rex_str);

// release internal represenation
void rex_free(struct rex_buffer * pat);

// search for any occurrence of pat in str
// return position (starting from 0)
// upon failure, return negative value
int rex_search(struct rex_buffer * pat, const char * str, int len, int count, int start[], int end[]);

// make a null-terminated dst string out of a substring of
// src, identified by start and end
void copy_str(char * dst, int maxlen, const char * src, int start, int end);


////////////////////////////////////////////////////////////////////

/* Here are some common regexes.
 * TODO: add further frequently used ones
 */

extern const char * rexstr_brick;
extern struct rex_buffer * rexbuf_brick;
int parse_brick(char * buf, int len, char * res_op, char * res_name, int len_name);

extern const char * rexstr_connector;
extern struct rex_buffer * rexbuf_connector;
int parse_connector(char * buf, int len, char * res_name, int len_name, index_t * res_index, char * res_op, char * res_other, int len_other, int * reslen_other);

extern const char * rexstr_elem;
extern struct rex_buffer * rexbuf_elem;
int parse_elem(char * buf, int len, struct conn_info * conn);

extern const char * rexstr_inout;
extern struct rex_buffer * rexbuf_inout;
int parse_inout(char * buf, int len, int * res_type, char * res_op, struct conn_info * conn);


///////////////////////////////////////////////////////////////////

// this must be called once upon initilaization (usually by control_*)

void rex_init(void);

#endif
