#pragma once

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "util.h"

#define LIST_OF_TTS \
	X(TT_NUM, "%lu", (tok).num) \
	X(TT_PLUS, "%.0s", "") \
	X(TT_EOF, "%.0s", "") \
	X(TT_UNKNOWN, "%.*s", (int)(tok).span.len, (tok).span.pos.at)

#define LIST_OF_STR_TTS \
	X(TT_PLUS, "+")

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
	struct span span;
};
void token_repr(String_Builder *sb, struct token tok);
bool token_cmp(struct token a, struct token b);

#define make_token(ttype, tpos, tlen, ...) ((struct token) { .type = ttype, .span = { .pos = tpos, .len = tlen, }, __VA_ARGS__ })

struct lexer_errors {
	struct lexer_error {
		struct span span;
		const char *msg;
	} *items;
	size_t count;
	size_t capacity;
};
void le_push(struct lexer_errors *this, struct position pos, size_t len, const char *msg);
void le_pop(struct lexer_errors *this);
void le_free(struct lexer_errors *this);
#define le_pushf(le, pos, len, fmt, ...) do { \
	String_Builder _le_pushf_sb = {0}; \
	sb_appendf(&_le_pushf_sb, fmt, __VA_ARGS__); \
	sb_append_null(&_le_pushf_sb); \
	le_push((le), (pos), (len), _le_pushf_sb.items); \
	sb_free(_le_pushf_sb); \
} while (0)

struct lexer {
	struct position pos;
	bool has_tok;
	union { struct token tok; };
};

struct lexer lexer_new(const struct source *src);
struct token lexer_next(struct lexer *next, struct lexer_errors *err);

struct token_array {
	struct token *items;
	size_t count;
	size_t capacity;
};

struct token_array lex_source(const struct source *src, struct lexer_errors *err);
