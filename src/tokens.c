#include "tokens.h"

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

const char *tt_repr(enum token_type tt) {
	switch (tt) {
#define X(tt, ...) case tt: return #tt;
		LIST_OF_TTS
#undef X
	}
}

void token_repr(String_Builder *sb, struct token tok) {
	switch (tok.type) {
#define X(tt, fmt, ...) case tt: sb_appendf(sb, "%s("fmt")", tt_repr(tok.type), __VA_ARGS__); break;
		LIST_OF_TTS
#undef X
	}
}

void le_push(struct lexer_errors *this, const char *pos, const char *msg) {
	da_append(this, ((struct lexer_error) { .pos = pos, .msg = strdup(msg) }));
}
void le_pop(struct lexer_errors *this) {
	if (this->count) free((void*)this->items[this->count--].msg);
}
void le_free(struct lexer_errors *this) {
	while (this->count) le_pop(this);
	da_free(*this);
}

bool lexer_atend(const struct lexer *this) {
	return *this->pos == 0;
}
bool lexer_test(const struct lexer *this, int(*test)(int)) {
	return !lexer_atend(this) && test(*this->pos);
}
bool lexer_cmp(const struct lexer *this, char c) {
	return !lexer_atend(this) && *this->pos == c;
}
void lexer_adv(struct lexer *this) {
	if (!lexer_atend(this)) ++this->pos;
}
void lexer_skip_ws(struct lexer *this) {
	while (lexer_test(this, isspace)) {
		lexer_adv(this);
	}
}

struct lexer lexer_new(const char *src) {
	struct lexer lex = {0};
	lex.src = lex.pos = src;

	return lex;
}
struct token read_num(struct lexer *this, struct lexer_errors *err);
struct token lexer_next(struct lexer *this, struct lexer_errors *err) {
	lexer_skip_ws(this);

	if (lexer_atend(this)) {
		return make_token(TT_EOF, this->pos, 0);
	} else if (lexer_test(this, isdigit)) {
		return read_num(this, err);
	} else {
		le_push(err, this->pos, "unexpected character");
		struct token res = make_token(TT_UNKNOWN, this->pos, 1);
		lexer_adv(this);
		return res;
	}

	assert(false && "unreachable");
}

struct token read_num(struct lexer *this, struct lexer_errors *err) {
	assert(lexer_test(this, isdigit));

	const char *const begin = this->pos;

	uint64_t num = 0;
	bool hit_overflow = false;
	while (lexer_test(this, isdigit)) {
		const uint64_t old = num;
		num *= 10;
		num += *this->pos - '0';

		if (!hit_overflow && num < old) {
			le_push(err, this->pos, "integer overflow while parsing number");
		}

		lexer_adv(this);
	}

	return make_token(
		TT_NUM, begin, this->pos - begin,
		.num = hit_overflow ? 0 : num
	);
}
