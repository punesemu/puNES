/*
 * cmd_line.c
 *
 *  Created on: 03/ago/2011
 *      Author: fhorse
 */

#include <getopt.h>
#include <libgen.h>
#include "emu.h"
#include "gui.h"
#define __CMDLINE__
#include "param.h"
#undef  __CMDLINE__

void usage(char *name);

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

void usage(char *name) {
	char *usage_string;
	char *istructions = {
			"Usage: %s [options] file...\n\n"
			"Options:\n"
			"-h, --help                print this help\n"
			"-V, --version             print the version\n"
			"    --portable            start in portable mode\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
	};

	usage_string = malloc(1024 * 8);
	sprintf(usage_string, istructions, name,
			param[P_MODE].help,
			param[P_FPS].help,
	        param[P_FSK].help,
	        param[P_SIZE].help,
	        param[P_OVERSCAN].help,
	        param[P_FILTER].help,
	        param[P_NTSCFORMAT].help,
	        param[P_PALETTE].help,
			param[P_RENDER].help,
			param[P_VSYNC].help,
			param[P_FSCREEN].help,
			param[P_STRETCH].help,
			param[P_AUDIO].help,
			param[P_SAMPLERATE].help,
			param[P_CHANNELS].help,
			param[P_AUDIO_QUALITY].help,
			param[P_SWAP_DUTY].help,
			param[P_GAMEGENIE].help
	);
	gui_print_usage(usage_string);
	free(usage_string);

	emu_quit(EXIT_SUCCESS);
}
