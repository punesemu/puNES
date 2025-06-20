/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#if defined(_WIN32)
#include <QtCore/QProcess>
#endif
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QListView>
#include "dlgHeaderEditor.hpp"
#include "mainWindow.hpp"
#include "info.h"
#include "gui.h"
#include "ines.h"
#include "conf.h"

// ----------------------------------------------------------------------------------------------

wdgDlgHeaderEditor::wdgDlgHeaderEditor(QWidget *parent) : wdgTitleBarDialog(parent) {
	wd = new dlgHeaderEditor(this);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/header.svgz"));
	set_border_color("chocolate");
	set_buttons(barButton::Close);
	set_permit_resize(false);
	add_widget(wd);
	init_geom_variable(cfg->lg_header_editor);

	connect(wd, SIGNAL(et_adjust_size(void)), this, SLOT(s_adjust_size(void)));
	connect(wd->pushButton_Cancel, SIGNAL(clicked(bool)), this, SLOT(close(void)));
}
wdgDlgHeaderEditor::~wdgDlgHeaderEditor() = default;

void wdgDlgHeaderEditor::closeEvent(QCloseEvent *event) {
	geom = geometry();
	wdgTitleBarDialog::closeEvent(event);
}
void wdgDlgHeaderEditor::hideEvent(QHideEvent *event) {
	geom = geometry();
	wdgTitleBarDialog::hideEvent(event);
}

void wdgDlgHeaderEditor::s_adjust_size(void) {
	adjustSize();
}

// ----------------------------------------------------------------------------------------------

dlgHeaderEditor::dlgHeaderEditor(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	grp = new QButtonGroup(this);
	grp->addButton(radioButton_ines10);
	grp->setId(radioButton_ines10, iNES_1_0);
	grp->addButton(radioButton_nes20);
	grp->setId(radioButton_nes20, NES_2_0);

	connect(pushButton_Folder, SIGNAL(clicked(bool)), this, SLOT(s_open_folder(bool)));
	connect(grp, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(s_grp_type(QAbstractButton*)));
	connect(comboBox_Console_Type, SIGNAL(currentIndexChanged(int)), this, SLOT(s_console_type(int)));
	connect(checkBox_Battery, SIGNAL(clicked(bool)), this, SLOT(s_battery(bool)));

	connect(pushButton_Reset, SIGNAL(clicked(bool)), this, SLOT(s_reset_clicked(bool)));
	connect(pushButton_Save, SIGNAL(clicked(bool)), this, SLOT(s_save_clicked(bool)));

	connect(grp, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(s_control_changed()));
	connect(spinBox_Mapper, SIGNAL(valueChanged(int)), this, SLOT(s_control_changed()));
	connect(spinBox_Submapper, SIGNAL(valueChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_CPU_Timing, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_Mirroring, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(spinBox_PRG_Rom, SIGNAL(valueChanged(int)), this, SLOT(s_control_changed()));
	connect(spinBox_CHR_Rom, SIGNAL(valueChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_PRG_Ram, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_CHR_Ram, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(checkBox_Battery, SIGNAL(clicked(bool)), this, SLOT(s_control_changed()));
	connect(checkBox_Trainer, SIGNAL(clicked(bool)), this, SLOT(s_control_changed()));
	connect(comboBox_PRG_Ram_Battery, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_CHR_Ram_Battery, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(spinBox_Misc_Roms, SIGNAL(valueChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_Console_Type, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_VS_Type, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_VS_PPU, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
	connect(comboBox_Input, SIGNAL(currentIndexChanged(int)), this, SLOT(s_control_changed()));
}
dlgHeaderEditor::~dlgHeaderEditor() {
	delete (grp);
}

void dlgHeaderEditor::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

bool dlgHeaderEditor::read_header(const uTCHAR *rom) {
	QFile file(uQString(rom));
	_header_info hi;

	finfo.setFile(uQString(rom));

	if (!file.open(QIODevice::ReadOnly)) {
		gui_critical(uQStringCD(QString("Error on opening '%0'.").arg(finfo.fileName())));
		return (false);
	}

	if (file.read((char *)horg.data(), HEADER_SIZE) != HEADER_SIZE) {
		file.close();
		gui_critical(uQStringCD(QString("Error on reading header from '%0'.").arg(finfo.fileName())));
		return (false);
	}

	file.close();

	if (!header_to_struct(hi, horg)) {
		gui_critical(uQStringCD(QString("Unknow format.")));
		return (false);
	}

	struct_to_dialog(hi, false);
	return (true);
}
bool dlgHeaderEditor::write_header(void) {
	std::array<BYTE, HEADER_SIZE> tmp = { 0 };
	QFile file(finfo.filePath());
	_header_info hi;

	dialog_to_struct(hi);
	struct_to_header(hi, tmp);

	if (!file.open(QIODevice::ReadWrite)) {
		gui_critical(uQStringCD(QString("Error on opening '%0'.").arg(finfo.fileName())));
		return (false);
	}

	if (!file.seek(0)) {
		file.close();
		gui_critical(uQStringCD(QString("Error on writing header on '%0'.").arg(finfo.fileName())));
		return (false);
	}

	if (file.write((char *)tmp.data(), HEADER_SIZE) != HEADER_SIZE) {
		file.close();
		gui_critical(uQStringCD(QString("Error on writing header on '%0'.").arg(finfo.fileName())));
		return (false);
	}

	file.close();
	horg = tmp;
	return (true);
}
void dlgHeaderEditor::reset_dialog(void) {
	_header_info hi;

	header_to_struct(hi, horg);
	struct_to_dialog(hi, false);
}

bool dlgHeaderEditor::header_to_struct(_header_info &hi, const std::array<BYTE, HEADER_SIZE> &header) const {
	hi = {};

	if ((header[0] != 'N') || (header[1] != 'E') || (header[2] != 'S') || (header[3] != '\32')) {
		return (false);
	}

	hi.format = (header[7] & 0x0C) == 0x08 ? NES_2_0 : iNES_1_0;

	if (hi.format == NES_2_0) {
		size_t prg_size = 0, chr_size = 0;

		hi.console_type = (header[7] & 0x03) == 0x03 ? header[13] & 0x0F : header[7] & 0x03;

		hi.mapper = ((header[8] & 0x0F) << 8) | (header[7] & 0xF0) | (header[6] >> 4);
		hi.submapper = (header[8] & 0xF0) >> 4;

		prg_size = ((header[9] & 0x0F) < 0x0F)
			? (size_t)(header[4] | ((header[9] & 0x0F) << 8)) * S16K
			: (size_t)pow(2.0, (double)(header[4] >> 2)) * ((header[4] & 0x03) * 2 + 1);
		hi.prg_rom_kib = prg_size / S1K;

		chr_size = ((header[9] >> 4) < 0x0F)
			? (size_t)(header[5] | ((header[9] & 0xF0) << 4)) * S8K
			: (size_t)pow(2.0, (double)(header[5] >> 2)) * ((header[5] & 0x03) * 2 + 1);
		hi.chr_rom_kib = chr_size / S1K;

		hi.prg_ram = header[10] & 0x0F;
		hi.chr_ram = header[11] & 0x0F;

		hi.prg_ram_bat = header[10] >> 4;
		hi.chr_ram_bat = header[11] >> 4;

		hi.cpu_timing = header[12] & 0x03;

		hi.vs_ppu = header[13] & 0x0F;
		hi.vs_type = header[13] >> 4;

		hi.misc_roms = header[14] & 0x03;

		hi.input = header[15] & 0x3F;
		if (hi.input >= comboBox_Input->count()) {
			hi.input = 0;
		}
	} else {
		hi.prg_rom_kib = header[4];
		hi.chr_rom_kib = header[5];

		// Older versions of the iNES emulator ignored bytes 7-15, and several ROM management tools
		// wrote messages in there. Commonly, these will be filled with "DiskDude!", which results
		// in 64 being added to the mapper number. A general rule of thumb: if the last 4 bytes are
		// not all zero, and the header is not marked for NES 2.0 format, an emulator should either
		// mask off the upper 4 bits of the mapper number or simply refuse to load the ROM.
		if (header[12] | header[13] | header[14] | header[15]) {
			hi.mapper = header[6] >> 4;
			hi.cpu_timing = 0;
		} else {
			hi.mapper = (header[7] & 0xF0) | (header[6] >> 4);
			hi.cpu_timing = header[9] & 0x01;
		}
		hi.submapper = 0;
	}

	hi.battery = (header[6] & 0x02) >> 1;
	hi.trainer = (header[6] & 0x04) >> 2;

	hi.mirroring = header[6] & 0x08 ? 2 : header[6] & 0x01 ? 1 : 0;

	return (true);
}
void dlgHeaderEditor::struct_to_header(const _header_info &hi, std::array<BYTE, HEADER_SIZE> &header) {
	const bool is_nes_20 = hi.format == NES_2_0;
	int prg_size = 0, chr_size = 0;

	header.fill(0x00);

	header[0] = 'N';
	header[1] = 'E';
	header[2] = 'S';
	header[3] = '\32';

	// PRG ROM
	prg_size = (int)hi.prg_rom_kib * S1K;
	header[4] = (prg_size / S16K) & 0xFF;
	// CHR ROM
	chr_size = (int)hi.chr_rom_kib * S1K;
	header[5] = (chr_size / S8K) & 0xFF;
	// Mirroring
	header[6] = (header[6] & 0xF6) | (hi.mirroring > 1 ? 0x08 : (hi.mirroring & 0X01));
	// Battery
	header[6] = (header[6] & 0xFD) | (hi.battery ? 0x02 : 0x00);
	// Trainer
	header[6] = (header[6] & 0xFB) | (hi.trainer ? 0x04 : 0x00);

	if (is_nes_20) {
		// NES 2.0
		header[7] = (header[7] & 0xF3) | 0x08;
		// PRG ROM
		{
			header[9] = (header[9] & 0xF0) | (((prg_size / S16K) & 0xF00) >> 8);
			if (prg_size >= (64 * S1K * S1K) || (prg_size % S16K)) {
				const int multiplier = find_multiplier(prg_size);
				int exponent = 0;

				header[9] = (header[9] & 0xF0) | 0x0F;
				exponent = (int)log2(prg_size / multiplier);
				header[4] = (exponent << 2) | ((multiplier - 1) >> 1);
			}
		}
		// CHR ROM
		{
			header[9] = (header[9] & 0x0F) | (((chr_size / S8K) & 0xF00) >> 4);
			if (chr_size >= (32 * S1K * S1K) || (chr_size % S8K)) {
				const int multiplier = find_multiplier(chr_size);
				int exponent = 0;

				header[9] = (header[9] & 0x0F) | 0xF0;
				exponent = (int)log2(chr_size / multiplier);
				header[5] = (exponent << 2) | ((multiplier - 1) >> 1);
			}
		}
		// Mapper
		header[6] = (header[6] & 0x0F) | ((hi.mapper & 0x00F) << 4);
		header[7] = (header[7] & 0x0F) | ((hi.mapper & 0x0F0) >> 0);
		header[8] = (header[8] & 0xF0) | ((hi.mapper & 0xF00) >> 8);
		// Submapper
		header[8] = (header[8] & 0x0F) | ((hi.submapper & 0x0F) << 4);
		// Console type
		if (hi.console_type <= 2) {
			header[7] = (header[7] & 0xFC) | (hi.console_type & 0x03);
			// VS. System
			if (hi.console_type == 1) {
				header[13] = ((hi.vs_type & 0x0F) << 4) | (hi.vs_ppu & 0x0F);
			}
		} else {
			// Extended Console Type
			header[7] = (header[7] & 0xFC) | 0x03;
			header[13] = (header[13] & 0xF0) | (hi.console_type & 0x0F);
		}
		// PRG RAM
		header[10] = (header[10] & 0xF0) | (hi.prg_ram & 0x0F);
		if (hi.battery) {
			header[10] = (header[10] & 0x0F) | ((hi.prg_ram_bat & 0x0F) << 4);
		}
		// CHR RAM
		header[11] = (header[11] & 0xF0) | (hi.chr_ram & 0x0F);
		if (hi.battery) {
			header[11] = (header[11] & 0x0F) | ((hi.chr_ram_bat & 0x0F) << 4);
		}
		// CPU Timing
		header[12] = (header[12] & 0xFC) | (hi.cpu_timing & 0x03);
		// Misc ROMs
		header[14] = (header[14] & 0xFC) | (hi.misc_roms & 0x03);
		// Input
		header[15] = (header[15] & 0xC0) | (hi.input & 0x3F);
	} else {
		// Mapper
		header[6] = (header[6] & 0x0F) | ((hi.mapper & 0x00F) << 4);
		header[7] = (header[7] & 0x0F) | ((hi.mapper & 0x0F0) >> 0);
		// CPU Timing
		header[9] = (header[9] & 0xFE) | (hi.cpu_timing & 0x01);
	}
}
void dlgHeaderEditor::dialog_to_struct(_header_info &hi) const {
	hi.format = grp->checkedId();
	hi.mapper = spinBox_Mapper->value();
	hi.submapper = spinBox_Submapper->value();
	hi.cpu_timing = comboBox_CPU_Timing->currentIndex();
	hi.prg_ram = comboBox_PRG_Ram->currentIndex();
	hi.chr_ram = comboBox_CHR_Ram->currentIndex();
	hi.prg_ram_bat = comboBox_PRG_Ram_Battery->currentIndex();
	hi.chr_ram_bat = comboBox_CHR_Ram_Battery->currentIndex();
	hi.console_type = comboBox_Console_Type->currentIndex();
	hi.mirroring = comboBox_Mirroring->currentIndex();
	hi.input = comboBox_Input->currentIndex();
	hi.misc_roms = spinBox_Misc_Roms->value();
	hi.vs_type = comboBox_VS_Type->currentIndex();
	hi.vs_ppu = comboBox_VS_PPU->currentIndex();
	hi.prg_rom_kib = spinBox_PRG_Rom->value();
	hi.chr_rom_kib = spinBox_CHR_Rom->value();
	hi.battery = checkBox_Battery->isChecked();
	hi.trainer = checkBox_Trainer->isChecked();
}
void dlgHeaderEditor::struct_to_dialog(const _header_info &hi, const bool save_enabled) {
	const bool is_ines_10 = hi.format == iNES_1_0;
	const bool is_nes_20 = hi.format == NES_2_0;

	// File
	lineEdit_File->setText(finfo.filePath());
	// iNES 1.0
	grp->blockSignals(true);
	radioButton_ines10->setChecked(is_ines_10);
	// NES 2.0
	radioButton_nes20->setChecked(is_nes_20);
	grp->blockSignals(false);
	// Mapper
	qtHelper::spinbox_set_value(spinBox_Mapper, hi.mapper);
	// Submapper
	qtHelper::spinbox_set_value(spinBox_Submapper, hi.submapper);
	// CPU Timing
	qtHelper::combox_set_index(comboBox_CPU_Timing, hi.cpu_timing);
	// Mirroring
	qtHelper::combox_set_index(comboBox_Mirroring, hi.mirroring);
	// PRG ROM
	qtHelper::spinbox_set_value(spinBox_PRG_Rom, (int)hi.prg_rom_kib);
	// CHR ROM
	qtHelper::spinbox_set_value(spinBox_CHR_Rom, (int)hi.chr_rom_kib);
	// PRG RAM
	qtHelper::combox_set_index(comboBox_PRG_Ram, hi.prg_ram);
	// CHR RAM
	qtHelper::combox_set_index(comboBox_CHR_Ram, hi.chr_ram);
	// Battery
	qtHelper::checkbox_set_checked(checkBox_Battery, hi.battery);
	// Trainer
	qtHelper::checkbox_set_checked(checkBox_Trainer, hi.trainer);
	// PRG RAM Battery
	qtHelper::combox_set_index(comboBox_PRG_Ram_Battery, hi.prg_ram_bat);
	// CHR RAM Battery
	qtHelper::combox_set_index(comboBox_CHR_Ram_Battery, hi.chr_ram_bat);
	// Misc ROMs
	qtHelper::spinbox_set_value(spinBox_Misc_Roms, hi.misc_roms);
	// Console Type
	qtHelper::combox_set_index(comboBox_Console_Type, hi.console_type);
	// VS Type
	qtHelper::combox_set_index(comboBox_VS_Type, hi.vs_type);
	// VS PPU
	qtHelper::combox_set_index(comboBox_VS_PPU, hi.vs_ppu);
	// Input
	qtHelper::combox_set_index(comboBox_Input, hi.input);

	s_grp_type(nullptr);

	pushButton_Reset->setEnabled(save_enabled);
	pushButton_Save->setEnabled(save_enabled);
}
int dlgHeaderEditor::find_multiplier(int size) {
	int multiplier = 1;

	if ((size % 3) == 0) {
		multiplier = 3;
	}
	if ((size % 5) == 0) {
		multiplier = 5;
	}
	if ((size % 7) == 0) {
		multiplier = 7;
	}
	return (multiplier);
}
void dlgHeaderEditor::resize_request(void) {
	adjustSize();
	emit et_adjust_size();
}

void dlgHeaderEditor::s_control_changed(void) {
	std::array<BYTE, HEADER_SIZE> tmp = { 0 };
	_header_info hi;

	dialog_to_struct(hi);
	struct_to_header(hi, tmp);
	pushButton_Reset->setEnabled(!std::equal(horg.begin(), horg.end(), tmp.begin()));
	pushButton_Save->setEnabled(pushButton_Reset->isEnabled());
}
void dlgHeaderEditor::s_open_folder(UNUSED(bool checked)) const {
#if defined(_WIN32)
	const QString explorer = "explorer";
	QStringList param;

	param << QLatin1String("/select,");
	param << QDir::toNativeSeparators(finfo.absoluteFilePath());
	QProcess::startDetached(explorer, param);
#else
	QDesktopServices::openUrl(QUrl(finfo.absolutePath()));
#endif
}
void dlgHeaderEditor::s_grp_type(UNUSED(QAbstractButton *button)) {
	const bool is_ines_10 = grp->checkedId() == iNES_1_0;
	const bool is_nes_20 = grp->checkedId() == NES_2_0;

	// iNES 1.0
	radioButton_ines10->setEnabled(is_ines_10);
	// NES 2.0
	radioButton_nes20->setEnabled(is_ines_10 || is_nes_20);
	// Mapper
	spinBox_Mapper->setMaximum(is_ines_10 ? 255 : 1000);
	// Submapper
	label_Submapper->setVisible(is_nes_20);
	spinBox_Submapper->setVisible(is_nes_20);
	// CPU Timing
	{
		QListView *view = qobject_cast<QListView *>(comboBox_CPU_Timing->view());

		view->setRowHidden(2, is_ines_10);
		view->setRowHidden(3, is_ines_10);
	}
	// Mirroring
	// PRG Rom
	// CHR Rom
	// PRG Ram
	label_PRG_Ram->setVisible(is_nes_20);
	comboBox_PRG_Ram->setVisible(is_nes_20);
	// CHR Ram
	label_CHR_Ram->setVisible(is_nes_20);
	comboBox_CHR_Ram->setVisible(is_nes_20);
	// Battery
	// Trainer
	// PRG Ram Battery e CHR Ram Battery
	s_battery(checkBox_Battery->isChecked());
	// Misc Roms
	label_Misc_Roms->setVisible(is_nes_20);
	spinBox_Misc_Roms->setVisible(is_nes_20);
	// Console Type
	label_Console_Type->setVisible(is_nes_20);
	comboBox_Console_Type->setVisible(is_nes_20);
	// VS Type e VS PPU
	s_console_type(comboBox_Console_Type->currentIndex());
	// Input
	label_Input->setVisible(is_nes_20);
	comboBox_Input->setVisible(is_nes_20);

	resize_request();
}
void dlgHeaderEditor::s_console_type(int index) const {
	const bool is_nes_20 = radioButton_nes20->isChecked();
	const bool is_vs_system = index == 1;

	// VS Type
	label_VS_Type->setEnabled(is_vs_system);
	label_VS_Type->setVisible(is_nes_20);
	comboBox_VS_Type->setEnabled(is_vs_system);
	comboBox_VS_Type->setVisible(is_nes_20);
	// VS PPU
	label_VS_PPU->setEnabled(is_vs_system);
	label_VS_PPU->setVisible(is_nes_20);
	comboBox_VS_PPU->setEnabled(is_vs_system);
	comboBox_VS_PPU->setVisible(is_nes_20);
}
void dlgHeaderEditor::s_battery(bool checked) const {
	const bool is_nes_20 = radioButton_nes20->isChecked();
	const bool is_battery = checked;

	// PRG RAM Battery
	label_PRG_Ram_Battery->setEnabled(is_battery);
	label_PRG_Ram_Battery->setVisible(is_nes_20);
	comboBox_PRG_Ram_Battery->setEnabled(is_battery);
	comboBox_PRG_Ram_Battery->setVisible(is_nes_20);
	// CHR RAM Battery
	label_CHR_Ram_Battery->setEnabled(is_battery);
	label_CHR_Ram_Battery->setVisible(is_nes_20);
	comboBox_CHR_Ram_Battery->setEnabled(is_battery);
	comboBox_CHR_Ram_Battery->setVisible(is_nes_20);
}
void dlgHeaderEditor::s_reset_clicked(UNUSED(bool checked)) {
	reset_dialog();
}
void dlgHeaderEditor::s_save_clicked(UNUSED(bool checked)) {
	if (write_header()) {
		const QMessageBox::StandardButton reply = QMessageBox::question(this,
			tr("Attention"),
			tr("Do you want to boot the ROM with the changes made?"),
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::Yes) {
			info.block_recent_roms_update = TRUE;
			gui_emit_et_reset(CHANGE_ROM);
		}
	}
}
