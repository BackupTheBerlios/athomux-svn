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

////////////////////////////////////////////////////////////////////

/* WARNING! These are DEPRECATED routines which should DISAPPEAR!
 * Don't use for new bricks!
 */

// make a null-terminated dst string out of a substring of
// src, identified by start and end
void copy_str(char * dst, int maxlen, const char * src, int start, int end);


int parse_brick(char * buf, int len, char * res_op, char * res_name, int len_name);

int parse_connector(char * buf, int len, char * res_name, int len_name, index_t * res_index, char * res_op, char * res_other, int len_other, int * reslen_other);

int parse_elem(char * buf, int len, struct conn_info * conn);

int parse_inout(char * buf, int len, int * res_type, char * res_op, struct conn_info * conn);


#endif
