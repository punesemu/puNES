/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#include "dlgApuChannels.moc"
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDesktopWidget>
#else
#include <QtWidgets/QDesktopWidget>
#endif
#include "mainWindow.hpp"
#include "snd.h"
#include "conf.h"
#include "gui.h"

dlgApuChannels::dlgApuChannels(QWidget *parent = 0) : QDialog(parent) {
	in_update = false;

	setupUi(this);

	setFont(parent->font());
	setStyleSheet(tools_stylesheet());

	horizontalSlider_Master->setRange(0, 100);
	horizontalSlider_Square1->setRange(0, 100);
	horizontalSlider_Square2->setRange(0, 100);
	horizontalSlider_Triangle->setRange(0, 100);
	horizontalSlider_Noise->setRange(0, 100);
	horizontalSlider_DMC->setRange(0, 100);
	horizontalSlider_Extra->setRange(0, 100);

	horizontalSlider_Master->setProperty("myIndex", QVariant(APU_MASTER));
	horizontalSlider_Square1->setProperty("myIndex", QVariant(APU_S1));
	horizontalSlider_Square2->setProperty("myIndex", QVariant(APU_S2));
	horizontalSlider_Triangle->setProperty("myIndex", QVariant(APU_TR));
	horizontalSlider_Noise->setProperty("myIndex", QVariant(APU_NS));
	horizontalSlider_DMC->setProperty("myIndex", QVariant(APU_DMC));
	horizontalSlider_Extra->setProperty("myIndex", QVariant(APU_EXTRA));

	connect(horizontalSlider_Master, SIGNAL(valueChanged(int)), this,
			SLOT(s_slider_value_changed(int)));
	connect(horizontalSlider_Square1, SIGNAL(valueChanged(int)), this,
			SLOT(s_slider_value_changed(int)));
	connect(horizontalSlider_Square2, SIGNAL(valueChanged(int)), this,
			SLOT(s_slider_value_changed(int)));
	connect(horizontalSlider_Triangle, SIGNAL(valueChanged(int)), this,
			SLOT(s_slider_value_changed(int)));
	connect(horizontalSlider_Noise, SIGNAL(valueChanged(int)), this,
			SLOT(s_slider_value_changed(int)));
	connect(horizontalSlider_DMC, SIGNAL(valueChanged(int)), this,
			SLOT(s_slider_value_changed(int)));
	connect(horizontalSlider_Extra, SIGNAL(valueChanged(int)), this,
			SLOT(s_slider_value_changed(int)));

	checkBox_Master->setProperty("myIndex", QVariant(APU_MASTER));
	checkBox_Square1->setProperty("myIndex", QVariant(APU_S1));
	checkBox_Square2->setProperty("myIndex", QVariant(APU_S2));
	checkBox_Triangle->setProperty("myIndex", QVariant(APU_TR));
	checkBox_Noise->setProperty("myIndex", QVariant(APU_NS));
	checkBox_DMC->setProperty("myIndex", QVariant(APU_DMC));
	checkBox_Extra->setProperty("myIndex", QVariant(APU_EXTRA));

	connect(checkBox_Master, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));
	connect(checkBox_Square1, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));
	connect(checkBox_Square2, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));
	connect(checkBox_Triangle, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));
	connect(checkBox_Noise, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));
	connect(checkBox_DMC, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));
	connect(checkBox_Extra, SIGNAL(stateChanged(int)), this,
			SLOT(s_checkbox_state_changed(int)));

	pushButton_Active_all->setProperty("myIndex", QVariant(TRUE));
	pushButton_Disable_all->setProperty("myIndex", QVariant(FALSE));
	pushButton_Defaults->setProperty("myIndex", QVariant(2));

	connect(pushButton_Active_all, SIGNAL(clicked(bool)), this, SLOT(s_toggle_all_clicked(bool)));
	connect(pushButton_Disable_all, SIGNAL(clicked(bool)), this, SLOT(s_toggle_all_clicked(bool)));
	connect(pushButton_Defaults, SIGNAL(clicked(bool)), this, SLOT(s_toggle_all_clicked(bool)));

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setAttribute(Qt::WA_DeleteOnClose);
	setFixedSize(width(), height());

	installEventFilter(this);
}
dlgApuChannels::~dlgApuChannels() {}
int dlgApuChannels::update_pos(int startY) {
	int screenNumber = qApp->desktop()->screenNumber(parentWidget());
	int x = parentWidget()->pos().x() + parentWidget()->frameGeometry().width();
	int y = parentWidget()->geometry().y() + startY;

	if ((parentWidget()->pos().x() + parentWidget()->frameGeometry().width()
			+ frameGeometry().width() - qApp->desktop()->screenGeometry(screenNumber).left())
			> qApp->desktop()->screenGeometry(screenNumber).width()) {
		x = parentWidget()->pos().x() - frameGeometry().width();
	}
	move(QPoint(x, y));

	if (isHidden() == true) {
		return (0);
	}

	return (frameGeometry().height());
}
bool dlgApuChannels::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		APU_channels::retranslateUi(this);
	}

	gui_control_pause_bck(event->type());

	return (QObject::eventFilter(obj, event));
}
void dlgApuChannels::update_dialog(void) {
	in_update = true;

	checkBox_Master->setChecked(cfg->apu.channel[APU_MASTER]);
	checkBox_Square1->setChecked(cfg->apu.channel[APU_S1]);
	checkBox_Square2->setChecked(cfg->apu.channel[APU_S2]);
	checkBox_Triangle->setChecked(cfg->apu.channel[APU_TR]);
	checkBox_Noise->setChecked(cfg->apu.channel[APU_NS]);
	checkBox_DMC->setChecked(cfg->apu.channel[APU_DMC]);
	checkBox_Extra->setChecked(cfg->apu.channel[APU_EXTRA]);

	horizontalSlider_Master->setValue(cfg->apu.volume[APU_MASTER] * 100);
	horizontalSlider_Square1->setValue(cfg->apu.volume[APU_S1] * 100);
	horizontalSlider_Square2->setValue(cfg->apu.volume[APU_S2] * 100);
	horizontalSlider_Triangle->setValue(cfg->apu.volume[APU_TR] * 100);
	horizontalSlider_Noise->setValue(cfg->apu.volume[APU_NS] * 100);
	horizontalSlider_DMC->setValue(cfg->apu.volume[APU_DMC] * 100);
	horizontalSlider_Extra->setValue(cfg->apu.volume[APU_EXTRA] * 100);

	in_update = false;
}
void dlgApuChannels::s_checkbox_state_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();

	if (in_update == true) {
		return;
	}

	cfg_from_file.apu.channel[index] = 1;

	if (state == Qt::Unchecked) {
		cfg_from_file.apu.channel[index] = 0;
	}

	if (index == APU_MASTER) {
		if (cfg->apu.channel[APU_MASTER]) {
			snd_playback_start();
		} else {
			snd_playback_stop();
		}
		gui_update();
	}
}
void dlgApuChannels::s_slider_value_changed(int value) {
	int index = QVariant(((QSlider *)sender())->property("myIndex")).toInt();

	cfg->apu.volume[index] = (double) value / 100.0f;
}
void dlgApuChannels::s_toggle_all_clicked(bool checked) {
	int mode = QVariant(((QPushButton *)sender())->property("myIndex")).toInt();
	BYTE i;

	if (mode == 2) {
		for (i = APU_S1; i <= APU_MASTER; i++) {
			cfg->apu.volume[i] = 1.0f;
		}
		mode = TRUE;
	}
	/*
	 * non devo forzare cfg->apu.channel[APU_MASTER] perche'
	 * lo utilizzo per abilitare o disabilitare il suono
	 * globalmente e viene impostato altrove.
	 */
	for (i = APU_S1; i <= APU_EXTRA; i++) {
		cfg->apu.channel[i] = mode;
	}

	update_dialog();
}
