#include "test_tokens.h"

bool test_tokens(struct tokens_test test, String_Builder *out) {
	struct source source = { .name = "<test>", .src = test.src };
	struct lexer_errors err = {0};
	struct token_array lex_toks = lex_source(&source, &err);

	da_foreach(struct token, it, &lex_toks) {
		struct token lex_tok = *it;

		struct token token = *test.toks++;
		token.span.pos.src = &source;

		String_Builder sb_gen = {0};
		token_repr(&sb_gen, lex_tok);
		String_Builder sb_cst = {0};
		token_repr(&sb_cst, token);

		if (!span_cmp(lex_tok.span, token.span)) {
			sb_appendf(out, "  Position mismatch:\n");
			sb_appendf(out, "  got      token %.*s\n", (int)sb_gen.count, sb_gen.items);
			span_pretty(out, lex_tok.span, 4);
			sb_appendf(out, "  expected token %.*s\n", (int)sb_cst.count, sb_cst.items);
			span_pretty(out, token.span, 4);
			return false;
		}

		if (!token_cmp(token, lex_tok)) {
			sb_appendf(out, "  Token mismatch:\n");
			sb_appendf(out, "  got      token %.*s\n", (int)sb_gen.count, sb_gen.items);
			span_pretty(out, lex_tok.span, 4);
			sb_appendf(out, "  expected token %.*s\n", (int)sb_cst.count, sb_cst.items);
			span_pretty(out, token.span, 4);
			return false;
		}

		sb_free(sb_gen);
		sb_free(sb_cst);
	}

	if (lex_toks.items[lex_toks.count - 1].type != TT_EOF) {
		struct token lex_tok = lex_toks.items[lex_toks.count - 1];

		sb_appendf(out, "  Expected EOF at end of tokens, got different token instead:\n");
		sb_appendf(out, "  ");
		token_repr(out, lex_tok);
		sb_append(out, '\n');
		span_pretty(out, lex_tok.span, 4);
		return false;
	}

	sb_free(lex_toks);

	da_foreach(struct lexer_error, it, &err) {
		struct lexer_error error = *test.errs++;
		error.span.pos.src = &source;

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
		sb_appendf(out, "  Didn't get expected errors:\n");
		sb_appendf(out, "  got      no error\n");
		sb_appendf(out, "  expected error %s\n", test.errs->msg);
		span_pretty(out, test.errs->span, 4);
		return false;
	}

	return true;
}

#define tmp_pos(ssrc, offset, line_offset, line_num) ((struct position) { .src = NULL, .at = (ssrc) + (offset), .line_begin = (ssrc) + (line_offset), .line = (line_num) })

static const char *const src0 = "42";
static const struct token toks0[] = {
	make_token(TT_NUM, tmp_pos(src0, 0, 0, 1), 2, .num = 42),
	make_token(TT_EOF, tmp_pos(src0, 2, 0, 1), 0),
};
static const struct lexer_error errs0[] = {{0}};
static const char *const src1 = "   1337   ";
static const struct token toks1[] = {
	make_token(TT_NUM, tmp_pos(src1, 3, 0, 1), 4, .num = 1337),
	make_token(TT_EOF, tmp_pos(src1, 10, 0, 1), 0),
};
static const struct lexer_error errs1[] = {{0}};
static const char *const src2 = "18446744073709551615";
static const struct token toks2[] = {
	make_token(TT_NUM, tmp_pos(src2, 0, 0, 1), 20, .num = ~0ul),
	make_token(TT_EOF, tmp_pos(src2, 20, 0, 1), 0),
};
static const struct lexer_error errs2[] = {{0}};
static const char *const src3 = "18446744073709551616";
static const struct token toks3[] = {
	make_token(TT_NUM, tmp_pos(src3, 0, 0, 1), 20, .num = 0),
	make_token(TT_EOF, tmp_pos(src3, 20, 0, 1), 0),
};
static const struct lexer_error errs3[] = {
	{ .span = { .pos = tmp_pos(src3, 0, 0, 1), .len = 20 }, .msg = "integer overflow while parsing number" },
	{0},
};
static const char *const src4 = "1 +2";
static const struct token toks4[] = {
	make_token(TT_NUM, tmp_pos(src4, 0, 0, 1), 1, .num = 1),
	make_token(TT_PLUS, tmp_pos(src4, 2, 0, 1), 1),
	make_token(TT_NUM, tmp_pos(src4, 3, 0, 1), 1, .num = 2),
	make_token(TT_EOF, tmp_pos(src4, 4, 0, 1), 0),
};
static const struct lexer_error errs4[] = {{0}};
static const char *const src5 = "64-(13+9)";
static const struct token toks5[] = {
	make_token(TT_NUM, tmp_pos(src5, 0, 0, 1), 2, .num = 64),
	make_token(TT_MINUS, tmp_pos(src5, 2, 0, 1), 1),
	make_token(TT_LPAREN, tmp_pos(src5, 3, 0, 1), 1),
	make_token(TT_NUM, tmp_pos(src5, 4, 0, 1), 2, .num = 13),
	make_token(TT_PLUS, tmp_pos(src5, 6, 0, 1), 1),
	make_token(TT_NUM, tmp_pos(src5, 7, 0, 1), 1, .num = 9),
	make_token(TT_RPAREN, tmp_pos(src5, 8, 0, 1), 1),
	make_token(TT_EOF, tmp_pos(src5, 9, 0, 1), 0),
};
static const struct lexer_error errs5[] = {{0}};

const struct tokens_test token_tests[TOKENS_TESTS_COUNT] = {
	{ .src = src0, .toks = toks0, .errs = errs0 },
	{ .src = src1, .toks = toks1, .errs = errs1 },
	{ .src = src2, .toks = toks2, .errs = errs2 },
	{ .src = src3, .toks = toks3, .errs = errs3 },
	{ .src = src4, .toks = toks4, .errs = errs4 },
	{ .src = src5, .toks = toks5, .errs = errs5 },
};
