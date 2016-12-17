/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMessageBox>
#endif
#include <QtCore/QFileInfo>
#include "info.h"
#include "settings.h"
#include "cmd_line.h"
#include "conf.h"
#include "version.h"
#include "gfx.h"
#include "gui.h"

#define req_arg true
#define no_arg false
#define oarg uQStringCD(value)
#define set_int(cfg, ind)\
{\
	int rc = settings_val_to_int(ind, oarg);\
	if (rc >= 0) cfg = rc;\
}
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
	{ "fps",                   req_arg, "f"},
	{ "frameskip",             req_arg, "k"},
	{ "size",                  req_arg, "s"},
	{ "overscan",              req_arg, "o"},
	{ "filter",                req_arg, "i"},
	{ "ntsc-format",           req_arg, "n"},
	{ "palette",               req_arg, "p"},
#if defined (WITH_OPENGL)
	{ "rendering",             req_arg, "r"},
#endif
	{ "vsync",                 req_arg, "v"},
	{ "pixel-aspect-ratio",    req_arg, "e"},
	{ "interpolation",         req_arg, "j"},
	{ "fullscreen",            req_arg, "u"},
	{ "stretch-fullscreen",    req_arg, "t"},
	{ "audio",                 req_arg, "a"},
	{ "audio-buffer-factor",   req_arg, "b"},
	{ "samplerate",            req_arg, "l"},
	{ "channels",              req_arg, "c"},
	{ "stereo-delay",          req_arg, "d"},
	{ "audio-quality",         req_arg, "q"},
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
	{ "disable-new-menu",      req_arg,  0 },
};

void cmd_line_parse(int argc, uTCHAR **argv) {
	QStringList splitted;
	QString arg, key, skey, value, exe = QFileInfo(uQString(argv[0])).baseName();
	int opt = 0;

	for (int a = 1; a < argc; a++) {
		arg = uQString(argv[a]);
		splitted = arg.split("=");
		key = QString(splitted.at(0));

		if (key.startsWith("--") || key.startsWith("-")) {
			key = key.replace("-", "");

			for (unsigned int b = 0; b < LENGTH(opt_long); b++) {
				if ((opt_long[b].lopt == key) || (opt_long[b].sopt == key)) {
					skey = opt_long[b].sopt;
					if (opt_long[b].ra == req_arg) {
						if (splitted.count() > 1) {
							value = QString(splitted.at(1));
						} else {
							if ((a + 1) >= argc) {
								QMessageBox::warning(0,
									"Error",
									QString("%1: the option needs an arguments -- \"%2\"").arg(exe, key));
								usage(exe);
							} else {
								value = uQString(argv[++a]);
							}
						}
					}
					opt = (*((char *) skey.toLatin1().constData()));
				}
			}
		} else {
			umemset(info.rom_file, 0x00, usizeof(info.rom_file));
			ustrncpy(info.rom_file, uQStringCD(key), usizeof(info.rom_file) - 1);
			continue;
		}

		switch (opt) {
			case 0:
				// long options
				if (key == "swap-duty") {
					set_int(cfg_from_file.swap_duty, SET_SWAP_DUTY);
				} else if (key == "swap-emphasis") {
					set_int(cfg_from_file.disable_swap_emphasis_pal, SET_SWAP_EMPHASIS_PAL);
				} else if (key == "portable") {
					// l'ho gia' controllato quindi qui non faccio niente
				} else if (key == "txt-on-screen") {
					set_int(cfg_from_file.txt_on_screen, SET_TEXT_ON_SCREEN);
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
				} else if (key == "disable-new-menu") {
					set_int(cfg_from_file.disable_new_menu, SET_GUI_DISABLE_NEW_MENU);
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
				usage(exe);
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
#if defined (WITH_OPENGL)
			case 'r':
				set_int(cfg_from_file.render, SET_RENDERING);
				gfx_set_render(cfg_from_file.render);
				break;
#endif
			case 's':
				set_int(cfg_from_file.scale, SET_SCALE);
				gfx.scale_before_fscreen = cfg_from_file.scale;
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
	QMessageBox *box = new QMessageBox();
	uTCHAR *usage_string;
	const uTCHAR *istructions = {
			uL("Usage: %1 [options] file...\n\n")
			uL("Options:\n")
			uL("-h, --help                print this help\n")
			uL("-V, --version             print the version\n")
			uL("    --portable            start in portable mode\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
#if defined (WITH_OPENGL)
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
#endif
	};

	usage_string = (uTCHAR *) malloc(1024 * 8);
	usnprintf(usage_string, 1024 * 8, istructions,
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
#if defined (WITH_OPENGL)
			main_cfg[SET_RENDERING].hlp,
#endif
			main_cfg[SET_SWAP_EMPHASIS_PAL].hlp,
			main_cfg[SET_VSYNC].hlp,
			main_cfg[SET_INTERPOLATION].hlp,
			main_cfg[SET_TEXT_ON_SCREEN].hlp,
			main_cfg[SET_INPUT_DISPLAY].hlp,
			main_cfg[SET_DISABLE_TV_NOISE].hlp,
			main_cfg[SET_DISABLE_SEPIA_PAUSE].hlp,
#if defined (WITH_OPENGL)
			main_cfg[SET_DISABLE_SRGB_FBO].hlp,
#endif
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
			main_cfg[SET_HIDE_SPRITES].hlp,
			main_cfg[SET_HIDE_BACKGROUND].hlp,
			main_cfg[SET_UNLIMITED_SPRITES].hlp,
			main_cfg[SET_BCK_PAUSE].hlp,
			main_cfg[SET_CHEAT_MODE].hlp,
			main_cfg[SET_GUI_LANGUAGE].hlp,
			main_cfg[SET_GUI_DISABLE_NEW_MENU].hlp
	);

	if (box->font().pointSize() > 9) {
		QFont font;

		font.setPointSize(9);
		box->setFont(font);
	}

	box->setAttribute(Qt::WA_DeleteOnClose);
	box->setWindowTitle(uQString(uL("" NAME)));

	box->setWindowModality(Qt::WindowModal);

	// monospace
	box->setText("<pre>" + uQString(usage_string).arg(name) + "</pre>");

	box->setStandardButtons(QMessageBox::Ok);
	box->setDefaultButton(QMessageBox::Ok);

	box->show();
	box->exec();

	free(usage_string);

	emu_quit(EXIT_SUCCESS);
}
