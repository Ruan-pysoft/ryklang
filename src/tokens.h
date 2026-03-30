#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

struct source {
	const char *name;
	const char *src;
};

struct position {
	const struct source *src;
	const char *at;
	size_t line;
	const char *line_begin;
};
struct position pos_begin(const struct source *src);
struct position pos_at(const struct source *src, const char *at);
void pos_adv(struct position *this);
struct position pos_advanced(struct position pos);
bool pos_cmp(struct position a, struct position b);

#define POS_FMT "%s:%lu:%lu"
#define POS_ARGS(pos) (pos).src->name, (pos).line, (pos).at - (pos).line_begin

#define sb_print(func, ...) do { \
	String_Builder _sb_print_sb = {0}; \
	func(&_sb_print_sb, __VA_ARGS__); \
	printf("%.*s", (int)_sb_print_sb.count, _sb_print_sb.items); \
	sb_free(_sb_print_sb); \
} while (0)

struct span {
	struct position pos;
	size_t len;
};
void span_pretty(String_Builder *sb, struct span span, size_t indent);
bool span_cmp(struct span a, struct span b);

#define LIST_OF_TTS \
	X(TT_NUM, "%lu", (tok).num) \
	X(TT_EOF, "%.0s", "") \
	X(TT_UNKNOWN, "%.*s", (int)(tok).span.len, (tok).span.pos.at)

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
	le_push((le), (pos), (len), _le_pushf_sb->items); \
	sb_free(&_le_pushf_sb); \
} while (0)

struct lexer {
	struct position pos;
	bool has_tok;
	union { struct token tok; };
};

struct lexer lexer_new(const struct source *src);
struct token lexer_next(struct lexer *next, struct lexer_errors *err);
