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

#include <QtWidgets/QFileDialog>
#include "wdgSettingsVideo.moc"
#include "wdgPaletteEditor.hpp"
#include "mainWindow.hpp"
#include "emu_thread.h"
#include "conf.h"
#include "clock.h"
#include "shaders.h"
#include "settings.h"

enum wdgSettingsVideo_shader_parameter_colums {
	WSV_SP_DESC,
	WSV_SP_SLIDER,
	WSV_SP_SPIN,
	WSV_SP_BUTTON,
	WSV_SP_COLUMNS
};

static QString styled_button(
"QPushButton {"
"   margin: 0; padding: 2px; border: 2px groove gray;"
"}"
"QPushButton:checked {"
"   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                              stop: 0 #aaabae, stop: 1 #f6f7fa);"
"}");
static QString title_bold(
"QGroupBox {"
"   font-weight: bold;"
"}");

wdgSettingsVideo::wdgSettingsVideo(QWidget *parent) : QWidget(parent) {
	vsync = cfg->vsync;

	setupUi(this);

	setFocusProxy(comboBox_Scale);

	groupBox_Oscan_pergame_setting->setStyleSheet(title_bold + styled_button);
	groupBox_Oscan_def_value->setStyleSheet(title_bold + styled_button);
	groupBox_Oscan_NTSC_brd->setStyleSheet(title_bold);
	groupBox_Oscan_PAL_brd->setStyleSheet(title_bold);
	groupBox_PAR->setStyleSheet("QGroupBox { font-weight: bold; }" + styled_button);
	groupBox_Screen_Rotation->setStyleSheet("QGroupBox { font-weight: bold; }" + styled_button);

	connect(comboBox_Scale, SIGNAL(activated(int)), this, SLOT(s_scale(int)));

	pushButton_Oscan_pergame_setting_on->setProperty("mtype", QVariant(OSCAN_ON));
	pushButton_Oscan_pergame_setting_off->setProperty("mtype", QVariant(OSCAN_OFF));
	pushButton_Oscan_pergame_setting_use_def->setProperty("mtype", QVariant(OSCAN_DEFAULT));

	connect(pushButton_Oscan_pergame_setting_on, SIGNAL(toggled(bool)), this, SLOT(s_oscan(bool)));
	connect(pushButton_Oscan_pergame_setting_off, SIGNAL(toggled(bool)), this, SLOT(s_oscan(bool)));
	connect(pushButton_Oscan_pergame_setting_use_def, SIGNAL(toggled(bool)), this, SLOT(s_oscan(bool)));

	pushButton_Oscan_def_value_on->setProperty("mtype", QVariant(OSCAN_ON));
	pushButton_Oscan_def_value_off->setProperty("mtype", QVariant(OSCAN_OFF));

	connect(pushButton_Oscan_def_value_on, SIGNAL(toggled(bool)), this, SLOT(s_oscan_def_value(bool)));
	connect(pushButton_Oscan_def_value_off, SIGNAL(toggled(bool)), this, SLOT(s_oscan_def_value(bool)));

	{
		spinBox_Oscan_NTSC_brd_up->setProperty("mtype", QVariant(0));
		spinBox_Oscan_NTSC_brd_down->setProperty("mtype", QVariant(0));
		spinBox_Oscan_NTSC_brd_left->setProperty("mtype", QVariant(0));
		spinBox_Oscan_NTSC_brd_right->setProperty("mtype", QVariant(0));

		spinBox_Oscan_NTSC_brd_up->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_NTSC_brd_down->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_NTSC_brd_left->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_NTSC_brd_right->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);

		connect(spinBox_Oscan_NTSC_brd_up, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_NTSC_brd_down, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_NTSC_brd_left, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_NTSC_brd_right, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));

		pushButton_Oscan_NTSC_brd_reset->setProperty("mtype", QVariant(0));

		connect(pushButton_Oscan_NTSC_brd_reset, SIGNAL(clicked(bool)), this, SLOT(s_oscan_reset(bool)));

		spinBox_Oscan_PAL_brd_up->setProperty("mtype", QVariant(1));
		spinBox_Oscan_PAL_brd_down->setProperty("mtype", QVariant(1));
		spinBox_Oscan_PAL_brd_left->setProperty("mtype", QVariant(1));
		spinBox_Oscan_PAL_brd_right->setProperty("mtype", QVariant(1));

		spinBox_Oscan_PAL_brd_up->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_PAL_brd_down->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_PAL_brd_left->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_PAL_brd_right->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);

		connect(spinBox_Oscan_PAL_brd_up, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_PAL_brd_down, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_PAL_brd_left, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_PAL_brd_right, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));

		pushButton_Oscan_PAL_brd_reset->setProperty("mtype", QVariant(1));

		connect(pushButton_Oscan_PAL_brd_reset, SIGNAL(clicked(bool)), this, SLOT(s_oscan_reset(bool)));
	}
	connect(checkBox_Oscan_brd_black_window, SIGNAL(clicked(bool)), this, SLOT(s_oscan_brd_black_w(bool)));
	connect(checkBox_Oscan_brd_black_flscreen, SIGNAL(clicked(bool)), this, SLOT(s_oscan_brd_black_f(bool)));

	pushButton_PAR_11->setProperty("mtype", QVariant(PAR11));
	pushButton_PAR_54->setProperty("mtype", QVariant(PAR54));
	pushButton_PAR_87->setProperty("mtype", QVariant(PAR87));
	pushButton_PAR_118->setProperty("mtype", QVariant(PAR118));

	connect(pushButton_PAR_11, SIGNAL(toggled(bool)), this, SLOT(s_par(bool)));
	connect(pushButton_PAR_54, SIGNAL(toggled(bool)), this, SLOT(s_par(bool)));
	connect(pushButton_PAR_87, SIGNAL(toggled(bool)), this, SLOT(s_par(bool)));
	connect(pushButton_PAR_118, SIGNAL(toggled(bool)), this, SLOT(s_par(bool)));
	connect(checkBox_PAR_Soft_Stretch, SIGNAL(clicked(bool)), this, SLOT(s_par_stretch(bool)));

	pushButton_Screen_Rotation_0d->setProperty("mtype", QVariant(ROTATE_0));
	pushButton_Screen_Rotation_90d->setProperty("mtype", QVariant(ROTATE_90));
	pushButton_Screen_Rotation_180d->setProperty("mtype", QVariant(ROTATE_180));
	pushButton_Screen_Rotation_270d->setProperty("mtype", QVariant(ROTATE_270));

	connect(pushButton_Screen_Rotation_0d, SIGNAL(toggled(bool)), this, SLOT(s_screen_rotation(bool)));
	connect(pushButton_Screen_Rotation_90d, SIGNAL(toggled(bool)), this, SLOT(s_screen_rotation(bool)));
	connect(pushButton_Screen_Rotation_180d, SIGNAL(toggled(bool)), this, SLOT(s_screen_rotation(bool)));
	connect(pushButton_Screen_Rotation_270d, SIGNAL(toggled(bool)), this, SLOT(s_screen_rotation(bool)));
	connect(checkBox_Horizontal_Flip_Screen, SIGNAL(clicked(bool)), this, SLOT(s_horizontal_flip_screen(bool)));
	connect(checkBox_Input_Rotation, SIGNAL(clicked(bool)), this, SLOT(s_input_rotation(bool)));
	connect(checkBox_Text_Rotation, SIGNAL(clicked(bool)), this, SLOT(s_text_rotation(bool)));

	connect(comboBox_Software_Filters, SIGNAL(activated(int)), this, SLOT(s_sfilter(int)));
	connect(comboBox_Shaders, SIGNAL(activated(int)), this, SLOT(s_shader(int)));
	connect(pushButton_Shaders_file, SIGNAL(clicked(bool)), this, SLOT(s_shader_file(bool)));
	connect(pushButton_Shaders_file_clear, SIGNAL(clicked(bool)), this, SLOT(s_shader_file_clear(bool)));
	{
		int i;

		tableWidget_Shader_Parameters->setColumnCount(0);
		tableWidget_Shader_Parameters->setColumnCount(WSV_SP_COLUMNS);

		for (i = 0; i < WSV_SP_COLUMNS; i++) {
			QTableWidgetItem *item = new QTableWidgetItem();

			tableWidget_Shader_Parameters->setHorizontalHeaderItem(i, item);
		}

		shdp_brush.fg = tableWidget_Shader_Parameters->horizontalHeaderItem(0)->foreground();
		shdp_brush.bg = tableWidget_Shader_Parameters->horizontalHeaderItem(0)->background();

		connect(pushButton_Shader_Parameters_reset_alls, SIGNAL(clicked(bool)), this, SLOT(s_shader_param_all_defaults(bool)));
	}
#if defined (WITH_OPENGL)
	connect(checkBox_Disable_sRGB_FBO, SIGNAL(clicked(bool)), this, SLOT(s_disable_srgb_fbo(bool)));
#endif

	connect(comboBox_Palette, SIGNAL(activated(int)), this, SLOT(s_palette(int)));
	connect(pushButton_Palette_file, SIGNAL(clicked(bool)), this, SLOT(s_palette_file(bool)));
	connect(pushButton_Palette_file_clear, SIGNAL(clicked(bool)), this, SLOT(s_palette_file_clear(bool)));
	connect(checkBox_Disable_emphasis_swap_PAL, SIGNAL(clicked(bool)), this, SLOT(s_disable_emphasis_swap_pal(bool)));

	connect(checkBox_Vsync, SIGNAL(clicked(bool)), this, SLOT(s_vsync(bool)));
	connect(checkBox_Interpolation, SIGNAL(clicked(bool)), this, SLOT(s_interpolation(bool)));
	connect(checkBox_Text_on_screen, SIGNAL(clicked(bool)), this, SLOT(s_text_on_screen(bool)));
	connect(checkBox_Show_FPS, SIGNAL(clicked(bool)), this, SLOT(s_show_fps(bool)));
	connect(checkBox_Input_display, SIGNAL(clicked(bool)), this, SLOT(s_input_display(bool)));
	connect(checkBox_Disable_TV_noise_emulation, SIGNAL(clicked(bool)), this, SLOT(s_disable_tv_noise(bool)));
	connect(checkBox_Disable_sepia_color_on_pause, SIGNAL(clicked(bool)), this, SLOT(s_disable_sepia(bool)));
	connect(checkBox_Fullscreen_in_window, SIGNAL(clicked(bool)), this, SLOT(s_fullscreen_in_window(bool)));
	connect(checkBox_Use_integer_scaling_in_fullscreen, SIGNAL(clicked(bool)), this, SLOT(s_integer_in_fullscreen(bool)));
	connect(checkBox_Stretch_in_fullscreen, SIGNAL(clicked(bool)), this, SLOT(s_stretch_in_fullscreen(bool)));

	tabWidget_Video->setCurrentIndex(0);
}
wdgSettingsVideo::~wdgSettingsVideo() {}

void wdgSettingsVideo::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgSettingsVideo::showEvent(UNUSED(QShowEvent *event)) {
	int dim = label_Scale->size().height() - 10;

	icon_Scale->setPixmap(QIcon(":/icon/icons/scale.svg").pixmap(dim, dim));
}

void wdgSettingsVideo::retranslateUi(QWidget *wdgSettingsVideo) {
	Ui::wdgSettingsVideo::retranslateUi(wdgSettingsVideo);
	mainwin->qaction_shcut.scale_1x->setText(comboBox_Scale->itemText(0));
	mainwin->qaction_shcut.scale_2x->setText(comboBox_Scale->itemText(1));
	mainwin->qaction_shcut.scale_3x->setText(comboBox_Scale->itemText(2));
	mainwin->qaction_shcut.scale_4x->setText(comboBox_Scale->itemText(3));
	mainwin->qaction_shcut.scale_5x->setText(comboBox_Scale->itemText(4));
	mainwin->qaction_shcut.scale_6x->setText(comboBox_Scale->itemText(5));
	mainwin->qaction_shcut.interpolation->setText(checkBox_Interpolation->text());
	mainwin->qaction_shcut.integer_in_fullscreen->setText(checkBox_Use_integer_scaling_in_fullscreen->text());
	mainwin->qaction_shcut.stretch_in_fullscreen->setText(checkBox_Stretch_in_fullscreen->text());
	update_widget();
}
void wdgSettingsVideo::update_widget(void) {
	{
		oscan_set();
		oscan_def_value_set();
		oscan_brd_set();

		checkBox_Oscan_brd_black_window->setChecked(cfg->oscan_black_borders);
		checkBox_Oscan_brd_black_flscreen->setChecked(cfg->oscan_black_borders_fscr);
	}

	{
#if defined (WITH_OPENGL)
		checkBox_PAR_Soft_Stretch->setText(tr("GLSL &soft stretch"));
#elif defined (WITH_D3D9)
		checkBox_PAR_Soft_Stretch->setText(tr("HLSL &soft stretch"));
#endif
		par_set();

		if (cfg->PAR_soft_stretch == TRUE) {
			checkBox_PAR_Soft_Stretch->setChecked(true);
		} else {
			checkBox_PAR_Soft_Stretch->setChecked(false);
		}

		if (cfg->filter == NTSC_FILTER) {
			widget_Oscan->setEnabled(false);
		} else {
			widget_Oscan->setEnabled(true);
			if (cfg->pixel_aspect_ratio != PAR11) {
				checkBox_PAR_Soft_Stretch->setEnabled(true);
			} else {
				checkBox_PAR_Soft_Stretch->setEnabled(false);
			}
		}
	}

	{
		srotation_set();
		checkBox_Horizontal_Flip_Screen->setChecked(cfg->hflip_screen);
		checkBox_Input_Rotation->setChecked(cfg->input_rotation);
		checkBox_Text_Rotation->setChecked(cfg->text_rotation);
	}

	{
		sfilter_set();
		shader_set();
#if defined (WITH_OPENGL)
		checkBox_Disable_sRGB_FBO->setChecked(cfg->disable_srgb_fbo);
#else
		label_Filters_misc->setVisible(false);
		line_Filters_misc->setVisible(false);
		checkBox_Disable_sRGB_FBO->setVisible(false);
#endif
	}

	{
		palette_set();
		checkBox_Disable_emphasis_swap_PAL->setChecked(cfg->disable_swap_emphasis_pal);
	}

	checkBox_Vsync->setChecked(cfg->vsync);
	if (cfg->vsync != vsync) {
		label_Vsync->setVisible(true);
	} else {
		label_Vsync->setVisible(false);
	}

	checkBox_Interpolation->setChecked(cfg->interpolation);
	checkBox_Text_on_screen->setChecked(cfg->txt_on_screen);
	checkBox_Show_FPS->setChecked(cfg->show_fps);
	checkBox_Input_display->setChecked(cfg->input_display);
	checkBox_Disable_TV_noise_emulation->setChecked(cfg->disable_tv_noise);
	checkBox_Disable_sepia_color_on_pause->setChecked(cfg->disable_sepia_color);
	checkBox_Fullscreen_in_window->setChecked(cfg->fullscreen_in_window);
	checkBox_Use_integer_scaling_in_fullscreen->setChecked(cfg->integer_scaling);
	checkBox_Stretch_in_fullscreen->setChecked(cfg->stretch);
}
void wdgSettingsVideo::change_rom(void) {
	update_widget();
}

void wdgSettingsVideo::scale_set(void) {
	comboBox_Scale->setCurrentIndex(cfg->scale - 1);
}
void wdgSettingsVideo::oscan_set(void) {
	pushbutton_set_checked(pushButton_Oscan_pergame_setting_on, false);
	pushbutton_set_checked(pushButton_Oscan_pergame_setting_off, false);
	pushbutton_set_checked(pushButton_Oscan_pergame_setting_use_def, false);
	switch (cfg->oscan) {
		case OSCAN_ON:
			pushbutton_set_checked(pushButton_Oscan_pergame_setting_on, true);
			break;
		case OSCAN_OFF:
			pushbutton_set_checked(pushButton_Oscan_pergame_setting_off, true);
			break;
		case OSCAN_DEFAULT:
			pushbutton_set_checked(pushButton_Oscan_pergame_setting_use_def, true);
			break;
	}
}
void wdgSettingsVideo::oscan_def_value_set(void) {
	pushbutton_set_checked(pushButton_Oscan_def_value_on, false);
	pushbutton_set_checked(pushButton_Oscan_def_value_off, false);
	if (cfg->oscan_default == OSCAN_ON) {
		pushbutton_set_checked(pushButton_Oscan_def_value_on, true);
	} else {
		pushbutton_set_checked(pushButton_Oscan_def_value_off, true);
	}
}
void wdgSettingsVideo::oscan_brd_set(void) {
	_overscan_borders *borders;

	borders = &overscan_borders[0];
	spinbox_set_value(spinBox_Oscan_NTSC_brd_up, borders->up);
	spinbox_set_value(spinBox_Oscan_NTSC_brd_down, borders->down);
	spinbox_set_value(spinBox_Oscan_NTSC_brd_left, borders->left);
	spinbox_set_value(spinBox_Oscan_NTSC_brd_right, borders->right);

	borders = &overscan_borders[1];
	spinbox_set_value(spinBox_Oscan_PAL_brd_up, borders->up);
	spinbox_set_value(spinBox_Oscan_PAL_brd_down, borders->down);
	spinbox_set_value(spinBox_Oscan_PAL_brd_left, borders->left);
	spinbox_set_value(spinBox_Oscan_PAL_brd_right, borders->right);
}
void wdgSettingsVideo::par_set(void) {
	pushbutton_set_checked(pushButton_PAR_11, false);
	pushbutton_set_checked(pushButton_PAR_54, false);
	pushbutton_set_checked(pushButton_PAR_87, false);
	pushbutton_set_checked(pushButton_PAR_118, false);
	if (cfg->pixel_aspect_ratio == PAR11) {
		pushbutton_set_checked(pushButton_PAR_11, true);
	} else if (cfg->pixel_aspect_ratio == PAR54) {
		pushbutton_set_checked(pushButton_PAR_54, true);
	} else if (cfg->pixel_aspect_ratio == PAR87) {
		pushbutton_set_checked(pushButton_PAR_87, true);
	} else {
		pushbutton_set_checked(pushButton_PAR_118, true);
	}
}
void wdgSettingsVideo::srotation_set(void) {
	pushbutton_set_checked(pushButton_Screen_Rotation_0d, false);
	pushbutton_set_checked(pushButton_Screen_Rotation_90d, false);
	pushbutton_set_checked(pushButton_Screen_Rotation_180d, false);
	pushbutton_set_checked(pushButton_Screen_Rotation_270d, false);
	if (cfg->screen_rotation == ROTATE_0) {
		pushbutton_set_checked(pushButton_Screen_Rotation_0d, true);
	} else if (cfg->screen_rotation == ROTATE_90) {
		pushbutton_set_checked(pushButton_Screen_Rotation_90d, true);
	} else if (cfg->screen_rotation == ROTATE_180) {
		pushbutton_set_checked(pushButton_Screen_Rotation_180d, true);
	} else {
		pushbutton_set_checked(pushButton_Screen_Rotation_270d, true);
	}
}
void wdgSettingsVideo::sfilter_set(void) {
	int filter = 0;

	switch (cfg->filter) {
		case NO_FILTER:
			filter = 0;
			break;
		case SCALE2X:
			filter = 1;
			break;
		case SCALE3X:
			filter = 2;
			break;
		case SCALE4X:
			filter = 3;
			break;
		case HQ2X:
			filter = 4;
			break;
		case HQ3X:
			filter = 5;
			break;
		case HQ4X:
			filter = 6;
			break;
		case XBRZ2X:
			filter = 7;
			break;
		case XBRZ3X:
			filter = 8;
			break;
		case XBRZ4X:
			filter = 9;
			break;
		case XBRZ5X:
			filter = 10;
			break;
		case XBRZ6X:
			filter = 11;
			break;
		case XBRZ2XMT:
			filter = 12;
			break;
		case XBRZ3XMT:
			filter = 13;
			break;
		case XBRZ4XMT:
			filter = 14;
			break;
		case XBRZ5XMT:
			filter = 15;
			break;
		case XBRZ6XMT:
			filter = 16;
			break;
		case NTSC_FILTER: {
			switch (cfg->ntsc_format) {
				case COMPOSITE:
					filter = 17;
					break;
				case SVIDEO:
					filter = 18;
					break;
				case RGBMODE:
					filter = 19;
					break;
			}
			break;
		}
	}

	comboBox_Software_Filters->setCurrentIndex(filter);
}
void wdgSettingsVideo::shader_set(void) {
	int shader = 0;

	if (ustrlen(cfg->shader_file) != 0) {
		lineEdit_Shaders_file->setEnabled(true);
		lineEdit_Shaders_file->setText(QFileInfo(uQString(cfg->shader_file)).baseName());
	} else {
		lineEdit_Shaders_file->setEnabled(false);
		lineEdit_Shaders_file->setText(tr("[Select a file]"));
	}

	switch (cfg->shader) {
		case NO_SHADER:
			shader = 0;
			break;
		case SHADER_CRTDOTMASK:
			shader = 1;
			break;
		case SHADER_CRTSCANLINES:
			shader = 2;
			break;
		case SHADER_CRTWITHCURVE:
			shader = 3;
			break;
		case SHADER_EMBOSS:
			shader = 4;
			break;
		case SHADER_NOISE:
			shader = 5;
			break;
		case SHADER_NTSC2PHASECOMPOSITE:
			shader = 6;
			break;
		case SHADER_OLDTV:
			shader = 7;
			break;
		case SHADER_FILE:
			shader = 8;
			break;
	}

	comboBox_Shaders->setCurrentIndex(shader);
	shader_param_set();
}
void wdgSettingsVideo::shader_param_set(void) {
	int i, row = 0;

	tableWidget_Shader_Parameters->setRowCount(0);

	for (i = 0; i < shader_effect.params; i++) {
		_param_shd *pshd = &shader_effect.param[i];
		QTableWidgetItem *col;

		if (!pshd->desc[0]) {
			continue;
		}

		tableWidget_Shader_Parameters->insertRow(row);

		col = new QTableWidgetItem();
		col->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
		col->setText(QString(pshd->desc));
		tableWidget_Shader_Parameters->setItem(row, 0, col);
		tableWidget_Shader_Parameters->resizeColumnToContents(0);

		if (pshd->value != pshd->initial) {
			col->setBackground(Qt::yellow);
		}

		{
			QWidget *widget = new QWidget(this);
			QHBoxLayout* layout = new QHBoxLayout(widget);
			QSlider *slider = new QSlider(widget);
			double steps = (pshd->max -pshd->min) / pshd->step;

			widget->setObjectName("widget_slider");
			slider->setObjectName("slider");
			slider->setOrientation(Qt::Horizontal);
			slider->setProperty("myIndex", QVariant(i));
			slider->setProperty("myValue", QVariant(row));
			slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			slider->setRange(0, steps);
			slider->setSingleStep(1);
			slider->setValue((steps / (pshd->max - pshd->min)) * (pshd->value - pshd->min));
			connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_shader_param_slider(int)));
			layout->addWidget(slider);
			layout->setAlignment(Qt::AlignCenter);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(0);
			tableWidget_Shader_Parameters->setCellWidget(row, WSV_SP_SLIDER, widget);
		}

		{
			QWidget *widget = new QWidget(this);
			QHBoxLayout* layout = new QHBoxLayout(widget);
			QDoubleSpinBox *spin = new QDoubleSpinBox(widget);

			widget->setObjectName("widget_spin");
			spin->setObjectName("spin");
			spin->setButtonSymbols(QAbstractSpinBox::PlusMinus);
			spin->setProperty("myIndex", QVariant(i));
			spin->setProperty("myValue", QVariant(row));
			spin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			spin->setAlignment(Qt::AlignRight);
			spin->setDecimals(6);
			spin->setRange(pshd->min, pshd->max);
			spin->setSingleStep(pshd->step);
			spin->setValue((double)(pshd->value));
			connect(spin, SIGNAL(valueChanged(double)), this, SLOT(s_shader_param_spin(double)));
			layout->addWidget(spin);
			layout->setAlignment(Qt::AlignCenter);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(0);
			tableWidget_Shader_Parameters->setCellWidget(row, WSV_SP_SPIN, widget);
			tableWidget_Shader_Parameters->resizeColumnToContents(WSV_SP_SPIN);
		}

		{
			QWidget *widget = new QWidget(this);
			QHBoxLayout* layout = new QHBoxLayout(widget);
			QPushButton *def = new QPushButton(widget) ;

			widget->setObjectName("widget_button");
			def->setObjectName("default");
			def->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			def->setIcon(QIcon(":/icon/icons/default.svg"));
			def->setToolTip(tr("Default"));
			def->setProperty("myIndex", QVariant(i));
			def->setProperty("myValue", QVariant(row));
			connect(def, SIGNAL(clicked(bool)), this, SLOT(s_shader_param_default(bool)));
			layout->addWidget(def);
			layout->setAlignment(Qt::AlignCenter);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(0);
			tableWidget_Shader_Parameters->setCellWidget(row, WSV_SP_BUTTON, widget);
			tableWidget_Shader_Parameters->resizeColumnToContents(WSV_SP_BUTTON);
		}

		row++;
	}

	if (row == 0) {
		pushButton_Shader_Parameters_reset_alls->setEnabled(false);
	} else {
		pushButton_Shader_Parameters_reset_alls->setEnabled(true);
	}
}
void wdgSettingsVideo::palette_set(void) {
	int palette = 0;

	if (ustrlen(cfg->palette_file) != 0) {
		lineEdit_Palette_file->setEnabled(true);
		lineEdit_Palette_file->setText(QFileInfo(uQString(cfg->palette_file)).baseName());
	} else {
		lineEdit_Palette_file->setEnabled(false);
		lineEdit_Palette_file->setText(tr("[Select a file]"));
	}

	switch (cfg->palette) {
		case PALETTE_PAL:
			palette = 0;
			break;
		case PALETTE_NTSC:
			palette = 1;
			break;
		case PALETTE_SONY:
			palette = 2;
			break;
		case PALETTE_FRBX_NOSTALGIA:
			palette = 3;
			break;
		case PALETTE_FRBX_YUV:
			palette = 4;
			break;
		case PALETTE_MONO:
			palette = 5;
			break;
		case PALETTE_GREEN:
			palette = 6;
			break;
		case PALETTE_RAW:
			palette = 7;
			break;
		case PALETTE_FILE:
			palette = 8;
			break;
	}

	comboBox_Palette->setCurrentIndex(palette);
}
bool wdgSettingsVideo::call_gfx_set_screen(int mtype) {
	if (mtype == 0) {
		if (machine.type == NTSC) {
			return (true);
		}
	} else if (machine.type != NTSC) {
		return (true);
	}
	return (false);
}
void wdgSettingsVideo::spinbox_set_value(QSpinBox *spn, int value) {
	spn->blockSignals(true);
	spn->setValue(value);
	spn->blockSignals(false);
}
void wdgSettingsVideo::pushbutton_set_checked(QPushButton *btn, bool mode) {
	btn->blockSignals(true);
	btn->setChecked(mode);
	btn->blockSignals(false);
}

void wdgSettingsVideo::s_scale(int index) {
	int scale = index + 1;

	if (cfg->fullscreen) {
		return;
	}

	emu_thread_pause();
	gfx_set_screen(scale, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_par(bool checked) {
	if (checked) {
		int par = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (cfg->pixel_aspect_ratio == par) {
			return;
		}

		emu_thread_pause();
		cfg->pixel_aspect_ratio = par;
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
		emu_thread_continue();
	}
	update_widget();
}
void wdgSettingsVideo::s_par_stretch(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->PAR_soft_stretch = !cfg->PAR_soft_stretch;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan(bool checked) {
	if (checked) {
		emu_thread_pause();
		cfg->oscan = QVariant(((QPushButton *)sender())->property("mtype")).toInt();
		settings_pgs_save();
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
		emu_thread_continue();
	}
	oscan_set();
}
void wdgSettingsVideo::s_oscan_def_value(bool checked) {
	if (checked) {
		emu_thread_pause();
		cfg->oscan_default = QVariant(((QPushButton *)sender())->property("mtype")).toInt();
		if (cfg->oscan == OSCAN_DEFAULT) {
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
		}
		emu_thread_continue();
	}
	oscan_def_value_set();
}
void wdgSettingsVideo::s_oscan_brd_black_w(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->oscan_black_borders = !cfg->oscan_black_borders;
	if (overscan.enabled) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_brd_black_f(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->oscan_black_borders_fscr = !cfg->oscan_black_borders_fscr;
	if (overscan.enabled) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_spinbox(int i) {
	int mtype = QVariant(((QPushButton *)sender())->property("mtype")).toInt();
	QString name = ((QSpinBox *)sender())->objectName();
	_overscan_borders *borders;

	borders = &overscan_borders[mtype];

	if (name.contains("brd_up") == true) {
		borders->up = i;
	} else if (name.contains("brd_down") == true) {
		borders->down = i;
	} else if (name.contains("brd_left") == true) {
		borders->left = i;
	} else if (name.contains("brd_right") == true) {
		borders->right = i;
	}

	emu_thread_pause();
	if (call_gfx_set_screen(mtype)) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_reset(UNUSED(bool checked)) {
	int mtype = QVariant(((QPushButton *)sender())->property("mtype")).toInt();
	_overscan_borders *borders;

	borders = &overscan_borders[mtype];

	emu_thread_pause();
	settings_set_overscan_default(borders, mtype + NTSC);
	oscan_brd_set();
	if (call_gfx_set_screen(mtype)) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_sfilter(int index) {
	int filter = index;

	switch (filter) {
		default:
		case 0:
			filter = NO_FILTER;
			break;
		case 1:
			filter = SCALE2X;
			break;
		case 2:
			filter = SCALE3X;
			break;
		case 3:
			filter = SCALE4X;
			break;
		case 4:
			filter = HQ2X;
			break;
		case 5:
			filter = HQ3X;
			break;
		case 6:
			filter = HQ4X;
			break;
		case 7:
			filter = XBRZ2X;
			break;
		case 8:
			filter = XBRZ3X;
			break;
		case 9:
			filter = XBRZ4X;
			break;
		case 10:
			filter = XBRZ5X;
			break;
		case 11:
			filter = XBRZ6X;
			break;
		case 12:
			filter = XBRZ2XMT;
			break;
		case 13:
			filter = XBRZ3XMT;
			break;
		case 14:
			filter = XBRZ4XMT;
			break;
		case 15:
			filter = XBRZ5XMT;
			break;
		case 16:
			filter = XBRZ6XMT;
			break;
		case 17:
			filter = NTSC_FILTER;
			cfg->ntsc_format = COMPOSITE;
			break;
		case 18:
			filter = NTSC_FILTER;
			cfg->ntsc_format = SVIDEO;
			break;
		case 19:
			filter = NTSC_FILTER;
			cfg->ntsc_format = RGBMODE;
			break;
	}

	emu_thread_pause();
	gfx_set_screen(NO_CHANGE, filter, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	if (cfg->filter == NTSC_FILTER) {
		ntsc_set(NULL, cfg->ntsc_format, 0, 0, (BYTE *)palette_RGB.noswap, 0);
		ntsc_set(NULL, cfg->ntsc_format, 0, 0, (BYTE *)palette_RGB.swapped, 0);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_shader(int index) {
	int shader = index;

	switch (shader) {
		default:
		case 0:
			shader = NO_SHADER;
			break;
		case 1:
			shader = SHADER_CRTDOTMASK;
			break;
		case 2:
			shader = SHADER_CRTSCANLINES;
			break;
		case 3:
			shader = SHADER_CRTWITHCURVE;
			break;
		case 4:
			shader = SHADER_EMBOSS;
			break;
		case 5:
			shader = SHADER_NOISE;
			break;
		case 6:
			shader = SHADER_NTSC2PHASECOMPOSITE;
			break;
		case 7:
			shader = SHADER_OLDTV;
			break;
		case 8:
			shader = SHADER_FILE;
			break;
	}

	emu_thread_pause();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, shader, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	shader_param_set();
	emu_thread_continue();
}
#if defined (WITH_OPENGL)
void wdgSettingsVideo::s_disable_srgb_fbo(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->disable_srgb_fbo = !cfg->disable_srgb_fbo;
	if (info.sRGB_FBO_in_use == TRUE) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
#endif
void wdgSettingsVideo::s_shader_file(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Shaders files"));
	filters.append(tr("All files"));

#if defined (WITH_OPENGL_CG)
	filters[0].append(" (*.cgp *.glslp)");
#elif defined (WITH_OPENGL)
	filters[0].append(" (*.glslp)");
#elif defined (WITH_D3D9)
	filters[0].append(" (*.cgp)");
#endif
	filters[1].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open Shader file"),
		QFileInfo(uQString(cfg->shader_file)).dir().absolutePath(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->shader_file, 0x00, usizeof(cfg->shader_file));
			ustrncpy(cfg->shader_file, uQStringCD(fileinfo.absoluteFilePath()), usizeof(cfg->shader_file) - 1);
			emu_thread_pause();
			gfx_set_screen(NO_CHANGE, NO_CHANGE, SHADER_FILE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
			emu_thread_continue();
		} else {
			gui_overlay_info_append_msg_precompiled(25, NULL);
		}
	}

	update_widget();

	emu_pause(FALSE);
}
void wdgSettingsVideo::s_shader_file_clear(UNUSED(bool checked)) {
	umemset(cfg->shader_file, 0x00, usizeof(cfg->shader_file));
	shader_set();
}
void wdgSettingsVideo::s_shader_param_slider(int value) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();
	_param_shd *pshd = &shader_effect.param[index];
	float remain = pshd->initial - (pshd->step * (float)((int)(pshd->initial / pshd->step)));
	float fvalue = pshd->min + ((pshd->step * (float)value) + remain);

	tableWidget_Shader_Parameters->cellWidget(row, WSV_SP_SPIN)->findChild<QDoubleSpinBox *>("spin")->setValue((double)fvalue);
}
void wdgSettingsVideo::s_shader_param_spin(double d) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();
	_param_shd *pshd = &shader_effect.param[index];
	QSlider *slider = tableWidget_Shader_Parameters->cellWidget(row, WSV_SP_SLIDER)->findChild<QSlider *>("slider");

	pshd->value = (float)d;
	slider->blockSignals(true);
	slider->setValue(((float)slider->maximum() / (pshd->max - pshd->min)) * (pshd->value - pshd->min));
	slider->blockSignals(false);

	if (pshd->value == pshd->initial) {
		tableWidget_Shader_Parameters->item(row, WSV_SP_DESC)->setForeground(shdp_brush.fg);
		tableWidget_Shader_Parameters->item(row, WSV_SP_DESC)->setBackground(shdp_brush.bg);
	} else {
		tableWidget_Shader_Parameters->item(row, WSV_SP_DESC)->setBackground(Qt::yellow);
	}
}
void wdgSettingsVideo::s_shader_param_default(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myIndex")).toInt();
	int row = QVariant(((QObject *)sender())->property("myValue")).toInt();
	_param_shd *pshd = &shader_effect.param[index];

	tableWidget_Shader_Parameters->cellWidget(row, WSV_SP_SPIN)->findChild<QDoubleSpinBox *>("spin")->setValue(pshd->initial);
}
void wdgSettingsVideo::s_shader_param_all_defaults(UNUSED(bool checked)) {
	int i, row = 0;

	for (i = 0; i < shader_effect.params; i++) {
		_param_shd *pshd = &shader_effect.param[i];

		if (!pshd->desc[0]) {
			continue;
		}

		tableWidget_Shader_Parameters->cellWidget(row, WSV_SP_SPIN)->findChild<QDoubleSpinBox *>("spin")->setValue(pshd->initial);
		row++;
	}
}
void wdgSettingsVideo::s_palette(int index) {
	int palette = index;

	switch (palette) {
		default:
		case 0:
			palette = PALETTE_PAL;
			break;
		case 1:
			palette = PALETTE_NTSC;
			break;
		case 2:
			palette = PALETTE_SONY;
			break;
		case 3:
			palette = PALETTE_FRBX_NOSTALGIA;
			break;
		case 4:
			palette = PALETTE_FRBX_YUV;
			break;
		case 5:
			palette = PALETTE_MONO;
			break;
		case 6:
			palette = PALETTE_GREEN;
			break;
		case 7:
			palette = PALETTE_RAW;
			break;
		case 8:
			palette = PALETTE_FILE;
			break;
	}

	emu_thread_pause();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, palette, FALSE, FALSE);
	widget_Palette_Editor->palette_changed();
	emu_thread_continue();
}
void wdgSettingsVideo::s_palette_file(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Palette files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.pal *.PAL)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open palette file"),
		QFileInfo(uQString(cfg->palette_file)).dir().absolutePath(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->palette_file, 0x00, usizeof(cfg->palette_file));
			ustrncpy(cfg->palette_file, uQStringCD(fileinfo.absoluteFilePath()), usizeof(cfg->palette_file) - 1);
			emu_thread_pause();
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTE_FILE, FALSE, TRUE);
			widget_Palette_Editor->palette_changed();
			emu_thread_continue();
		} else {
			gui_overlay_info_append_msg_precompiled(26, NULL);
		}
	}

	emu_pause(FALSE);
}
void wdgSettingsVideo::s_palette_file_clear(UNUSED(bool checked)) {
	umemset(cfg->palette_file, 0x00, usizeof(cfg->palette_file));
	palette_set();
}
void wdgSettingsVideo::s_disable_emphasis_swap_pal(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->disable_swap_emphasis_pal = !cfg->disable_swap_emphasis_pal;
	gfx_palette_update();
	emu_thread_continue();
}
void wdgSettingsVideo::s_vsync(UNUSED(bool checked)) {
	cfg->vsync = !cfg->vsync;
	update_widget();
}
void wdgSettingsVideo::s_interpolation(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->interpolation = !cfg->interpolation;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_text_on_screen(UNUSED(bool checked)) {
	cfg->txt_on_screen = !cfg->txt_on_screen;
	gui_overlay_update();
}
void wdgSettingsVideo::s_show_fps(UNUSED(bool checked)) {
	cfg->show_fps = !cfg->show_fps;
	gui_overlay_update();
}
void wdgSettingsVideo::s_input_display(UNUSED(bool checked)) {
	cfg->input_display = !cfg->input_display;
	gui_overlay_update();
}
void wdgSettingsVideo::s_disable_tv_noise(UNUSED(bool checked)) {
	cfg->disable_tv_noise = !cfg->disable_tv_noise;
}
void wdgSettingsVideo::s_disable_sepia(UNUSED(bool checked)) {
	cfg->disable_sepia_color = !cfg->disable_sepia_color;
}
void wdgSettingsVideo::s_fullscreen_in_window(UNUSED(bool checked)) {
	cfg->fullscreen_in_window = !cfg->fullscreen_in_window;
}
void wdgSettingsVideo::s_integer_in_fullscreen(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->integer_scaling = !cfg->integer_scaling;

	if (cfg->fullscreen == FULLSCR) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_stretch_in_fullscreen(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->stretch = !cfg->stretch;

	if (cfg->fullscreen == FULLSCR) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_screen_rotation(bool checked) {
	if (checked) {
		int rotation = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		if (rotation == cfg->screen_rotation) {
			return;
		}

		emu_thread_pause();
		cfg->screen_rotation = rotation;
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
		emu_thread_continue();
	}
	srotation_set();
}
void wdgSettingsVideo::s_horizontal_flip_screen(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->hflip_screen = !cfg->hflip_screen;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_input_rotation(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->input_rotation = !cfg->input_rotation;
	emu_thread_continue();
}
void wdgSettingsVideo::s_text_rotation(UNUSED(bool checked)) {
	emu_thread_pause();
	cfg->text_rotation = !cfg->text_rotation;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	emu_thread_continue();
}
