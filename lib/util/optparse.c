#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "optparse_helper.h"
#include "warnp.h"

#include "optparse.h"

/* Usage description. */
static const char * DESCRIPTION = "Tarsnap GUI - Online backups for the"
    " truly paranoid (yet graphically inclined)";

/* Arguments description. */
static const struct args ARGS[] = {
	{ "-h", "--help", "", "Displays this help." },
	{ "-v", "--version", "", "Displays version information." },
	{ "-j", "--jobs", "", "Executes all Jobs sequentially that have the"
	    " 'Automatic backup schedule' option enabled.  The application"
	    " runs headless and useful information is printed to the standard"
	    " out and error." },
	{ "-a", "--appdata", "<directory>", "Use the specified config"
	    " directory.  Useful for multiple configurations on the same"
	    " machine (INI format is implied)." },
	{ "", "--check", "", "Check that Tarsnap GUI is correctly installed." }
};
static const size_t NUM_ARGS = sizeof(ARGS) / sizeof(ARGS[0]);

/* Allocate and initialize optparse. */
static struct optparse *
optparse_init()
{
	struct optparse * opt;

	/* Allocate structure. */
	if ((opt = malloc(sizeof(struct optparse))) == NULL) {
		warnp("malloc");
		goto err0;
	}

	/* Initialize structure. */
	memset(opt, 0, sizeof(struct optparse));
	opt->config_dir = NULL;

	/* Success! */
	return (opt);

err0:
	/* Failure! */
	return (NULL);
}

/**
 * optparse_parse(argc, argv):
 * Parse the command line and return options, setting up default
 * values as necessary.  Return NULL upon error.
 */
struct optparse *
optparse_parse(int argc, char * const argv[])
{
	struct optparse * opt;
	const char * ch;

	/* Allocate and initialize structure. */
	if ((opt = optparse_init()) == NULL)
		goto err0;

	/* Parse command-line options. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-h"):
		GETOPT_OPT("--help"):
			print_help(stdout, DESCRIPTION, ARGS, NUM_ARGS);
			goto quit1;
		GETOPT_OPT("-v"):
		GETOPT_OPT("--version"):
			fprintf(stdout, "tarsnap-gui " APP_VERSION "\n");
			goto err1;
		GETOPT_OPT("-j"):
		GETOPT_OPT("--jobs"):
			if (opt->jobs != 0)
				OPT_ALREADY_GIVEN("-j or --jobs");
			opt->jobs = 1;
			break;
		GETOPT_OPT("--check"):
			if (opt->check != 0)
				OPT_ALREADY_GIVEN("--check");
			opt->check = 1;
			break;
		GETOPT_OPTARG("-a"):
		GETOPT_OPTARG("--appdata"):
			if (opt->config_dir != NULL)
				OPT_ALREADY_GIVEN("--appdata");
			if ((opt->config_dir = strdup(optarg)) == NULL)
				OPT_MEM_FAIL(ch);
			break;
		GETOPT_MISSING_ARG:
			warn0("missing argument -- %s\n", ch);
			print_help(stderr, DESCRIPTION, ARGS, NUM_ARGS);
			goto err1;
		GETOPT_DEFAULT:
			/* Don't complain about unrecognized options. */
			break;
		}
	}
	argc -= optind;
	argv += optind;

	(void)argc; /* argc is not used beyond this point. */
	(void)argv; /* argv is not used beyond this point. */

	/* We might have unused arguments for Qt (like --platform offscreen). */

	/* Success! */
	return (opt);

quit1:
	optparse_free(opt);

	/* Acceptable exit. */
	exit(0);

err1:
	optparse_free(opt);
err0:
	/* Failure! */
	return (NULL);
}

/**
 * optparse_free(opt):
 * Free memory from ${opt}.
 */
void
optparse_free(struct optparse * opt)
{

	/* Behave consistently with free(NULL). */
	if (opt == NULL)
		return;

	free(opt->config_dir);
	free(opt);
}
