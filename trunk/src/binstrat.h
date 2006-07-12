#ifndef __BINSTRAT_H__
#define __BINSTRAT_H__

#include "common.h"

// Data types for binary, machine-readable strategy information
enum strat_action_t { STRAT_READ, STRAT_CREATE, STRAT_DESTROY, STRAT_INIT, STRAT_SHUTDOWN, STRAT_REINIT };
enum conn_t { CONN_INPUT, CONN_OUTPUT };

#define GET_BRICK_ACTION(BRICK) ((enum strat_action_t)((BRICK).flags & 0x07))
#define IS_BRICK_ACTIVE(BRICK) (((BRICK).flags & 0x08) != 0)
#define GET_CONN_ACTION(CONN) ((enum strat_action_t)((CONN).flags & 0x07))
#define GET_CONN_TYPE(CONN) ((enum conn_t)(((CONN).flags & 0x08) >> 3))
#define IS_CONN_INPUT(CONN) (((CONN).flags & 0x08) == 0)
#define IS_CONN_OUTPUT(CONN) (((CONN).flags & 0x08) != 0)
#define IS_CONN_ACTIVE(CONN) (((CONN).flags & 0x10) != 0)
#define GET_CONN_TARGET_ACTION(TARGET) ((enum strat_action_t)((TARGET).flags & 0x07))

// A brick/connector attribute. Part of a NULL-terminated list.
struct attr_rec {
	struct attr_rec *next_attr;
	char *name;
	char *value;
};

// A connection target, input or output. Part of a NULL-terminated list.
// contains the target brick's address in the strategy nest, and the name of the connector.
struct conn_target_rec {
	struct conn_target_rec *next_target;
	short flags;
	addr_t brick_addr;
	char *name;
};

// A connector. Part of a NULL-terminated list. Contains name, attributes, and, if connected, connection target(s).
struct conn_rec {
	struct conn_rec *next_conn;
	short flags;
	const char *name;
	struct attr_rec *attr;
	struct conn_target_rec *target;
};

// A brick. Contains type, attributes, and connectors.	
struct brick_rec {
	short flags;
	char *name;
	struct attr_rec *attr;
	struct conn_rec *conn;
};

/*
 * Functions to convert a brick description read from control_simple into a binary representation.
 * All these functions get a **textbuf and return TRUE on success, FALSE otherwise. 
 * On failure, no parameters are modified.
 * On success, *textbuf points to the text after the region just parsed, afterwards.
 * Functions that take a **binbuf write the translation of the parsed text to *binbuf.
 * Such functions also take a *binbuf_size which should contain the space left in the bin buffer.
 * On success, *binbuf points to the first byte after the data just written, *binbuf_size is decreased by the number of bytes just written.
 * All functions will stop on encounter of a '\0' or when running out of space in binbuf, and fail if they have not finished conversion yet.
 */ 

// Always succeeds. Skips a number of whitespace characters at *textbuf, including newlines.
extern success_t skip_whitespace(const char **const textbuf);

// Skips whitespaces followed by a newline. Fails if there are any non-blanks until the newline.
extern success_t skip_end_of_line(const char **const textbuf);

// Skips the rest of the line, no matter what characters encountered.
extern success_t skip_rest_of_line(const char **const textbuf);

extern success_t skip_op(const char **const textbuf, enum strat_action_t *action);

// Skips a character sequence defined by 'skip_str': a space matches any number of blanks (including newline),
// any other character matches itself exactly once. Example:
// *skip_str==" Hello \n World" would match **textbuf=="Hello\t\t\n    World" but not "Hello World".
extern success_t skip_str(const char **const textbuf, const char *skip_str);

// Skips an exact match of *match_str, which may only consist of typical identifier chars: 0..9, a..z, A..Z and _
// If match_str==NULL, it successfully skips any identifier.
// Fails on mismatch, or if unable to find even one identifier character.
extern success_t skip_word(const char **const textbuf, const char *match_str);

// Copies a word of identifier characters (see "skip_word") to **binbuf.
// Fails if there is no identifier character at **textbuf.
extern success_t convert_word(const char **const textbuf, void **const binbuf, size_t *const binbuf_size);

// Reads a hexadecimal string, converts it to an addr_t.
// Fails if there is no hexadecimal number at **textbuf (examples: "0x00FF" "aac3" "00066c1"
extern success_t convert_logaddr(const char **const textbuf, addr_t *logaddr);

// Converts an "attribute" line to a "struct attr_rec", saving the string contents behind.
extern success_t convert_attr(const char **const textbuf, void **const binbuf, size_t *const binbuf_size);

// Converts a "connect" line to a "struct conn_target_rec", storing subrecord contents behind.
extern success_t convert_conn_target(const char **const textbuf, void **const binbuf, size_t *const binbuf_size);

// Converts an "input" or "output" block to a "struct conn_rec", storing subrecord contents behind.
extern success_t convert_conn(const char **const textbuf, void **const binbuf, size_t *const binbuf_size);

// Converts a "brick" block to a "struct brick_rec", storing subrecord contents behind.
extern success_t convert_brick(const char **const textbuf, void **const binbuf, size_t *const binbuf_size);

// Converts a brick record to a strategy text. *textbuf_len must contain the size of textbuf before the call, contains the space left thereafter.
extern success_t brick_to_string(char *textbuf, int *textbuf_len, struct brick_rec *brick);

#endif
