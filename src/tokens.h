#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#define LIST_OF_TTS \
	X(TT_NUM, "%ld", (tok).num) \
	X(TT_EOF, "%.0s", NULL) \
	X(TT_UNKNOWN, "%.*s", (int)(tok).len, (tok).pos)

enum token_type {
#define X(tt, ...) tt,
	LIST_OF_TTS
#undef X
};
const char *tt_repr(enum token_type tt);

struct token {
	enum token_type type;
	union {
		int64_t num;
	};
	const char *pos;
	size_t len;
};
void token_repr(String_Builder *sb, struct token tok);

struct lexer {
	const char *src;
	const char *pos;
	bool has_tok;
	union { struct token tok; };
};

struct lexer lexer_new(const char *src);
struct lexer lexer_adv(struct lexer lex);
