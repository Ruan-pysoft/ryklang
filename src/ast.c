#include "ast.h"

const char *binop_type_repr(enum binop_type type) {
	switch (type) {
#define X(tt, str) case tt: return str;
		LIST_OF_BINOPS
#undef X
	}
	assert(false && "unreachable");
}

void ast_free(struct ast *ast) {
	switch (ast->type) {
		case ANT_NUM: break;
		case ANT_BINOP: {
			ast_free(ast->binop.lhs);
			ast_free(ast->binop.rhs);
		} break;
	}

	free(ast);
}
void ast_repr(String_Builder *sb, const struct ast *ast) {
	switch (ast->type) {
		case ANT_NUM: sb_appendf(sb, "%lu", ast->num); break;
		case ANT_BINOP: {
			sb_appendf(sb, "(");
			ast_repr(sb, ast->binop.lhs);
			sb_appendf(sb, ") %s (", binop_type_repr(ast->binop.type));
			ast_repr(sb, ast->binop.rhs);
			sb_appendf(sb, ")");
		} break;
	}
}
bool ast_cmp(const struct ast *a, const struct ast *b) {
	if (a == b) return true;
	if (a->type != b->type) return false;

	switch (a->type) {
		case ANT_NUM: return a->num == b->num;
		case ANT_BINOP: {
			return a->binop.type == b->binop.type
				&& ast_cmp(a->binop.lhs, b->binop.lhs)
				&& ast_cmp(a->binop.rhs, b->binop.rhs);
		} break;
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

struct ast *parse_base(struct arena *arena, struct token_array *toks, struct parser_errors *err);
struct ast *parse_expr(struct arena *arena, struct token_array *toks, struct parser_errors *err);

struct ast *parse(struct arena *arena, struct token_array toks, struct parser_errors *err) {
	assert(toks.count != 0);

	struct ast *ast = parse_expr(arena, &toks, err);

	if (toks.count == 0) {
		struct token prev_tok = toks.items[-1];
		struct position pos = prev_tok.span.pos;
		for (size_t i = 0; i < prev_tok.span.len; ++i) {
			pos_adv(&pos);
		}
		pe_push(err, pos, 0, "expected an eof token at the end of the token array");
	} else if (toks.items[0].type != TT_EOF) {
		struct token tok = toks.items[0];
		pe_pushf(err, tok.span.pos, tok.span.len, "unexpected token of type %s, expected EOF", tt_repr(tok.type));
	} else if (toks.count != 1) {
		struct token tok = toks.items[1];
		pe_pushf(err, tok.span.pos, tok.span.len, "unexpected token of type %s after EOF; EOF should be the last token", tt_repr(tok.type));
	}

	return ast;
}

struct ast *parse_base(struct arena *arena, struct token_array *toks, struct parser_errors *err) {
	// TODO: return error ast node rather than NULL
	assert(toks->count != 0);

	struct token tok = toks->items[0];

	if (tok.type == TT_NUM) {
		struct ast *res = arena_alloc(arena, sizeof(*res));
		*res = (struct ast) {
			.type = ANT_NUM,
			.span = tok.span,

			.num = tok.num,
		};

		++toks->items;
		--toks->count;

		return res;
	} else if (tok.type == TT_LPAREN) {
		++toks->items;
		--toks->count;

		struct ast *res = parse_expr(arena, toks, err);

		assert(toks->count);
		if (toks->items[0].type == TT_RPAREN) {
			++toks->items;
			--toks->count;
		} else {
			struct token tok = toks->items[0];
			pe_push(err, tok.span.pos, tok.span.len, "unclosed parenthesis");
		}

		return res;
	} else if (tok.type == TT_EOF) {
		pe_push(err, tok.span.pos, tok.span.len, "unexpected eof, expected a number");
		return NULL;
	} else {
		pe_pushf(err, tok.span.pos, tok.span.len, "unexpected token of type %s, expected a number", tt_repr(tok.type));
		return NULL;
	}
}
struct ast *parse_expr(struct arena *arena, struct token_array *toks, struct parser_errors *err) {
	struct ast *lhs = parse_base(arena, toks, err);

	assert(toks->count != 0);

	while (toks->items[0].type == TT_PLUS || toks->items[0].type == TT_MINUS) {
		enum token_type op = toks->items[0].type;

		++toks->items;
		--toks->count;

		struct ast *rhs = parse_base(arena, toks, err);

		struct ast *binop = arena_alloc(arena, sizeof(*binop));
		*binop = (struct ast) {
			.type = ANT_BINOP,
			.binop = {
				.type = op == TT_PLUS ? BT_ADD : BT_SUB,
				.lhs = lhs,
				.rhs = rhs,
			},
		};

		lhs = binop;

		assert(toks->count != 0);
	}

	return lhs;
}
