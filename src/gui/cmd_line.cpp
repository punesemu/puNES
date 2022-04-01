/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <QtWidgets/QMessageBox>
#include <QtCore/QFileInfo>
#include "dlgCmdLineHelp.hpp"
#include "info.h"
#include "settings.h"
#include "cmd_line.h"
#include "conf.h"
#include "version.h"
#include "gui.h"

#define req_arg true
#define no_arg false
#define oarg uQStringCD(value)
#define set_int(cfg, ind)\
{\
	int rc = settings_val_to_int(ind, oarg);\
	if (rc >= 0) cfg = rc;\
}
#define set_cpy_utchar_to_val(cfg, ind)\
	settings_cpy_utchar_to_val(ind, oarg);\
	ustrncpy(cfg, oarg, usizeof(cfg) - 1)
#define set_double(rnd)\
	settings_val_to_double(rnd, oarg)
#define set_oscan(set, ind)\
	settings_val_to_oscan(set, &overscan_borders[ind], oarg)

static void usage(QString name);

static struct _cl_option {
	QString lopt;
	int ra;
	QString sopt;
} opt_long[] = {
	{ "mode",                  req_arg, "m"},
	{ "size",                  req_arg, "s"},
	{ "overscan",              req_arg, "o"},
	{ "filter",                req_arg, "i"},
	{ "ntsc-format",           req_arg, "n"},
	{ "palette",               req_arg, "p"},
	{ "vsync",                 req_arg, "v"},
	{ "pixel-aspect-ratio",    req_arg, "e"},
	{ "interpolation",         req_arg, "j"},
	{ "fullscreen",            req_arg, "u"},
	{ "int-scl-fullscreen",    req_arg, "r"},
	{ "stretch-fullscreen",    req_arg, "t"},
#if defined (FULLSCREEN_RESFREQ)
	{ "adaptive-rrate",        req_arg,  0 },
	{ "fullscreen-res",        req_arg,  0 },
#endif
	{ "hflip-screen",          req_arg,  0 },
	{ "screen-rotation",       req_arg,  0 },
	{ "audio",                 req_arg, "a"},
	{ "audio-buffer-factor",   req_arg, "b"},
	{ "samplerate",            req_arg, "l"},
	{ "channels",              req_arg, "c"},
	{ "stereo-delay",          req_arg, "d"},
	{ "reverse-bits-dpcm",     req_arg,  0 },
	{ "swap-duty",             req_arg,  0 },
	{ "swap-emphasis",         req_arg,  0 },
	{ "gamegenie",             req_arg, "g"},
	{ "help",                  no_arg,  "h"},
	{ "version",               no_arg,  "V"},
	{ "portable",              no_arg,   0 },
	{ "txt-on-screen",         req_arg,  0 },
	{ "input-display",         req_arg,  0 },
	{ "disable-tv-noise",      req_arg,  0 },
	{ "disable-sepia",         req_arg,  0 },
#if defined (WITH_OPENGL)
	{ "disable-srgb-fbo",      req_arg,  0 },
#endif
	{ "overscan-brd-ntsc",     req_arg,  0 },
	{ "overscan-brd-pal",      req_arg,  0 },
	{ "par-soft-stretch",      req_arg,  0 },
	{ "hide-sprites",          req_arg,  0 },
	{ "hide-background",       req_arg,  0 },
	{ "unlimited-sprites",     req_arg,  0 },
	{ "background-pause",      req_arg,  0 },
	{ "save-battery-ram-file", req_arg,  0 },
	{ "language",              req_arg,  0 },
	{ "fullscreen-window",     req_arg,  0 },
	{ "audio-output-device",   req_arg,  0 },
	{ "shader",                req_arg,  0 },
	{ "overscan-blk-brd",      req_arg,  0 },
	{ "rewind-minutes",        req_arg,  0 },
	{ "hidden-gui",            no_arg,   0 }
};

BYTE cmd_line_parse(int argc, uTCHAR **argv) {
	QStringList splitted;
	QString arg, key, skey, value, exe = QFileInfo(uQString(argv[0])).baseName();
	int opt = 0;

	for (int a = 1; a < argc; a++) {
		arg = uQString(argv[a]);
		splitted = arg.split("=");
		key = QString(splitted.at(0));
		bool elaborate = false;

		if (key.startsWith("--")) {
			key.replace(0, 2, "");
			elaborate = true;
		} else if (key.startsWith("-")) {
			key.replace(0, 1, "");
			elaborate = true;
		}

		if (elaborate == true) {
			for (unsigned int b = 0; b < LENGTH(opt_long); b++) {
				if ((opt_long[b].lopt == key) || (opt_long[b].sopt == key)) {
					skey = opt_long[b].sopt;
					if (opt_long[b].ra == req_arg) {
						if (splitted.count() > 1) {
							value = QString(splitted.at(1));
						} else {
							if ((a + 1) >= argc) {
								QMessageBox::warning(0, "Error",
									QString("%1: the option needs an arguments -- \"%2\"").arg(exe, key));
								usage(exe);
								return (EXIT_ERROR);
							} else {
								value = uQString(argv[++a]);
							}
						}
					}
					opt = (*((char *)skey.toLatin1().constData()));
					break;
				}
			}
		} else {
			umemset(info.rom.file, 0x00, usizeof(info.rom.file));
			ustrncpy(info.rom.file, uQStringCD(key), usizeof(info.rom.file) - 1);
			continue;
		}

		switch (opt) {
			case 0:
				// long options
				if (key == "reverse-bits-dpcm") {
					set_int(cfg_from_file.reverse_bits_dpcm, SET_REVERSE_BITS_DPCM);
				} else if (key == "swap-duty") {
					set_int(cfg_from_file.swap_duty, SET_SWAP_DUTY);
				} else if (key == "swap-emphasis") {
					set_int(cfg_from_file.disable_swap_emphasis_pal, SET_SWAP_EMPHASIS_PAL);
				} else if (key == "portable") {
					// l'ho gia' controllato quindi qui non faccio niente
				} else if (key == "txt-on-screen") {
					set_int(cfg_from_file.txt_on_screen, SET_TEXT_ON_SCREEN);
				} else if (key == "hflip-screen") {
					set_int(cfg_from_file.hflip_screen, SET_HORIZONTAL_FLIP_SCREEN);
				} else if (key == "screen-rotation") {
					set_int(cfg_from_file.screen_rotation, SET_SCREEN_ROTATION);
				} else if (key == "input-display") {
					set_int(cfg_from_file.input_display, SET_INPUT_DISPLAY);
				} else if (key == "disable-tv-noise") {
					set_int(cfg_from_file.disable_tv_noise, SET_DISABLE_TV_NOISE);
				} else if (key == "disable-sepia") {
					set_int(cfg_from_file.disable_sepia_color, SET_DISABLE_SEPIA_PAUSE);
#if defined (WITH_OPENGL)
				} else if (key == "disable-srgb-fbo") {
					set_int(cfg_from_file.disable_srgb_fbo, SET_DISABLE_SRGB_FBO);
#endif
				} else if (key == "overscan-brd-ntsc") {
					set_oscan(SET_OVERSCAN_BRD_NTSC, 0);
				} else if (key == "overscan-brd-pal") {
					set_oscan(SET_OVERSCAN_BRD_PAL, 1);
				} else if (key == "par-soft-stretch") {
					set_int(cfg_from_file.PAR_soft_stretch, SET_PAR_SOFT_STRETCH);
				} else if (key == "hide-sprites") {
					set_int(cfg_from_file.hide_sprites, SET_HIDE_SPRITES);
				} else if (key == "hide-background") {
					set_int(cfg_from_file.hide_background, SET_HIDE_BACKGROUND);
				} else if (key == "unlimited-sprites") {
					set_int(cfg_from_file.unlimited_sprites, SET_UNLIMITED_SPRITES);
				} else if (key == "save-battery-ram-file") {
					set_int(cfg_from_file.save_battery_ram_file, SET_BATTERY_RAM_FILE_EVEY_TOT);
				} else if (key == "background-pause") {
					set_int(cfg_from_file.bck_pause, SET_BCK_PAUSE);
				} else if (key == "language") {
					set_int(cfg_from_file.language, SET_GUI_LANGUAGE);
				} else if (key == "fullscreen-window") {
					set_int(cfg_from_file.fullscreen_in_window, SET_FULLSCREEN_IN_WINDOW);
				} else if (key == "audio-output-device") {
					set_cpy_utchar_to_val(cfg_from_file.audio_output, SET_AUDIO_OUTPUT_DEVICE);
				} else if (key == "shader") {
					set_int(cfg_from_file.shader, SET_SHADER);
				} else if (key == "overscan-blk-brd") {
					set_int(cfg_from_file.oscan_black_borders, SET_OVERSCAN_BLACK_BORDERS);
				} else if (key == "overscan-blk-brd-f") {
					set_int(cfg_from_file.oscan_black_borders_fscr, SET_OVERSCAN_BLACK_BORDERS_FSCR);
				} else if (key == "rewind-minutes") {
					set_int(cfg_from_file.rewind_minutes, SET_REWIND_MINUTES);
#if defined (FULLSCREEN_RESFREQ)
				} else if (key == "adaptive-rrate") {
					set_int(cfg_from_file.adaptive_rrate, SET_ADAPTIVE_RRATE_FULLSCREEN);
				} else if (key == "fullscreen-res") {
					settings_resolution_val_to_int(&cfg_from_file.fullscreen_res_w, &cfg_from_file.fullscreen_res_h, oarg);
#endif
				} else if (key == "hidden-gui") {
					info.start_with_hidden_gui = TRUE;
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
			case 'g':
				set_int(cfg_from_file.cheat_mode, SET_CHEAT_MODE);
				break;
			case 'h':
			case '?':
				usage(exe);
				return (EXIT_ERROR);
				break;
			case 'V': {
				if (!info.portable) {
					fprintf(stdout, "%s %s\n", NAME, VERSION);
				} else {
					fprintf(stdout, "Portable %s %s\n", NAME, VERSION);
				}
				return (EXIT_ERROR);
				break;
			}
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
			case 's':
				set_int(cfg_from_file.scale, SET_SCALE);
				gfx.scale_before_fscreen = cfg_from_file.scale;
				break;
			case 'r':
				set_int(cfg_from_file.integer_scaling, SET_INTEGER_FULLSCREEN);
				break;
			case 't':
				{
					int rc = settings_val_to_int(SET_STRETCH_FULLSCREEN, oarg);

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
	}

	return (EXIT_OK);
}
BYTE cmd_line_check_portable(int argc, uTCHAR **argv) {
	if (QFileInfo(uQString(argv[0])).completeBaseName().right(2) == "_p") {
		return (TRUE);
	}
	for (int opt = 0; opt < argc; opt++) {
		QString arg = uQString(argv[opt]);

		if (arg == "--portable") {
			return (TRUE);
		}
	}
	return (FALSE);
}

static void usage(QString name) {
	dlgCmdLineHelp *box = new dlgCmdLineHelp(0, name);

	box->show();
	box->exec();
}
