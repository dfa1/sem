/*
 * main.c -- The main() and other stuff
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

#include "sem.h"

#include <getopt.h>

static void usage(int sts) {
  FILE *target = (sts == EXIT_SUCCESS)? stdout : stderr;
  
  fprintf(target, "\r\
Usage: sem [options] file\n			\
\n\
Options:\n\
  -h      : print this help message and exit\n\
  -m      : set the data memory size (the default is 64)\n\
  -s      : set the evaluation stack size (the default is 32)\n\
  -v      : print the version and exit\n\
\n\
Report bugs to <davide.angelocola@gmail.com>\n");
  exit(sts);
}

int main(int argc, char **argv) {
    const char licenseMsg[] = "\r\
sem %s -- A SIMPLESEM interpreter\n\
Copyright (C) 2003-2011 Davide Angelocola <davide.angelocola@gmail.com>\n\
\n\
This program is free software, and you are welcome to redistribute\n\
it and/or modify it under the terms of the GNU General Public License.\n\
There is ABSOLUTELY NO WARRANTY for this program. \n\
\n\
See the LICENSE for more details.\n";
    int memSize = 64;
    int stackSize = 32;
    int opt = 0;
    struct option long_options[] = {
	{ "version", 0, 0, 'v' },
	{ "help",    0, 0, 'h' },
	{ NULL,      0, 0, 'm' },
	{ NULL,      0, 0, 's' },	    

	/* Sentinel. */
	{ 0,         0, 0, 0   }
    };

    while ((opt = getopt_long(argc, argv, "hm:s:v", long_options, NULL)) 
	   != EOF) {
	switch (opt) {

	case 'h':
	    usage(EXIT_SUCCESS);
	    break;

	case 'm':
	  if (sscanf(optarg, "%d", &memSize) != 1 || memSize < 1) {
	    fprintf(stderr, "sem: invalid memory size (%s)\n", optarg);
	    return EXIT_FAILURE;
	  }
	  break;

	case 's':
	  if (sscanf(optarg, "%d", &stackSize) != 1 || stackSize < 1) {
	    fprintf(stderr, "sem: invalid stack size (%s)\n", optarg);
	    return EXIT_FAILURE;
	  }
	  break;

	case 'v':
	  fprintf(stdout, licenseMsg, VERSION);
	  return EXIT_SUCCESS;

	default:
	  fprintf(stderr, "Try `sem -h' for more information.\n");
	  return EXIT_FAILURE;
	}
    }

    if (optind < argc && strcmp(argv[optind], "-") == 0) {
      fprintf(stderr, "sem: no input\n");
      return EXIT_FAILURE;
    }
    
    struct Code *code = compileSource(argv[optind]);
    
    if (code == NULL) {
      // error message should be already displayed
      return EXIT_FAILURE;
    }

    struct VM *vm= initVM(code, memSize, stackSize);
    int status = evalCode(vm);
    finiCompiler(code);
    finiVM(vm);
    return status;
}
