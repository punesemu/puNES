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

#include <math.h>
#include "wdgNTSCFilter.moc"
#include "mainWindow.hpp"
#include "video/filters/ntsc.h"
#include "conf.h"
#include "emu_thread.h"
#include "gui.h"

static const char parameters_desc[][15] = {
	"Hue",   "Saturation", "Contrast",  "Brightness", "Sharpness",
	"Gamma", "Resolution", "Artifacts", "Fringing",   "Bleed",
	"Scanline"
};

wdgNTSCFilter::wdgNTSCFilter(QWidget *parent) : QWidget(parent) {
	unsigned int i;

	setupUi(this);

	setFocusProxy(horizontalSlider_Hue);

	for (i = 0; i < LENGTH(parameters_desc); i++) {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(parameters_desc[i]));
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(parameters_desc[i]));
		QPushButton *btn = findChild<QPushButton *>("pushButton_" + QString(parameters_desc[i]));

		slider->setProperty("myIndex", QVariant(i));
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		sbox->setProperty("myIndex", QVariant(i));
		connect(sbox, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		btn->setProperty("myIndex", QVariant(i));
		connect(btn, SIGNAL(clicked(bool)), this, SLOT(s_default_value_clicked(bool)));
	}

	checkBox_Merge_Fields->setProperty("myIndex", QVariant(0));
	connect(checkBox_Merge_Fields, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	checkBox_Vertical_Blend->setProperty("myIndex", QVariant(1));
	connect(checkBox_Vertical_Blend, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	connect(pushButton_MFields_VBlend, SIGNAL(clicked(bool)), this, SLOT(s_default_value_mv_clicked(bool)));
	connect(pushButton_NTSC_Parameters_reset, SIGNAL(clicked(bool)), this, SLOT(s_reset(bool)));

	installEventFilter(this);
}
wdgNTSCFilter::~wdgNTSCFilter() {}

void wdgNTSCFilter::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgNTSCFilter::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgNTSCFilter::update_widget(void) {
	if (cfg->filter == NTSC_FILTER) {
		set_sliders_spins();
	}
	setEnabled(cfg->filter == NTSC_FILTER);
}

void wdgNTSCFilter::ntsc_update_paramaters(void) {
	emu_thread_pause();
	ntsc_effect_parameters_changed();
	emu_thread_continue();
}
void wdgNTSCFilter::set_sliders_spins(void) {
	nes_ntsc_setup_t *format = &ntsc_filter.format[cfg->ntsc_format];

	qtHelper::slider_set_value(horizontalSlider_Hue, round(format->hue * 100));
	qtHelper::slider_set_value(horizontalSlider_Saturation, round(format->saturation * 100));
	qtHelper::slider_set_value(horizontalSlider_Contrast, round(format->contrast * 100));
	qtHelper::slider_set_value(horizontalSlider_Brightness, round(format->brightness * 100));
	qtHelper::slider_set_value(horizontalSlider_Sharpness, round(format->sharpness * 100));
	qtHelper::slider_set_value(horizontalSlider_Gamma, round(format->gamma * 100));
	qtHelper::slider_set_value(horizontalSlider_Resolution, round(format->resolution * 100));
	qtHelper::slider_set_value(horizontalSlider_Artifacts, round(format->artifacts * 20));
	qtHelper::slider_set_value(horizontalSlider_Fringing, round(format->fringing * 20));
	qtHelper::slider_set_value(horizontalSlider_Bleed, round(format->bleed * 100));
	qtHelper::slider_set_value(horizontalSlider_Scanline, round((1.0f - format->scanline_intensity) * 100));

	qtHelper::spinbox_set_value(spinBox_Hue, round(format->hue * 100));
	qtHelper::spinbox_set_value(spinBox_Saturation, round(format->saturation * 100));
	qtHelper::spinbox_set_value(spinBox_Contrast, round(format->contrast * 100));
	qtHelper::spinbox_set_value(spinBox_Brightness, round(format->brightness * 100));
	qtHelper::spinbox_set_value(spinBox_Sharpness, round(format->sharpness * 100));
	qtHelper::spinbox_set_value(spinBox_Gamma, round(format->gamma * 100));
	qtHelper::spinbox_set_value(spinBox_Resolution, round(format->resolution * 100));
	qtHelper::spinbox_set_value(spinBox_Artifacts, round(format->artifacts * 20));
	qtHelper::spinbox_set_value(spinBox_Fringing, round(format->fringing * 20));
	qtHelper::spinbox_set_value(spinBox_Bleed, round(format->bleed * 100));
	qtHelper::spinbox_set_value(spinBox_Scanline, round((1.0f - format->scanline_intensity) * 100));

	qtHelper::checkbox_set_checked(checkBox_Merge_Fields, format->merge_fields);
	qtHelper::checkbox_set_checked(checkBox_Vertical_Blend, format->vertical_blend);
}

void wdgNTSCFilter::s_slider_spin_changed(int value) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	nes_ntsc_setup_t *format = &ntsc_filter.format[cfg->ntsc_format];

	switch (index) {
		default:
		case 0:
			format->hue = (double)value / 100.0f;
			break;
		case 1:
			format->saturation = (double)value / 100.0f;
			break;
		case 2:
			format->contrast = (double)value / 100.0f;
			break;
		case 3:
			format->brightness = (double)value / 100.0f;
			break;
		case 4:
			format->sharpness = (double)value / 100.0f;
			break;
		case 5:
			format->gamma = (double)value / 100.0f;
			break;
		case 6:
			format->resolution = (double)value / 100.0f;
			break;
		case 7:
			format->artifacts = (double)value / 20.0f;
			break;
		case 8:
			format->fringing = (double)value / 20.0f;
			break;
		case 9:
			format->bleed = (double)value / 100.0f;
			break;
		case 10:
			format->scanline_intensity = (double)(100 - value) / 100.0f;
			break;
	}
	if (((QObject *)sender())->objectName().contains("horizontalSlider_", Qt::CaseSensitive)) {
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(parameters_desc[index]));

		qtHelper::spinbox_set_value((void *)sbox, value);
	} else {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(parameters_desc[index]));

		qtHelper::slider_set_value((void *)slider, value);
	}
	ntsc_effect_parameters_changed();
}
void wdgNTSCFilter::s_checkbox_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	nes_ntsc_setup_t *format = &ntsc_filter.format[cfg->ntsc_format];

	switch (index) {
		case 0:
			format->merge_fields = state > 0;
			break;
		case 1:
			format->vertical_blend = state > 0;
			break;
	}
	ntsc_effect_parameters_changed();
}
void wdgNTSCFilter::s_default_value_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();

	ntsc_effect_parameter_default(index);
	gui_update_ntsc_widgets();
	ntsc_effect_parameters_changed();
}
void wdgNTSCFilter::s_default_value_mv_clicked(UNUSED(bool checked)) {
	ntsc_effect_parameter_mv_default();
	gui_update_ntsc_widgets();
	ntsc_effect_parameters_changed();
}
void wdgNTSCFilter::s_reset(UNUSED(bool checked)) {
	ntsc_effect_parameters_default();
	gui_update_ntsc_widgets();
	ntsc_effect_parameters_changed();
}
