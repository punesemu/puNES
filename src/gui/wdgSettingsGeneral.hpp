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

#ifndef WDGSETTINGSGENERAL_HPP_
#define WDGSETTINGSGENERAL_HPP_

#include <QtWidgets/QWidget>
#include "wdgSettingsGeneral.hh"

class wdgSettingsGeneral : public QWidget, public Ui::wdgSettingsGeneral {
		Q_OBJECT

	public:
		wdgSettingsGeneral(QWidget *parent = 0);
		~wdgSettingsGeneral();

	private:
		void changeEvent(QEvent *event);
		void showEvent(QShowEvent *event);

	public:
		void retranslateUi(QWidget *wdgSettingsGeneral);
		void update_widget(void);
		void shcut_mode(int mode);

	private:
		void mode_set(void);
		void fast_forward_velocity_set(void);
		void rewind_minutes_set(void);
		void language_set(void);

	private slots:
		void s_mode(bool checked);
		void s_fast_forward_velocity(bool checked);
		void s_rewind_minutes(bool checked);
		void s_language(int index);
		void s_game_genie_rom_file(bool checked);
		void s_game_genie_rom_file_clear(bool checked);
		void s_fds_bios_file(bool checked);
		void s_fds_bios_file_clear(bool checked);
		void s_save_battery_every_tot(bool checked);
		void s_pause_in_background(bool checked);
		void s_save_settings_on_exit(bool checked);
};

#endif /* WDGSETTINGSGENERAL_HPP_ */
