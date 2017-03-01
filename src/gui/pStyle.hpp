/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#ifndef PSTYLE_HPP_
#define PSTYLE_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QProxyStyle>
#else
#include <QtWidgets/QProxyStyle>
#endif

class pStyle: public QProxyStyle {
		Q_OBJECT

	public:
		bool newMenuMenagement;
		bool newMenuAllowActiveAndDisabled;

	public:
		pStyle();
		~pStyle();

		int styleHint(StyleHint hint, const QStyleOption* opt, const QWidget* widget,
				QStyleHintReturn* returnData) const;
};

#endif /* PSTYLE_HPP_ */
