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

#include <QtWidgets/QDesktopWidget>
#include "wdgAPUChannels.moc"
#include "dlgSettings.hpp"
#include "audio/snd.h"
#include "conf.h"
#include "gui.h"

wdgAPUChannels::wdgAPUChannels(QWidget *parent) : QWidget(parent) {
	setupUi(this);

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

	connect(horizontalSlider_Master, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));
	connect(horizontalSlider_Square1, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));
	connect(horizontalSlider_Square2, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));
	connect(horizontalSlider_Triangle, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));
	connect(horizontalSlider_Noise, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));
	connect(horizontalSlider_DMC, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));
	connect(horizontalSlider_Extra, SIGNAL(valueChanged(int)), this, SLOT(s_apu_ch_slider(int)));

	checkBox_Master->setProperty("myIndex", QVariant(APU_MASTER));
	checkBox_Square1->setProperty("myIndex", QVariant(APU_S1));
	checkBox_Square2->setProperty("myIndex", QVariant(APU_S2));
	checkBox_Triangle->setProperty("myIndex", QVariant(APU_TR));
	checkBox_Noise->setProperty("myIndex", QVariant(APU_NS));
	checkBox_DMC->setProperty("myIndex", QVariant(APU_DMC));
	checkBox_Extra->setProperty("myIndex", QVariant(APU_EXTRA));

	connect(checkBox_Master, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));
	connect(checkBox_Square1, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));
	connect(checkBox_Square2, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));
	connect(checkBox_Triangle, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));
	connect(checkBox_Noise, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));
	connect(checkBox_DMC, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));
	connect(checkBox_Extra, SIGNAL(clicked(bool)), this, SLOT(s_apu_ch_checkbox(bool)));

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
}

void wdgAPUChannels::s_apu_ch_checkbox(UNUSED(bool checked)) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();

	cfg_from_file.apu.channel[index] = !cfg_from_file.apu.channel[index];

	if (index == APU_MASTER) {
		if (cfg->apu.channel[APU_MASTER]) {
			snd_playback_start();
		} else {
			snd_playback_stop();
		}
	}
	gui_apu_channels_widgets_update();
}
void wdgAPUChannels::s_apu_ch_slider(int value) {
	int index = QVariant(((QSlider *)sender())->property("myIndex")).toInt();

	cfg->apu.volume[index] = (double)value / 100.0f;
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
