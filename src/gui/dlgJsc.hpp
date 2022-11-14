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

#ifndef DLGJSC_HPP_
#define DLGJSC_HPP_

#include <QtWidgets/QDialog>
#include <QtCore/QMutex>
#include "ui_dlgJsc.h"
#include "jstick.h"

class dlgJsc : public QDialog, public Ui::dlgJsc {
	Q_OBJECT

	private:
		_input_guid guid;
		QTimer *timer;
		QMutex *mutex;
		QRect geom;
		bool first_time;

	public:
		explicit dlgJsc(QWidget *parent = nullptr);
		~dlgJsc() override;

	signals:
		void et_update_joy_combo(void);

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void showEvent(QShowEvent *event) override;
		void hideEvent(QHideEvent *event) override;
		void closeEvent(QCloseEvent *event) override;

	private:
		void joy_combo_init(void);
		void clear_all(void);
		void update_info_lines(void);
		int js_jdev_index(void);
		int axes_disabled(void);
		int hats_disabled(void);
		int buttons_disabled(void);

	private slots:
		void s_joy_read_timer(void);
		void s_combobox_joy_activated(int index);
		void s_combobox_joy_index_changed(int index);
		void s_axis_cb_clicked(bool checked);
		void s_button_cb_clicked(bool checked);
		void s_save_clicked(bool checked);
		void s_close_clicked(bool checked);

	private slots:
		void s_et_update_joy_combo(void);
};

#endif /* DLGJSC_HPP_ */
