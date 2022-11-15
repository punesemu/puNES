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

#ifndef WDGSETTINGSCHEATS_HPP_
#define WDGSETTINGSCHEATS_HPP_

#include <QtWidgets/QWidget>
#include "ui_wdgSettingsCheats.h"

class wdgSettingsCheats : public QWidget, public Ui::wdgSettingsCheats {
	Q_OBJECT

	public:
		explicit wdgSettingsCheats(QWidget *parent = nullptr);
		~wdgSettingsCheats() override;

	private:
		void changeEvent(QEvent *event) override;
		void showEvent(QShowEvent *event) override;

	public:
		void retranslateUi(QWidget *wdgSettingsCheats);
		void update_widget(void);

	private:
		void cheat_mode_set(void);
		void cheat_editor_control(void);

	private slots:
		void s_cheat_mode(bool checked);
};

#endif /* WDGSETTINGSCHEATS_HPP_ */
