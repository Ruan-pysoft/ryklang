#include "tokens.h"

size_t line(const char *src, const char *pos) {
	size_t line = 1;

	while (src < pos) {
		if (*src++ == '\n') ++line;
	}

	return line;
}
size_t col(const char *src, const char *pos) {
	size_t col = 1;

	while (src < pos && *--pos != '\n') {
		++col;
	}

	return col;
}

bool test_tokens(const char *src, const struct token *tokens) {
	struct lexer_errors err = {0};
	struct lexer lex = lexer_new(src);

	for (;;) {
		lex = lexer_adv(lex, &err);
		assert(lex.has_tok);

		struct token token = *tokens++;

		String_Builder sb_gen = {0};
		token_repr(&sb_gen, lex.tok);
		String_Builder sb_cst = {0};
		token_repr(&sb_cst, token);

		if (lex.tok.pos != token.pos) {
			printf("  Position mismatch:\n");
			printf("  got      @ %lu,%lu %.*s\n", line(src, lex.tok.pos), col(src, lex.tok.pos), (int)sb_gen.count, sb_gen.items);
			printf("  expected @ %lu,%lu %.*s\n", line(src, token.pos), col(src, token.pos), (int)sb_gen.count, sb_gen.items);
			return false;
		}
		if (lex.tok.len != token.len) {
			printf("  Length mismatch:\n");
			printf("  got      @ %lu,%lu %.*s\n", line(src, lex.tok.pos), col(src, lex.tok.pos), (int)sb_gen.count, sb_gen.items);
			printf("  expected @ %lu,%lu %.*s\n", line(src, token.pos), col(src, token.pos), (int)sb_gen.count, sb_gen.items);
			return false;
		}

		if (strncmp(sb_gen.items, sb_cst.items, sb_gen.count) != 0) {
			printf("  Token mismatch:\n");
			printf("  got      @ %lu,%lu %.*s\n", line(src, lex.tok.pos), col(src, lex.tok.pos), (int)sb_gen.count, sb_gen.items);
			printf("  expected @ %lu,%lu %.*s\n", line(src, token.pos), col(src, token.pos), (int)sb_gen.count, sb_gen.items);
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
const char *const src1 = "   1337   ";
const struct token toks1[] = {
	make_token(TT_NUM, src1 + 3, 4, .num = 1337),
	make_token(TT_EOF, src1 + 10, 0),
};
const char *const src2 = "18446744073709551615";
const struct token toks2[] = {
	make_token(TT_NUM, src2 + 0, 20, .num = ~0ul),
	make_token(TT_EOF, src2 + 20, 0),
};

struct {
	const char *src;
	const struct token *toks;
} tests[] = {
	{ .src = src0, .toks = toks0 },
	{ .src = src1, .toks = toks1 },
	{ .src = src2, .toks = toks2 },
};

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	for (size_t i = 0; i < ARRAY_LEN(tests); ++i) {
		printf("TEST %lu:\n", i);
		puts(test_tokens(tests[i].src, tests[i].toks) ? "  PASS" : "  FAIL");
	}

	const char *src = "18446744073709551616";
	struct lexer_errors err = {0};
	struct lexer lex = lexer_new(src);

	for (;;) {
		lex = lexer_adv(lex, &err);
		assert(lex.has_tok);

		if (lex.tok.type == TT_EOF) break;

		String_Builder sb = {0};
		token_repr(&sb, lex.tok);
		printf(" - %.*s\n", (int)sb.count, sb.items);
		sb_free(sb);
	}

	if (err.count) {
		printf("Encountered %lu errors:\n", err.count);
		da_foreach(struct lexer_error, it, &err) {
			printf("  @ %lu,%lu: %s\n", line(lex.src, it->pos), col(lex.src, it->pos), it->msg);
		}
	}
}

#define NOB_IMPLEMENTATION
#include "nob.h"
