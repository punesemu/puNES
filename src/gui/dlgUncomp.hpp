/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#ifndef DLGUNCOMP_HPP_
#define DLGUNCOMP_HPP_

#include <QtWidgets/QDialog>
#include "ui_dlgUncomp.h"
#include "common.h"
#include "uncompress.h"

class dlgUncomp : public QDialog, public Ui::dlgUncomp {
	Q_OBJECT

	private:
		int selected;

	public:
		explicit dlgUncomp(QWidget *parent = nullptr, void *uncompress_archive = nullptr, BYTE type = UNCOMPRESS_TYPE_ALL);
		~dlgUncomp() override;

	private:
		void closeEvent(QCloseEvent *event) override;

	private slots:
		void s_doubleclick(int row, int column);
		void s_ok_clicked(bool checked);
		void s_none_clicked(bool checked);
};

#endif /* DLGUNCOMP_HPP_ */
