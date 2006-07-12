
/*****
 ***** BINARY STRATEGY INTERFACE
 *****/

#include "binstrat.h"
#include "strat.h"
#include "ath_stdio.h"

#define MASK_BRICK_ACTION (0x07)
#define MASK_BRICK_ACTIVE (0x08)
#define MASK_CONN_ACTION (0x07)
#define MASK_CONN_TYPE (0x08)
#define MASK_CONN_ACTIVE (0x10)
#define MASK_CONN_TARGET_ACTION (0x07)

// *textbuf points to the first non-blank character from *buf afterwards.
// Blanks include a newline '\n'.
// Always returns TRUE.
success_t skip_whitespace(const char **const textbuf) {
	// consider all control characters whitespaces.
	while (ws_table[(int)**textbuf]) (*textbuf)++;
	return TRUE;
}

// Returns TRUE, if there are only whitespaces left until the end of the current line.
// On success, *textbuf points to the first character of the next line.
success_t skip_end_of_line(const char **const textbuf) {
	const char *text = *textbuf;
	while (bl_table[(int)*text]) text++;
	if (*text == '\n') {
		*textbuf = text;
		return TRUE;
	}
	else return FALSE;
}

// *textbuf points to the first character of the next line afterwards, or to the terminator if there is no more line.
// Always returns TRUE.
success_t skip_rest_of_line(const char **const textbuf) {
	while (**textbuf && **textbuf != '\n') {
		(*textbuf)++;
	}
	if (**textbuf) (*textbuf)++;
	return TRUE;
}

// skips one of the operators = /= += ­-= ^=
success_t convert_op(const char **const textbuf, enum strat_action_t *action) {
	static const char actionsub[256] = { ['='] = ('='-STRAT_READ), [':'] = (':'-STRAT_CREATE), ['/'] = ('/'-STRAT_DESTROY), ['+'] = ('+'-STRAT_INIT), ['-'] = ('-'-STRAT_SHUTDOWN), ['^'] = ('^'-STRAT_REINIT) };
	const char *text = *textbuf;
	enum strat_action_t a;
	printf("converting operator ...");
	if (!actionsub[(int)*text]) { printf("mismatch \'%c\'\n", *text); return FALSE; }
	a = (*text - actionsub[(int)*text]);
	text++;
	if (*text == '=') { text++; }
	else if (a != STRAT_READ) { printf("missing \'=\', found \'%c\'\n", *text); return FALSE; }
	printf("\"%.*s\"==%d\n", (int)(text - *textbuf), *textbuf, a);
	*textbuf = text;
	if (action) { *action = a; }
	return TRUE;
}

// Skips constant strings. A space in 'skip_str' is treated as any number of whitespaces (including none).
// On success, *textbuf points to the first character after the match afterwards.
// Returns TRUE on a complete match, FALSE otherwise.
// Warning: this function does not check word boundaries! skip_str(text, " dear ") will be true on text=="dearrangement"!
success_t skip_str(const char **const textbuf, const char *skip_str) {
	const char *text = *textbuf;
	printf("skipping \"%s\" --> ", skip_str);
	while (*skip_str) {
		const char *tmp = text;
		if (*skip_str == ' ') {
			skip_whitespace(&text);
			printf("(' '^%d),", (int)(text - tmp));
			skip_str++;
		}
		else {
			if (*skip_str == *text) {
				printf("'%c',", *skip_str);
				text++;
				skip_str++;
			}
			else {
				printf("mismatch '%c'!='%c'\n", *text, *skip_str);
				return FALSE;
			}
		}
	}
	printf("done\n");
	*textbuf = text;
	return TRUE;
}

// Ignore the next identifier.
// Returns TRUE on a match, FALSE otherwise.
// "match" must be either NULL or must contain solely of identifier characters (see "id_table" in "strat.h").
// If "match" is != null, the word must match *match, else it can be any identifier of length > 0.
// On success, *textbuf points to the first character behind the identifier, is unchanged otherwise.
success_t skip_word(const char **const textbuf, const char *match_str) {
	const char *text = *textbuf;
	if (match_str) {
		printf("skipping \"%s\"...", match_str);
		while (id_table[(int)*text] && *text == *match_str) {
			text++;
			match_str++;
		}
		if (id_table[(int)*text] || *match_str) { printf("mismatch: \"%.*s\"...\n", 4, text); return FALSE; }
		printf("ok\n");
	}
	else {
		if (!id_table[(int)*text])  { printf("no word found to skip!\n"); return FALSE; }
		while (id_table[(int)*text]) text++;
		printf("skipped \"%.*s\"\n", (int)(text - *textbuf), *textbuf);
	}
	*textbuf = text;
	return TRUE;
}

// Reads as many identifier-class characters as possible from '*buf', stores them in '*word' and adds a '\0'.
// On success, *textbuf points to the first non-identifier char from *textbuf afterwards.
// On success, *binbuf points to the first unallocated byte in the buffer, *binbuf_size is reduced accordingly.
success_t convert_word(const char **const textbuf, void **const binbuf, size_t *const binbuf_size) {
	const char *text = *textbuf;
	char *bin = (char *)*binbuf;
	size_t bin_size = *binbuf_size;
	printf("binbuf==%p *binbuf==%p bin==%p\n", binbuf, *binbuf, bin);
	printf("converting word \"%.*s\"...", 4, text);
	fflush(stdout);
	if (!id_table[(int)*text]) { printf("mismatch: '%c'\n", *text); return FALSE; }
	while (id_table[(int)*text]) { // "id_table" and other char classes are declared in "strat.h"
		if (bin_size-- == 0) { printf("out of memory!\n"); return FALSE; }
		*bin++ = *text++;
	}
	if (bin_size-- == 0) { printf("out of memory!\n"); return FALSE; }
	*bin++ = '\0';
	printf("\"%s\"\n", (const char *)*binbuf);
	*textbuf = text;
	*binbuf = (void *)bin;
	*binbuf_size = bin_size;
	return TRUE;
}

success_t convert_composed_identifier(const char **const textbuf, void **const binbuf, size_t *const binbuf_size) {
	const char *text = *textbuf;
	char *bin = (char *)*binbuf;
	size_t bin_size_remainder = *binbuf_size;
	if (!convert_word(&text, (void **)&bin, &bin_size_remainder)) return FALSE;
	while (*text == '.') {
		printf("encountered '.'\n");
		text++;
		bin = (char *)*binbuf;
		bin_size_remainder = *binbuf_size;
		if (!convert_word(&text, (void **)&bin, &bin_size_remainder)) return FALSE;		
	}
	*textbuf = text;
	*binbuf = bin;
	*binbuf_size = bin_size_remainder;
	return TRUE;
}

// Reads a hexadecimal number (with or without "0x" prefix) and stores it in an addr_t at **binbuf.
// On success, *textbuf points to the first non-identifier char from *textbuf afterwards.
// On success, *binbuf points to the first unallocated byte in the buffer, *binbuf_size is reduced accordingly.
success_t convert_logaddr(const char **textbuf, addr_t *laddr) {
	static const char hexsub[256] = { ['0'...'9'] = '0', ['a'...'f'] = 'a' - 10, ['A'...'F'] = 'A' - 10 };
	const char *text = *textbuf;
	*laddr = 0;
	if (*text == '0' && (*(text + 1) == 'x' || *(text + 1) == 'X')) { text += 2; }
	if (!hexsub[(int)*text]) { printf("No hexnumber: %.4s\n", text); return FALSE; }
	while (hexsub[(int)*text]) {
		if (*laddr >= (1LL << 63)) { printf("Integer overflow!\n"); return FALSE; }
		*laddr = *laddr * 0x10 + (*text - hexsub[(int)*text]);
		text++;
	}
	*textbuf = text;
	printf("recognized logaddr 0x%llX\n", *laddr);
	return TRUE;
}

// Reads an "attr" line (brick or connector attribute).
// On success, *textbuf points to the first character of the next line.
// On success, a 'struct attr_rec' has been written to *binbuf, *binbuf and the 'next' pointer in the record just created point to the first byte behind it.
// On success, *binbuf_size is reduced by the number of bytes *binbuf has been increased.
// On failure, *textbuf, *binbuf, and *binbuf_size remain unchanged.
success_t convert_attr(const char **const textbuf, void **const binbuf, size_t *const binbuf_size) {
	const char *text = *textbuf;
	struct attr_rec *const attr = (struct attr_rec *)*binbuf;
	void *bin = *binbuf + sizeof *attr;
	size_t bin_size = *binbuf_size;
	skip_whitespace(&text);
	if (!skip_word(&text, "attr")) return FALSE;
	skip_whitespace(&text);
	attr->name = (char *)bin;
	if (!convert_composed_identifier(&text, &bin, &bin_size)) return FALSE;
	if (!convert_op(&text, NULL)) return FALSE;
	attr->value = (char *)bin;
	if (!convert_word(&text, &bin, &bin_size)) return FALSE;
	if (!skip_end_of_line(&text)) return FALSE;
	attr->next_attr = (struct attr_rec *)bin;
	*textbuf = text;
	*binbuf = bin;
	*binbuf_size = bin_size;
	printf("done attr\n");
	return TRUE;
}

// Reads a conn target line.
// On success, *textbuf points to the first character of the next line.
// On success, a 'struct conn_target_rec' has been written to *binbuf, *binbuf and the 'next' pointer in the record just created point to the first byte behind it.
// On success, *binbuf_size is redurec by the number of bytes *binbuf as been increased.
// On failure, *textbuf, *binbuf and *binbuf_size remain unchanged.
success_t convert_conn_target(const char **const textbuf, void **const binbuf, size_t *const binbuf_size) {
	const char *text = *textbuf;
	size_t bin_size = *binbuf_size;
	struct conn_target_rec *const target = (struct conn_target_rec *)*binbuf;
	void *bin = *binbuf + sizeof *target;
	enum strat_action_t action;
	skip_whitespace(&text);
	if (skip_word(&text, "connect")) {
		skip_whitespace(&text);
		if (!skip_word(&text, NULL)) { return FALSE; }
		if (!convert_op(&text, &action)) { return FALSE; };
		target->flags = action;
		if (!convert_logaddr(&text, &target->brick_addr)) { return FALSE; }
		if (*text != ':') { printf("mismatch! \':\' expected, \'%c\' found!\n", *text); return FALSE; }
		text++;
		target->name = (char *)bin;
		if (!convert_word(&text, &bin, &bin_size)) { return FALSE; }
		target->next_target = (struct conn_target_rec *)bin;
		*textbuf = text;
		*binbuf = bin;
		*binbuf_size = bin_size;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// Reads a "conn" block.
// On success, *textbuf points to the first character behind the block.
// On success, a 'struct conn_rec' has been written to *binbuf,  *binbuf and the 'next' pointer in the record just created point to the first byte behind it.
// On success, *binbuf_size is redurec by the number of bytes *binbuf as been increased.
// On failure, *textbuf, *binbuf and *binbuf_size remain unchanged.
success_t convert_conn(const char **const textbuf, void **const binbuf, size_t *const binbuf_size) {
	const char *text = *textbuf;
	struct conn_rec *const conn = (struct conn_rec *)*binbuf;
	void *bin = *binbuf + sizeof *conn;
	size_t bin_size = *binbuf_size - sizeof *conn;
	enum strat_action_t action;
	conn->flags = MASK_CONN_ACTIVE; // assume connector to be initialized
	skip_whitespace(&text);
	if (*text == '(') {
		// uninitialized connector
		conn->flags &= ~MASK_CONN_ACTIVE;
		text++;
	}
	if (skip_word(&text, "output")) conn->flags |= MASK_CONN_TYPE;
	else if (!skip_word(&text, "input")) return FALSE;
	if (!IS_CONN_ACTIVE(*conn)) {
		// uninitialized connector needs closing parentheses
		if (*text != ')') { printf("Closing parenthesis of uninitialized connector missing!\n"); return FALSE; }
		text++;
	}
	skip_whitespace(&text); 
	if (!convert_op(&text, &action)) { return FALSE; }
	conn->flags |= action;
	conn->name = (char *)bin;
	if (!convert_composed_identifier(&text, &bin, &bin_size)) return FALSE;
	skip_str(&text, " { ");
	if (skip_word(&text, "c_param")) {
		// Ignore Parameter string.
		skip_rest_of_line(&text);
	}
	conn->attr = (struct attr_rec *)bin;
	struct attr_rec *attr = conn->attr;
	if (convert_attr(&text, &bin, &bin_size)) {
		struct attr_rec *new_attr = (struct attr_rec *)bin;
		while (convert_attr(&text, &bin, &bin_size)) {
			attr = new_attr;
			new_attr = (struct attr_rec *)bin;
		}
		// Terminate the list of attributes.
		attr->next_attr = NULL;
	}
	else {
		// Empty list of attributes.
		conn->attr = NULL;
	}
	skip_whitespace(&text);
	conn->target = (struct conn_target_rec *)bin;
	struct conn_target_rec *target = conn->target;
	if (convert_conn_target(&text, &bin, &bin_size)) {
		skip_whitespace(&text);
		while (*text == ',') {
			// Only outputs are allowed multiple connections.
			if (IS_CONN_OUTPUT (*conn)) { return FALSE; }
			text++;
			struct conn_target_rec *new_target = (struct conn_target_rec *)bin;
			if (!convert_conn_target(&text, &bin, &bin_size)) { return FALSE; }
			target = new_target;
			new_target = (struct conn_target_rec *)bin;
		}
		// terminate the list of connections.
		target->next_target = NULL;
	}
	else {
		// Unconnected yet.
		conn->target = NULL;
	}
	if (!skip_str(&text, " } ")) return FALSE;
	conn->next_conn = (struct conn_rec *)bin;
	*textbuf = text;
	*binbuf = bin;
	*binbuf_size = bin_size;
	return TRUE;
}

// Reads out a brick record from a textual strategy nest and returns it in binary form in *((struct brick_rec *)binbuf).
// *binbuf_size should contain the maximum size of binbuf before , and contains the true length after the call.
success_t convert_brick(const char **const textbuf, void **const binbuf, size_t *const binbuf_size) {
	const char *text = *textbuf;
	void *bin;
	size_t bin_size;
	struct brick_rec *const brick = (struct brick_rec *)*binbuf;
	enum strat_action_t action;
	brick->flags = MASK_BRICK_ACTIVE;
	bin = *binbuf + sizeof (struct brick_rec);
	bin_size = *binbuf_size;
	skip_whitespace(&text);
	if (*text == '(') {
		brick->flags &= ~MASK_BRICK_ACTIVE;
		text++;
	}
	if (!skip_word(&text, "brick")) return FALSE;
	if (!IS_BRICK_ACTIVE(*brick)) {
		if (*text != ')') { return FALSE; }
		text++;
	}
	skip_whitespace(&text);
	if (!convert_op(&text, &action)) { return FALSE; }
	brick->flags |= action;
	brick->name = (char *)bin;
	if (!convert_word(&text, &bin, &bin_size)) return FALSE;
	if (!skip_str(&text, " { ")) return FALSE;
	if (skip_word(&text, "b_param")) {
		// Ignore Parameter string.
		skip_rest_of_line(&text);
	}
	brick->attr = (struct attr_rec *)bin;
	struct attr_rec *attr = (struct attr_rec *)bin;
	if (convert_attr(&text, &bin, &bin_size)) {
		struct attr_rec *new_attr = (struct attr_rec *)bin;
		while (convert_attr(&text, &bin, &bin_size)) {
			attr = new_attr;
			new_attr = (struct attr_rec *)bin;
		}
		// Terminate the list of attributes.
		attr->next_attr = NULL;
	}
	else {
		// Empty list of attributes.
		brick->attr = NULL;
	}
	brick->conn = (struct conn_rec *)bin;
	struct conn_rec *conn = (struct conn_rec *)bin;
	if (convert_conn(&text, &bin, &bin_size)) {
		//printf("convert_brick stage 1.5 ---------------- bin==%p bin_size=0x%X\n", bin, (int)bin_size);
		struct conn_rec *new_conn = (struct conn_rec *)bin;
		while (convert_conn(&text, &bin, &bin_size)) {
			conn = new_conn;
			new_conn = (struct conn_rec *)bin;
		}
		// Terminate the list of connections.
		conn->next_conn = NULL;
	}
	else {
		// Empty list of connections.
		brick->conn = NULL;
	}
	//printf("convert_brick stage 3 ---------------- bin==%p bin_size=0x%X\n", bin, (int)bin_size);
	if (!skip_str(&text, " } ")) { return FALSE; }
	*textbuf = text;
	*binbuf = bin;
	*binbuf_size = bin_size;
	//printf("convert_brick() finished, now binbuf==%p *binbuf==%p *binbuf_size==0x%X bin==%p\n", binbuf, *binbuf, (int)*binbuf_size, bin);
	return TRUE;
}

// This is the reverse op to 'convert_brick': it creates a strategy string out of a binary brick representation.
success_t brick_to_string(char *textbuf, int *textbuf_len, struct brick_rec *brick) {
	struct attr_rec *attr;
	struct conn_rec *conn;
	struct conn_target_rec *target;
	char *textpos;
	printf("brick_to_string(textbuf==%p, *text_len==%d, brick==%p)-----------------\n", textbuf, *textbuf_len, brick);
	textpos = athsnprintf(textbuf, *textbuf_len, "brick=%s {\n", brick->name);
	*textbuf_len -= (textpos - textbuf);
	textbuf = textpos;
	for (attr = brick->attr; attr; attr = attr->next_attr) {
		textpos = athsnprintf(textbuf, *textbuf_len, "attr %s.%s=%s {\n", brick->name, attr->name, attr->value);
		*textbuf_len -= (textpos - textbuf);
		textbuf = textpos;
	}
	for (conn = brick->conn; conn; conn = conn->next_conn) {
		const char *const fmt_str = IS_CONN_ACTIVE(*conn) ? "  %s==%s {\n" : "  (%s)==%s {\n";
		textpos = athsnprintf(textbuf, *textbuf_len, fmt_str, IS_CONN_INPUT (*conn) ? "input" : "output", conn->name);
		*textbuf_len -= (textpos - textbuf);
		textbuf = textpos;
		for (attr = conn->attr; attr; attr = attr->next_attr) {
			textpos = athsnprintf(textbuf, *textbuf_len, "    attr %s.%s.%s=%s\n", brick->name, conn->name, attr->name, attr->value);
			*textbuf_len -= (textpos - textbuf);
			textbuf = textpos;
		}
		if (conn->target) {
			target = conn->target;
			textpos = athsnprintf(textbuf, *textbuf_len, "    connect %s==%L:%s", conn->name, target->brick_addr, target->name);
			*textbuf_len -= (textpos - textbuf);
			textbuf = textpos;
			target = target->next_target;
			while (target) {
				textpos = athsnprintf(textbuf, *textbuf_len, ",%L:%s", target->brick_addr, target->name);
				*textbuf_len -= (textpos - textbuf);
				textbuf = textpos;
				target = target->next_target;
			}
			if (*textbuf_len < 1) { printf("out of space!\n"); return FALSE; }
			*textbuf++ = '\n';
			*textbuf_len -= 1;
		}
		if (*textbuf_len < 4) { printf("out of space\n"); return FALSE; }
		strcpy(textbuf, "  }\n");
		textbuf += 4;
		*textbuf_len -= 4;
	}
	if (*textbuf_len < 3) { printf("out of space\n"); return FALSE; }
	*textbuf++ = '}';
	*textbuf++ = '\n';
	*textbuf++ = '\0';
	*textbuf_len -= 3;
	return TRUE;
}
