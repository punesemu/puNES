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

#ifndef DLGUNCOMP_HPP_
#define DLGUNCOMP_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgUncomp.hh"

class dlgUncomp : public QDialog, public Ui::Uncompress_selection {
		Q_OBJECT

	private:
		int selected;

	public:
		dlgUncomp(QWidget *parent);
		~dlgUncomp();

	private:
		void closeEvent(QCloseEvent *e);

	private slots:
		void s_ok_clicked(bool checked);
		void s_cancel_clicked(bool checked);
};

#endif /* DLGUNCOMP_HPP_ */
