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

/* These are some new hand-scrafted parsing routines.
 * Further ones chould be added.
 */

/* Match a single keyword starting at a new line,
 * return pointer to new position after matching.
 * Leading and trailing blanks are skipped.
 */
const char * scan_single(const char * str, const char * search, int searchlen);

/* Dto for multiple alternative keywords.
 * Return the index into the array (or -1 on failure).
 */
int scan_multi(const char ** str, const char * search[], const int lens[], int nr);

/* Default strategy keywords
 */
typedef enum {
  scod_brick,
  scod_input,
  scod_output,
  scod_connect,
  scod_max
} strat_codes;

const char * strat_keywords[scod_max];

const int strat_keylens[scod_max];

/* After skipping leading blanks, scan over the characters specified
 * by table[], return start position in *res, length of found string as
 * result, and finally advance *str over the whole thing.
 */
int scan_table(const char ** str, const bool table[], const char ** res);

/* Dto, but scan over characters _not_ in table[]
 */
int scan_not_table(const char ** str, const bool table[], const char ** res);

/* Same as before, but make a null-terminated copy to res.
 * When resles is not sufficient, -1 is returned.
 */
int scan_table_copy(const char ** str, const bool table[], char * res, int reslen);
int scan_not_table_copy(const char ** str, const bool table[], char * res, int reslen);

/* Some predefined character scan tables
 */
const bool id_table[256];    // identifiers
const bool num_table[256];   // decimals
const bool hex_table[256];   // hex numbers in C syntax
const bool ws_table[256];    // whitespace including newline
const bool nl_table[256];    // only newline
const bool bl_table[256];    // whitespace other than newline

/* A trivial parser for ':=', "/=" and '=='
 */
char scan_op(const char ** str);

/* Skip over blanks or until the next newline
 */
void skip_line(const char ** str);

void skip_blanks(const char ** str);

/* Parse a simple connector specifier 'name[index]'
 */
bool scan_connstr(const char ** str, struct conn_info * conn);

/* Parse an address connector specifier addr:name[index]'
 */
bool scan_connstraddr(const char ** str, struct conn_info * conn);

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
