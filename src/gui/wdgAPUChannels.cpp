/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include "wdgAPUChannels.moc"
#include "dlgSettings.hpp"
#include "audio/snd.h"
#include "conf.h"
#include "gui.h"

static const char channels_desc[7][15] = { "Square1",  "Square2", "Triangle", "Noise", "DMC", "Extra", "Master" };

wdgAPUChannels::wdgAPUChannels(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	setFocusProxy(checkBox_Master);

	for (int i = APU_S1; i <= APU_MASTER; i++) {
		QCheckBox *cbox = findChild<QCheckBox *>("checkBox_" + QString(channels_desc[i]));
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(channels_desc[i]));
		QLabel *label = findChild<QLabel *>("label_value_slider_" + QString(channels_desc[i]));

		cbox->setProperty("myIndex", QVariant(i));
		connect(cbox, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));

		slider->setRange(0, 100);
		slider->setProperty("myIndex", QVariant(i));
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));

		label->setFixedWidth(QLabel("000").sizeHint().width());
	}

	pushButton_APU_Channels_active_all->setProperty("myIndex", QVariant(TRUE));
	pushButton_APU_Channels_disable_all->setProperty("myIndex", QVariant(FALSE));
	pushButton_APU_Channels_reset->setProperty("myIndex", QVariant(2));

	connect(pushButton_APU_Channels_active_all, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_toggle_all(bool)));
	connect(pushButton_APU_Channels_disable_all, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_toggle_all(bool)));
	connect(pushButton_APU_Channels_reset, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_toggle_all(bool)));

	installEventFilter(this);
}
wdgAPUChannels::~wdgAPUChannels() {}

void wdgAPUChannels::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgAPUChannels::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgAPUChannels::update_widget(void) {
	for (int i = APU_S1; i <= APU_MASTER; i++) {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(channels_desc[i]));
		QCheckBox *cbox = findChild<QCheckBox *>("checkBox_" + QString(channels_desc[i]));

		cbox->setChecked(cfg->apu.channel[i]);
		slider->setValue(cfg->apu.volume[i] * 100);
	}
}

void wdgAPUChannels::volume_update_label(int type, int value) {
	QLabel *label = findChild<QLabel *>("label_value_slider_" + QString(channels_desc[type]));

	label->setText(QString("%1").arg(value, 3));
}

void wdgAPUChannels::s_apu_ch_checkbox(UNUSED(bool checked)) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();

	cfg_from_file.apu.channel[index] = !cfg_from_file.apu.channel[index];
	gui_apu_channels_widgets_update();
}
void wdgAPUChannels::s_apu_ch_slider(int value) {
	int index = QVariant(((QSlider *)sender())->property("myIndex")).toInt();

	cfg->apu.volume[index] = (double)value / 100.0f;
	volume_update_label(index, value);
	gui_apu_channels_widgets_update();
}
void wdgAPUChannels::s_apu_ch_toggle_all(UNUSED(bool checked)) {
	int mode = QVariant(((QPushButton *)sender())->property("myIndex")).toInt();
	BYTE i;

	if (mode == 2) {
		for (i = APU_S1; i <= APU_MASTER; i++) {
			cfg->apu.volume[i] = 1.0f;
		}
		mode = TRUE;
	}
	// non devo forzare cfg->apu.channel[APU_MASTER] perche'
	// lo utilizzo per abilitare o disabilitare il suono
	// globalmente e viene impostato altrove.
	for (i = APU_S1; i <= APU_EXTRA; i++) {
		cfg->apu.channel[i] = mode;
	}

	gui_apu_channels_widgets_update();
}
