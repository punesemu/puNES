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

#ifndef DLGJSC_HPP_
#define DLGJSC_HPP_

#include <QtCore/QMutex>
#include "ui_dlgJsc.h"
#include "wdgTitleBarWindow.hpp"
#include "jstick.h"

// ----------------------------------------------------------------------------------------------

class dlgJsc final : public QWidget, public Ui::dlgJsc {
	Q_OBJECT

	private:
		_input_guid guid;
		QTimer *timer;
		QMutex mutex;

	signals:
		void et_update_joy_combo(void);

	public:
		explicit dlgJsc(QWidget *parent = nullptr);
		~dlgJsc() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void changeEvent(QEvent *event) override;
		void showEvent(QShowEvent *event) override;
		void hideEvent(QHideEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	private:
		void joy_combo_init(void);
		void clear_all(void) const;
		void update_info_lines(void) const;
		int js_jdev_index(void) const;
		int axes_disabled(void) const;
		int hats_disabled(void) const;
		int buttons_disabled(void) const;

	private slots:
		void s_joy_read_timer(void);
		void s_combobox_joy_activated(int index);
		void s_combobox_joy_index_changed(int index);
		void s_axis_cb_clicked(bool checked) const;
		void s_button_cb_clicked(bool checked) const;
		void s_save_clicked(bool checked) const;

	private slots:
		void s_et_update_joy_combo(void);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgJsc final : public wdgTitleBarDialog {
	public:
		dlgJsc *wd;

	public:
		explicit wdgDlgJsc(QWidget *parent = nullptr);
		~wdgDlgJsc() override;

	protected:
		void closeEvent(QCloseEvent *event) override;
		void hideEvent(QHideEvent *event) override;
};

#endif /* DLGJSC_HPP_ */
