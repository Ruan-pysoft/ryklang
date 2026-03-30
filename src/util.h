#pragma once

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

struct arena {
	void *memory;
	size_t count;
	size_t capacity;
};
struct arena arena_new(size_t initial_size);
void *arena_alloc(struct arena *this, size_t size);
void arena_clear(struct arena *this);
void arena_free(struct arena *this);
