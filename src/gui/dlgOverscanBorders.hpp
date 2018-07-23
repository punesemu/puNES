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

#ifndef DLGOVERSCANBORDERS_HPP_
#define DLGOVERSCANBORDERS_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgOverscanBorders.hh"
#include "overscan.h"

class dlgOverscanBorders : public QDialog, public Ui::Set_borders {
		Q_OBJECT

	private:
		struct _data {
			BYTE save_overscan;

			int mode;

			_overscan_borders save_borders;
			_overscan_borders preview;
			_overscan_borders overscan_borders[2];
			_overscan_borders *borders;
		} data;

	public:
		dlgOverscanBorders(QWidget *parent);
		~dlgOverscanBorders();

	private:
		bool eventFilter(QObject *obj, QEvent *event);
		void update_dialog();

	private slots:
		void s_combobox_activated(int index);
		void s_preview_clicked(bool checked);
		void s_default_clicked(bool checked);
		void s_spinbox_value_changed(int i);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGOVERSCANBORDERS_HPP_ */
