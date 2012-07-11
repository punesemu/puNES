/*
 * cmdline.c
 *
 *  Created on: 03/ago/2011
 *      Author: fhorse
 */

#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include "cmdline.h"
#include "cfgfile.h"
#include "sdlgfx.h"
#include "version.h"
#include "emu.h"
#include "gamegenie.h"
#ifdef OPENGL
#include "opengl.h"
#endif
#define __CMDLINE__
#include "param.h"

void usage(char *name);

BYTE cmdlineParse(int argc, char **argv) {
	int longIndex = 0, opt = 0;

	opt = getopt_long(argc, argv, optShort, optLong, &longIndex);
	while (opt != -1) {
		switch (opt) {
			case 'a':
				paramSearch(0, optarg, pOffOn, cfg_from_file.audio = index);
				break;
			case 'c':
				paramSearch(0, optarg, pChannels, cfg_from_file.channels = index);
				break;
			case 'e':
				paramSearch(0, optarg, pAudioFilter, cfg_from_file.audio_filter = index);
				break;
			case 'f':
				paramSearch(0, optarg, pFps, cfg_from_file.fps = index);
				break;
			case 'g':
				paramSearch(0, optarg, pNoYes, gamegenie.enabled = index);
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
				emuQuit(EXIT_SUCCESS);
				break;
			}
			case 'k':
				paramSearch(0, optarg, pFsk, cfg_from_file.frameskip = index);
				break;
			case 'i':
				paramSearch(0, optarg, pFilter, gfx.filter = index);
				break;
			case 'l':
				paramSearch(0, optarg, pSamplerate, cfg_from_file.samplerate = index);
				break;
			case 'm':
				paramSearch(0, optarg, pMode, cfg_from_file.mode = index);
				break;
			case 'n':
				paramSearch(0, optarg, pNtsc, gfx.ntscFormat = index);
				break;
			case 'o':
				paramSearch(0, optarg, pOverscan, gfx.overscan = index);
				break;
			case 'p':
				paramSearch(0, optarg, pNtsc, gfx.palette = index);
				break;
			case 's':
				paramSearch(1, optarg, pSize, gfx.scale = gfx.scaleBeforeFullscreen = index);
				break;
#ifdef OPENGL
			case 'r': {
				BYTE render = 0;

				paramSearch(0, optarg, pRendering, render = index);

				switch (render) {
					case 0:
						gfx.opengl = FALSE;
						opengl.glsl.enabled = FALSE;
						break;
					case 1:
						gfx.opengl = TRUE;
						opengl.glsl.enabled = FALSE;
						break;
					case 2:
						gfx.opengl = TRUE;
						opengl.glsl.enabled = TRUE;
						break;
				}
				break;
			}
			case 'v':
				paramSearch(0, optarg, pOffOn, gfx.vsync = index);
				break;
			case 't':
				paramSearch(0, optarg, pNoYes, opengl.aspectRatio = !index);
				break;
			case 'u':
				paramSearch(0, optarg, pNoYes, gfx.fullscreen = index);
				break;
#endif
			default:
				break;
		}

		opt = getopt_long(argc, argv, optShort, optLong, &longIndex);
	}
	return (optind);
}
void usage(char *name) {
	char *istructions = { "Usage: %s [options] file...\n\n"
			"Options:\n"
			"-h, --help                print this help\n"
			"-V, --version             print the version\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
#ifdef OPENGL
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
#endif
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
	};
	fprintf(stdout, istructions, name,
			param[P_MODE].help,
			param[P_FPS].help,
	        param[P_FSK].help,
	        param[P_SIZE].help,
	        param[P_OVERSCAN].help,
	        param[P_FILTER].help,
	        param[P_NTSCFORMAT].help,
	        param[P_PALETTE].help,
#ifdef OPENGL
			param[P_RENDER].help,
			param[P_VSYNC].help,
			param[P_FSCREEN].help,
			param[P_STRETCH].help,
#endif
			param[P_AUDIO].help,
			param[P_SAMPLERATE].help,
			param[P_CHANNELS].help,
			param[P_AUDIO_FILTER].help,
			param[P_GAMEGENIE].help
	);
	emuQuit(EXIT_SUCCESS);
}
