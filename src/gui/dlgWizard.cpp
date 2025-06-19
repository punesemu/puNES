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

#include "dlgWizard.hpp"
#include "info.h"

// ----------------------------------------------------------------------------------------------

wdgDlgWizard::wdgDlgWizard(QWidget *parent, const QString &config_folder, const QString &application_folder) : wdgTitleBarDialog(parent) {
	setAttribute(Qt::WA_DeleteOnClose);
	wd = new dlgWizard(this, config_folder, application_folder);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/preferences_other.svgz"));
	set_border_color("fuchsia");
	set_buttons(barButton::Close);
	set_permit_resize(false);
	add_widget(wd);

	connect(wd->pushButton_Start, SIGNAL(clicked(bool)), this, SLOT(s_accept(void)));
}
wdgDlgWizard::~wdgDlgWizard() = default;

// ----------------------------------------------------------------------------------------------

dlgWizard::dlgWizard(QWidget *parent, const QString &config_folder, const QString &application_folder) : QWidget(parent) {
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
}
dlgWizard::~dlgWizard() = default;

void dlgWizard::showEvent(UNUSED(QShowEvent *event)) {
	radioButton_Data_Storage_Portable->click();
	pushButton_Start->setFocus();
	QWidget::showEvent(event);
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
