/*
 * Sample command line parser.
 *
 * Implements sub-commands with their own option handlers.
 * 
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <argp.h>


// The idea is to load each command and have them provide their own
// option handlers.
// #include "commands/init_db.h"
// #include "commands/create_su.h"
// #include "commands/create_org.h"
// #include "commands/create_site.h"
// #include "commands/run.h
// run: server(admin), renderer, resource-manager, [defaults to: all]

/** Argp Wrapper Functions **/

/** Local Prototypes **/

struct arg_global;

const char* argp_key(int key, char* keystr);
void log_printf(struct arg_global* g, int level, const char* fmt, ...);
void cmd_global(int argc, char**argv);
void cmd_aa(struct argp_state* state);

/** Global Options **/

struct arg_global
{
	int verbosity;
};

const char *argp_program_version = "walden 0.1";
const char *argp_program_bug_address = "https://github.com/kitdallege/walden/issues";

error_t argp_err_exit_status = 1;

static struct argp_option opt_global[] =
{
/* { name, key, arg-name, flags,
 *			doc, group } */

	{ "verbose", 'v', "level", OPTION_ARG_OPTIONAL,
				"Increase or set the verbosity level.", -1},

	{ "quiet", 'q', 0, 0,
				"Set verbosity to 0.", -1},

	// make -h an alias for --help
	{ 0 }
};

static char doc_global[] =
	"\n"
	"Example of parsing a nested command line."
	"\v"
	"Supported commands are:\n"
	"  aa			Do aa with great panache."
	;

static error_t parse_global(int key, char* arg, struct argp_state* state)
{
	struct arg_global* global = state->input;
	char keystr[2];

	log_printf(global, 3, "x: parsing %s = '%s'\n",
			argp_key(key, keystr), arg ? arg : "(null)");

	switch(key)
	{
		case 'v':
			if(arg)
				global->verbosity = atoi(arg);
			else
				global->verbosity++;
			log_printf(global, 2, "x: set verbosity to %d\n", global->verbosity);
			break;

		case 'q':
			log_printf(global, 2, "x: setting verbosity to 0\n");
			global->verbosity = 0;
			break;
		
		case ARGP_KEY_ARG:
			assert( arg );
			if(strcmp(arg, "aa") == 0) {
				cmd_aa(state);
			} else {
				argp_error(state, "%s is not a valid command", arg);
			}
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp =
{
	opt_global,
	parse_global,
	"[<cmd> [CMD-OPTIONS]]...",
	doc_global,
};

void cmd_global(int argc, char**argv)
{
	struct arg_global global =
	{
			1, /* default verbosity */
	};

	log_printf(&global, 3, "x: begin (argc = %d, argv[0] = %s)\n",
			argc, argv[0]);

	argp_parse(&argp, argc, argv, ARGP_IN_ORDER, NULL, &global);
}

/** AA Command **/

struct arg_aa
{
	struct arg_global* global;

	char* name;
};

static struct argp_option opt_aa[] =
{
	{ "out", 'o', "file", 0,
				"The output file." },

	{ "external", 'x', 0, 0,
				"External." },

	{ 0 }
};

static char doc_aa[] =
	"\n"
	"The aa doc prefix."
	"\v"
	"The aa doc suffix."
	;

static error_t parse_aa(int key, char* arg, struct argp_state* state)
{
	struct arg_aa* aa = state->input;
	char keystr[2];
	
	assert( aa );
	assert( aa->global );

	log_printf(aa->global, 3, "x aa: parsing %s = '%s'\n",
			argp_key(key, keystr), arg ? arg : "(null)");

	switch(key)
	{
		case 'o':
			log_printf(aa->global, 2, "x aa: -o, output = %s \n", arg);
			break;

		case 'x':
			log_printf(aa->global, 2, "x aa: -x, external\n");
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp_aa =
{
	opt_aa,
	parse_aa,
	0,
	doc_aa
};

void cmd_aa(struct argp_state* state)
{
	struct arg_aa aa = { 0, };
	int			argc = state->argc - state->next + 1;
	char** argv = &state->argv[state->next - 1];
	char*  argv0 =	argv[0];

	aa.global = state->input;

	log_printf(aa.global, 3, "x aa: begin (argc = %d, argv[0] = %s)\n",
			argc, argv[0]);

	argv[0] = malloc(strlen(state->name) + strlen(" aa") + 1);

	if(!argv[0])
		argp_failure(state, 1, ENOMEM, 0);

	sprintf(argv[0], "%s aa", state->name);

	argp_parse(&argp_aa, argc, argv, ARGP_IN_ORDER, &argc, &aa);

	free(argv[0]);

	argv[0] = argv0;

	state->next += argc - 1;

	log_printf(aa.global, 3, "x aa: end (next = %d, argv[next] = %s)\n",
			state->next, state->argv[state->next]);

	return;
}

/** Main **/

int main(int argc, char** argv)
{
	setvbuf(stdout, (char *)NULL, _IOLBF, 0);
	setvbuf(stderr, (char *)NULL, _IOLBF, 0);

	cmd_global(argc, argv);

	return 0;
}

/** Logging **/

void log_printf(struct arg_global* g, int level, const char* fmt, ...)
{
	va_list ap;
	FILE* f = stdout;

	if(g->verbosity < level)
		return;

	if(level == 0)
		f = stderr;

	va_start(ap, fmt);

	vfprintf(f, fmt, ap);

	va_end(ap);
}

const char* argp_key(int key, char* keystr)
{
	keystr[0] = key;
	keystr[1] = 0;

	switch(key)
	{
		case ARGP_KEY_ARG:			return "ARGP_KEY_ARG";
		case ARGP_KEY_ARGS:			return "ARGP_KEY_ARGS";
		case ARGP_KEY_END:			return "ARGP_KEY_END";
		case ARGP_KEY_NO_ARGS: return "ARGP_KEY_NO_ARGS";
		case ARGP_KEY_INIT:			return "ARGP_KEY_INIT";
		case ARGP_KEY_SUCCESS: return "ARGP_KEY_SUCCESS";
		case ARGP_KEY_ERROR:		return "ARGP_KEY_ERROR";
		case ARGP_KEY_FINI:			return "ARGP_KEY_FINI";
	}

	return keystr;
}

