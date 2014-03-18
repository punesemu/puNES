/*
 * cmd_line.c
 *
 *  Created on: 03/ago/2011
 *      Author: fhorse
 */

#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include "cmd_line.h"
#include "cfg_file.h"
#include "version.h"
#include "gfx.h"
#define __GUI_BASE__
#include "gui.h"
#undef __GUI_BASE__
#define __CMDLINE__
#include "param.h"
#undef  __CMDLINE__

void usage(char *name);

BYTE cmd_line_parse(int argc, char **argv) {
	int longIndex = 0, opt = 0;

	opt = getopt_long(argc, argv, opt_short, opt_long, &longIndex);
	while (opt != -1) {
		switch (opt) {
			case 0:
				/* long options */
				if (!(strcmp(opt_long[longIndex].name, "swap-duty"))) {
					param_search(0, optarg, param_no_yes, cfg_from_file.swap_duty = index);
				} else if (!(strcmp(argv[opt], "portable"))) {
					/* l'ho gia' controllato quindi qui non faccio niente */
					;
				}
				break;
			case 'a':
				param_search(0, optarg, param_off_on, cfg_from_file.apu.channel[APU_MASTER] = index);
				break;
			case 'c':
				param_search(0, optarg, param_channels, cfg_from_file.channels = index);
				break;
			case 'd':
				param_double_search(optarg, cfg_from_file.stereo_delay, 5);
				break;
			case 'f':
				param_search(0, optarg, param_fps, cfg_from_file.fps = index);
				break;
			case 'g':
				param_search(0, optarg, param_no_yes, cfg_from_file.gamegenie = index);
				break;
			case 'h':
			case '?':
				usage(basename(argv[0]));
				break;
			case 'V': {
				if (!info.portable) {
					fprintf(stdout, "%s %s\n", NAME, VERSION);
				} else {
					fprintf(stdout, "Portable %s %s\n", NAME, VERSION);
				}
				emu_quit(EXIT_SUCCESS);
				break;
			}
			case 'k':
				param_search(0, optarg, param_fsk, cfg_from_file.frameskip = index);
				break;
			case 'i':
				param_search(0, optarg, param_filter, cfg_from_file.filter = index);
				break;
			case 'l':
				param_search(0, optarg, param_samplerate, cfg_from_file.samplerate = index);
				break;
			case 'm':
				param_search(0, optarg, param_mode, cfg_from_file.mode = index);
				break;
			case 'n':
				param_search(0, optarg, param_ntsc, cfg_from_file.ntsc_format = index);
				break;
			case 'o':
				param_search(0, optarg, param_oscan, cfg_from_file.oscan = index);
				break;
			case 'p':
				param_search(0, optarg, param_ntsc, cfg_from_file.palette = index);
				break;
			case 'q':
				param_search(0, optarg, param_audio_quality, cfg_from_file.audio_quality = index);
				break;
			case 'r':
				param_search(0, optarg, param_render, cfg_from_file.render = index);
				gfx_set_render(cfg_from_file.render);
				break;
			case 's':
				param_search(1, optarg, param_size, cfg_from_file.scale = index);
				gfx.scale_before_fscreen = cfg_from_file.scale;
				break;
			case 't':
				param_search(0, optarg, param_no_yes, cfg_from_file.stretch = !index);
				break;
			case 'u':
				param_search(0, optarg, param_no_yes, cfg_from_file.fullscreen = index);
				break;
			case 'v':
				param_search(0, optarg, param_off_on, cfg_from_file.vsync = index);
				break;
			case 'e':
				param_search(0, optarg, param_no_yes, cfg_from_file.tv_aspect_ratio = index);
				break;
			case 'j':
				param_search(0, optarg, param_no_yes, cfg_from_file.interpolation = index);
				break;
			default:
				break;
		}

		opt = getopt_long(argc, argv, opt_short, opt_long, &longIndex);
	}
	return (optind);
}
BYTE cmd_line_check_portable(int argc, char **argv) {
	int opt;

#if defined (MINGW32) || defined (MINGW64)
	if (!(strncmp(argv[0] + (strlen(argv[0]) - 6), "_p", 2))) {
#else
	if (!(strcmp(argv[0] + (strlen(argv[0]) - 2), "_p"))) {
#endif
		return (TRUE);
	}

	for (opt = 0; opt < argc; opt++) {
		if (!(strcmp(argv[opt], "--portable"))) {
			return (TRUE);
		}
	}

	return (FALSE);
}

void usage(char *name) {
	char *usage_string;
	const char *istructions = {
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
			"%s\n"
			"%s\n"
			"%s\n"
	};

	usage_string = (char *) malloc(1024 * 8);
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
			param[P_TV_ASPECT_RATIO].help,
			param[P_INTERPOLATION].help,
			param[P_FSCREEN].help,
			param[P_STRETCH].help,
			param[P_AUDIO].help,
			param[P_SAMPLERATE].help,
			param[P_CHANNELS].help,
			param[P_STEREODELAY].help,
			param[P_AUDIO_QUALITY].help,
			param[P_SWAP_DUTY].help,
			param[P_GAMEGENIE].help
	);
	gui_print_usage(usage_string);
	free(usage_string);

	emu_quit(EXIT_SUCCESS);
}
