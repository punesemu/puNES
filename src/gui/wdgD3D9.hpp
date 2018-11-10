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

#ifndef WDGD3D9_HPP_
#define WDGD3D9_HPP_

#include <QtWidgets/QWidget>
#include <QtCore/QElapsedTimer>
#include <QtGui/QPaintEvent>
#include <QtGui/QPaintEngine>

class wdgD3D9 : public QWidget {
	private:
		struct _gui_fps {
			double count;
			double frequency;
			QElapsedTimer timer;
		} gfps;

	public:
		wdgD3D9(QWidget *parent);
		~wdgD3D9();

	protected:
		void paintEvent(QPaintEvent *event);

	protected:
		QPaintEngine *paintEngine() const { return 0; }
};

#endif /* WDGD3D9_HPP_ */
