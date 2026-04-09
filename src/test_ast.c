#include "test_ast.h"

static bool ast_span_cmp(struct ast *a, struct ast *b, struct ast **aout, struct ast **bout) {
	assert(a->type == b->type);

	if (!span_cmp(a->span, b->span)) {
		*aout = a;
		*bout = b;
		return false;
	}

	switch (a->type) {
		case ANT_NUM: break;
		case ANT_BINOP: {
			if (!ast_span_cmp(a->binop.lhs, b->binop.lhs, aout, bout)) return false;
			if (!ast_span_cmp(a->binop.rhs, b->binop.rhs, aout, bout)) return false;
		} break;
	}

	return true;
}

bool test_ast(struct ast_test test, String_Builder *out) {
	struct arena arena = arena_new(0);
	struct ast *expected_ast = test.builder(&arena);

	struct parser_errors err = {0};
	struct token_array tokens = {
		.items = test.toks,
		.count = 0,
		.capacity = 0,
	};
	while (tokens.items[tokens.count].type != TT_EOF) ++tokens.count;
	++tokens.count;
	struct ast *generated_ast = parse(&arena, tokens, &err);

	if (!ast_cmp(expected_ast, generated_ast)) {
		sb_appendf(out, "  error: incorrect ast produced:\n");
		sb_appendf(out, "  got:\n    ");
		ast_repr(out, generated_ast);
		da_append(out, '\n');
		sb_appendf(out, "  expected:\n    ");
		ast_repr(out, expected_ast);
		da_append(out, '\n');

		return false;
	}

	if (!ast_span_cmp(expected_ast, generated_ast, &expected_ast, &generated_ast)) {
		sb_appendf(out, "  error: ast has incorrect positioning:\n");
		sb_appendf(out, "  got:\n    ");
		ast_repr(out, generated_ast);
		da_append(out, '\n');
		generated_ast->span.pos.src = &test.src;
		generated_ast->span.pos.at += (size_t)test.src.src;
		generated_ast->span.pos.line_begin += (size_t)test.src.src;
		span_pretty(out, generated_ast->span, 4);
		sb_appendf(out, "  expected:\n    ");
		ast_repr(out, expected_ast);
		expected_ast->span.pos.src = &test.src;
		expected_ast->span.pos.at += (size_t)test.src.src;
		expected_ast->span.pos.line_begin += (size_t)test.src.src;
		da_append(out, '\n');
		span_pretty(out, expected_ast->span, 4);
	}

	arena_free(&arena);

	da_foreach(struct parser_error, it, &err) {
		struct parser_error error = *test.errs++;
		it->span.pos.src = &test.src;
		it->span.pos.at += (size_t)test.src.src;
		it->span.pos.line_begin += (size_t)test.src.src;
		error.span.pos.src = &test.src;
		error.span.pos.at += (size_t)test.src.src;
		error.span.pos.line_begin += (size_t)test.src.src;

		if (error.msg == NULL) {
			sb_appendf(out, "  Unexpected error:\n");
			sb_appendf(out, "  got      error %s\n", it->msg);
			span_pretty(out, it->span, 4);
			sb_appendf(out, "  expected no error\n");
			return false;
		}

		if (!span_cmp(error.span, it->span)) {
			sb_appendf(out, "  Got errors at wrong position:\n");
			sb_appendf(out, "  got      error %s\n", it->msg);
			span_pretty(out, it->span, 4);
			sb_appendf(out, "  expected error %s\n", error.msg);
			span_pretty(out, error.span, 4);
			return false;
		}

		if (strcmp(error.msg, it->msg) != 0) {
			sb_appendf(out, "  Got unexpected errors message:\n");
			sb_appendf(out, "  got      error %s\n", it->msg);
			span_pretty(out, it->span, 4);
			sb_appendf(out, "  expected error %s\n", error.msg);
			span_pretty(out, error.span, 4);
			return false;
		}
	}

	if (test.errs->msg != NULL) {
		struct parser_error error = *test.errs;
		error.span.pos.src = &test.src;
		error.span.pos.at += (size_t)test.src.src;
		error.span.pos.line_begin += (size_t)test.src.src;

		sb_appendf(out, "  Didn't get expected errors:\n");
		sb_appendf(out, "  got      no error\n");
		sb_appendf(out, "  expected error %s\n", error.msg);
		span_pretty(out, error.span, 4);
		return false;
	}

	return true;
}

#define tmp_pos(offset, line_offset, line_num) ((struct position) { .src = NULL, .at = (char*)(offset), .line_begin = (char*)(line_offset), .line = (line_num) })

#define src_of(str) static const struct source curr_src = { .name = "<test>", .src = (str) };

#define curr_src src0
src_of("42");
static struct token toks0[] = {
	make_token(TT_NUM, tmp_pos(0, 0, 1), 2, .num = 42),
	make_token(TT_EOF, tmp_pos(2, 0, 1), 0),
};
static const struct parser_error errs0[] = {{0}};
static struct ast *builder0(struct arena *arena) {
	struct ast *res = arena_alloc(arena, sizeof(*res));
	*res = (struct ast) {
		.type = ANT_NUM,
		.span = { .pos = tmp_pos(0, 0, 1), .len = 2 },
		.num = 42,
	};

	return res;
}
#undef curr_src
#define curr_src src1
src_of("1 +2");
static struct token toks1[] = {
	make_token(TT_NUM, tmp_pos(0, 0, 1), 1, .num = 1),
	make_token(TT_PLUS, tmp_pos(2, 0, 1), 1),
	make_token(TT_NUM, tmp_pos(3, 0, 1), 1, .num = 2),
	make_token(TT_EOF, tmp_pos(4, 0, 1), 0),
};
static const struct parser_error errs1[] = {{0}};
static struct ast *builder1(struct arena *arena) {
	struct ast *lhs = arena_alloc(arena, sizeof(*lhs));
	*lhs = (struct ast) {
		.type = ANT_NUM,
		.span = { .pos = tmp_pos(0, 0, 1), .len = 1 },
		.num = 1,
	};
	struct ast *rhs = arena_alloc(arena, sizeof(*rhs));
	*rhs = (struct ast) {
		.type = ANT_NUM,
		.span = { .pos = tmp_pos(3, 0, 1), .len = 1 },
		.num = 2,
	};
	struct ast *res = arena_alloc(arena, sizeof(*res));
	*res = (struct ast) {
		.type = ANT_BINOP,
		.span = { .pos = tmp_pos(0, 0, 1), .len = 4 },
		.binop = {
			.type = BT_ADD,
			.lhs = lhs,
			.rhs = rhs,
		},
	};

	return res;
}
#undef curr_src
#define curr_src src2
src_of("64-(13+9)");
static struct token toks2[] = {
	make_token(TT_NUM, tmp_pos(0, 0, 1), 2, .num = 64),
	make_token(TT_MINUS, tmp_pos(2, 0, 1), 1),
	make_token(TT_LPAREN, tmp_pos(3, 0, 1), 1),
	make_token(TT_NUM, tmp_pos(4, 0, 1), 2, .num = 13),
	make_token(TT_PLUS, tmp_pos(6, 0, 1), 1),
	make_token(TT_NUM, tmp_pos(7, 0, 1), 1, .num = 9),
	make_token(TT_RPAREN, tmp_pos(8, 0, 1), 1),
	make_token(TT_EOF, tmp_pos(9, 0, 1), 0),
};
static const struct parser_error errs2[] = {{0}};
static struct ast *builder2(struct arena *arena) {
	struct ast *lhs = arena_alloc(arena, sizeof(*lhs));
	*lhs = (struct ast) {
		.type = ANT_NUM,
		.span = { .pos = tmp_pos(4, 0, 1), .len = 2 },
		.num = 13,
	};
	struct ast *rhs = arena_alloc(arena, sizeof(*rhs));
	*rhs = (struct ast) {
		.type = ANT_NUM,
		.span = { .pos = tmp_pos(7, 0, 1), .len = 1 },
		.num = 9,
	};
	struct ast *res = arena_alloc(arena, sizeof(*res));
	*res = (struct ast) {
		.type = ANT_BINOP,
		.span = { .pos = tmp_pos(4, 0, 1), .len = 6 },
		.binop = {
			.type = BT_ADD,
			.lhs = lhs,
			.rhs = rhs,
		},
	};
	lhs = arena_alloc(arena, sizeof(*lhs));
	*lhs = (struct ast) {
		.type = ANT_NUM,
		.span = { .pos = tmp_pos(0, 0, 1), .len = 2 },
		.num = 64,
	};
	rhs = res;
	res = arena_alloc(arena, sizeof(*res));
	*res = (struct ast) {
		.type = ANT_BINOP,
		.span = { .pos = tmp_pos(0, 0, 1), .len = 9 },
		.binop = {
			.type = BT_SUB,
			.lhs = lhs,
			.rhs = rhs,
		},
	};

	return res;
}
#undef curr_src

const struct ast_test ast_tests[AST_TESTS_COUNT] = {
	{ .src = src0, .toks = toks0, .builder = builder0, .errs = errs0 },
	{ .src = src1, .toks = toks1, .builder = builder1, .errs = errs1 },
	{ .src = src2, .toks = toks2, .builder = builder2, .errs = errs2 },
};
