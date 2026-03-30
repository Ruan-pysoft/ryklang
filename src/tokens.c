#include "tokens.h"

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
bool token_cmp(struct token a, struct token b) {
	if (a.type != b.type) return false;

	switch (a.type) {
		case TT_EOF: return true;
		case TT_NUM: return a.num == b.num;
		case TT_UNKNOWN: return span_cmp(a.span, b.span);
	}

	assert(false && "unreachable");
}

void le_push(struct lexer_errors *this, struct position pos, size_t len, const char *msg) {
	da_append(this, ((struct lexer_error) {
		.span = { .pos = pos, .len = len },
		.msg = strdup(msg)
	}));
}
void le_pop(struct lexer_errors *this) {
	if (this->count) free((void*)this->items[this->count--].msg);
}
void le_free(struct lexer_errors *this) {
	while (this->count) le_pop(this);
	da_free(*this);
}

bool lexer_atend(const struct lexer *this) {
	return *this->pos.at == 0;
}
bool lexer_test(const struct lexer *this, int(*test)(int)) {
	return !lexer_atend(this) && test(*this->pos.at);
}
bool lexer_cmp(const struct lexer *this, char c) {
	return !lexer_atend(this) && *this->pos.at == c;
}
void lexer_adv(struct lexer *this) {
	if (!lexer_atend(this)) pos_adv(&this->pos);
}
void lexer_skip_ws(struct lexer *this) {
	while (lexer_test(this, isspace)) {
		lexer_adv(this);
	}
}

struct lexer lexer_new(const struct source *src) {
	struct lexer lex = {0};
	lex.pos = pos_begin(src);

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
		le_push(err, this->pos, 1, "unexpected character");
		struct token res = make_token(TT_UNKNOWN, this->pos, 1);
		lexer_adv(this);
		return res;
	}

	assert(false && "unreachable");
}

struct token read_num(struct lexer *this, struct lexer_errors *err) {
	assert(lexer_test(this, isdigit));

	const struct position begin = this->pos;

	uint64_t num = 0;
	bool hit_overflow = false;
	while (lexer_test(this, isdigit)) {
		const uint64_t old = num;
		num *= 10;
		num += *this->pos.at - '0';

		if (!hit_overflow && num < old) {
			le_push(err, begin, this->pos.at - begin.at + 1, "integer overflow while parsing number");
		}

		lexer_adv(this);
	}

	return make_token(
		TT_NUM, begin, this->pos.at - begin.at,
		.num = hit_overflow ? 0 : num
	);
}
