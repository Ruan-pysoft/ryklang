#pragma once

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "util.h"
#include "tokens.h"

enum ast_type {
	ANT_NUM,
};

struct ast {
	enum ast_type type;
	struct span span;

	union {
		uint64_t num;
	};
};
void ast_free(struct ast *ast);
void ast_repr(String_Builder *sb, const struct ast *ast);
bool ast_cmp(const struct ast *a, const struct ast *b);

struct parser_errors {
	struct parser_error {
		struct span span;
		const char *msg;
	} *items;
	size_t count;
	size_t capacity;
};
void pe_push(struct parser_errors *this, struct position pos, size_t len, const char *msg);
void pe_pop(struct parser_errors *this);
void pe_free(struct parser_errors *this);
#define pe_pushf(pe, pos, len, fmt, ...) do { \
	String_Builder _pe_pushf_sb = {0}; \
	sb_appendf(&_pe_pushf_sb, fmt, __VA_ARGS__); \
	sb_append_null(&_pe_pushf_sb); \
	pe_push((pe), (pos), (len), _pe_pushf_sb.items); \
	sb_free(_pe_pushf_sb); \
} while (0)

struct ast *parse(struct arena *arena, struct token_array toks, struct parser_errors *err);
