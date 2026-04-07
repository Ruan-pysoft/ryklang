#include "tokens.h"
#include "ast.h"

#include "test_tokens.h"
#include "test_ast.h"

void load_literal(String_Builder *out, const char *reg, uint64_t lit) {
	sb_appendf(out, "  mov %s, #%lu\n", reg, lit);
}

void compile_num(String_Builder *out, uint64_t num) {
	load_literal(out, "w0", num);
}

void compile_expr(String_Builder *out, const struct ast *ast);
void compile_binop(String_Builder *out, struct binop binop) {
	compile_expr(out, binop.lhs);
	sb_appendf(out, "  str w0, [sp, #-16]!\n");
	compile_expr(out, binop.rhs);
	sb_appendf(out, "  ldr w1, [sp], #16\n");

	switch (binop.type) {
		case BT_ADD: {
			sb_appendf(out, "  add w0, w1, w0\n");
		} break;
	}
}

void compile_expr(String_Builder *out, const struct ast *ast) {
	switch (ast->type) {
		case ANT_NUM: {
			compile_num(out, ast->num);
		} break;
		case ANT_BINOP: {
			compile_binop(out, ast->binop);
		} break;
	}
}

void compile(String_Builder *out, const struct ast *ast) {
	assert(ast != NULL);

	sb_appendf(out,
		".text\n"
		".global _start\n"
		"_start:\n"
	);
	compile_expr(out, ast);
	sb_appendf(out,
		"  mov w8, #93\n"
		"  svc #0\n"
	);
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	puts("Running lexer tests...");
	for (size_t i = 0; i < TOKENS_TESTS_COUNT; ++i) {
		printf("TEST %lu:\n", i);
		String_Builder sb = {0};
		const bool res = test_tokens(token_tests[i], &sb);
		if (sb.count) {
			printf("%.*s", (int)sb.count, sb.items);
		}
		puts(res ? "  PASS" : "  FAIL");
	}

	puts("Running parser tests...");
	for (size_t i = 0; i < AST_TESTS_COUNT; ++i) {
		printf("TEST %lu:\n", i);
		String_Builder sb = {0};
		const bool res = test_ast(ast_tests[i], &sb);
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

	puts("AST:");

	struct arena arena = arena_new(0);
	struct parser_errors perr = {0};
	struct ast *program = parse(&arena, toks, &perr);
	assert(perr.count == 0);
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
