#include "tokens.h"
#include "ast.h"

bool test_tokens(const char *src, const struct token *tokens, const struct lexer_error *errors) {
	struct source source = { .name = "<test>", .src = src };
	struct lexer_errors err = {0};
	struct lexer lex = lexer_new(&source);

	for (;;) {
		struct token lex_tok = lexer_next(&lex, &err);

		struct token token = *tokens++;
		token.span.pos.src = &source;

		String_Builder sb_gen = {0};
		token_repr(&sb_gen, lex_tok);
		String_Builder sb_cst = {0};
		token_repr(&sb_cst, token);

		if (!span_cmp(lex_tok.span, token.span)) {
			printf("  Position mismatch:\n");
			printf("  got      token %.*s\n", (int)sb_gen.count, sb_gen.items);
			sb_print(span_pretty, lex_tok.span, 4);
			printf("  expected token %.*s\n", (int)sb_cst.count, sb_cst.items);
			sb_print(span_pretty, token.span, 4);
			return false;
		}

		if (!token_cmp(token, lex_tok)) {
			printf("  Token mismatch:\n");
			printf("  got      token %.*s\n", (int)sb_gen.count, sb_gen.items);
			sb_print(span_pretty, lex_tok.span, 4);
			printf("  expected token %.*s\n", (int)sb_cst.count, sb_cst.items);
			sb_print(span_pretty, token.span, 4);
			return false;
		}

		sb_free(sb_gen);
		sb_free(sb_cst);

		if (lex_tok.type == TT_EOF) break;
	}

	da_foreach(struct lexer_error, it, &err) {
		struct lexer_error error = *errors++;
		error.span.pos.src = &source;

		if (error.msg == NULL) {
			printf("  Unexpected error:\n");
			printf("  got      error %s\n", it->msg);
			sb_print(span_pretty, it->span, 4);
			printf("  expected no error\n");
			return false;
		}

		if (!span_cmp(error.span, it->span)) {
			printf("  Got errors at wrong position:\n");
			printf("  got      error %s\n", it->msg);
			sb_print(span_pretty, it->span, 4);
			printf("  expected error %s\n", error.msg);
			sb_print(span_pretty, error.span, 4);
			return false;
		}

		if (strcmp(error.msg, it->msg) != 0) {
			printf("  Got unexpected errors message:\n");
			printf("  got      error %s\n", it->msg);
			sb_print(span_pretty, it->span, 4);
			printf("  expected error %s\n", error.msg);
			sb_print(span_pretty, error.span, 4);
			return false;
		}
	}

	if (errors->msg != NULL) {
		printf("  Didn't get expected errors:\n");
		printf("  got      no error\n");
		printf("  expected error %s\n", errors->msg);
		sb_print(span_pretty, errors->span, 4);
		return false;
	}

	return true;
}

#define tmp_pos(ssrc, offset, line_offset, line_num) ((struct position) { .src = NULL, .at = (ssrc) + (offset), .line_begin = (ssrc) + (line_offset), .line = (line_num) })

const char *const src0 = "42";
const struct token toks0[] = {
	make_token(TT_NUM, tmp_pos(src0, 0, 0, 1), 2, .num = 42),
	make_token(TT_EOF, tmp_pos(src0, 2, 0, 1), 0),
};
const struct lexer_error errs0[] = {{0}};
const char *const src1 = "   1337   ";
const struct token toks1[] = {
	make_token(TT_NUM, tmp_pos(src1, 3, 0, 1), 4, .num = 1337),
	make_token(TT_EOF, tmp_pos(src1, 10, 0, 1), 0),
};
const struct lexer_error errs1[] = {{0}};
const char *const src2 = "18446744073709551615";
const struct token toks2[] = {
	make_token(TT_NUM, tmp_pos(src2, 0, 0, 1), 20, .num = ~0ul),
	make_token(TT_EOF, tmp_pos(src2, 20, 0, 1), 0),
};
const struct lexer_error errs2[] = {{0}};
const char *const src3 = "18446744073709551616";
const struct token toks3[] = {
	make_token(TT_NUM, tmp_pos(src3, 0, 0, 1), 20, .num = 0),
	make_token(TT_EOF, tmp_pos(src3, 20, 0, 1), 0),
};
const struct lexer_error errs3[] = {
	{ .span = { .pos = tmp_pos(src3, 0, 0, 1), .len = 20 }, .msg = "integer overflow while parsing number" },
	{0},
};

struct {
	const char *src;
	const struct token *toks;
	const struct lexer_error *errs;
} tests[] = {
	{ .src = src0, .toks = toks0, .errs = errs0 },
	{ .src = src1, .toks = toks1, .errs = errs1 },
	{ .src = src2, .toks = toks2, .errs = errs2 },
	{ .src = src3, .toks = toks3, .errs = errs3 },
};

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	for (size_t i = 0; i < ARRAY_LEN(tests); ++i) {
		printf("TEST %lu:\n", i);
		puts(test_tokens(tests[i].src, tests[i].toks, tests[i].errs) ? "  PASS" : "  FAIL");
	}

	const char *code = "42";
	const char *file = "<test input>";
	struct source source = { .name = file, .src = code };
	struct lexer_errors err = {0};
	struct lexer lex = lexer_new(&source);
	const struct lexer saved = lex;

	for (
		struct token tok = lexer_next(&lex, &err);
		tok.type != TT_EOF;
		tok = lexer_next(&lex, &err)
	) {
		String_Builder sb = {0};
		token_repr(&sb, tok);
		printf(" - %.*s\n", (int)sb.count, sb.items);
		sb_free(sb);
	}

	if (err.count) {
		printf("Encountered %lu errors:\n", err.count);
		da_foreach(struct lexer_error, it, &err) {
			printf("  got error %s\n", it->msg);
			sb_print(span_pretty, it->span, 4);
		}
	}

	le_free(&err);
	lex = saved;
	struct arena arena = arena_new(0);
	struct parser parser = parser_new(&arena, &lex, &err);

	struct ast *program = parse(&parser);
	assert(parser.err.count == 0);
	sb_print(ast_repr, program);
	putchar('\n');
}

#define NOB_IMPLEMENTATION
#include "nob.h"
