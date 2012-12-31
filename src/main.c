/*
 * main.c -- The main()
 *
 * Copyright (C) 2003-2011  Davide Angelocola <davide.angelocola@gmail.com>
 
 * Sem is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "sem.h"
#include "version.h"

#define S(s) STRINGIFY(s)
#define STRINGIFY(s) #s
#define DEFAULT_DATA_SIZE 64
#define DEFAULT_STACK_SIZE 32

static char license[] = "\r\
sem " VERSION " -- A SIMPLESEM interpreter\n\
Copyright (C) 2003-2011 Davide Angelocola <davide.angelocola@gmail.com>\n\
\n\
This program is free software, and you are welcome to redistribute\n\
it and/or modify it under the terms of the GNU General Public License.\n\
There is ABSOLUTELY NO WARRANTY for this program.\n\
\n\
See the COPYING filefor more details.\n";

static char help[] = "\r\
Usage: sem [options] file\n\
\n\
Options:\n\
  -h : print this help message and exit\n\
  -d : interactive debugger\n\
  -m : set the data memory size (the default is " S(DEFAULT_DATA_SIZE) ")\n\
  -s : set the stack size (the default is " S(DEFAULT_STACK_SIZE) ")\n\
  -v : print the version and exit\n\
\n\
Report bugs to <davide.angelocola@gmail.com>\n";

static void usage(int sts)
{
	FILE *target = (sts == EXIT_SUCCESS) ? stdout : stderr;
	fprintf(target, "%s", help);
	exit(sts);
}

int main(int argc, char **argv)
{
	int debugger = 0;
	int mem_size = DEFAULT_DATA_SIZE;
	int stack_size = DEFAULT_STACK_SIZE;
	int opt = 0;
	struct option long_options[] = {
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{"debug", 0, 0, 'd'},
		{NULL, 0, 0, 'm'},
		{NULL, 0, 0, 's'},

		/* Sentinel. */
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long(argc, argv, "hm:s:vd", long_options, NULL))
	       != EOF) {
		switch (opt) {
		case 'h':
			usage(EXIT_SUCCESS);

		case 'm':
			if (sscanf(optarg, "%d", &mem_size) != 1
			    || mem_size < 1) {
				fprintf(stderr,
					"sem: invalid memory size (%s)\n",
					optarg);
				return EXIT_FAILURE;
			}
			break;

		case 's':
			if (sscanf(optarg, "%d", &stack_size) != 1
			    || stack_size < 1) {
				fprintf(stderr,
					"sem: invalid stack size (%s)\n",
					optarg);
				return EXIT_FAILURE;
			}
			break;

		case 'd':
			debugger = 1;
			break;

		case 'v':
			fprintf(stdout, "%s", license);
			return EXIT_SUCCESS;

		default:
			usage(EXIT_FAILURE);
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "sem: no input\n");
		return EXIT_FAILURE;
	}

	int status;
	char *filename = argv[optind];
	struct code *code = compile_code(filename);

	if (code == NULL) {
		// error message should be already displayed at this point
		return EXIT_FAILURE;
	}

	struct vm *vm = vm_init(mem_size, stack_size);
	if (debugger) {
		status = debug_code(vm, code);
	} else {
		status = eval_code(vm, code);
	}
	code_destroy(code);
	vm_destroy(vm);
	return status;
}
