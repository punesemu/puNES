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

#ifndef WDGSETTINGSCHEATS_HPP_
#define WDGSETTINGSCHEATS_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif
#include "wdgSettingsCheats.hh"

class wdgSettingsCheats : public QWidget, public Ui::wdgSettingsCheats {
		Q_OBJECT

	public:
		wdgSettingsCheats(QWidget *parent = 0);
		~wdgSettingsCheats();

	private:
		void changeEvent(QEvent *event);

	public:
		void retranslateUi(QWidget *wdgSettingsInput);
		void update_widget(void);

	private:
		void cheat_mode_set(void);

	private slots:
		void s_cheat_mode(int index);
};

#endif /* WDGSETTINGSCHEATS_HPP_ */
