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

#ifndef DLGHEADEREDITOR_HPP_
#define DLGHEADEREDITOR_HPP_

#include <array>
#include <QtCore/QFileInfo>
#include <QtWidgets/QButtonGroup>
#include "ui_dlgHeaderEditor.h"
#include "wdgTitleBarWindow.hpp"
#include "common.h"

enum _header_misc {
	HEADER_SIZE = 16
};

// ----------------------------------------------------------------------------------------------

class dlgHeaderEditor final : public QWidget, public Ui::dlgHeaderEditor {
	Q_OBJECT

	private:
		typedef struct _header_info {
			_header_info() : format(255), mapper(0), submapper(0), cpu_timing(0), prg_ram(0), chr_ram(0),
				prg_ram_bat(0), chr_ram_bat(0), console_type(0), mirroring(0), input(0), misc_roms(0), vs_type(0),
				vs_ppu(0), prg_rom_kib(0), chr_rom_kib(0), battery(false), trainer(false) {}
			BYTE format;
			int mapper;
			int submapper;
			int cpu_timing;
			int prg_ram;
			int chr_ram;
			int prg_ram_bat;
			int chr_ram_bat;
			int console_type;
			int mirroring;
			int input;
			int misc_roms;
			int vs_type;
			int vs_ppu;
			DBWORD prg_rom_kib;
			DBWORD chr_rom_kib;
			bool battery;
			bool trainer;
		} _header_info;

	private:
		QButtonGroup *grp;
		QFileInfo finfo;
		std::array<BYTE, HEADER_SIZE> horg;

	signals:
		void et_adjust_size(void);

	public:
		explicit dlgHeaderEditor(QWidget *parent = nullptr);
		~dlgHeaderEditor() override;

	protected:
		void changeEvent(QEvent *event) override;

	public:
		bool read_header(const uTCHAR *rom);
		bool write_header(void);
		void reset_dialog(void);

	private:
		bool header_to_struct(_header_info &hi, const std::array<BYTE, HEADER_SIZE> &header) const;
		static void struct_to_header(const _header_info &hi, std::array<BYTE, HEADER_SIZE> &header);
		void dialog_to_struct(_header_info &hi) const;
		void struct_to_dialog(const _header_info &hi, bool save_enabled);
		static int find_multiplier(int size);
		void resize_request(void);

	private slots:
		void s_control_changed(void);
		void s_open_folder(bool checked) const;
		void s_grp_type(QAbstractButton *button);
		void s_battery(bool checked) const;
		void s_console_type(int index) const;
		void s_reset_clicked(bool checked);
		void s_save_clicked(bool checked);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgHeaderEditor final : public wdgTitleBarDialog {
	Q_OBJECT

	public:
		dlgHeaderEditor *wd;

	public:
		explicit wdgDlgHeaderEditor(QWidget *parent = nullptr);
		~wdgDlgHeaderEditor() override;

	protected:
		void hideEvent(QHideEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	private slots:
		void s_adjust_size(void);
};

#endif /* dlgHeaderEditor_HPP_ */
