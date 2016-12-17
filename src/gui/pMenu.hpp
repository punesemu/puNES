/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef PMENU_HPP_
#define PMENU_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionMenuItem>
#else
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionMenuItem>
#endif
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

class pMenu: public QMenu {
	Q_OBJECT

	private:
		QAction *lastAction;
		bool newMenagement;
		bool hasCheckableItems;
		bool fromMousePressed;
		bool permitDisabledAction;
		int tabWidth, maxIconWidth;
		mutable QVector<QPoint> actionPoint;

	public:
		pMenu(QWidget *parent);
		~pMenu();

	public:
		void setNewMenagement(bool state);
		void setFromMousePressed(bool state) { fromMousePressed = state; }

	protected:
		void showEvent(QShowEvent *event);
		void hideEvent(QHideEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void paintEvent(QPaintEvent *event);

	private:
		bool execQMenuEvent();
		bool controlMenuParent(QObject *o, const QPoint &p);
		void updateActionRects();
		void initMenuStyleOption(QStyleOptionMenuItem* option, QAction *action,
				QAction *currentAction);
		void resetLastAction();
		void setFirstActionActive();
		void setCurrentAction(QAction *action);
		bool triggerLastAction();
		bool triggerLastActionAndHide();
		void hideToMenubar();
		int getLastVisibleAction();
};

#endif /* PMENU_HPP_ */
