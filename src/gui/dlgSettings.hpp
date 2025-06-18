/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef DLGSETTINGS_HPP_
#define DLGSETTINGS_HPP_

#include "ui_dlgSettings.h"
#include "wdgTitleBarWindow.hpp"

// ----------------------------------------------------------------------------------------------

class dlgSettings final : public QWidget, public Ui::dlgSettings {
	Q_OBJECT

	signals:
		void et_close(void);

	public:
		explicit dlgSettings(QWidget *parent = nullptr);
		~dlgSettings() override;

	private:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void changeEvent(QEvent *event) override;

	public:
		void retranslateUi(QWidget *dlgSettings);
		void update_dialog(void) const;
		void change_rom(void) const;
		void shcut_mode(int mode) const;
		void shcut_scale(int scale) const;

	private:
		void update_tab_general(void) const;
		void update_tab_input(void) const;
		void update_tab_ppu(void) const;
		void update_tab_cheats(void) const;

	public:
		void update_tab_video(void) const;
		void update_tab_audio(void) const;
		void update_tab_recording(void) const;

	private slots:
		static void s_save_settings(bool checked);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgSettings final : public wdgTitleBarDialog {
	public:
		dlgSettings *wd;

	public:
		explicit wdgDlgSettings(QWidget *parent = nullptr);
		~wdgDlgSettings() override;

	protected:
		void hideEvent(QHideEvent *event) override;
		void closeEvent(QCloseEvent *event) override;
};

#endif /* DLGSETTINGS_HPP_ */
