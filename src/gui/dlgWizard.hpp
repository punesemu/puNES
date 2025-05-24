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

#ifndef DLGWIZARD_HPP_
#define DLGWIZARD_HPP_

#include <QtWidgets/QButtonGroup>
#include "ui_dlgWizard.h"
#include "wdgTitleBarWindow.hpp"
#include "common.h"

// ----------------------------------------------------------------------------------------------

class dlgWizard : public QWidget, public Ui::dlgWizard {
	Q_OBJECT

	private:
		QButtonGroup *grp;
		QString cfg_folder;
		QString app_folder;

	public:
		explicit dlgWizard(QWidget *parent = nullptr, const QString &config_folder = "", const QString &application_folder = "");
		~dlgWizard() override;

	private:
		void showEvent(QShowEvent *event) override;

	private slots:
		void s_grp_storage_type(QAbstractButton *button);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgWizard : public wdgTitleBarDialog {
	Q_OBJECT

	public:
		dlgWizard *wd;

	public:
		explicit wdgDlgWizard(QWidget *parent = nullptr, const QString &config_folder = "", const QString &application_folder = "");
		~wdgDlgWizard() override;
};

#endif /* DLGWIZARD_HPP_ */
