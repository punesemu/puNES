/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include "dlgCmdLineHelp.hpp"
#include "info.h"
#include "settings.h"
#include "version.h"
#include "gui.h"

// ----------------------------------------------------------------------------------------------

dlgCmdLineHelp::dlgCmdLineHelp(QWidget *parent, const QString title, const uTCHAR *usage_string) : wdgTitleBarDialog(parent) {
	wd = new wdgDialogCmdLineHelp(this, title, usage_string);
	init();
}
dlgCmdLineHelp::dlgCmdLineHelp(QWidget *parent, const QString name) : wdgTitleBarDialog(parent) {
	wd = new wdgDialogCmdLineHelp(this, name);
	init();
}
dlgCmdLineHelp::~dlgCmdLineHelp() = default;

void dlgCmdLineHelp::closeEvent(QCloseEvent *event) {
	emit et_close();
	wdgTitleBarDialog::closeEvent(event);
}

void dlgCmdLineHelp::init(void) {
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/application.png"));
	set_border_color(Qt::darkGreen);
	set_buttons(barButton::Close);
	add_widget(wd);

	connect(wd->pushButton_Close, SIGNAL(clicked(bool)), this, SLOT(close(void)));
}

// ----------------------------------------------------------------------------------------------

wdgDialogCmdLineHelp::wdgDialogCmdLineHelp(QWidget *parent, const QString title, const uTCHAR *usage_string) : QWidget(parent) {
	init(title, usage_string, false);
}
wdgDialogCmdLineHelp::wdgDialogCmdLineHelp(QWidget *parent, const QString name) : QWidget(parent) {
	uTCHAR *usage_string;
	const uTCHAR *istructions = {
			uL("Usage: %%1 [options] file...\n\n")
			uL("Options:\n")
			uL("-h, --help                     print this help\n")
			uL("-V, --version                  print the version\n")
			uL("    --portable                 start in portable mode\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			#if defined (WITH_OPENGL)
			uL("" uPs("") "\n")
			#endif
			#if defined (FULLSCREEN_RESFREQ)
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
			#endif
			uL("" uPs("") "\n")
			uL("" uPs("") "\n")
	};
	const uTCHAR *sch_input = {
			uL("    --shortcut.[type].[desc]   set up the shortcut   : [keyboard] or [joystick]" NEWLINE)
			uL("                                                       the [type] must be k for keyboard mapping" NEWLINE)
			uL("                                                       and j for joystick mapping." NEWLINE)
			uL("                                                       [desc] can be found in the puNES.cfg" NEWLINE)
			uL("                                                       e.g." NEWLINE)
			uL("                                                       --shortcut.k.open=Alt+O" NEWLINE)
			uL("                                                       --shortcut.j.hard_reset=BTN05" NEWLINE)
			uL("    --input.[type].[desc]      input sequence        : [keyboard] or [joystick]" NEWLINE)
			uL("                                                       the [type] must be p1k, p2k, p3k and p4k" NEWLINE)
			uL("                                                       for keyboard mapping and p1j, p2j, p3j and p4j" NEWLINE)
			uL("                                                       for joystick mapping." NEWLINE)
			uL("                                                       [desc] can be found in the input.cfg" NEWLINE)
			uL("                                                       e.g." NEWLINE)
			uL("                                                       --input.p1k.up=Up" NEWLINE)
			uL("                                                       --input.p1j.turboa=BTN05")
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
#if defined (FULLSCREEN_RESFREQ)
			  main_cfg[SET_ADAPTIVE_RRATE_FULLSCREEN].hlp,
			  main_cfg[SET_RESOLUTION_FULLSCREEN].hlp,
#endif
			  main_cfg[SET_HORIZONTAL_FLIP_SCREEN].hlp,
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
			  main_cfg[SET_REWIND_MINUTES].hlp,
			  main_cfg[SET_ONLYCMDLINE_HIDDEN_GUI].hlp,
			  sch_input
	);
	init(uQString(uL("" NAME " Command Line Help")), uQStringCD(uQString(usage_string).arg(name)));
	free(usage_string);
}
wdgDialogCmdLineHelp::~wdgDialogCmdLineHelp() = default;

void wdgDialogCmdLineHelp::init(const QString title, const uTCHAR *usage_string, bool use_html) {
	setupUi(this);

	setWindowTitle(title);

	if (font().pointSize() > 9) {
		QFont font;

		font.setPointSize(9);
		setFont(font);
	}

	{
		QFont monospace("Monospace");

		monospace.setPointSize(font().pointSize());
		textEdit_cmdLineHelp->setFont(monospace);
	}

	if (use_html) {
		textEdit_cmdLineHelp->setHtml("<pre>" + uQString(usage_string) + "</pre>");
	} else {
		// in questo modo il QTextEdit utilizza il lineWrapMode, cosa che non farebbe con
		// l'html inserito nel tag '<pre>...</pre>'
		textEdit_cmdLineHelp->setPlainText(uQString(usage_string));
	}
}
