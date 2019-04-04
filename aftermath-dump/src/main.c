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

#include <aftermath/core/io_error.h>
#include <aftermath/core/on_disk.h>

struct am_dump_options {
	const char* filename;
	int verbose;
	int print_usage;
	off_t start_offs;
	off_t end_offs;
	int dump_offsets;
};

#define AM_DUMP_MAX_ERRSTACK_NESTING 10
#define AM_DUMP_MAX_ERRSTACK_MSGLEN 256
#define AM_DUMP_MAX_FRAME_TYPES 256

static void print_usage(void)
{
	puts("Aftermath-dump, a utility dumping Aftermath trace files to stdout.\n"
	     "\n"
	     "  Usage: aftermath-dump input_file [-v] [-s offset] [-e offset] [-x]\n"
	     "\n"
	     "  -h              Display this help message.\n"
	     "  -e end_offset   Dump data structures until specified offset in bytes is\n"
	     "                  reached. A value of zero indicates that all data structures\n"
	     "                  after the start offset should be dumped. This is the default\n"
	     "                  value.\n"
	     "                  The offset might also be specified relative to the start offset\n"
	     "                  with a + prefix (e.g., -s 10 -e +100 indicates and end offset\n"
             "                  of 110). If the prefix is used, the start offset must be specified\n"
	     "                  before the end offset.\n"
	     "  -O              Dump the offset of each data structure as a comment\n"
	     "  -s start_offset Dump data structures starting from the specified offset\n"
	     "                  in bytes. Default is zero.\n"
	     "  -v              Verbose output on stderror.\n");
}

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
static int parse_options(struct am_dump_options* o,
			 int argc,
			 char** argv,
			 struct am_io_error_stack* estack)
{
	static const char* options_str = "e:hOs:v";
	int opt;
	char c;
	int start_offs_specified = 0;
	int relative_end_offs = 0;

	/* Default values */
	o->filename = NULL;
	o->verbose = 0;
	o->print_usage = 0;
	o->start_offs = 0;
	o->end_offs = 0;
	o->dump_offsets = 0;

	opterr = 0;

	while((opt = getopt(argc, argv, options_str)) != -1) {
		switch(opt) {
			case 'h':
				o->print_usage = 1;
				break;
			case 'e':
				/* Direct indexing with index 0 is OK here,
				 * since optarg is at least the empty string,
				 * i.e., optarg[0] == '0'. */
				if(optarg[0] == '+') {
					if(!start_offs_specified) {
						am_io_error_stack_push(
							estack,
							AM_IOERR_ASSERT,
							"Start offset must be "
							"specified before a "
							"relative value for the "
							"end offset");
						return 1;
					}

					relative_end_offs = 1;

					/* Skip the plus sign */
					optarg++;
				}

				if(sscanf(optarg, "%jd", &o->end_offs) != 1) {
					am_io_error_stack_push(
						estack,
						AM_IOERR_ASSERT,
						"Invalid value for end "
						"offset: %s.",
						argv[optind-1]);

					return 1;
				}

				if(relative_end_offs) {
					if(o->end_offs < 0) {
						am_io_error_stack_push(
							estack,
							AM_IOERR_ASSERT,
							"Relative end offset "
							"must be positive.");

						return 1;
					}

					o->end_offs += o->start_offs;

					/* Check for overflow */
					if(o->end_offs < o->start_offs) {
						am_io_error_stack_push(
							estack,
							AM_IOERR_ASSERT,
							"Cannot calculate end "
							"offset: overflow.");
						return 1;
					}
				}

				break;
			case 'O':
				o->dump_offsets = 1;
				break;
			case 's':
				if(sscanf(optarg, "%jd", &o->start_offs) != 1) {
					am_io_error_stack_push(
						estack,
						AM_IOERR_ASSERT,
						"Invalid value for start "
						"offset: %s.",
						argv[optind-1]);

					return 1;
				}

				start_offs_specified = 1;
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

					return 1;
				}
				break;
		}
	}

	if(optind < argc) {
		do {
			if(o->filename) {
				am_io_error_stack_push(
					estack,
					AM_IOERR_ASSERT,
					"Filename already specified.");
				return 1;
			} else {
				o->filename = argv[optind];
			}
		} while(++optind < argc);
	}

	return 0;
}

/* Dumps the entire trace specified in the options to stdout.
 *
 * Returns 0 on success, otherwise 1.
 */
static int dump_trace(const struct am_dump_options* o,
		      struct am_io_error_stack* estack)
{
	struct am_io_context ctx;
	struct am_frame_type_registry ftr;
	int ret = 1;

	if(am_frame_type_registry_init(&ftr, AM_DUMP_MAX_FRAME_TYPES))
		goto out;

	if(am_dsk_register_frame_types(&ftr))
		goto out_ftr;

	if(am_io_context_init(&ctx, &ftr))
		goto out_ftr;

	if(am_dsk_dump_trace(&ctx,
			     o->filename,
			     o->start_offs,
			     o->end_offs,
			     o->dump_offsets))
	{
		goto out_ctx;
	}

	ret = 0;

out_ctx:
	am_io_error_stack_move(estack, &ctx.error_stack);
	am_io_context_destroy(&ctx);
out_ftr:
	am_frame_type_registry_destroy(&ftr);
out:
	return ret;
}

int main(int argc, char** argv)
{
	struct am_dump_options options;
	struct am_io_error_stack estack;
	int ret = 1;

	if(am_io_error_stack_init(&estack,
				  AM_DUMP_MAX_ERRSTACK_NESTING,
				  AM_DUMP_MAX_ERRSTACK_MSGLEN))
	{
		goto out;
	}

	if(parse_options(&options, argc, argv, &estack)) {
		am_io_error_stack_push(&estack,
				       AM_IOERR_ASSERT,
				       "Could not parse options.");
		goto out_errstack;
	}

	if(options.print_usage) {
		print_usage();
		ret = 0;
		goto out_errstack;
	}

	if(!options.filename) {
		am_io_error_stack_push(&estack,
				       AM_IOERR_ASSERT,
				       "No input file specified.");
		goto out_errstack;
	}

	if(dump_trace(&options, &estack))
		goto out_dump;

	ret = 0;

out_dump:
	;;
out_errstack:
	if(!am_io_error_stack_empty(&estack) && options.verbose)
		am_io_error_stack_dump_stderr(&estack);

	am_io_error_stack_destroy(&estack);
out:
	return ret;
}
