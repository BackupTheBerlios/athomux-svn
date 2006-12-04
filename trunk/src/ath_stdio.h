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
 * %S:     a string of limited length (does not have to be null-terminated), a 'plen_t' must be provided, followed by a 'const char *'.
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
 * The functions athsnprintf and athvsnprintf write to a string buffer, just like snprintf.
 * The string written gets _always_ terminated, even if the string gets cut.
 * Both functions return a pointer to the terminating '\0' character to ease string concatenation.
 * athpipeprintf and athnestprintf both write to a connector.
 * Both functions return the number of bytes written, 0 on failure.
 * athnestprintf writes to a certain address of an Athomux nest via the operation $transwait(direct_write, prio_normal).
 * athpipeprintf is a shortcut for writes to an Athomux pipe via the operation $gadrcreatetranswaitpadr(action_wait).
 * You must specify the type of connector because inputs and outputs cannot be distinguished at runtime.
 * Examples:
 * athnestprintf(&:<in, FALSE, 0x3800LL, @#._mand, "Duke wrote this to hex-address %L!\n", 0x3800LL);
 * athnestprintf(&:>control, TRUE, strat_addr, 0, "brick:=mem{bparam:=%s\n}\n", @param);
 * athpipeprintf(&:>writer, TRUE, @#._mand, "Duke was here for the %ith time!\n", (index_t)count_times);
 */

extern char *athsnprintf(char *buf, int bufsize, const char *fmt, ...);
extern char *athvsnprintf(char *buf, int bufsize, const char *fmt, va_list args);
extern plen_t athpipeprintf(void *connector, bool is_output, mand_t mand, const char *fmt, ...);
extern plen_t athnestprintf(void *connector, bool is_output, addr_t addr, mand_t mand, const char *fmt, ...);

#endif
