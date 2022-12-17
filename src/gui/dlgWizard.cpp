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

#include "dlgWizard.hpp"
#include "info.h"

dlgWizard::dlgWizard(QWidget *parent, const QString &config_folder, const QString &application_folder) : QDialog(parent) {
	setupUi(this);

	cfg_folder = config_folder;
	app_folder = application_folder;

	{
		grp = new QButtonGroup(this);

		grp->addButton(radioButton_Data_Storage_Portable);
		grp->setId(radioButton_Data_Storage_Portable, 0);
		grp->addButton(radioButton_Data_Storage_User);
		grp->setId(radioButton_Data_Storage_User, 1);

		connect(grp, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(s_grp_storage_type(QAbstractButton*)));
	}
	connect(pushButton_Start, SIGNAL(clicked(bool)), this, SLOT(s_accepted(bool)));

	setAttribute(Qt::WA_DeleteOnClose);
}
dlgWizard::~dlgWizard() = default;

void dlgWizard::showEvent(UNUSED(QShowEvent *event)) {
	radioButton_Data_Storage_Portable->click();
	pushButton_Start->setFocus();
}

void dlgWizard::s_grp_storage_type(UNUSED(QAbstractButton *button)) {
	QString folder;

	switch (grp->checkedId()) {
		default:
		case 0:
			folder = app_folder;
			info.portable = TRUE;
			break;
		case 1:
			folder = cfg_folder;
			info.portable = FALSE;
			break;
	}
	label_Folder->setText(folder);
}
void dlgWizard::s_accepted(UNUSED(bool checked)) {
	accept();
}
