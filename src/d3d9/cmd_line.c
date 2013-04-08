/*
 * cmd_line.c
 *
 *  Created on: 03/ago/2011
 *      Author: fhorse
 */

#include <getopt.h>
#include "emu.h"
#define __CMDLINE__
#include "param.h"
#undef  __CMDLINE__

BYTE cmd_line_parse(int argc, char **argv) {
	int long_index = 0, opt = 0;

	opt = getopt_long(argc, argv, opt_short, opt_long, &long_index);
	while (opt != -1) {
		switch (opt) {

		}

		opt = getopt_long(argc, argv, opt_short, opt_long, &long_index);
	}
	return (optind);
}
BYTE cmd_line_check_portable(int argc, char **argv) {
	int opt;

	for (opt = 0; opt < argc; opt++) {
		//if (!(strcmp(argv[opt], "--portable"))) {
		//	return (TRUE);
		//}
	}

	return (FALSE);
}
