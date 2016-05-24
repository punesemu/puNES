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

#include "dlgOverscanBorders.moc"
#include "mainWindow.hpp"
#include "settings.h"
#include "conf.h"
#include "clock.h"
#include "gfx.h"
#include "emu.h"
#include "gui.h"
#if defined (__linux__) || defined (WITH_D3D9)
#define __GFX_FORCE_SCALE__
#include "gfx_functions_inline.h"
#undef __GFX_FORCE_SCALE__
#endif

dlgOverscanBorders::dlgOverscanBorders(QWidget *parent = 0) : QDialog(parent) {
	memset(&data, 0x00, sizeof(data));
	memcpy(&data.overscan_borders, &overscan_borders, sizeof(overscan_borders));

	/*
	 * salvo sia il parametro dell'overscan
	 * che il settaggio attuale dei bordi.
	 */
	data.save_overscan = cfg->oscan;
	data.save_borders = (*overscan.borders);

	data.borders = &data.overscan_borders[0];

	setupUi(this);

	setFont(parent->font());

	frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	{
		comboBox_Mode->addItem(tr("NTSC"));
		comboBox_Mode->addItem(tr("PAL/Dendy"));

		if (machine.type == NTSC) {
			data.mode = 0;
		} else {
			data.mode = 1;
		}

		comboBox_Mode->setCurrentIndex(data.mode);
		data.borders = &data.overscan_borders[data.mode];

		connect(comboBox_Mode, SIGNAL(activated(int)), this, SLOT(s_combobox_activated(int)));
	}

	connect(pushButton_Preview, SIGNAL(clicked(bool)), this, SLOT(s_preview_clicked(bool)));
	connect(pushButton_Defaults, SIGNAL(clicked(bool)), this, SLOT(s_default_clicked(bool)));

	{
		spinBox_Up->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Down->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Left->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Right->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);

		connect(spinBox_Up, SIGNAL(valueChanged(int)), this, SLOT(s_spinbox_value_changed(int)));
		connect(spinBox_Down, SIGNAL(valueChanged(int)), this, SLOT(s_spinbox_value_changed(int)));
		connect(spinBox_Left, SIGNAL(valueChanged(int)), this, SLOT(s_spinbox_value_changed(int)));
		connect(spinBox_Right, SIGNAL(valueChanged(int)), this, SLOT(s_spinbox_value_changed(int)));
	}

	update_dialog();

	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));
	connect(pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(s_discard_clicked(bool)));

	setAttribute(Qt::WA_DeleteOnClose);
	setFixedSize(width(), height());

	installEventFilter(this);

	emu_pause(TRUE);
}
dlgOverscanBorders::~dlgOverscanBorders() {}
void dlgOverscanBorders::update_dialog(void) {
	spinBox_Up->setValue(data.borders->up);
	spinBox_Down->setValue(data.borders->down);
	spinBox_Left->setValue(data.borders->left);
	spinBox_Right->setValue(data.borders->right);
}
bool dlgOverscanBorders::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::Show) {
		parentMain->ui->action_Oscan_Set_Borders->setEnabled(false);
	} else if (event->type() == QEvent::Close) {
		BYTE force;

		/* aggiorno l'attuale tabella */
		force = overscan_set_mode(machine.type);

		/* ripristino il valore originario del parametro */
		if (data.save_overscan != cfg->oscan) {
			force = TRUE;
			cfg->oscan = data.save_overscan;
		}

		/*
		 * se le dimensioni dei bordi sono cambiati rispetto ai
		 * valori di ingresso allora forzo il gfx_set_screen.
		 */
		{
			BYTE i, *src = (BYTE *) &data.save_borders, *dst = (BYTE *) overscan.borders;

			for (i = 0; i < sizeof(_overscan_borders); i++) {
				if ((*(src + i)) != (*(dst + i))) {
					force = TRUE;
					break;
				}
			}
		}

		if (force == TRUE) {
#if defined (WITH_OPENGL) && defined (__WIN32__)
			gfx_sdlwe_set(SDLWIN_FORCE_SCALE, SDLWIN_NONE);
#else
			gfx_FORCE_SCALE();
#endif
		}

		emu_pause(FALSE);

		parentMain->ui->action_Oscan_Set_Borders->setEnabled(true);
	} else if (event->type() == QEvent::LanguageChange) {
		Set_borders::retranslateUi(this);
	}

	return (QObject::eventFilter(obj, event));
}
void dlgOverscanBorders::s_combobox_activated(int index) {
	data.mode = index;

	data.borders = &data.overscan_borders[data.mode];
	update_dialog();
}
void dlgOverscanBorders::s_preview_clicked(bool checked) {
	cfg->oscan = OSCAN_ON;
	data.preview = (*data.borders);
	overscan.borders = &data.preview;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
void dlgOverscanBorders::s_default_clicked(bool checked) {
	settings_set_overscan_default(data.borders, data.mode + NTSC);
	update_dialog();
}
void dlgOverscanBorders::s_spinbox_value_changed(int i) {
	QString txt = ((QSpinBox *)sender())->objectName();

	if (txt == "spinBox_Up") {
		data.borders->up = i;
	} else if (txt == "spinBox_Down") {
		data.borders->down = i;
	} else if (txt == "spinBox_Left") {
		data.borders->left = i;
	} else if (txt == "spinBox_Right") {
		data.borders->right = i;
	}
}
void dlgOverscanBorders::s_apply_clicked(bool checked) {
	memcpy(&overscan_borders, &data.overscan_borders, sizeof(overscan_borders));
	close();
}
void dlgOverscanBorders::s_discard_clicked(bool checked) {
	close();
}
