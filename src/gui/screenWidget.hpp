/*
 * screenWidget.hpp
 *
 *  Created on: 22/ott/2014
 *      Author: fhorse
 */

#ifndef SCREENWIDGET_HPP_
#define SCREENWIDGET_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include "mainWindow.hpp"
#include "gui.h"

class screenWidget: public QWidget {
		Q_OBJECT

	private:
		mainWindow *mwin;
#if defined (SDL) && defined (__WIN32__)
		struct _data {
			LONG_PTR WINAPI (*qt)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			LONG_PTR WINAPI (*sdl)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			LONG_PTR WINAPI (*tmp)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		} data;
#endif

	public:
		screenWidget(QWidget *parent, mainWindow *mw);
		~screenWidget();
#if defined (SDL) && defined (__WIN32__)
		void controlEventFilter();
#endif

	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);
		bool eventFilter(QObject *obj, QEvent *event);
};

#endif /* SCREENWIDGET_HPP_ */
