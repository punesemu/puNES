/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <cmath>
#include "wdgNTSCFilter.hpp"
#include "mainWindow.hpp"
#include "conf.h"
#include "emu_thread.h"
#include "video/gfx_thread.h"
#include "gui.h"

// wdgNTSCFilter ------------------------------------------------------------------------------------------------------

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
}
wdgNTSCFilter::~wdgNTSCFilter() = default;

void wdgNTSCFilter::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgNTSCFilter::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgNTSCFilter::update_widget(void) {
	bool enabled = (cfg->filter == NTSC_FILTER);

	if (enabled) {
		set_sliders_spins();
	}
	setVisible(enabled);
}

void wdgNTSCFilter::ntsc_update_paramaters(void) {
	emu_thread_pause();
	ntsc_filter_parameters_changed();
	emu_thread_continue();
}
void wdgNTSCFilter::set_sliders_spins(void) {
	nes_ntsc_setup_t *format = &ntsc_filter.format[cfg->ntsc_format];

	qtHelper::slider_set_value(horizontalSlider_Hue, (int)round(format->hue * 100));
	qtHelper::slider_set_value(horizontalSlider_Saturation, (int)round(format->saturation * 100));
	qtHelper::slider_set_value(horizontalSlider_Contrast, (int)round(format->contrast * 100));
	qtHelper::slider_set_value(horizontalSlider_Brightness, (int)round(format->brightness * 100));
	qtHelper::slider_set_value(horizontalSlider_Sharpness, (int)round(format->sharpness * 100));
	qtHelper::slider_set_value(horizontalSlider_Gamma, (int)round(format->gamma * 100));
	qtHelper::slider_set_value(horizontalSlider_Resolution, (int)round(format->resolution * 100));
	qtHelper::slider_set_value(horizontalSlider_Artifacts, (int)round(format->artifacts * 20));
	qtHelper::slider_set_value(horizontalSlider_Fringing, (int)round(format->fringing * 20));
	qtHelper::slider_set_value(horizontalSlider_Bleed, (int)round(format->bleed * 100));
	qtHelper::slider_set_value(horizontalSlider_Scanline, (int)round((1.0f - format->scanline_intensity) * 100));

	qtHelper::spinbox_set_value(spinBox_Hue, (int)round(format->hue * 100));
	qtHelper::spinbox_set_value(spinBox_Saturation, (int)round(format->saturation * 100));
	qtHelper::spinbox_set_value(spinBox_Contrast, (int)round(format->contrast * 100));
	qtHelper::spinbox_set_value(spinBox_Brightness, (int)round(format->brightness * 100));
	qtHelper::spinbox_set_value(spinBox_Sharpness, (int)round(format->sharpness * 100));
	qtHelper::spinbox_set_value(spinBox_Gamma, (int)round(format->gamma * 100));
	qtHelper::spinbox_set_value(spinBox_Resolution, (int)round(format->resolution * 100));
	qtHelper::spinbox_set_value(spinBox_Artifacts, (int)round(format->artifacts * 20));
	qtHelper::spinbox_set_value(spinBox_Fringing, (int)round(format->fringing * 20));
	qtHelper::spinbox_set_value(spinBox_Bleed, (int)round(format->bleed * 100));
	qtHelper::spinbox_set_value(spinBox_Scanline, (int)round((1.0f - format->scanline_intensity) * 100));

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
	ntsc_filter_parameters_changed();
}
void wdgNTSCFilter::s_checkbox_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	nes_ntsc_setup_t *format = &ntsc_filter.format[cfg->ntsc_format];

	switch (index) {
		default:
		case 0:
			format->merge_fields = state > 0;
			break;
		case 1:
			format->vertical_blend = state > 0;
			break;
	}
	ntsc_filter_parameters_changed();
}
void wdgNTSCFilter::s_default_value_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();

	ntsc_filter_parameter_default(index);
	gui_update_ntsc_widgets();
	ntsc_filter_parameters_changed();
}
void wdgNTSCFilter::s_default_value_mv_clicked(UNUSED(bool checked)) {
	ntsc_filter_parameter_mv_default();
	gui_update_ntsc_widgets();
	ntsc_filter_parameters_changed();
}
void wdgNTSCFilter::s_reset(UNUSED(bool checked)) {
	ntsc_filter_parameters_default();
	gui_update_ntsc_widgets();
	ntsc_filter_parameters_changed();
}

// wdgNTSCBisqwitFilter -----------------------------------------------------------------------------------------------

static const char bisqwit_parameters_desc[][15] = {
	"Hue", "Saturation", "Contrast", "Brightness", "YWidth", "IWidth", "QWidth", "Scanline"
};

wdgNTSCBisqwitFilter::wdgNTSCBisqwitFilter(QWidget *parent) : QWidget(parent) {
	unsigned int i;

	setupUi(this);

	setFocusProxy(horizontalSlider_Hue);

	for (i = 0; i < LENGTH(bisqwit_parameters_desc); i++) {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(bisqwit_parameters_desc[i]));
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(bisqwit_parameters_desc[i]));
		QPushButton *btn = findChild<QPushButton *>("pushButton_" + QString(bisqwit_parameters_desc[i]));

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
}
wdgNTSCBisqwitFilter::~wdgNTSCBisqwitFilter() = default;

void wdgNTSCBisqwitFilter::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgNTSCBisqwitFilter::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgNTSCBisqwitFilter::update_widget(void) {
	bool enabled = (cfg->filter >= NTSC_BISQWIT_2X) && (cfg->filter <= NTSC_BISQWIT_8X);

	if (enabled) {
		set_sliders_spins();
	}
	setVisible(enabled);
}

void wdgNTSCBisqwitFilter::ntsc_update_paramaters(void) {
	emu_thread_pause();
	ntsc_bisqwit_init();
	emu_thread_continue();
}
void wdgNTSCBisqwitFilter::set_sliders_spins(void) {
	_ntsc_bisqwit_setup_t *format = &ntsc_bisqwit;

	qtHelper::slider_set_value(horizontalSlider_Hue, (int)round(format->hue * 100));
	qtHelper::slider_set_value(horizontalSlider_Saturation, (int)round(format->saturation * 100));
	qtHelper::slider_set_value(horizontalSlider_Contrast, (int)round(format->contrast * 100));
	qtHelper::slider_set_value(horizontalSlider_Brightness, (int)round(format->brightness * 100));
	qtHelper::slider_set_value(horizontalSlider_YWidth, format->ywidth);
	qtHelper::slider_set_value(horizontalSlider_IWidth, format->iwidth);
	qtHelper::slider_set_value(horizontalSlider_QWidth, format->qwidth);
	qtHelper::slider_set_value(horizontalSlider_Scanline, (int)round((1.0f - format->scanline_intensity) * 100));

	qtHelper::spinbox_set_value(spinBox_Hue, (int)round(format->hue * 100));
	qtHelper::spinbox_set_value(spinBox_Saturation, (int)round(format->saturation * 100));
	qtHelper::spinbox_set_value(spinBox_Contrast, (int)round(format->contrast * 100));
	qtHelper::spinbox_set_value(spinBox_Brightness, (int)round(format->brightness * 100));
	qtHelper::spinbox_set_value(spinBox_YWidth, format->ywidth);
	qtHelper::spinbox_set_value(spinBox_IWidth, format->iwidth);
	qtHelper::spinbox_set_value(spinBox_QWidth, format->qwidth);
	qtHelper::spinbox_set_value(spinBox_Scanline, (int)round((1.0f - format->scanline_intensity) * 100));

	qtHelper::checkbox_set_checked(checkBox_Merge_Fields, format->merge_fields);
	qtHelper::checkbox_set_checked(checkBox_Vertical_Blend, format->vertical_blend);
}

void wdgNTSCBisqwitFilter::s_slider_spin_changed(int value) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	_ntsc_bisqwit_setup_t *format = &ntsc_bisqwit;

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
			format->ywidth = value;
			break;
		case 5:
			format->iwidth = value;
			break;
		case 6:
			format->qwidth = value;
			break;
		case 7:
			format->scanline_intensity = (double)(100 - value) / 100.0f;
			break;
	}
	if (((QObject *)sender())->objectName().contains("horizontalSlider_", Qt::CaseSensitive)) {
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(bisqwit_parameters_desc[index]));

		qtHelper::spinbox_set_value((void *)sbox, value);
	} else {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(bisqwit_parameters_desc[index]));

		qtHelper::slider_set_value((void *)slider, value);
	}
	ntsc_bisqwit_filter_parameters_changed();
}
void wdgNTSCBisqwitFilter::s_checkbox_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	_ntsc_bisqwit_setup_t *format = &ntsc_bisqwit;

	switch (index) {
		default:
		case 0:
			format->merge_fields = state > 0;
			break;
		case 1:
			format->vertical_blend = state > 0;
			break;
	}
	ntsc_bisqwit_filter_parameters_changed();
}
void wdgNTSCBisqwitFilter::s_default_value_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();

	ntsc_bisqwit_filter_parameter_default(index);
	gui_update_ntsc_widgets();
	ntsc_bisqwit_filter_parameters_changed();
}
void wdgNTSCBisqwitFilter::s_default_value_mv_clicked(UNUSED(bool checked)) {
	ntsc_bisqwit_filter_parameter_mv_default();
	gui_update_ntsc_widgets();
	ntsc_bisqwit_filter_parameters_changed();
}
void wdgNTSCBisqwitFilter::s_reset(UNUSED(bool checked)) {
	ntsc_bisqwit_filter_parameters_default();
	gui_update_ntsc_widgets();
	ntsc_bisqwit_filter_parameters_changed();
}

// wdgNTSCLMP88959Filter ----------------------------------------------------------------------------------------------

static const char ntsc_lmp88959_parameters_desc[][15] = {
	"Brightness", "Hue", "Saturation", "Contrast", "Black_Point", "White_Point", "Noise"
};

wdgNTSCLMP88959Filter::wdgNTSCLMP88959Filter(QWidget *parent) : QWidget(parent) {
	unsigned int i;

	setupUi(this);

	setFocusProxy(horizontalSlider_Brightness);

	for (i = 0; i < LENGTH(ntsc_lmp88959_parameters_desc); i++) {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(ntsc_lmp88959_parameters_desc[i]));
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(ntsc_lmp88959_parameters_desc[i]));
		QPushButton *btn = findChild<QPushButton *>("pushButton_" + QString(ntsc_lmp88959_parameters_desc[i]));

		slider->setProperty("myIndex", QVariant(i));
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		sbox->setProperty("myIndex", QVariant(i));
		connect(sbox, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		btn->setProperty("myIndex", QVariant(i));
		connect(btn, SIGNAL(clicked(bool)), this, SLOT(s_default_value_clicked(bool)));
	}

	checkBox_Scanline->setProperty("myIndex", QVariant(0));
	connect(checkBox_Scanline, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	checkBox_Merge_Fields->setProperty("myIndex", QVariant(1));
	connect(checkBox_Merge_Fields, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	checkBox_Vertical_Blend->setProperty("myIndex", QVariant(2));
	connect(checkBox_Vertical_Blend, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	connect(pushButton_Scanline_MFields_VBlend, SIGNAL(clicked(bool)), this, SLOT(s_default_value_smv_clicked(bool)));
	connect(pushButton_NTSC_Parameters_reset, SIGNAL(clicked(bool)), this, SLOT(s_reset(bool)));
}
wdgNTSCLMP88959Filter::~wdgNTSCLMP88959Filter() = default;

void wdgNTSCLMP88959Filter::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgNTSCLMP88959Filter::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgNTSCLMP88959Filter::update_widget(void) {
	bool enabled = cfg->filter == NTSC_CRT_LMP88959;

	if (enabled) {
		set_sliders_spins();
	}
	setVisible(enabled);
}

void wdgNTSCLMP88959Filter::ntsc_update_paramaters(void) {
	emu_thread_pause();
	ntsc_lmp88959_init();
	emu_thread_continue();
}
void wdgNTSCLMP88959Filter::set_sliders_spins(void) {
	_ntsc_lmp88959_setup_t *format = &ntsc_lmp88959;

	qtHelper::slider_set_value(horizontalSlider_Brightness, format->brightness);
	qtHelper::slider_set_value(horizontalSlider_Hue, format->hue);
	qtHelper::slider_set_value(horizontalSlider_Saturation, format->saturation);
	qtHelper::slider_set_value(horizontalSlider_Contrast, format->contrast);
	qtHelper::slider_set_value(horizontalSlider_Black_Point, format->black_point);
	qtHelper::slider_set_value(horizontalSlider_White_Point, format->white_point);
	qtHelper::slider_set_value(horizontalSlider_Noise, format->noise);

	qtHelper::spinbox_set_value(spinBox_Brightness, format->brightness);
	qtHelper::spinbox_set_value(spinBox_Hue, format->hue);
	qtHelper::spinbox_set_value(spinBox_Saturation, format->saturation);
	qtHelper::spinbox_set_value(spinBox_Contrast, format->contrast);
	qtHelper::spinbox_set_value(spinBox_Black_Point, format->black_point);
	qtHelper::spinbox_set_value(spinBox_White_Point, format->white_point);
	qtHelper::spinbox_set_value(spinBox_Noise, format->noise);

	qtHelper::checkbox_set_checked(checkBox_Scanline, format->scanline);
	qtHelper::checkbox_set_checked(checkBox_Merge_Fields, format->merge_fields);
	qtHelper::checkbox_set_checked(checkBox_Vertical_Blend, format->vertical_blend);
}

void wdgNTSCLMP88959Filter::s_slider_spin_changed(int value) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	_ntsc_lmp88959_setup_t *format = &ntsc_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->brightness = value;
			break;
		case 1:
			format->hue = value;
			break;
		case 2:
			format->saturation = value;
			break;
		case 3:
			format->contrast = value;
			break;
		case 4:
			format->black_point = value;
			break;
		case 5:
			format->white_point = value;
			break;
		case 6:
			format->noise = value;
			break;
	}
	if (((QObject *)sender())->objectName().contains("horizontalSlider_", Qt::CaseSensitive)) {
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(ntsc_lmp88959_parameters_desc[index]));

		qtHelper::spinbox_set_value((void *)sbox, value);
	} else {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(ntsc_lmp88959_parameters_desc[index]));

		qtHelper::slider_set_value((void *)slider, value);
	}
	ntsc_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCLMP88959Filter::s_checkbox_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	_ntsc_lmp88959_setup_t *format = &ntsc_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->scanline = state > 0;
			break;
		case 1:
			format->merge_fields = state > 0;
			break;
		case 2:
			format->vertical_blend = state > 0;
			break;
	}
	ntsc_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCLMP88959Filter::s_default_value_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();

	gfx_thread_pause();
	ntsc_lmp88959_filter_parameter_default(index);
	gui_update_ntsc_widgets();
	ntsc_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCLMP88959Filter::s_default_value_smv_clicked(UNUSED(bool checked)) {
	gfx_thread_pause();
	ntsc_lmp88959_filter_parameter_smv_default();
	gui_update_ntsc_widgets();
	ntsc_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCLMP88959Filter::s_reset(UNUSED(bool checked)) {
	gfx_thread_pause();
	ntsc_lmp88959_filter_parameters_default();
	gui_update_ntsc_widgets();
	ntsc_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}

// wdgNTSCNESRGBLMP88959Filter ----------------------------------------------------------------------------------------

static const char ntsc_nesrgb_lmp88959_parameters_desc[][15] = {
	"Brightness", "Hue", "Saturation", "Contrast", "Black_Point", "White_Point", "Noise", "Dot_Crawl"
};

wdgNTSCNESRGBLMP88959Filter::wdgNTSCNESRGBLMP88959Filter(QWidget *parent) : QWidget(parent) {
	unsigned int i;

	setupUi(this);

	setFocusProxy(horizontalSlider_Brightness);

	for (i = 0; i < LENGTH(ntsc_nesrgb_lmp88959_parameters_desc); i++) {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(ntsc_nesrgb_lmp88959_parameters_desc[i]));
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(ntsc_nesrgb_lmp88959_parameters_desc[i]));
		QPushButton *btn = findChild<QPushButton *>("pushButton_" + QString(ntsc_nesrgb_lmp88959_parameters_desc[i]));

		slider->setProperty("myIndex", QVariant(i));
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		sbox->setProperty("myIndex", QVariant(i));
		connect(sbox, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		btn->setProperty("myIndex", QVariant(i));
		connect(btn, SIGNAL(clicked(bool)), this, SLOT(s_default_value_clicked(bool)));
	}

	checkBox_Scanline->setProperty("myIndex", QVariant(0));
	connect(checkBox_Scanline, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	checkBox_Vertical_Blend->setProperty("myIndex", QVariant(1));
	connect(checkBox_Vertical_Blend, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	connect(pushButton_Scanline_VBlend, SIGNAL(clicked(bool)), this, SLOT(s_default_value_sv_clicked(bool)));
	connect(pushButton_NTSC_Parameters_reset, SIGNAL(clicked(bool)), this, SLOT(s_reset(bool)));
}
wdgNTSCNESRGBLMP88959Filter::~wdgNTSCNESRGBLMP88959Filter() = default;

void wdgNTSCNESRGBLMP88959Filter::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgNTSCNESRGBLMP88959Filter::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgNTSCNESRGBLMP88959Filter::update_widget(void) {
	bool enabled = cfg->filter == NTSC_NESRGB_LMP88959;

	if (enabled) {
		set_sliders_spins();
	}
	setVisible(enabled);
}

void wdgNTSCNESRGBLMP88959Filter::ntsc_update_paramaters(void) {
	emu_thread_pause();
	ntsc_nesrgb_lmp88959_init();
	emu_thread_continue();
}
void wdgNTSCNESRGBLMP88959Filter::set_sliders_spins(void) {
	_ntsc_lmp88959_setup_t *format = &ntsc_nesrgb_lmp88959;

	qtHelper::slider_set_value(horizontalSlider_Brightness, format->brightness);
	qtHelper::slider_set_value(horizontalSlider_Hue, format->hue);
	qtHelper::slider_set_value(horizontalSlider_Saturation, format->saturation);
	qtHelper::slider_set_value(horizontalSlider_Contrast, format->contrast);
	qtHelper::slider_set_value(horizontalSlider_Black_Point, format->black_point);
	qtHelper::slider_set_value(horizontalSlider_White_Point, format->white_point);
	qtHelper::slider_set_value(horizontalSlider_Noise, format->noise);
	qtHelper::slider_set_value(horizontalSlider_Dot_Crawl, format->dot_crowl);

	qtHelper::spinbox_set_value(spinBox_Brightness, format->brightness);
	qtHelper::spinbox_set_value(spinBox_Hue, format->hue);
	qtHelper::spinbox_set_value(spinBox_Saturation, format->saturation);
	qtHelper::spinbox_set_value(spinBox_Contrast, format->contrast);
	qtHelper::spinbox_set_value(spinBox_Black_Point, format->black_point);
	qtHelper::spinbox_set_value(spinBox_White_Point, format->white_point);
	qtHelper::spinbox_set_value(spinBox_Noise, format->noise);
	qtHelper::spinbox_set_value(spinBox_Dot_Crawl, format->dot_crowl);

	qtHelper::checkbox_set_checked(checkBox_Scanline, format->scanline);
	qtHelper::checkbox_set_checked(checkBox_Vertical_Blend, format->vertical_blend);
}

void wdgNTSCNESRGBLMP88959Filter::s_slider_spin_changed(int value) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	_ntsc_lmp88959_setup_t *format = &ntsc_nesrgb_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->brightness = value;
			break;
		case 1:
			format->hue = value;
			break;
		case 2:
			format->saturation = value;
			break;
		case 3:
			format->contrast = value;
			break;
		case 4:
			format->black_point = value;
			break;
		case 5:
			format->white_point = value;
			break;
		case 6:
			format->noise = value;
			break;
		case 7:
			format->dot_crowl = value;
			break;
	}
	if (((QObject *)sender())->objectName().contains("horizontalSlider_", Qt::CaseSensitive)) {
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(ntsc_nesrgb_lmp88959_parameters_desc[index]));

		qtHelper::spinbox_set_value((void *)sbox, value);
	} else {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(ntsc_nesrgb_lmp88959_parameters_desc[index]));

		qtHelper::slider_set_value((void *)slider, value);
	}
	ntsc_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCNESRGBLMP88959Filter::s_checkbox_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	_ntsc_lmp88959_setup_t *format = &ntsc_nesrgb_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->scanline = state > 0;
			break;
		case 1:
			format->vertical_blend = state > 0;
			break;
	}
	ntsc_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCNESRGBLMP88959Filter::s_default_value_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();

	gfx_thread_pause();
	ntsc_nesrgb_lmp88959_filter_parameter_default(index);
	gui_update_ntsc_widgets();
	ntsc_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCNESRGBLMP88959Filter::s_default_value_sv_clicked(UNUSED(bool checked)) {
	gfx_thread_pause();
	ntsc_nesrgb_lmp88959_filter_parameter_sv_default();
	gui_update_ntsc_widgets();
	ntsc_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgNTSCNESRGBLMP88959Filter::s_reset(UNUSED(bool checked)) {
	gfx_thread_pause();
	ntsc_nesrgb_lmp88959_filter_parameters_default();
	gui_update_ntsc_widgets();
	ntsc_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}

// wdgPALLMP88959Filter -----------------------------------------------------------------------------------------------

static const char pal_lmp88959_parameters_desc[][15] = {
	"Brightness" , "Saturation", "Contrast", "Black_Point", "White_Point", "Noise", "Color_Phase", "Chroma_Lag"
};

wdgPALLMP88959Filter::wdgPALLMP88959Filter(QWidget *parent) : QWidget(parent) {
	unsigned int i;

	setupUi(this);

	setFocusProxy(horizontalSlider_Brightness);

	for (i = 0; i < LENGTH(pal_lmp88959_parameters_desc); i++) {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(pal_lmp88959_parameters_desc[i]));
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(pal_lmp88959_parameters_desc[i]));
		QPushButton *btn = findChild<QPushButton *>("pushButton_" + QString(pal_lmp88959_parameters_desc[i]));

		slider->setProperty("myIndex", QVariant(i));
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		sbox->setProperty("myIndex", QVariant(i));
		connect(sbox, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		btn->setProperty("myIndex", QVariant(i));
		connect(btn, SIGNAL(clicked(bool)), this, SLOT(s_default_value_clicked(bool)));
	}

	checkBox_Scanline->setProperty("myIndex", QVariant(0));
	connect(checkBox_Scanline, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	checkBox_Merge_Fields->setProperty("myIndex", QVariant(1));
	connect(checkBox_Merge_Fields, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	checkBox_Vertical_Blend->setProperty("myIndex", QVariant(2));
	connect(checkBox_Vertical_Blend, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	connect(pushButton_Scanline_MFields_VBlend, SIGNAL(clicked(bool)), this, SLOT(s_default_value_smv_clicked(bool)));
	connect(pushButton_PAL_Parameters_reset, SIGNAL(clicked(bool)), this, SLOT(s_reset(bool)));
}
wdgPALLMP88959Filter::~wdgPALLMP88959Filter() = default;

void wdgPALLMP88959Filter::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgPALLMP88959Filter::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgPALLMP88959Filter::update_widget(void) {
	bool enabled = cfg->filter == PAL_CRT_LMP88959;

	if (enabled) {
		set_sliders_spins();
	}
	setVisible(enabled);
}

void wdgPALLMP88959Filter::pal_update_paramaters(void) {
	emu_thread_pause();
	pal_lmp88959_init();
	emu_thread_continue();
}
void wdgPALLMP88959Filter::set_sliders_spins(void) {
	_pal_lmp88959_setup_t *format = &pal_lmp88959;

	qtHelper::slider_set_value(horizontalSlider_Brightness, format->brightness);
	qtHelper::slider_set_value(horizontalSlider_Saturation, format->saturation);
	qtHelper::slider_set_value(horizontalSlider_Contrast, format->contrast);
	qtHelper::slider_set_value(horizontalSlider_Black_Point, format->black_point);
	qtHelper::slider_set_value(horizontalSlider_White_Point, format->white_point);
	qtHelper::slider_set_value(horizontalSlider_Noise, format->noise);
	qtHelper::slider_set_value(horizontalSlider_Color_Phase, format->color_phase);
	qtHelper::slider_set_value(horizontalSlider_Chroma_Lag, format->chroma_lag);

	qtHelper::spinbox_set_value(spinBox_Brightness, format->brightness);
	qtHelper::spinbox_set_value(spinBox_Saturation, format->saturation);
	qtHelper::spinbox_set_value(spinBox_Contrast, format->contrast);
	qtHelper::spinbox_set_value(spinBox_Black_Point, format->black_point);
	qtHelper::spinbox_set_value(spinBox_White_Point, format->white_point);
	qtHelper::spinbox_set_value(spinBox_Noise, format->noise);
	qtHelper::spinbox_set_value(spinBox_Color_Phase, format->color_phase);
	qtHelper::spinbox_set_value(spinBox_Chroma_Lag, format->chroma_lag);

	qtHelper::checkbox_set_checked(checkBox_Scanline, format->scanline);
	qtHelper::checkbox_set_checked(checkBox_Merge_Fields, format->merge_fields);
	qtHelper::checkbox_set_checked(checkBox_Vertical_Blend, format->vertical_blend);
}

void wdgPALLMP88959Filter::s_slider_spin_changed(int value) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	_pal_lmp88959_setup_t *format = &pal_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->brightness = value;
			break;
		case 1:
			format->saturation = value;
			break;
		case 2:
			format->contrast = value;
			break;
		case 3:
			format->black_point = value;
			break;
		case 4:
			format->white_point = value;
			break;
		case 5:
			format->noise = value;
			break;
		case 6:
			format->color_phase = value;
			break;
		case 7:
			format->chroma_lag = value;
			break;
	}
	if (((QObject *)sender())->objectName().contains("horizontalSlider_", Qt::CaseSensitive)) {
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(pal_lmp88959_parameters_desc[index]));

		qtHelper::spinbox_set_value((void *)sbox, value);
	} else {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(pal_lmp88959_parameters_desc[index]));

		qtHelper::slider_set_value((void *)slider, value);
	}
	pal_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALLMP88959Filter::s_checkbox_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	_pal_lmp88959_setup_t *format = &pal_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->scanline = state > 0;
			break;
		case 1:
			format->merge_fields = state > 0;
			break;
		case 2:
			format->vertical_blend = state > 0;
			break;
//		case 3:
//			format->chroma_correction = state > 0;
//			break;
	}
	pal_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALLMP88959Filter::s_default_value_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();

	gfx_thread_pause();
	pal_lmp88959_filter_parameter_default(index);
	gui_update_ntsc_widgets();
	pal_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALLMP88959Filter::s_default_value_smv_clicked(UNUSED(bool checked)) {
	gfx_thread_pause();
	pal_lmp88959_filter_parameter_smv_default();
	gui_update_ntsc_widgets();
	pal_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALLMP88959Filter::s_reset(UNUSED(bool checked)) {
	gfx_thread_pause();
	pal_lmp88959_filter_parameters_default();
	gui_update_ntsc_widgets();
	pal_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}

// wdgPALNESRGBLMP88959Filter -----------------------------------------------------------------------------------------

static const char pal_nesrgb_lmp88959_parameters_desc[][15] = {
	"Brightness" , "Saturation", "Contrast", "Black_Point", "White_Point", "Noise", "Chroma_Lag"
};

wdgPALNESRGBLMP88959Filter::wdgPALNESRGBLMP88959Filter(QWidget *parent) : QWidget(parent) {
	unsigned int i;

	setupUi(this);

	setFocusProxy(horizontalSlider_Brightness);

	for (i = 0; i < LENGTH(pal_nesrgb_lmp88959_parameters_desc); i++) {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(pal_nesrgb_lmp88959_parameters_desc[i]));
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(pal_nesrgb_lmp88959_parameters_desc[i]));
		QPushButton *btn = findChild<QPushButton *>("pushButton_" + QString(pal_nesrgb_lmp88959_parameters_desc[i]));

		slider->setProperty("myIndex", QVariant(i));
		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		sbox->setProperty("myIndex", QVariant(i));
		connect(sbox, SIGNAL(valueChanged(int)), this, SLOT(s_slider_spin_changed(int)));

		btn->setProperty("myIndex", QVariant(i));
		connect(btn, SIGNAL(clicked(bool)), this, SLOT(s_default_value_clicked(bool)));
	}

	checkBox_Scanline->setProperty("myIndex", QVariant(0));
	connect(checkBox_Scanline, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	checkBox_Vertical_Blend->setProperty("myIndex", QVariant(1));
	connect(checkBox_Vertical_Blend, SIGNAL(stateChanged(int)), this, SLOT(s_checkbox_changed(int)));

	connect(pushButton_Scanline_VBlend, SIGNAL(clicked(bool)), this, SLOT(s_default_value_sv_clicked(bool)));
	connect(pushButton_PAL_Parameters_reset, SIGNAL(clicked(bool)), this, SLOT(s_reset(bool)));
}
wdgPALNESRGBLMP88959Filter::~wdgPALNESRGBLMP88959Filter() = default;

void wdgPALNESRGBLMP88959Filter::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgPALNESRGBLMP88959Filter::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgPALNESRGBLMP88959Filter::update_widget(void) {
	bool enabled = cfg->filter == PAL_NESRGB_LMP88959;

	if (enabled) {
		set_sliders_spins();
	}
	setVisible(enabled);
}

void wdgPALNESRGBLMP88959Filter::pal_update_paramaters(void) {
	emu_thread_pause();
	pal_nesrgb_lmp88959_init();
	emu_thread_continue();
}
void wdgPALNESRGBLMP88959Filter::set_sliders_spins(void) {
	_pal_lmp88959_setup_t *format = &pal_nesrgb_lmp88959;

	qtHelper::slider_set_value(horizontalSlider_Brightness, format->brightness);
	qtHelper::slider_set_value(horizontalSlider_Saturation, format->saturation);
	qtHelper::slider_set_value(horizontalSlider_Contrast, format->contrast);
	qtHelper::slider_set_value(horizontalSlider_Black_Point, format->black_point);
	qtHelper::slider_set_value(horizontalSlider_White_Point, format->white_point);
	qtHelper::slider_set_value(horizontalSlider_Noise, format->noise);
	qtHelper::slider_set_value(horizontalSlider_Chroma_Lag, format->chroma_lag);

	qtHelper::spinbox_set_value(spinBox_Brightness, format->brightness);
	qtHelper::spinbox_set_value(spinBox_Saturation, format->saturation);
	qtHelper::spinbox_set_value(spinBox_Contrast, format->contrast);
	qtHelper::spinbox_set_value(spinBox_Black_Point, format->black_point);
	qtHelper::spinbox_set_value(spinBox_White_Point, format->white_point);
	qtHelper::spinbox_set_value(spinBox_Noise, format->noise);
	qtHelper::spinbox_set_value(spinBox_Chroma_Lag, format->chroma_lag);

	qtHelper::checkbox_set_checked(checkBox_Scanline, format->scanline);
	qtHelper::checkbox_set_checked(checkBox_Vertical_Blend, format->vertical_blend);
}

void wdgPALNESRGBLMP88959Filter::s_slider_spin_changed(int value) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	_pal_lmp88959_setup_t *format = &pal_nesrgb_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->brightness = value;
			break;
		case 1:
			format->saturation = value;
			break;
		case 2:
			format->contrast = value;
			break;
		case 3:
			format->black_point = value;
			break;
		case 4:
			format->white_point = value;
			break;
		case 5:
			format->noise = value;
			break;
		case 6:
			format->chroma_lag = value;
			break;
	}
	if (((QObject *)sender())->objectName().contains("horizontalSlider_", Qt::CaseSensitive)) {
		QSpinBox *sbox = findChild<QSpinBox *>("spinBox_" + QString(pal_nesrgb_lmp88959_parameters_desc[index]));

		qtHelper::spinbox_set_value((void *)sbox, value);
	} else {
		QSlider *slider = findChild<QSlider *>("horizontalSlider_" + QString(pal_nesrgb_lmp88959_parameters_desc[index]));

		qtHelper::slider_set_value((void *)slider, value);
	}
	pal_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALNESRGBLMP88959Filter::s_checkbox_changed(int state) {
	int index = QVariant(((QCheckBox *)sender())->property("myIndex")).toInt();
	_pal_lmp88959_setup_t *format = &pal_nesrgb_lmp88959;

	gfx_thread_pause();
	switch (index) {
		default:
		case 0:
			format->scanline = state > 0;
			break;
		case 1:
			format->vertical_blend = state > 0;
			break;
//		case 2:
//			format->chroma_correction = state > 0;
//			break;
	}
	pal_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALNESRGBLMP88959Filter::s_default_value_clicked(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();

	gfx_thread_pause();
	pal_nesrgb_lmp88959_filter_parameter_default(index);
	gui_update_ntsc_widgets();
	pal_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALNESRGBLMP88959Filter::s_default_value_sv_clicked(UNUSED(bool checked)) {
	gfx_thread_pause();
	pal_nesrgb_lmp88959_filter_parameter_smv_default();
	gui_update_ntsc_widgets();
	pal_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
void wdgPALNESRGBLMP88959Filter::s_reset(UNUSED(bool checked)) {
	gfx_thread_pause();
	pal_nesrgb_lmp88959_filter_parameters_default();
	gui_update_ntsc_widgets();
	pal_nesrgb_lmp88959_filter_parameters_changed();
	gfx_thread_continue();
}
