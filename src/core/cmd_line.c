/*
 * cmd_line.c
 *
 *  Created on: 03/ago/2011
 *      Author: fhorse
 */

#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include "info.h"
#include "settings.h"
#include "cmd_line.h"
#include "conf.h"
#include "version.h"
#include "gfx.h"
#include "gui.h"

#define set_int(ind) settings_val_to_int(ind, optarg)
#define set_double(rnd) settings_val_to_double(rnd, optarg)
#define set_oscan(set, ind) settings_val_to_oscan(set, &overscan_borders[ind], optarg)

void usage(char *name);

static const char *opt_short = "m:f:k:s:o:i:n:p:r:v:e:j:u:t:a:l:c:d:q:g:Vh?";
static const struct option opt_long[] = {
	{ "mode",               required_argument, NULL, 'm'},
	{ "fps",                required_argument, NULL, 'f'},
	{ "frameskip",          required_argument, NULL, 'k'},
	{ "size",               required_argument, NULL, 's'},
	{ "overscan",           required_argument, NULL, 'o'},
	{ "filter",             required_argument, NULL, 'i'},
	{ "ntsc-format",        required_argument, NULL, 'n'},
	{ "palette",            required_argument, NULL, 'p'},
	{ "rendering",          required_argument, NULL, 'r'},
	{ "vsync",              required_argument, NULL, 'v'},
	{ "pixel-aspect-ratio", required_argument, NULL, 'e'},
	{ "interpolation",      required_argument, NULL, 'j'},
	{ "fullscreen",         required_argument, NULL, 'u'},
	{ "stretch-fullscreen", required_argument, NULL, 't'},
	{ "audio",              required_argument, NULL, 'a'},
	{ "samplerate",         required_argument, NULL, 'l'},
	{ "channels",           required_argument, NULL, 'c'},
	{ "stereo-delay",       required_argument, NULL, 'd'},
	{ "audio-quality",      required_argument, NULL, 'q'},
	{ "swap-duty",          required_argument, NULL,  0 },
	{ "swap-emphasis",      required_argument, NULL,  0 },
	{ "gamegenie",          required_argument, NULL, 'g'},
	{ "help",               no_argument,       NULL, 'h'},
	{ "version",            no_argument,       NULL, 'V'},
	{ "portable",           no_argument,       NULL,  0 },
	{ "txt-on-screen",      required_argument, NULL,  0 },
	{ "overscan-brd-ntsc",  required_argument, NULL,  0 },
	{ "overscan-brd-pal",   required_argument, NULL,  0 },
	{ "par-soft-stretch",   required_argument, NULL,  0 },
	{ "background-pause",   required_argument, NULL,  0 },
	{ "language",           required_argument, NULL,  0 },
	{ 0,                    0,                 0,     0 }
};

BYTE cmd_line_parse(int argc, char **argv) {
	int longIndex = 0, opt = 0;

	opt = getopt_long(argc, argv, opt_short, opt_long, &longIndex);
	while (opt != -1) {
		switch (opt) {
			case 0:
				/* long options */
				if (!(strcmp(opt_long[longIndex].name, "swap-duty"))) {
					cfg_from_file.swap_duty = set_int(SET_SWAP_DUTY);
				} else if (!(strcmp(opt_long[longIndex].name, "swap-emphasis"))) {
					cfg_from_file.disable_swap_emphasis_pal = set_int(SET_SWAP_EMPHASIS_PAL);
				} else if (!(strcmp(opt_long[longIndex].name, "portable"))) {
					/* l'ho gia' controllato quindi qui non faccio niente */
				} else if (!(strcmp(opt_long[longIndex].name, "txt-on-screen"))) {
					cfg_from_file.txt_on_screen = set_int(SET_TEXT_ON_SCREEN);
				} else if (!(strcmp(opt_long[longIndex].name, "overscan-brd-ntsc"))) {
					set_oscan(SET_OVERSCAN_BRD_NTSC, 0);
				} else if (!(strcmp(opt_long[longIndex].name, "overscan-brd-pal"))) {
					set_oscan(SET_OVERSCAN_BRD_PAL, 1);
				} else if (!(strcmp(opt_long[longIndex].name, "par-soft-stretch"))) {
					cfg_from_file.PAR_soft_stretch = set_int(SET_PAR_SOFT_STRETCH);
				} else if (!(strcmp(opt_long[longIndex].name, "background-pause"))) {
					cfg_from_file.bck_pause = set_int(SET_BCK_PAUSE);
				} else if (!(strcmp(opt_long[longIndex].name, "language"))) {
					cfg_from_file.language = set_int(SET_GUI_LANGUAGE);
				}
				break;
			case 'a':
				cfg_from_file.apu.channel[APU_MASTER] = set_int(SET_AUDIO);
				break;
			case 'c':
				cfg_from_file.channels = set_int(SET_CHANNELS);
				break;
			case 'd':
				cfg_from_file.stereo_delay = set_double(5);
				break;
			case 'f':
				cfg_from_file.fps = set_int(SET_FPS);
				break;
			case 'g':
				cfg_from_file.gamegenie = set_int(SET_GAMEGENIE);
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
				cfg_from_file.frameskip = set_int(SET_FRAMESKIP);
				break;
			case 'i':
				cfg_from_file.filter = set_int(SET_FILTER);
				break;
			case 'l':
				cfg_from_file.samplerate = set_int(SET_SAMPLERATE);
				break;
			case 'm':
				cfg_from_file.mode = set_int(SET_MODE);
				break;
			case 'n':
				cfg_from_file.ntsc_format = set_int(SET_NTSC_FORMAT);
				break;
			case 'o':
				cfg_from_file.oscan = set_int(SET_OVERSCAN_DEFAULT);
				break;
			case 'p':
				cfg_from_file.palette = set_int(SET_PALETTE);
				break;
			case 'q':
				cfg_from_file.audio_quality = set_int(SET_AUDIO_QUALITY);
				break;
			case 'r':
				cfg_from_file.render = set_int(SET_RENDERING);
				gfx_set_render(cfg_from_file.render);
				break;
			case 's':
				cfg_from_file.scale = set_int(SET_SCALE);
				gfx.scale_before_fscreen = cfg_from_file.scale;
				break;
			case 't':
				cfg_from_file.stretch = !set_int(SET_STRETCH_FULLSCREEN);
				break;
			case 'u':
				cfg_from_file.fullscreen = set_int(SET_FULLSCREEN);
				break;
			case 'v':
				cfg_from_file.vsync = set_int(SET_VSYNC);
				break;
			case 'e':
				cfg_from_file.pixel_aspect_ratio = set_int(SET_PAR);
				break;
			case 'j':
				cfg_from_file.interpolation = set_int(SET_INTERPOLATION);
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

#if defined (__WIN32__)
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
			main_cfg[SET_MODE].hlp,
			main_cfg[SET_FPS].hlp,
			main_cfg[SET_FRAMESKIP].hlp,
			main_cfg[SET_SCALE].hlp,
			main_cfg[SET_PAR].hlp,
			main_cfg[SET_PAR_SOFT_STRETCH].hlp,
			main_cfg[SET_OVERSCAN_DEFAULT].hlp,
			main_cfg[SET_FILTER].hlp,
			main_cfg[SET_NTSC_FORMAT].hlp,
			main_cfg[SET_PALETTE].hlp,
			main_cfg[SET_RENDERING].hlp,
			main_cfg[SET_SWAP_EMPHASIS_PAL].hlp,
			main_cfg[SET_VSYNC].hlp,
			main_cfg[SET_INTERPOLATION].hlp,
			main_cfg[SET_TEXT_ON_SCREEN].hlp,
			main_cfg[SET_OVERSCAN_BRD_NTSC].hlp,
			main_cfg[SET_OVERSCAN_BRD_PAL].hlp,
			main_cfg[SET_FULLSCREEN].hlp,
			main_cfg[SET_STRETCH_FULLSCREEN].hlp,
			main_cfg[SET_AUDIO].hlp,
			main_cfg[SET_SAMPLERATE].hlp,
			main_cfg[SET_CHANNELS].hlp,
			main_cfg[SET_STEREO_DELAY].hlp,
			main_cfg[SET_AUDIO_QUALITY].hlp,
			main_cfg[SET_SWAP_DUTY].hlp,
			main_cfg[SET_BCK_PAUSE].hlp,
			main_cfg[SET_GAMEGENIE].hlp,
			main_cfg[SET_GUI_LANGUAGE].hlp
	);
	gui_print_usage(usage_string);
	free(usage_string);

	emu_quit(EXIT_SUCCESS);
}
