#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#define LIST_OF_TTS \
	X(TT_NUM, "%lu", (tok).num) \
	X(TT_EOF, "%.0s", "") \
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
		uint64_t num;
	};
	const char *pos;
	size_t len;
};
void token_repr(String_Builder *sb, struct token tok);

#define make_token(ttype, tpos, tlen, ...) ((struct token) { .type = ttype, .pos = tpos, .len = tlen, __VA_ARGS__ })

struct lexer_errors {
	struct lexer_error {
		const char *pos;
		const char *msg;
	} *items;
	size_t count;
	size_t capacity;
};
void le_push(struct lexer_errors *this, const char *pos, const char *msg);
void le_pop(struct lexer_errors *this);
void le_free(struct lexer_errors *this);
#define le_pushf(le, pos, fmt, ...) do { \
	String_Builder _le_pushf_sb = {0}; \
	sb_appendf(&_le_pushf_sb, fmt, __VA_ARGS__); \
	sb_append_null(&_le_pushf_sb); \
	le_push((le), (pos), _le_pushf_sb->items); \
	sb_free(&_le_pushf_sb); \
} while (0)

struct lexer {
	const char *src;
	const char *pos;
	bool has_tok;
	union { struct token tok; };
};

struct lexer lexer_new(const char *src);
struct lexer lexer_adv(struct lexer lex, struct lexer_errors *err);
