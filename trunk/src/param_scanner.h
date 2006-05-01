/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

#ifndef _PARAM_SCANNER_H_
#define _PARAM_SCANNER_H_




#include "common.h"




// Searches in string 'param' for option 'name'.
// Options are of the form "name[=value]" and separated by whitespaces (space, tab, newline).
// "name" may consist of any non-whitespace characters.
// "value" is optional, to enable it to contain spaces, it may be quoted ("" or '').
// Examples for a paramstring:
// myoption youroption somevalue=12 others="bernie ert grooby" eo_num='unu du tri kvar kven'
//
// 'param' contains the string to search (usually @param)
// 'name' contains the name of the option to be queried
// In 'found' the result of the search is stored: whether the option was found or not
// On 'val'!=NULL, a pointer to the value of the option is returned there, NULL if the option has no value.
// On 'val_len'!=NULL, the length of the value is stored there, 0 if the option has no value.
void param_getopt(const char * const param, const char * const name, bool * found, const char ** val, int * val_len);




#endif
