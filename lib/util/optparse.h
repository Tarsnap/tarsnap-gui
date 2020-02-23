#ifndef OPTPARSE_H
#define OPTPARSE_H

/** \brief Command-line options. */
struct optparse {
	/** \brief Configuration direcotry. */
	char * config_dir;
	/** \brief Execute scheduled jobs, then quit. */
	int jobs;
	/** \brief Check that the GUI is correctly installed. */
	int check;
};

/**
 * optparse_parse(argc, argv):
 * Parse the command line and return options, setting up default
 * values as necessary.  Return NULL upon error.
 */
struct optparse * optparse_parse(int, char * const []);

/**
 * optparse_free(opt):
 * Free memory from ${opt}.
 */
void optparse_free(struct optparse *);

#endif
