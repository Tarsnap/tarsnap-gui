#ifndef OPTPARSE_HELPER_H
#define OPTPARSE_HELPER_H

#include <stdio.h>

/* --help messages for command-line arguments. */
struct args {
	const char * arg_short;
	const char * arg_long;
	const char * arg_optarg;
	const char * arg_explain;
};

/**
 * print_help(stream, description, args, num_args):
 * Print a --help message with a one-line ${description} and ${num_args}
 * nicely formatted arguments ${args}.
 */
void print_help(FILE *, const char *, const struct args *, const size_t);

/* Simplify error-handling in the command-line parse loop. */
#define OPT_EPARSE(opt, arg) do {					\
	warnp("Error parsing argument: %s %s", opt, arg);		\
	exit(1);							\
} while (0)

#define OPT_MEM_FAIL(opt) do {						\
	warnp("Failed to allocate memory while parsing argument: %s", opt); \
	exit(1);							\
} while (0)

#define OPT_ALREADY_GIVEN(opt) do {					\
	warnp("Command-line cannot contain more than one %s", opt);	\
	exit(1);							\
} while (0)

#endif
