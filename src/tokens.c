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
struct lexer lexer_adv(struct lexer lex) {
	lex = lexer_skip_ws(lex);

	if (lexer_atend(lex)) {
		lex.has_tok = true;
		lex.tok = (struct token) {
			.type = TT_EOF,
			.pos = lex.pos,
			.len = 0,
		};
		return lex;
	} else {
		lex.has_tok = true;
		lex.tok = (struct token) {
			.type = TT_UNKNOWN,
			.pos = lex.pos++,
			.len = 1,
		};
		return lex;
	}

	assert(false && "unreachable");
}
