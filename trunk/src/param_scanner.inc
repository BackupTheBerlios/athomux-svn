/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

@.define getoptval(const char * const par, const char * const name, char * const buf, int buflen)
{
  const char *val_ptr;
  int val_len;
  bool found;

  param_getopt(par, name, &found, &val_ptr, &val_len);
  @.check(found!=TRUE, "missing parameter '%s=..'", name);
  @.check(val_len<1, "parameter '%s' has no value", name);
  @.check(val_len>=buflen, "value of parameter '%s' too long", name);

  strncpy(buf, val_ptr, val_len);
  buf[val_len] = 0;
}
