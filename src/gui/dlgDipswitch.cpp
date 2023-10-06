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

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include "dlgDipswitch.hpp"
#include "dipswitch.h"
#include "conf.h"
#include "settings.h"

extern _dp_internal dp;

dlgDipswitch::dlgDipswitch(QWidget *parent) : QDialog(parent) {
	setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);

	for (int i = 0; i < dp.types.length(); i++) {
		QLabel *lb = new QLabel(this);
		QComboBox *cb = new QComboBox(this);
		int def = 0;

		lb->setObjectName(QString::fromUtf8("label_dipswitch_%1").arg(i));
		lb->setText(dp.types.at(i).name);
		lb->setProperty("myIndex", QVariant(i));

		cb->setObjectName(QString::fromUtf8("comboBox_dipswitch_%1").arg(i));
		cb->setProperty("myIndex", QVariant(i));
		for (int index = 0; index < dp.types.at(i).values.length(); index++) {
			const QString desc = dp.types.at(i).values.at(index).name;
			const unsigned int value = dp.types.at(i).values.at(index).value;

			if (dp.types.at(i).def == value) {
				cb->addItem(desc + "(*)", QVariant(value));
			} else {
				cb->addItem(desc, QVariant(value));
			}
			if (cfg->dipswitch == -1) {
				if (dp.types.at(i).def == value) {
					def = index;
				}
			} else if ((dipswitch.value & dp.types.at(i).mask) == value) {
				def = index;
			}
		}
		connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(s_dipswitch(int)));
		cb->setCurrentIndex(def);

		formLayout_Dipswitch->setWidget(i, QFormLayout::LabelRole, lb);
		formLayout_Dipswitch->setWidget(i, QFormLayout::FieldRole, cb);
	}
	connect(pushButton_Start, SIGNAL(clicked(bool)), this, SLOT(s_start(bool)));
	connect(pushButton_Default, SIGNAL(clicked(bool)), this, SLOT(s_default(bool)));

	if ((info.reset != CHANGE_ROM) && (info.reset != POWER_UP)) {
		pushButton_Start->setText(tr("Ok"));
	}
}
dlgDipswitch::~dlgDipswitch() = default;

void dlgDipswitch::s_dipswitch(int index) {
	const int type = QVariant((dynamic_cast<QComboBox *>(sender()))->property("myIndex")).toInt();
	const int value = QVariant(((QComboBox *)sender())->itemData(index)).toInt();
	const int mask = dp.types.at(type).mask;

	dipswitch.value = (dipswitch.value & ~mask) | value;
	cfg->dipswitch = dipswitch.value;
	settings_pgs_save();
}
void dlgDipswitch::s_start(UNUSED(bool checked)) {
	close();
}
void dlgDipswitch::s_default(UNUSED(bool checked)) {
	for (int i = 0; i < dp.types.length(); i++) {
		QComboBox *cb = findChild<QComboBox *>(QString("comboBox_dipswitch_%0").arg(i));

		if (cb) {
			int def = 0;

			for (int index = 0; index < dp.types.at(i).values.length(); index++) {
				if (dp.types.at(i).def == dp.types.at(i).values.at(index).value) {
					def = index;
				}
			}
			cb->setCurrentIndex(def);
		}
	}
}
