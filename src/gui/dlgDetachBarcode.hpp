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

#ifndef DLGDETACHBARCODE_HPP_
#define DLGDETACHBARCODE_HPP_

#include "ui_dlgDetachBarcode.h"
#include "wdgTitleBarWindow.hpp"

// ----------------------------------------------------------------------------------------------

class dlgDetachBarcode final : public QWidget, public Ui::dlgDetachBarcode {
	Q_OBJECT

	public:
		typedef struct _name_detach_barcode {
			QString name;
			QString code;
		} _name_detach_barcode;
		typedef QList<_name_detach_barcode> name_detach_barcode;

	private:
		int selected;
		name_detach_barcode ndb;

	public:
		QRect geom;

	public:
		explicit dlgDetachBarcode(QWidget *parent = nullptr);
		~dlgDetachBarcode() override;

	private:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void changeEvent(QEvent *event) override;

	public:
		void update_dialog(void) const;
		void change_rom(void);

	private:
		void apply_barcode(void) const;

	private slots:
		void s_barcode_click(QListWidgetItem *item);
		void s_barcode_doubleclick(QListWidgetItem *item);
		void s_apply_clicked(bool checked) const;
};

// ----------------------------------------------------------------------------------------------

class wdgDlgDetachBarcode final : public wdgTitleBarDialog {
	public:
		dlgDetachBarcode *wd;

	public:
		explicit wdgDlgDetachBarcode(QWidget *parent = nullptr);
		~wdgDlgDetachBarcode() override;

	protected:
		void closeEvent(QCloseEvent *event) override;
};

#endif /* DLGDETACHBARCODE_HPP_ */
