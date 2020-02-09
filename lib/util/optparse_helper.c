#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "optparse_helper.h"

/**
 * Print ${text} word-wrapped at 80 characters, with a left margin of
 * ${leftmargin} spaces.  The first line is not indented.
 */
static void
print_wordwrap(FILE * stream, const char * text, const int startpos)
{
	const char * word;
	const char * space;
	const char * endpos;
	int first_on_line = 1;
	int linepos = startpos;
	int wordlen;

	word = text;
	endpos = text + strlen(text);

	while ((word < endpos)) {
		/* Find length of next word. */
		if ((space = strchr(word, ' ')) == NULL)
			space = endpos;

		assert(space - word < INT_MAX);
		wordlen = (int)(space - word);

		/* Newline if necessary, but never for the first word. */
		if (!first_on_line && (linepos + wordlen + 1) > 80) {
			fprintf(stream, "\n%*c", startpos, ' ');
			linepos = startpos;
			first_on_line = 1;
		}

		if (first_on_line) {
			/* Print the first word, regardless of length. */
			fprintf(stream, "%.*s", wordlen, word);
			linepos += wordlen;
			first_on_line = 0;
		} else {
			/* Print a space, then the next word. */
			fprintf(stream, " %.*s", wordlen, word);
			linepos += wordlen + 1;
		}

		/* Advance to after the word we just printed. */
		word += wordlen + 1;
	}
}

/**
 * print_help(description, args, num_args):
 * Print a --help message with a one-line ${description} and ${num_args}
 * nicely formatted arguments ${args}.
 */
void
print_help(FILE * stream, const char * description, const struct args * args,
    const size_t num_args)
{
	size_t i;
	size_t longest = 0;
	size_t length;
	size_t remaining;

	/* Print one-line description. */
	fprintf(stream, "%s\n\n", description);

	/* Find longest set of short + long + arg. */
	for (i = 0; i < num_args; i++) {
		length = strlen(args[i].arg_short) + strlen(args[i].arg_long)
		    + strlen(args[i].arg_optarg) + 2 + 2 + 1 + 2;
		if(longest < length)
			longest = length;
	}

	/* Print args. */
	for (i = 0; i < num_args; i++) {
		remaining = longest;

		/* Initial indent. */
		fprintf(stream, "  ");
		remaining -= 2;

		/* Print short, long, and optarg (if applicable). */
		if((length = strlen(args[i].arg_short)) > 0) {
			fprintf(stream, "%s, ", args[i].arg_short);
			remaining -= (length + 2);
		}
		if((length = strlen(args[i].arg_long)) > 0) {
			fprintf(stream, "%s ", args[i].arg_long);
			remaining -= (length + 1);
		}
		if((length = strlen(args[i].arg_optarg)) > 0) {
			fprintf(stream, "%s  ", args[i].arg_optarg);
			remaining -= (length + 2);
		}

		/* Align the description with the longest set of args. */
		assert(remaining <= longest);
		if(remaining > 0)
			fprintf(stream, "%*c", (int)remaining, ' ');

		/* Print the description. */
		assert(longest < INT_MAX);
		print_wordwrap(stream, args[i].arg_explain, (int)longest);

		/* End of this entry. */
		fprintf(stream, "\n");
	}
}
