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

#ifndef DLGDIPSWITCH_HPP_
#define DLGDIPSWITCH_HPP_

#include <QtWidgets/QButtonGroup>
#include "ui_dlgDipswitch.h"
#include "wdgTitleBarWindow.hpp"
#include "common.h"

typedef struct _dp_value {
	QString name;
	WORD value;
} _dp_value;
typedef struct _dp_type {
	QString name;
	WORD mask;
	WORD def;
	QVector<_dp_value> values;
} _dp_type;
typedef struct _dp_internal {
	QString name;
	QVector<uint32_t> crc32s;
	QVector<_dp_type> types;
} _dp_internal;

// ----------------------------------------------------------------------------------------------

class dlgDipswitch : public QWidget, public Ui::dlgDipswitch {
	Q_OBJECT

	public:
		explicit dlgDipswitch(QWidget *parent = nullptr);
		~dlgDipswitch() override;

	private slots:
		void s_dipswitch(int index);
		void s_default(bool checked);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgDipswitch : public wdgTitleBarDialog {
	public:
		dlgDipswitch *wd;

	public:
		explicit wdgDlgDipswitch(QWidget *parent = nullptr);
		~wdgDlgDipswitch() override;
};

#endif /* DLGDIPSWITCH_HPP_ */
