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

#ifndef DLGVSSYSTEM_HPP_
#define DLGVSSYSTEM_HPP_

#include "ui_dlgVsSystem.h"
#include "wdgTitleBarWindow.hpp"

// ----------------------------------------------------------------------------------------------

class dlgVsSystem : public QWidget, public Ui::dlgVsSystem {
	Q_OBJECT

	public:
		explicit dlgVsSystem(QWidget *parent = nullptr);
		~dlgVsSystem() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void changeEvent(QEvent *event) override;

	public:
		void update_dialog(void) const;
		static void insert_coin(int index);

	private slots:
		void s_coins_clicked(bool checked) const;
		void s_monitor_clicked(bool checked) const;
		void s_ds_changed(int state) const;
		void s_ds_clicked(bool checked) const;
		void s_ds_defaults_clicked(bool checked) const;
};

// ----------------------------------------------------------------------------------------------

class wdgDlgVsSystem : public wdgTitleBarDialog {
	Q_OBJECT

	public:
		dlgVsSystem *wd;

	public:
		explicit wdgDlgVsSystem(QWidget *parent = nullptr);
		~wdgDlgVsSystem() override;

	private slots:
		static void s_x_clicked(void);
};

#endif /* DLGVSSYSTEM_HPP_ */
