#ifndef __ATH_STDIO_H__
#define __ATH_STDIO_H__

#include <stdarg.h>

/* These functions work similar to the corresponding stdio.h functions, except for these differences:
 *
 * They all understand type specifiers different from the stdio functions. Furthermore, they (currently) accept no modifiers.
 * types accepted are (uppercase type specifiers usually mean uppercase output):
 * %%:     no type, outputs a single '%'
 * %c:     a single 'char'
 * %s:     a string ('const char *')
 * %b/%B:  a 'bool', written as 'f' (false), 't' (true), respectively
 * %y/%Y:  a 'bool', written as 'n' (false), 'y' (true), respectively
 * %o/%O:  a 'success_t', written as 'fail' or 'succ'
 * %i:     an 'index_t' in decimal notation (remember: no modifiers like 'l' or "06")
 * %x/%X:  an 'index_t' in hexadecimal notation
 * %l/%L:  a 'log_addr' or 'log_len', in hexadecimal notation
 * %p/%P:  a 'phys_addr' or 'phys_len' in hexadecimal notation
 * %m:     a 'mand_t' in decimal notation
 * %k/%K:  a 'lock_t', written as '-' (lock_none), 'r' (lock_read), or 'w' (lock_write)
 * %a/%A:  an 'action_t' written as 'a' (action_ask), 't' (action_try), or 'w' (action_wait)
 * %d/%D:  a 'direction_t' written as 'r' (direct_read) or 'w' (direct_write)
 * %r/%R:  a 'prio_t', written as '-' (prio_none), 'b' (prio_background), 'n' (prio_normal), or 'u' (prio_urgent)
 * %v/%V:  a 'version_t' written as '?' (version_unknown), 'o' (version_old), 'n' (version_newest)
 *
 * 'athprintf' does not output to a stream, but instead to an Atomux pipe.
 * it takes both inputs and outputs as connectors, but the type must be specified in the 2nd parameter ('isOutput'), TRUE means 'output'
 * additional parameters are 'mandate' and 'wait' necessary for Athomux pipe op calls
 * for example, call: athprintf(&@#superbrick#subbrick:>out, TRUE, 0, TRUE, "Range is [0x%L..0x%L]\n", @log_addr, @log_addr + @log_len);
 */

extern char *athsnprintf(char *buf, int bufsize, const char *fmt, ...);
extern char *athvsnprintf(char *buf, int bufsize, const char *fmt, va_list args);
extern plen_t athprintf(void *connector, bool is_output, mand_t mand, bool wait, const char *fmt, ...);

#endif
