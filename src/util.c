#include "util.h"

#include <assert.h>
#include <ctype.h>

struct position pos_begin(const struct source *src) {
	return (struct position) {
		.src = src,
		.at = src->src,
		.line = 1,
		.line_begin = src->src,
	};
}
struct position pos_at(const struct source *src, const char *at) {
	struct position res = pos_begin(src);
	while (res.at < at) pos_adv(&res);
	return res;
}
void pos_adv(struct position *this) {
	assert(*this->at != '\0');
	if (*this->at++ == '\n') {
		++this->line;
		this->line_begin = this->at;
	}
}
struct position pos_advanced(struct position pos) {
	pos_adv(&pos);
	return pos;
}
bool pos_cmp(struct position a, struct position b) {
	const bool res = a.src == b.src && a.at == b.at;
	if (res) assert(a.line == b.line && a.line_begin == b.line_begin);
	return res;
}

void span_pretty(String_Builder *sb, struct span span, size_t indent) {
	assert(span.pos.line_begin <= span.pos.at);

	for (size_t i = 0; i < indent; ++i) sb_append(sb, ' ');
	size_t line_len = 0;
	while (span.pos.line_begin[line_len] != '\n' && span.pos.line_begin[line_len] != '\0') {
		++line_len;
	}
	sb_append_buf(sb, span.pos.line_begin, line_len);
	sb_append(sb, '\n');
	for (size_t i = 0; i < indent; ++i) sb_append(sb, ' ');
	for (size_t i = 0; i < (size_t)(span.pos.at - span.pos.line_begin); ++i) sb_append(sb, ' ');
	for (size_t i = 0; i < span.len; ++i) sb_append(sb, '^');
	if (span.len == 0) sb_append(sb, '^');
	sb_append(sb, '\n');
}
bool span_cmp(struct span a, struct span b) {
	return a.len == b.len && pos_cmp(a.pos, b.pos);
}

struct arena arena_new(size_t initial_size) {
	if (initial_size == 0) initial_size = 1024;

	return (struct arena) {
		.memory = malloc(initial_size),
		.count = 0,
		.capacity = initial_size,
	};
}
void *arena_alloc(struct arena *this, size_t size) {
	while (this->count + size > this->capacity) {
		this->capacity *= 2;
		this->memory = realloc(this->memory, this->capacity);
	}

	void *res = this->memory + this->count;
	this->count += size;
	return res;
}
void arena_clear(struct arena *this) {
	// zero memory to catch use-after-free earlier
	memset(this->memory, 0, this->count);
	this->count = 0;
}
void arena_free(struct arena *this) {
	free(this->memory);
	*this = (struct arena) {0};
}
