#pragma once

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "util.h"
#include "ast.h"

struct ast_test {
	struct source src;
	struct token *toks;
	struct ast *(*builder)(struct arena *arena);
	const struct parser_error *errs;
};

#define AST_TESTS_COUNT 3
extern const struct ast_test ast_tests[AST_TESTS_COUNT];

bool test_ast(struct ast_test test, String_Builder *out);
