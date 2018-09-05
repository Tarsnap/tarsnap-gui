#ifndef OPTPARSE_H
#define OPTPARSE_H

/* Command-line options. */
struct optparse {
	char * configdata;
	int jobs;
	int check;
};

/**
 * optparse_parse(argc, argv):
 * Parse the command line and return options, setting up default
 * values as necessary.  Return NULL upon error.
 */
struct optparse * optparse_parse(int, char **);

/**
 * optparse_free(opt):
 * Free memory from ${opt}.
 */
void optparse_free(struct optparse *);

#endif
