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
struct lexer read_num(struct lexer lex);
struct lexer lexer_adv(struct lexer lex) {
	lex = lexer_skip_ws(lex);

	if (lexer_atend(lex)) {
		lex.has_tok = true;
		lex.tok = make_token(TT_EOF, lex.pos, 0);
		return lex;
	} else if (lexer_test(lex, isdigit)) {
		return read_num(lex);
	} else {
		lex.has_tok = true;
		lex.tok = make_token(TT_UNKNOWN, lex.pos++, 1);
		return lex;
	}

	assert(false && "unreachable");
}

struct lexer read_num(struct lexer lex) {
	assert(lexer_test(lex, isdigit));

	const char *const begin = lex.pos;

	uint64_t num = 0;
	while (lexer_test(lex, isdigit)) {
		const uint64_t old = num;
		num *= 10;
		num += *lex.pos - '0';

		if (num < old) {
			assert(false && "TODO: proper error handling");
		}

		lex = lexer_inc(lex);
	}

	lex.has_tok = true;
	lex.tok = make_token(TT_NUM, begin, lex.pos - begin, .num = num);
	return lex;
}
