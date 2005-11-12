/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

#ifndef _PARAM_SCANNER_H_
#define _PARAM_SCANNER_H_




#include "common.h"




// search in string 'param' for option 'name'.
// options are of the form "name[=value]" and separated by whitespaces (space, tab, newline)
// "name" may consist of any non-whitespace characters.
// "value" is optional, to enable it to contain spaces, it may be quoted ("" or '').
// examples for a paramstring:
// myoption youroption somevalue=12 others="bernie ert grooby" eo_num='unu du tri kvar kven'
//
// if this option is found 'found' is set to TRUE, otherwise it is set to FALSE.
// if this option is found and has a value 'val' and 'val_len' are set accordingly, otherwise they are set to NULL and 0.
// finds the first match in 'param'.
void param_getopt(const char * const param, const char * const name, bool * found, const char ** val, int * val_len);




#endif
