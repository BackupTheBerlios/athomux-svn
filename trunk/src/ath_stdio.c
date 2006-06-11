#include <string.h>
#include <stdio.h>

#include "common.h"	
#include "ath_stdio.h"

char *athsnprintf(char *buf, int bufsize, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *retval = athvsnprintf(buf, bufsize, fmt, args);
	va_end (args);
	return retval;
}

char *athvsnprintf(char *buf, int bufsize, const char *fmt, va_list args) {
	static const char ch_bool_types[] = { 'f', 't' };
	static const char ch_bool_types_upper[] = { 'F', 'T' };
	static const char ch_bool_types_noyes[] = { 'n', 'y' };
	static const char ch_bool_types_noyes_upper[] = { 'N', 'Y' };
	static const char *const ch_success_types[] = { "fail", "succ" };
	static const char *const ch_success_types_upper[] = { "FAIL", "SUCC" };
	static const char ch_lock_types[] = { '-', 'r', 'w', '?' };
	static const char ch_lock_types_upper[] = { '-', 'R', 'W', '?' };
	static const char ch_action_types[] = { 'a', 't', 'w' };
	static const char ch_action_types_upper[] = { 'A', 'T', 'W' };
	static const char ch_direction_types[] = { 'r', 'w' };
	static const char ch_direction_types_upper[] = { 'R', 'W' };
	static const char ch_prio_types[] = { '-', 'b', 'n', 'u' };
	static const char ch_prio_types_upper[] = { '-', 'B', 'N', 'U' };
	static const char ch_version_types[] = { '?', 'o', 'n' };
	static const char ch_version_types_upper[] = { '?', 'O', 'N' };
	const char *fmtiter = fmt;
	char *wtbuf = buf;
	index_t i;
	char c;
	const char *s;
	mand_t m;
	addr_t addr_len;
	paddr_t paddr_plen;
	int n;

	bufsize--;
	while (*fmtiter && bufsize > 0) {
		while (*fmtiter && *fmtiter != '%' && bufsize > 0) {
			*wtbuf++ = *fmtiter++;
			bufsize--;
		}
		if (*fmtiter == '%' && bufsize > 0) {
			switch (*++fmtiter) {
			case 'c':
				c = (char)va_arg(args, int);
				*wtbuf++ = c;
				bufsize--;
				fmtiter++;
				break;
			case 's':
				s = va_arg(args, const char *);
				while (*s && bufsize > 0) {
					*wtbuf++ = *s++;
					bufsize--;
				}
				fmtiter++;
				break;
			case 'b':
			case 'B':
			case 'y':
			case 'Y':
				n = va_arg(args, int);
				if (n < 0 || n > sizeof ch_bool_types) {
					n = sizeof ch_bool_types;
				}
				switch (*fmtiter) {
				case 'b': *wtbuf = ch_bool_types[n]; break;
				case 'B': *wtbuf = ch_bool_types_upper[n]; break;
				case 'y': *wtbuf = ch_bool_types_noyes[n]; break;
				case 'Y': *wtbuf = ch_bool_types_noyes_upper[n]; break;
				}
				*wtbuf++;
				bufsize--;
				fmtiter++;
				break;
			case 'o':
			case 'O':
				n = va_arg(args, int);
				if (n < 0 || n > 1) {
					n = 1;
				}
				s = *fmtiter == 'o' ? ch_success_types[n] : ch_success_types_upper[n];
				while (*s && bufsize > 0) {
					*wtbuf++ = *s++;
					bufsize--;
				}
				fmtiter++;
				break;
			case 'i':
				i = va_arg(args, index_t);
				n = snprintf(wtbuf, bufsize, "%d", i);
				if (n > bufsize) {
					n = bufsize;
				}
				wtbuf += n;
				bufsize -= n;
				fmtiter++;
				break;
			case 'x':
			case 'X':
				i = va_arg(args, index_t);
				n = snprintf(wtbuf, bufsize, *fmtiter == 'x' ? "%x" : "%X", i);
				if (n > bufsize) {
					n = bufsize;
				}
				wtbuf += n;
				bufsize -= n;
				fmtiter++;
				break;
			case 'l':
			case 'L':
				addr_len = va_arg(args, addr_t);
				n = snprintf(wtbuf, bufsize, *fmtiter == 'l' ? "%llx" : "%llX", addr_len);
				if (n > bufsize) {
					n = bufsize;
				}
				wtbuf += n;
				bufsize -= n;
				fmtiter++;
				break;
			case 'p':
			case 'P':
				paddr_plen = va_arg(args, paddr_t);
				n = snprintf(wtbuf, bufsize, *fmtiter == 'p' ? "%lx" : "%lX", paddr_plen);
				if (n > bufsize) {
					n = bufsize;
				}
				wtbuf += n;
				bufsize -= n;
				fmtiter++;
				break;
			case 'm':
				m = va_arg(args, mand_t);
				n = snprintf(wtbuf, bufsize, "%d", m);
				if (n > bufsize) {
					n = bufsize;
				}
				wtbuf += n;
				bufsize -= n;
				fmtiter++;
				break;
			case 'k':
			case 'K':
				n = va_arg(args, int);
				if (n < 0 || n > sizeof ch_lock_types) {
					n = sizeof ch_lock_types;
				}
				*wtbuf++ = *fmtiter == 'k' ? ch_lock_types[n] : ch_lock_types_upper[n];
				bufsize--;
				fmtiter++;
				break;
			case 'a':
			case 'A':
				n = va_arg(args, int);
				if (n < 0 || n > sizeof ch_action_types) {
					n = sizeof ch_action_types;
				}
				*wtbuf++ = *fmtiter == 'a' ? ch_action_types[n] : ch_action_types_upper[n];
				bufsize--;
				fmtiter++;
				break;
			case 'd':
			case 'D':
				n = va_arg(args, int);
				if (n < 0 || n > sizeof ch_direction_types) {
					n = sizeof ch_direction_types;
				}
				*wtbuf++ = *fmtiter == 'd' ? ch_direction_types[n] : ch_direction_types_upper[n];
				bufsize--;
				fmtiter++;
				break;
			case 'r':
			case 'R':
				n = va_arg(args, int);
				if (n < 0 || n > sizeof ch_prio_types) {
					n = sizeof ch_prio_types;
				}
				*wtbuf++ = *fmtiter == 'r' ? ch_prio_types[n] : ch_prio_types_upper[n];
				bufsize--;
				fmtiter++;
				break;
			case 'v':
			case 'V':
				n = va_arg(args, int);
				if (n < 0 || n > sizeof ch_version_types) {
					n = sizeof ch_version_types;
				}
				*wtbuf++ = *fmtiter == 'v' ? ch_version_types[n] : ch_version_types_upper[n];
				bufsize--;
				fmtiter++;
				break;
			case '%':
				*wtbuf++ = *fmtiter++;
				bufsize--;
				break;
			default:
				*wtbuf++ = '%';
				bufsize--;
				break;
			}
		}
	}
	*wtbuf = '\0';
	return wtbuf++;
}

extern plen_t athprintf(void *connector, bool is_output, mand_t mand, bool wait, const char *fmt, ...) {
	static const char *const paramstr = "";
	char strbuf[0x400];
	char *strbuf_end;
	struct output *out = is_output ? (struct output *)connector : ((struct input *)connector)->connect;
	struct args args;
	va_list arglist;
	
	va_start(arglist, fmt);
	strbuf_end = athvsnprintf(strbuf, sizeof strbuf, fmt, arglist);
	args.success = FALSE;
	args.phys_addr = MAKE_PADDR (strbuf);
	args.phys_len = (plen_t)(strbuf_end - strbuf);
	args.mandate = mand;
	args.action = wait ? action_wait : action_try;
	args.melt = TRUE;
	args.op_code = opcode_gadrcreatetranswaitpadr;
	args.sect_code = 0;
	out->ops[0][opcode_gadrcreatetranswaitpadr]((union connector *)out, &args, paramstr);
	if (args.success) {
		return args.phys_len;
	}
	else {
		return 0;
	}
}
