#include "tokens.h"
#include "ast.h"

#include "test_tokens.h"

void compile(String_Builder *out, const struct ast *ast) {
	assert(ast != NULL);
	assert(ast->type == ANT_NUM);

	sb_appendf(out,
		".text\n"
		".global _start\n"
		"_start:\n"
		"  mov x0, #%lu\n"
		"  mov w8, #93\n"
		"  svc #0\n"
	, ast->num);
}

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
	if (argc == 2) code = argv[1];
	printf("SOURCE CODE: %s\n", code);
	const char *file = "<test input>";
	struct source source = { .name = file, .src = code };
	struct lexer_errors err = {0};

	puts("TOKENS:");

	struct token_array toks = lex_source(&source, &err);

	da_foreach(struct token, tok, &toks) {
		printf(" - ");
		sb_print(token_repr, *tok);
		putchar('\n');
	}

	if (err.count) {
		printf("Encountered %lu errors:\n", err.count);
		da_foreach(struct lexer_error, it, &err) {
			printf("  got error %s\n", it->msg);
			sb_print(span_pretty, it->span, 4);
		}
	}

	le_free(&err);
	struct lexer lex = lexer_new(&source);
	struct arena arena = arena_new(0);
	struct parser parser = parser_new(&arena, &lex, &err);

	puts("AST:");

	struct ast *program = parse(&parser);
	assert(parser.err.count == 0);
	sb_print(ast_repr, program);
	putchar('\n');

	String_Builder sb = {0};
	compile(&sb, program);

	FILE *outf = fopen("out.as", "w");
	fwrite(sb.items, 1, sb.count, outf);
	fclose(outf);

	Cmd cmd = {0};
	cmd_append(&cmd, "as", "out.as", "-o", "out.o");
	if (!cmd_run(&cmd)) return 1;
	cmd_append(&cmd, "ld", "out.o", "-o", "out");
	if (!cmd_run(&cmd)) return 1;
}

#define NOB_IMPLEMENTATION
#include "nob.h"
