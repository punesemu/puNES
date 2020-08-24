/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#include "dlgCmdLineHelp.moc"
#include "info.h"
#include "settings.h"
#include "cmd_line.h"
#include "conf.h"
#include "version.h"
#include "gui.h"

dlgCmdLineHelp::dlgCmdLineHelp(QWidget *parent, QString name) : QDialog(parent) {
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
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
			uL("" uPERCENTs "\n")
#if defined (WITH_OPENGL)
			uL("" uPERCENTs "\n")
#endif
	};

	usage_string = (uTCHAR *)malloc(1024 * 9);
	usnprintf(usage_string, 1024 * 9, istructions,
			main_cfg[SET_MODE].hlp,
			main_cfg[SET_SCALE].hlp,
			main_cfg[SET_PAR].hlp,
			main_cfg[SET_PAR_SOFT_STRETCH].hlp,
			main_cfg[SET_OVERSCAN_BLACK_BORDERS].hlp,
			main_cfg[SET_OVERSCAN_BLACK_BORDERS_FSCR].hlp,
			main_cfg[SET_OVERSCAN_DEFAULT].hlp,
			main_cfg[SET_FILTER].hlp,
			main_cfg[SET_NTSC_FORMAT].hlp,
			main_cfg[SET_SHADER].hlp,
			main_cfg[SET_PALETTE].hlp,
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
			main_cfg[SET_FULLSCREEN_IN_WINDOW].hlp,
			main_cfg[SET_INTEGER_FULLSCREEN].hlp,
			main_cfg[SET_STRETCH_FULLSCREEN].hlp,
			main_cfg[SET_SCREEN_ROTATION].hlp,
			main_cfg[SET_AUDIO_OUTPUT_DEVICE].hlp,
			main_cfg[SET_AUDIO].hlp,
			main_cfg[SET_AUDIO_BUFFER_FACTOR].hlp,
			main_cfg[SET_SAMPLERATE].hlp,
			main_cfg[SET_CHANNELS].hlp,
			main_cfg[SET_STEREO_DELAY].hlp,
			main_cfg[SET_REVERSE_BITS_DPCM].hlp,
			main_cfg[SET_SWAP_DUTY].hlp,
			main_cfg[SET_HIDE_SPRITES].hlp,
			main_cfg[SET_HIDE_BACKGROUND].hlp,
			main_cfg[SET_UNLIMITED_SPRITES].hlp,
			main_cfg[SET_BCK_PAUSE].hlp,
			main_cfg[SET_CHEAT_MODE].hlp,
			main_cfg[SET_GUI_LANGUAGE].hlp,
			main_cfg[SET_REWIND_MINUTES].hlp
	);

	setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(uQString(uL("" NAME " Command Line Help")));

	if (font().pointSize() > 9) {
		QFont font;

		font.setPointSize(9);
		setFont(font);
	}

	textEdit_cmdLineHelp->setHtml("<pre>" + uQString(usage_string).arg(name) + "</pre>");

	connect(pushButton_Close, SIGNAL(clicked(bool)), this, SLOT(s_close_clicked(bool)));

	free(usage_string);
}
dlgCmdLineHelp::~dlgCmdLineHelp() {}

void dlgCmdLineHelp::s_close_clicked(UNUSED(bool checked)) {
	close();
}
