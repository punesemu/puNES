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

#ifndef WDGSETTINGSPPU_HPP_
#define WDGSETTINGSPPU_HPP_

#include <QtWidgets/QWidget>
#include "wdgSettingsPPU.hh"

class wdgSettingsPPU : public QWidget, public Ui::wdgSettingsPPU {
		Q_OBJECT

	public:
		wdgSettingsPPU(QWidget *parent = 0);
		~wdgSettingsPPU();

	protected:
		void changeEvent(QEvent *event);
		void showEvent(QShowEvent *event);

	public:
		void update_widget(void);
		void lag_counter_update(void);

	private slots:
		void s_hide_sprites(bool checked);
		void s_hide_background(bool checked);
		void s_unlimited_sprites(bool checked);
		void s_ppu_overclock(bool checked);
		void s_disable_dmc_control(bool checked);
		void s_overclock_vb_slines(int i);
		void s_overclock_pr_slines(int i);
		void s_lag_counter_reset(bool checked);
};

#endif /* WDGSETTINGSPPU_HPP_ */
