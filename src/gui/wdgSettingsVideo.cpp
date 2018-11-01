/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
#include "mainWindow.hpp"
#include "emu_thread.h"
#include "conf.h"
#include "clock.h"

wdgSettingsVideo::wdgSettingsVideo(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	connect(comboBox_Scale, SIGNAL(activated(int)), this, SLOT(s_scale(int)));
	connect(comboBox_PAR, SIGNAL(activated(int)), this, SLOT(s_par(int)));
	connect(checkBox_PAR_Soft_Stretch, SIGNAL(clicked(bool)), this, SLOT(s_par_stretch(bool)));

	connect(comboBox_Oscan, SIGNAL(activated(int)), this, SLOT(s_oscan(int)));
	connect(comboBox_Oscan_def_value, SIGNAL(activated(int)), this, SLOT(s_oscan_def_value(int)));
	connect(comboBox_Oscan_brd_mode, SIGNAL(activated(int)), this, SLOT(s_oscan_brd_mode(int)));
	{
		spinBox_Oscan_brd_up->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_brd_down->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_brd_left->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);
		spinBox_Oscan_brd_right->setRange(OVERSCAN_BORDERS_MIN, OVERSCAN_BORDERS_MAX);

		connect(spinBox_Oscan_brd_up, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_brd_down, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_brd_left, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
		connect(spinBox_Oscan_brd_right, SIGNAL(valueChanged(int)), this, SLOT(s_oscan_spinbox(int)));
	}
	connect(pushButton_Oscan_brd_reset, SIGNAL(clicked(bool)), this, SLOT(s_oscan_reset(bool)));
	connect(checkBox_Oscan_brd_black_window, SIGNAL(clicked(bool)), this, SLOT(s_oscan_brd_black_w(bool)));
	connect(checkBox_Oscan_brd_black_flscreen, SIGNAL(clicked(bool)), this, SLOT(s_oscan_brd_black_f(bool)));

	connect(comboBox_Software_Filters, SIGNAL(activated(int)), this, SLOT(s_sfilters(int)));
	connect(comboBox_Shaders, SIGNAL(activated(int)), this, SLOT(s_shaders(int)));
	connect(pushButton_Shaders_file, SIGNAL(clicked(bool)), this, SLOT(s_shader_file(bool)));
	connect(pushButton_Shaders_file_clear, SIGNAL(clicked(bool)), this, SLOT(s_shader_file_clear(bool)));
#if defined (WITH_OPENGL)
	connect(checkBox_Disable_sRGB_FBO, SIGNAL(clicked(bool)), this, SLOT(s_disable_srgb_fbo(bool)));
#endif

	connect(comboBox_Palette, SIGNAL(activated(int)), this, SLOT(s_palette(int)));
	connect(pushButton_Palette_file, SIGNAL(clicked(bool)), this, SLOT(s_palette_file(bool)));
	connect(pushButton_Palette_file_clear, SIGNAL(clicked(bool)), this, SLOT(s_palette_file_clear(bool)));
	connect(pushButton_Palette_save, SIGNAL(clicked(bool)), this, SLOT(s_palette_save(bool)));
	connect(checkBox_Disable_emphasis_swap_PAL, SIGNAL(clicked(bool)), this, SLOT(s_disable_emphasis_swap_pal(bool)));

	connect(checkBox_Vsync, SIGNAL(clicked(bool)), this, SLOT(s_vsync(bool)));
	connect(checkBox_Interpolation, SIGNAL(clicked(bool)), this, SLOT(s_interpolation(bool)));
	connect(checkBox_Text_on_screen, SIGNAL(clicked(bool)), this, SLOT(s_text_on_screen(bool)));
	connect(checkBox_Input_display, SIGNAL(clicked(bool)), this, SLOT(s_input_display(bool)));
	connect(checkBox_Disable_TV_noise_emulation, SIGNAL(clicked(bool)), this, SLOT(s_disable_tv_noise(bool)));
	connect(checkBox_Disable_sepia_color_on_pause, SIGNAL(clicked(bool)), this, SLOT(s_disable_sepia(bool)));
	connect(checkBox_Fullscreen_in_window, SIGNAL(clicked(bool)), this, SLOT(s_fullscreen_in_window(bool)));
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
void wdgSettingsVideo::showEvent(QShowEvent *event) {
	int dim = label_Scale->size().height() - 10;

	icon_Scale->setPixmap(QIcon(":/icon/icons/scale.svg").pixmap(dim, dim));
	icon_Oscan_dev_value->setPixmap(QIcon(":/icon/icons/overscan_default.svg").pixmap(dim, dim));
	icon_Oscan_brd->setPixmap(QIcon(":/icon/icons/overscan_set_borders.svg").pixmap(dim, dim));
	icon_Shaders->setPixmap(QIcon(":/icon/icons/shader.svg").pixmap(dim, dim));
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
	mainwin->qaction_shcut.stretch_in_fullscreen->setText(checkBox_Stretch_in_fullscreen->text());
	update_widget();
}
void wdgSettingsVideo::update_widget(void) {
	scale_set();

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
			tab_PAR->setEnabled(false);
		} else {
			tab_PAR->setEnabled(true);
			if (cfg->pixel_aspect_ratio != PAR11) {
				checkBox_PAR_Soft_Stretch->setEnabled(true);
			} else {
				checkBox_PAR_Soft_Stretch->setEnabled(false);
			}
		}
	}

	{
		oscan_set();
		oscan_def_value_set();
		oscan_brd_set();

		checkBox_Oscan_brd_black_window->setChecked(cfg->oscan_black_borders);
		checkBox_Oscan_brd_black_flscreen->setChecked(cfg->oscan_black_borders_fscr);
	}

	{
		sfilters_set();
		shaders_set();
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
	checkBox_Interpolation->setChecked(cfg->interpolation);
	checkBox_Text_on_screen->setChecked(cfg->txt_on_screen);
	checkBox_Input_display->setChecked(cfg->input_display);
	checkBox_Disable_TV_noise_emulation->setChecked(cfg->disable_tv_noise);
	checkBox_Disable_sepia_color_on_pause->setChecked(cfg->disable_sepia_color);
	checkBox_Fullscreen_in_window->setChecked(cfg->fullscreen_in_window);
	checkBox_Stretch_in_fullscreen->setChecked(cfg->stretch);
}
void wdgSettingsVideo::change_rom(void) {
	oscan_brd_mode_set();
	update_widget();
}

void wdgSettingsVideo::scale_set(void) {
	comboBox_Scale->setCurrentIndex(cfg->scale - 1);
}
void wdgSettingsVideo::par_set(void) {
	comboBox_PAR->setCurrentIndex(cfg->pixel_aspect_ratio);
}
void wdgSettingsVideo::oscan_set(void) {
	int oscan = 0;

	switch (cfg->oscan) {
		case OSCAN_ON:
			oscan = 2;
			break;
		case OSCAN_OFF:
			oscan = 1;
			break;
		case OSCAN_DEFAULT:
			oscan = 0;
			break;
	}

	comboBox_Oscan->setCurrentIndex(oscan);
}
void wdgSettingsVideo::oscan_def_value_set(void) {
	comboBox_Oscan_def_value->setCurrentIndex(cfg->oscan_default);
}
void wdgSettingsVideo::oscan_brd_mode_set(void) {
	int mode = 0;

	if (machine.type == NTSC) {
		mode = 0;
	} else {
		mode = 1;
	}

	comboBox_Oscan_brd_mode->setCurrentIndex(mode);
}

void wdgSettingsVideo::oscan_brd_set(void) {
	_overscan_borders *borders;

	borders = &overscan_borders[comboBox_Oscan_brd_mode->currentIndex()];

	spinBox_Oscan_brd_up->blockSignals(true);
	spinBox_Oscan_brd_up->setValue(borders->up);
	spinBox_Oscan_brd_up->blockSignals(false);

	spinBox_Oscan_brd_down->blockSignals(true);
	spinBox_Oscan_brd_down->setValue(borders->down);
	spinBox_Oscan_brd_down->blockSignals(false);

	spinBox_Oscan_brd_left->blockSignals(true);
	spinBox_Oscan_brd_left->setValue(borders->left);
	spinBox_Oscan_brd_left->blockSignals(false);

	spinBox_Oscan_brd_right->blockSignals(true);
	spinBox_Oscan_brd_right->setValue(borders->right);
	spinBox_Oscan_brd_right->blockSignals(false);
}
void wdgSettingsVideo::sfilters_set(void) {
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
void wdgSettingsVideo::shaders_set(void) {
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
		case PALETTE_FILE:
			palette = 7;
			break;
	}

	comboBox_Palette->setCurrentIndex(palette);
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
void wdgSettingsVideo::s_par(int index) {
	int par = index;

	if (cfg->pixel_aspect_ratio == par) {
		return;
	}

	emu_thread_pause();
	cfg->pixel_aspect_ratio = par;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();

	update_widget();
}
void wdgSettingsVideo::s_par_stretch(bool checked) {
	emu_thread_pause();
	cfg->PAR_soft_stretch = !cfg->PAR_soft_stretch;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan(int index) {
	int oscan = index;

	switch (oscan) {
		case 2:
			oscan = OSCAN_ON;
			break;
		case 1:
			oscan = OSCAN_OFF;
			break;
		case 0:
			oscan = OSCAN_DEFAULT;
			break;
	}

	emu_thread_pause();
	cfg->oscan = oscan;
	settings_pgs_save();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_def_value(int index) {
	int def = index;

	emu_thread_pause();
	cfg->oscan_default = def;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_brd_black_w(bool checked) {
	emu_thread_pause();
	cfg->oscan_black_borders = !cfg->oscan_black_borders;
	if (overscan.enabled) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_brd_black_f(bool checked) {
	emu_thread_pause();
	cfg->oscan_black_borders_fscr = !cfg->oscan_black_borders_fscr;
	if (overscan.enabled) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_brd_mode(int index) {
	oscan_brd_set();
}
void wdgSettingsVideo::s_oscan_spinbox(int i) {
	QString name = ((QSpinBox *)sender())->objectName();
	_overscan_borders *borders;

	borders = &overscan_borders[comboBox_Oscan_brd_mode->currentIndex()];

	if (name == "spinBox_Oscan_brd_up") {
		borders->up = i;
	} else if (name == "spinBox_Oscan_brd_down") {
		borders->down = i;
	} else if (name == "spinBox_Oscan_brd_left") {
		borders->left = i;
	} else if (name == "spinBox_Oscan_brd_right") {
		borders->right = i;
	}

	emu_thread_pause();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_oscan_reset(bool checked) {
	_overscan_borders *borders;
	int mode;

	mode = comboBox_Oscan_brd_mode->currentIndex();
	borders = &overscan_borders[mode];

	emu_thread_pause();
	settings_set_overscan_default(borders, mode + NTSC);
	oscan_brd_set();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_sfilters(int index) {
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
		ntsc_set(NULL, cfg->ntsc_format, 0, 0, (BYTE *) palette_RGB, 0);
	}
	emu_thread_continue();
}
void wdgSettingsVideo::s_shaders(int index) {
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
	emu_thread_continue();
}
#if defined (WITH_OPENGL)
void wdgSettingsVideo::s_disable_srgb_fbo(bool checked) {
	emu_thread_pause();
	cfg->disable_srgb_fbo = !cfg->disable_srgb_fbo;
	if (info.sRGB_FBO_in_use == TRUE) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
	emu_thread_continue();
}
#endif
void wdgSettingsVideo::s_shader_file(bool checked) {
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
			text_add_line_info(1, "[red]error on shader file");
		}
	}

	update_widget();

	emu_pause(FALSE);
}
void wdgSettingsVideo::s_shader_file_clear(bool checked) {
	umemset(cfg->shader_file, 0x00, usizeof(cfg->shader_file));
	shaders_set();
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
			palette = PALETTE_FILE;
			break;
	}

	emu_thread_pause();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, palette, FALSE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_palette_file(bool checked) {
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
			emu_thread_continue();
		} else {
			text_add_line_info(1, "[red]error on palette file");
		}
	}

	emu_pause(FALSE);
}
void wdgSettingsVideo::s_palette_file_clear(bool checked) {
	umemset(cfg->palette_file, 0x00, usizeof(cfg->palette_file));
	palette_set();
}
void wdgSettingsVideo::s_palette_save(bool checked) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Palette files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.pal *.PAL)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getSaveFileName(this, tr("Save palette on file"),
		uQString(opt_palette[cfg->palette].lname).replace(" ", "_"),
		filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(QString(file) + ".pal");
		}

		palette_save_on_file(uQStringCD(fileinfo.absoluteFilePath()));
	}

	emu_pause(FALSE);
}
void wdgSettingsVideo::s_disable_emphasis_swap_pal(bool checked) {
	emu_thread_pause();
	cfg->disable_swap_emphasis_pal = !cfg->disable_swap_emphasis_pal;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, TRUE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_vsync(bool checked) {
	emu_thread_pause();
	cfg->vsync = !cfg->vsync;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_interpolation(bool checked) {
	emu_thread_pause();
	cfg->interpolation = !cfg->interpolation;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	emu_thread_continue();
}
void wdgSettingsVideo::s_text_on_screen(bool checked) {
	cfg->txt_on_screen = !cfg->txt_on_screen;
}
void wdgSettingsVideo::s_input_display(bool checked) {
	cfg->input_display = !cfg->input_display;

	if (!cfg->input_display) {
		BYTE i;

		for (i = 0; i < PORT_MAX; i++) {
			_txt_element *ele = &text.tas.controllers[i];

			if (ele->surface) {
				text_clear(ele);
				gfx_text_release_surface(ele);
			}
		}
	}
}
void wdgSettingsVideo::s_disable_tv_noise(bool checked) {
	cfg->disable_tv_noise = !cfg->disable_tv_noise;
}
void wdgSettingsVideo::s_disable_sepia(bool checked) {
	cfg->disable_sepia_color = !cfg->disable_sepia_color;
}
void wdgSettingsVideo::s_fullscreen_in_window(bool checked) {
	cfg->fullscreen_in_window = !cfg->fullscreen_in_window;
}
void wdgSettingsVideo::s_stretch_in_fullscreen(bool checked) {
	emu_thread_pause();
	cfg->stretch = !cfg->stretch;

	if (cfg->fullscreen == FULLSCR) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	}
	emu_thread_continue();
}
