#include "ast.h"

void ast_free(struct ast *ast) {
	switch (ast->type) {
		case ANT_NUM: break;
	}

	free(ast);
}
void ast_repr(String_Builder *sb, const struct ast *ast) {
	switch (ast->type) {
		case ANT_NUM: sb_appendf(sb, "%lu", ast->num); break;
	}
}
bool ast_cmp(const struct ast *a, const struct ast *b) {
	if (a == b) return true;
	if (a->type != b->type) return false;

	switch (a->type) {
		case ANT_NUM: return a->num == b->num;
	}

	assert(false && "unreachable");
}

void pe_push(struct parser_errors *this, struct position pos, size_t len, const char *msg) {
	da_append(this, ((struct parser_error) {
		.span = { .pos = pos, .len = len },
		.msg = strdup(msg)
	}));
}
void pe_pop(struct parser_errors *this) {
	if (this->count) free((void*)this->items[this->count--].msg);
}
void pe_free(struct parser_errors *this) {
	while (this->count) pe_pop(this);
	da_free(*this);
}

struct parser parser_new(struct arena *arena, struct lexer *lex, struct lexer_errors *lex_err) {
	return (struct parser) {
		.arena = arena,
		.lex = lex,
		.lex_err = lex_err,
		.err = {0},

		.has_last_tok = false,
		.last_tok = {0},
	};
}
struct ast *parse(struct parser *this) {
	assert(!this->has_last_tok);

	struct token tok = lexer_next(this->lex, this->lex_err);

	if (tok.type == TT_NUM) {
		struct ast *res = arena_alloc(this->arena, sizeof(*res));
		*res = (struct ast) {
			.type = ANT_NUM,
			.span = tok.span,

			.num = tok.num,
		};

		return res;
	} else if (tok.type == TT_EOF) {
		pe_push(&this->err, tok.span.pos, tok.span.len, "unexpected eof, expected a number");
		return NULL;
	} else {
		pe_pushf(&this->err, tok.span.pos, tok.span.len, "unexpected token of type %s, expected a number", tt_repr(tok.type));
		return NULL;
	}
}
