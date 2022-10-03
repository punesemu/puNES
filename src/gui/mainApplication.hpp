/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef MAINAPPLICATION_HPP_
#define MAINAPPLICATION_HPP_

#include <QtCore/QDir>
#include "extra/singleapplication/singleapplication.h"
#include "common.h"

class mainApplication : public SingleApplication {
	Q_OBJECT

	public:
		mainApplication(int &argc, char *argv[], bool allowSecondary = false, Options options = Mode::User,
			int timeout = 1000, const QString &userData = {});
		~mainApplication();

	public:
		bool notify(QObject *receiver, QEvent *event);

	public:
		BYTE base_folder(QDir *new_folder, QDir *old_folder, QString base, QString message);
		BYTE control_base_folders(void);

	private:
		QKeySequence key_sequence_from_key_event(QKeyEvent *event);
		bool is_set_inp_shortcut(QEvent *event, int set_inp);
		bool dlgkeyb_event(QEvent *event);
		bool shortcut_override_event(QEvent *event);
		bool key_release_event(QEvent *event);
};

#endif /* MAINAPPLICATION_HPP_ */
