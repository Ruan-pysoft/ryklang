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

struct ast *parse(struct arena *arena, struct token_array toks, struct parser_errors *err) {
	assert(toks.count != 0);

	struct token tok = toks.items[0];

	if (tok.type == TT_NUM) {
		struct ast *res = arena_alloc(arena, sizeof(*res));
		*res = (struct ast) {
			.type = ANT_NUM,
			.span = tok.span,

			.num = tok.num,
		};

		return res;
	} else if (tok.type == TT_EOF) {
		pe_push(err, tok.span.pos, tok.span.len, "unexpected eof, expected a number");
		return NULL;
	} else {
		pe_pushf(err, tok.span.pos, tok.span.len, "unexpected token of type %s, expected a number", tt_repr(tok.type));
		return NULL;
	}
}
