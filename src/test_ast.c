#include "test_ast.h"

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

	arena_free(&arena);

	da_foreach(struct parser_error, it, &err) {
		struct parser_error error = *test.errs++;
		//error.span.pos.src = &source;

		if (error.msg == NULL) {
			sb_appendf(out, "  Unexpected error:\n");
			sb_appendf(out, "  got      error %s\n", it->msg);
			//span_pretty(out, it->span, 4);
			sb_appendf(out, "  expected no error\n");
			return false;
		}

		if (!span_cmp(error.span, it->span)) {
			sb_appendf(out, "  Got errors at wrong position:\n");
			sb_appendf(out, "  got      error %s\n", it->msg);
			//span_pretty(out, it->span, 4);
			sb_appendf(out, "  expected error %s\n", error.msg);
			//span_pretty(out, error.span, 4);
			return false;
		}

		if (strcmp(error.msg, it->msg) != 0) {
			sb_appendf(out, "  Got unexpected errors message:\n");
			sb_appendf(out, "  got      error %s\n", it->msg);
			//span_pretty(out, it->span, 4);
			sb_appendf(out, "  expected error %s\n", error.msg);
			//span_pretty(out, error.span, 4);
			return false;
		}
	}

	if (test.errs->msg != NULL) {
		sb_appendf(out, "  Didn't get expected errors:\n");
		sb_appendf(out, "  got      no error\n");
		sb_appendf(out, "  expected error %s\n", test.errs->msg);
		//span_pretty(out, test.errs->span, 4);
		return false;
	}

	return true;
}

#define tmp_pos(offset, line_offset, line_num) ((struct position) { .src = NULL, .at = (char*)(offset), .line_begin = (char*)(line_offset), .line = (line_num) })

//const char *const src0 = "42";
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

const struct ast_test ast_tests[AST_TESTS_COUNT] = {
	{ .toks = toks0, .builder = builder0, .errs = errs0 },
};
