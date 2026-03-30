#include <string.h>
#include <stddef.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

bool endswith(const char *str, const char *end) {
	const size_t str_len = strlen(str);
	const size_t end_len = strlen(end);

	return str_len >= end_len && strcmp(str + str_len - end_len, end) == 0;
}

#define CC_FLAGS "-Wall", "-Wextra", "-std=c99", "-g", "-I."

Procs procs = {0};
struct {
	const char **items;
	size_t count;
	size_t capacity;
} objects = {0};

bool process_src(Nob_Walk_Entry entry) {
	printf("got file: %s at depth %lu and type %d\n", entry.path, entry.level, entry.type);
	if (entry.type == FILE_REGULAR && endswith(entry.path, ".c")) {
		puts("  got c file!");

		String_Builder sb = {0};

		sb_appendf(&sb, "build/%.*s.o", (int)strlen(entry.path) - 6, entry.path + 4);
		sb_append_null(&sb);

		da_append(&objects, sb.items);

		printf("  obj file: %s\n", sb.items);

		Cmd cmd = {0};
		cmd_append(&cmd, "gcc", "-c", CC_FLAGS, entry.path, "-o", sb.items);

		//return cmd_run(&cmd, .async = &procs);
		return cmd_run(&cmd);
	}

	return true;
}

int main(int argc, char **argv) {
	GO_REBUILD_URSELF(argc, argv);

	if (!mkdir_if_not_exists("build/")) return 1;

	if (!walk_dir("src/", process_src)) return 1;
	if (!procs_flush(&procs)) return 1;

	Cmd cmd = {0};

	cmd_append(&cmd, "gcc");
	cmd_extend(&cmd, &objects);
	cmd_append(&cmd, "-o", "prog");

	return cmd_run(&cmd) ? 0 : 1;
}
