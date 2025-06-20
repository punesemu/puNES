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

#ifndef RECENTFILES_H_
#define RECENTFILES_H_

#include <QtCore/QString>
#include "common.h"

enum recent_files_misc {
	RECENT_FILES_MAX = 15,
};

class recentFiles {
	private:
		QString file_name;
		struct _recent_list {
			int count{};
			QString item[RECENT_FILES_MAX];
			QString current;
		} list;

	public:
		recentFiles() = default;
		~recentFiles() = default;

	public:
		void init(const QString &recent_file_name);
		void add(uTCHAR *file);
		void parse(void);
		void save(void);
		int count(void) const;
		const char *item(int index) const;
		int item_size(int index) const;
		const char *current(void) const;
		int current_size(void) const;

	private:
		void clear(void);
};

#endif /* RECENTFILES_H_ */
