#include "tokens.h"
#include "ast.h"

#include "test_tokens.h"

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	for (size_t i = 0; i < TOKENS_TESTS_COUNT; ++i) {
		printf("TEST %lu:\n", i);
		String_Builder sb = {0};
		const bool res = test_tokens(token_tests[i], &sb);
		if (sb.count) {
			printf("%.*s", (int)sb.count, sb.items);
		}
		puts(res ? "  PASS" : "  FAIL");
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
