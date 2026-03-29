#include "tokens.h"

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
	}
}

#define NOB_IMPLEMENTATION
#include "nob.h"
