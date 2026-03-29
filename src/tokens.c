#include "tokens.h"

#include <assert.h>
#include <ctype.h>

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

bool lexer_atend(struct lexer lex) {
	return *lex.pos == 0;
}
bool lexer_test(struct lexer lex, int(*test)(int)) {
	return !lexer_atend(lex) && test(*lex.pos);
}
bool lexer_cmp(struct lexer lex, char c) {
	return !lexer_atend(lex) && *lex.pos == c;
}
struct lexer lexer_inc(struct lexer lex) {
	if (!lexer_atend(lex)) ++lex.pos;
	return lex;
}
struct lexer lexer_skip_ws(struct lexer lex) {
	while (lexer_test(lex, isspace)) {
		lex = lexer_inc(lex);
	}
	return lex;
}

struct lexer lexer_new(const char *src) {
	struct lexer lex = {0};
	lex.src = lex.pos = src;

	return lex;
}
struct lexer read_num(struct lexer lex, struct lexer_errors *err);
struct lexer lexer_adv(struct lexer lex, struct lexer_errors *err) {
	lex = lexer_skip_ws(lex);

	if (lexer_atend(lex)) {
		lex.has_tok = true;
		lex.tok = make_token(TT_EOF, lex.pos, 0);
		return lex;
	} else if (lexer_test(lex, isdigit)) {
		return read_num(lex, err);
	} else {
		lex.has_tok = true;
		le_push(err, lex.pos, "unexpected character");
		lex.tok = make_token(TT_UNKNOWN, lex.pos++, 1);
		return lex;
	}

	assert(false && "unreachable");
}

struct lexer read_num(struct lexer lex, struct lexer_errors *err) {
	assert(lexer_test(lex, isdigit));

	const char *const begin = lex.pos;

	uint64_t num = 0;
	bool hit_overflow = false;
	while (lexer_test(lex, isdigit)) {
		const uint64_t old = num;
		num *= 10;
		num += *lex.pos - '0';

		if (!hit_overflow && num < old) {
			le_push(err, lex.pos, "integer overflow while parsing number");
		}

		lex = lexer_inc(lex);
	}

	lex.has_tok = true;
	lex.tok = make_token(
		TT_NUM, begin, lex.pos - begin,
		.num = hit_overflow ? 0 : num
	);
	return lex;
}
