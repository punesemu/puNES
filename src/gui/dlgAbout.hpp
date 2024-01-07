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

#ifndef DLGABOUT_HPP_
#define DLGABOUT_HPP_

#include <QtWidgets/QDialog>
#include "ui_dlgAbout.h"
#include "common.h"

class dlgAbout : public QDialog, public Ui::dlgAbout {
	Q_OBJECT

	public:
		explicit dlgAbout(QWidget *parent = nullptr);
		~dlgAbout() override;

	protected:
		void changeEvent(QEvent *event) override;

	private slots:
		void s_ok_clicked(bool checked);
};

#endif /* DLGABOUT_HPP_ */
