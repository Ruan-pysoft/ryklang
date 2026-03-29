#include "tokens.h"

bool test_tokens(const char *src, const struct token *tokens) {
	struct lexer lex = lexer_new(src);

	for (;;) {
		lex = lexer_adv(lex);
		assert(lex.has_tok);

		struct token token = *tokens++;

		if (lex.tok.pos != token.pos) return false;
		if (lex.tok.len != token.len) return false;

		String_Builder sb_gen = {0};
		token_repr(&sb_gen, lex.tok);
		String_Builder sb_cst = {0};
		token_repr(&sb_cst, token);

		if (strncmp(sb_gen.items, sb_cst.items, sb_gen.count) != 0) {
			return false;
		}

		sb_free(sb_gen);
		sb_free(sb_cst);

		if (lex.tok.type == TT_EOF) return true;
	}
}

const char *const src0 = "42";
const struct token toks0[] = {
	make_token(TT_NUM, src0 + 0, 2, .num = 42),
	make_token(TT_EOF, src0 + 2, 0),
};

struct {
	const char *src;
	const struct token *toks;
} tests[] = {
	{ .src = src0, .toks = toks0 },
};

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	const char *src = "42";
	struct lexer lex = lexer_new(src);

	for (;;) {
		lex = lexer_adv(lex);
		assert(lex.has_tok);

		if (lex.tok.type == TT_EOF) break;

		String_Builder sb = {0};
		token_repr(&sb, lex.tok);
		printf(" - %.*s\n", (int)sb.count, sb.items);
		sb_free(sb);
	}

	for (size_t i = 0; i < ARRAY_LEN(tests); ++i) {
		printf("TEST %lu:\n", i);
		puts(test_tokens(tests[i].src, tests[i].toks) ? "  PASS" : "  FAIL");
	}
}

#define NOB_IMPLEMENTATION
#include "nob.h"
