/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

#include "param_scanner.h"
#include "common.h"
#include <string.h>




const char param_idchars[] = "_-.:/0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char param_nidchars[] = "\t\r\n =";
const char param_delimchars[] = "\t\r\n ";




// returns length of prefix of 'start' that contains no character listed in 'toc'
int excl_len(const char * const start, const char * const toc) {
  if ((start==NULL) || (toc==NULL)) return 0;

  int i=0;
  while (start[i]!=0) {
    if (strchr(toc, start[i])!=NULL) break;
    i++;
  }

  return i;
}




// returns length of prefix of 'start' that contains only characters listed in 'toc'
int incl_len(const char * const start, const char * const toc) {
  if ((start==NULL) || (toc==NULL)) return 0;

  int i=0;
  while (start[i]!=0) {
    if ((strchr(toc, start[i]))==NULL) break;
    i++;
  }

  return i;
}




// parses 'start' for an option or option/value pair.
// if found 'name', 'name_len', 'val' and 'val_len' are set accordingly, otherwise they are set to NULL and 0.
// the number of bytes read are returned.
int next_opt(const char * const start, const char ** name, int * name_len, const char ** val, int * val_len) {
  *name = NULL;
  *name_len = 0;
  *val = NULL;
  *val_len = 0;

  if (start==NULL) return 0;

  const char *p = start;

  int len;
  int r_sum = 0;

  len = incl_len(p, param_delimchars);
  p+=len;
  r_sum+=len;

  // read name
  *name = p;
  len = excl_len(p, param_nidchars);
  p+=len;
  r_sum+=len;
  *name_len=len;

  len = incl_len(p, param_delimchars);
  p+=len;
  r_sum+=len;

  if (p[0]=='=') {
    // name has a value
    p++;
    r_sum++;

    len = incl_len(p, param_delimchars);
    p+=len;
    r_sum+=len;

    if (p[0]=='"') {
      // read until second double quote
      p++;
      r_sum++;
      *val = p;
      len = excl_len(p, "\"");
      p+=len+1;
      r_sum+=len+1;
      *val_len=len;
    }
    else if (p[0]=='\'') {
      // read until second quote
      p++;
      r_sum++;
      *val = p;
      len = excl_len(p, "'");
      p+=len+1;
      r_sum+=len+1;
      *val_len=len;
    }
    else {
      // no quotes
      *val = p;
      len = excl_len(p, param_delimchars);
      p+=len;
      r_sum+=len;
      *val_len=len;
    }

    len = incl_len(p, param_delimchars);
    p+=len;
    r_sum+=len;
  }

  return r_sum;
}




// search in 'param' for option 'name'.
// if this option is found 'found' is set to TRUE, otherwise it is set to FALSE.
// if this option is found and has a value 'val' and 'val_len' are set accordingly, otherwise they are set to NULL and 0.
// finds the first match in 'param'.
void param_getopt(const char * const param, const char * const name, bool * found, const char ** val, int * val_len) {
  *val = NULL;
  *val_len = 0;
  *found = FALSE;

  if (param==NULL) return;
  if (name==NULL) return;

  const char *p = param;
  int len;

  while (p[0]!=0) {
    const char * cur_name;
    int cur_name_len;
    const char * cur_val;
    int cur_val_len;
    
    len = next_opt(p, &cur_name, &cur_name_len, &cur_val, &cur_val_len);
    p+=len;

    if ((cur_name_len>0) && (strncmp(cur_name, name, cur_name_len)==0)) {
      *found = TRUE;
      if (cur_val_len>0) {
        *val = cur_val;
        *val_len = cur_val_len;
      }
      return;
    }
  }
}
