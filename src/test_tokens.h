#pragma once

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "util.h"
#include "tokens.h"

struct tokens_test {
	const char *src;
	const struct token *toks;
	const struct lexer_error *errs;
};

#define TOKENS_TESTS_COUNT 6
extern const struct tokens_test token_tests[TOKENS_TESTS_COUNT];

bool test_tokens(struct tokens_test test, String_Builder *out);
