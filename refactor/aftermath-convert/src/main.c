/**
 * Author: Andi Drebes <andi@drebesium.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <aftermath/core/io_error.h>

#define MAX_ERRSTACK_NESTING 10
#define MAX_ERRSTACK_MSGLEN 256

struct am_convert_options {
	const char* input_filename;
	const char* output_filename;
	int verbose;
};

/* Checks if the short option c is specified as an option on a getopt option
 * string options.
 *
 * Returns 1 if c is a valid option, otherwise 0.
 */
static int is_option(char c, const char* options)
{
	for(; *options; options++)
		if(*options != ':' && c == *options)
			return 1;

	return 0;
}

/* Parses the options from the argument list argv and sets the options in o
 * accordingly. Estack is used to report errors.
 *
 * Returns 0 on success, otherwise 1.
 */
static int parse_options(struct am_convert_options* o,
			 int argc,
			 char** argv,
			 struct am_io_error_stack* estack)
{
	static const char* options_str = "i:o:v";
	int opt;
	char c;

	/* Default values */
	o->input_filename = NULL;
	o->output_filename = NULL;
	o->verbose = 0;

	opterr = 0;

	while((opt = getopt(argc, argv, options_str)) != -1) {
		switch(opt) {
			case 'i':
				o->input_filename = optarg;
				break;
			case 'o':
				o->output_filename = optarg;
				break;
			case 'v':
				o->verbose = 1;
				break;
			default:
				if(strlen(argv[optind-1]) > 1 &&
				   argv[optind-1][0] == '-')
				{
					c = argv[optind-1][1];

					if(!is_option(c, options_str)) {
						am_io_error_stack_push(
							estack,
							AM_IOERR_ASSERT,
							"Unknown option "
							"\"%s\".",
							argv[optind-1]);
					} else {
						am_io_error_stack_push(
							estack,
							AM_IOERR_ASSERT,
							"Option \"%s\" "
							"requires an "
							"argument.",
							argv[optind-1]);
					}
				}
				break;
		}
	}

	return 0;
}

int main(int argc, char** argv)
{
	struct am_convert_options options;
	struct am_io_error_stack estack;
	int ret = 1;
	FILE* in_file = stdin;
	FILE* out_file = stdout;

	if(am_io_error_stack_init(&estack,
				  MAX_ERRSTACK_NESTING,
				  MAX_ERRSTACK_MSGLEN))
	{
		goto out;
	}

	if(parse_options(&options, argc, argv, &estack)) {
		am_io_error_stack_push(&estack,
				       AM_IOERR_ASSERT,
				       "Could not parse options.");
		goto out_errstack;
	}

	if(options.input_filename && strcmp(options.input_filename, "-") != 0) {
		if(!(in_file = fopen(options.input_filename, "r"))) {
			am_io_error_stack_push(&estack,
					       AM_IOERR_READ,
					       "Could not open file \"%s\" "
					       "for reading.",
					       options.input_filename);
			goto out_errstack;
		}
	}

	if(options.output_filename && strcmp(options.output_filename, "-") != 0) {
		if(!(out_file = fopen(options.output_filename, "w+"))) {
			am_io_error_stack_push(&estack,
					       AM_IOERR_READ,
					       "Could not open file \"%s\" "
					       "for writing.",
					       options.output_filename);
			goto out_infile;
		}
	}

	ret = 0;

	if(out_file != stdout)
		fclose(out_file);
out_infile:
	if(in_file != stdin)
		fclose(in_file);
out_errstack:
	if(!am_io_error_stack_empty(&estack) && options.verbose)
		am_io_error_stack_dump_stderr(&estack);

	am_io_error_stack_destroy(&estack);
out:
	return ret;
}
