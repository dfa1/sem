/*
 * main.c -- The main()
 *
 * Copyright (C) 2003-2013  Davide Angelocola <davide.angelocola@gmail.com>
 
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
#include "config.h"

constexpr size_t MAX_DATA_SIZE = 1024;
constexpr size_t MAX_STACK_SIZE = 1024 ;
constexpr size_t DEFAULT_DATA_SIZE = 64;
constexpr size_t DEFAULT_STACK_SIZE = 64;

static char license[] = "\r\
sem " PACKAGE_VERSION " -- A SIMPLESEM interpreter\n\
Copyright (C) 2003-2013 Davide Angelocola\n\
\n\
This program is free software, and you are welcome to redistribute\n\
it and/or modify it under the terms of the GNU General Public License.\n\
There is ABSOLUTELY NO WARRANTY for this program.\n\
\n\
See the COPYING filefor more details.\n";

static char help_template[] = "\r\
Usage: sem [options] file\n\
\n\
Options:\n\
  -h : print this help message and exit\n\
  -d : interactive debugger\n\
  -m : set the data memory size (the default is %zu)\n\
  -s : set the stack size (the default is %u)\n\
  -v : print the version and exit\n\
\n\
Report bugs to <%s>\n";

static void usage(const int sts) {
	FILE *target = (sts == EXIT_SUCCESS) ? stdout : stderr;
	fprintf(target, help_template, DEFAULT_DATA_SIZE, DEFAULT_STACK_SIZE, PACKAGE_BUGREPORT);
	exit(sts);
}

int main(const int argc, char *argv[]) {
	size_t mem_size = DEFAULT_DATA_SIZE;
	size_t stack_size = DEFAULT_STACK_SIZE;
	int debugger = 0;
	int opt = 0;
	const struct option long_options[] = {
		{"version", 0, nullptr, 'v'},
		{"help", 0, nullptr, 'h'},
		{"debug", 0, nullptr, 'd'},
		{nullptr, 0, nullptr, 'm'},
		{nullptr, 0, nullptr, 's'},

		/* Sentinel. */
		{nullptr, 0, nullptr, 0}
	};

	while ((opt = getopt_long(argc, argv, "hm:s:vd", long_options, nullptr))
	       != EOF) {
		switch (opt) {
			case 'h':
				usage(EXIT_SUCCESS);

			case 'm':
				if (sscanf(optarg, "%d", &mem_size) != 1 || mem_size > MAX_DATA_SIZE) {
					fprintf(stderr,
							"sem: invalid memory size (%s)\n",
							optarg);
					return EXIT_FAILURE;
				}
				break;

			case 's':
				if (sscanf(optarg, "%d", &stack_size) != 1 || stack_size > MAX_STACK_SIZE) {
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
	const char *filename = argv[optind];
	struct code *code = compile_code(filename);

	if (code == nullptr) {
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
