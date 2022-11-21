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

#ifndef DLGSETTINGS_HPP_
#define DLGSETTINGS_HPP_

#include <QtWidgets/QDialog>
#include "ui_dlgSettings.h"

class dlgSettings : public QDialog, public Ui::dlgSettings {
	Q_OBJECT

	public:
		QRect geom;

	public:
		explicit dlgSettings(QWidget *parent = nullptr);
		~dlgSettings() override;

	private:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void changeEvent(QEvent *event) override;
		void hideEvent(QHideEvent *event) override;

	public:
		void retranslateUi(QDialog *dlgSettings);
		void update_dialog(void);
		void change_rom(void);
		void shcut_mode(int mode);
		void shcut_scale(int scale);

	private:
		void update_tab_general(void);
		void update_tab_input(void);
		void update_tab_ppu(void);
		void update_tab_cheats(void);

	public:
		void update_tab_video(void);
		void update_tab_audio(void);
		void update_tab_recording(void);

	private slots:
		void s_save_settings(bool checked);
		void s_close_settings(bool checked);
};

#endif /* DLGSETTINGS_HPP_ */
