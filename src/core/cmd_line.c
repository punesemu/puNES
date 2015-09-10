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

#define set_int(cfg, ind)\
{\
	int rc = settings_val_to_int(ind, optarg);\
	if (rc >= 0) cfg = rc;\
}
#define set_double(rnd) settings_val_to_double(rnd, optarg)
#define set_oscan(set, ind) settings_val_to_oscan(set, &overscan_borders[ind], optarg)

void usage(char *name);

static const char *opt_short = "m:f:k:s:o:i:n:p:r:v:e:j:u:t:a:b:l:c:d:q:g:Vh?";
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
	{ "audio-buffer-factor",required_argument, NULL, 'b'},
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
	{ "hide-mouse-cursor",  required_argument, NULL,  0 },
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
					set_int(cfg_from_file.swap_duty, SET_SWAP_DUTY);
				} else if (!(strcmp(opt_long[longIndex].name, "swap-emphasis"))) {
					set_int(cfg_from_file.disable_swap_emphasis_pal, SET_SWAP_EMPHASIS_PAL);
				} else if (!(strcmp(opt_long[longIndex].name, "portable"))) {
					/* l'ho gia' controllato quindi qui non faccio niente */
				} else if (!(strcmp(opt_long[longIndex].name, "txt-on-screen"))) {
					set_int(cfg_from_file.txt_on_screen, SET_TEXT_ON_SCREEN);
				} else if (!(strcmp(opt_long[longIndex].name, "overscan-brd-ntsc"))) {
					set_oscan(SET_OVERSCAN_BRD_NTSC, 0);
				} else if (!(strcmp(opt_long[longIndex].name, "overscan-brd-pal"))) {
					set_oscan(SET_OVERSCAN_BRD_PAL, 1);
				} else if (!(strcmp(opt_long[longIndex].name, "par-soft-stretch"))) {
					set_int(cfg_from_file.PAR_soft_stretch, SET_PAR_SOFT_STRETCH);
				} else if (!(strcmp(opt_long[longIndex].name, "background-pause"))) {
					set_int(cfg_from_file.bck_pause, SET_BCK_PAUSE);
				} else if (!(strcmp(opt_long[longIndex].name, "language"))) {
					set_int(cfg_from_file.language, SET_GUI_LANGUAGE);
				}
				break;
			case 'a':
				set_int(cfg_from_file.apu.channel[APU_MASTER], SET_AUDIO);
				break;
			case 'b':
				set_int(cfg_from_file.audio_buffer_factor, SET_AUDIO_BUFFER_FACTOR);
				break;
			case 'c':
				set_int(cfg_from_file.channels_mode, SET_CHANNELS);
				break;
			case 'd':
				cfg_from_file.stereo_delay = set_double(5);
				break;
			case 'f':
				set_int(cfg_from_file.fps, SET_FPS);
				break;
			case 'g':
				set_int(cfg_from_file.cheat_mode, SET_CHEAT_MODE);
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
				set_int(cfg_from_file.frameskip, SET_FRAMESKIP);
				break;
			case 'i':
				set_int(cfg_from_file.filter, SET_FILTER);
				break;
			case 'l':
				set_int(cfg_from_file.samplerate, SET_SAMPLERATE);
				break;
			case 'm':
				set_int(cfg_from_file.mode, SET_MODE);
				break;
			case 'n':
				set_int(cfg_from_file.ntsc_format, SET_NTSC_FORMAT);
				break;
			case 'o':
				set_int(cfg_from_file.oscan, SET_OVERSCAN_DEFAULT);
				break;
			case 'p':
				set_int(cfg_from_file.palette, SET_PALETTE);
				break;
			case 'q':
				set_int(cfg_from_file.audio_quality, SET_AUDIO_QUALITY);
				break;
			case 'r':
				set_int(cfg_from_file.render, SET_RENDERING);
				gfx_set_render(cfg_from_file.render);
				break;
			case 's':
				set_int(cfg_from_file.scale, SET_SCALE);
				gfx.scale_before_fscreen = cfg_from_file.scale;
				break;
			case 't':
				{
					int rc = settings_val_to_int(SET_STRETCH_FULLSCREEN, optarg);

					if (rc >= 0) {
						cfg_from_file.scale = !rc;
					}
				}
				break;
			case 'u':
				set_int(cfg_from_file.fullscreen, SET_FULLSCREEN);
				break;
			case 'v':
				set_int(cfg_from_file.vsync, SET_VSYNC);
				break;
			case 'e':
				set_int(cfg_from_file.pixel_aspect_ratio, SET_PAR);
				break;
			case 'j':
				set_int(cfg_from_file.interpolation, SET_INTERPOLATION);
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
			main_cfg[SET_AUDIO_BUFFER_FACTOR].hlp,
			main_cfg[SET_SAMPLERATE].hlp,
			main_cfg[SET_CHANNELS].hlp,
			main_cfg[SET_STEREO_DELAY].hlp,
			main_cfg[SET_AUDIO_QUALITY].hlp,
			main_cfg[SET_SWAP_DUTY].hlp,
			main_cfg[SET_BCK_PAUSE].hlp,
			main_cfg[SET_CHEAT_MODE].hlp,
			main_cfg[SET_GUI_LANGUAGE].hlp
	);
	gui_print_usage(usage_string);
	free(usage_string);

	emu_quit(EXIT_SUCCESS);
}
