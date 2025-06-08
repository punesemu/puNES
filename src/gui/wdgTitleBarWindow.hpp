/*
 *  Copyright (C) 2010-2025 Fabio Cavallo (aka FHorse)
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

// Based on the project:
// https://github.com/Nintersoft/QCustomWindow

#ifndef WDGWINDOW_HPP_
#define WDGWINDOW_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QSizeGrip>
#include "ui_wdgTitleBarWindow.h"
#include "ui_wdgTitleBar.h"
#include "conf.h"

enum dialogExitCode {
	ACCEPTED = 0x00,
	REJECTED = 0x01
};
enum barButton {
	Fullscreen = 0x01,
	Minimize = 0x02,
	Maximize = 0x04,
	Close = 0x08
};
Q_DECLARE_FLAGS(barButtons, barButton)
Q_DECLARE_OPERATORS_FOR_FLAGS(barButtons)

// ----------------------------------------------------------------------------------------------

class hoverWatcher final : public QWidget{
	Q_OBJECT

	signals:
		void et_entered();

	public:
		explicit hoverWatcher(QWidget *parent = nullptr);
		~hoverWatcher() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void paintEvent(QPaintEvent *event) override;

	public:
		QSize sizeHint(void) const override;
		QSize minimumSizeHint(void) const override;
};

// ----------------------------------------------------------------------------------------------

class wdgTitleBar final : public QWidget, public Ui::wdgTitleBar  {
	Q_OBJECT

	private:
		barButtons usedButtons;

	public:
		bool is_maximized;

	signals:
		void et_fullscreen(void);
		void et_minimize(void);
		void et_maximize(void);
		void et_close(void);
		void et_stop_window_move(void);
		void et_start_window_move(const QPoint &start);
		void et_change_window_position(const QPoint &to);

	public:
		explicit wdgTitleBar(QWidget *parent = nullptr);
		~wdgTitleBar() override;

	protected:
		void changeEvent(QEvent *event) override;
		void paintEvent(QPaintEvent *event) override;
		void mouseMoveEvent(QMouseEvent *event) override;
		void mousePressEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;
		void mouseDoubleClickEvent(QMouseEvent *event) override;

	private:
		void stylesheet_update(void) const;

	public:
		void set_maximized_button_icon(void) const;
		void set_buttons(barButtons buttons);
		void set_button_text(barButton button, const QString &text) const;
		void set_button_enabled(barButton button, bool enabled) const;

	private slots:
		void s_window_icon_changed(const QIcon &icon) const;
};

// ----------------------------------------------------------------------------------------------

class wdgTitleBarStatus final : public QStatusBar {
	Q_OBJECT

	public:
		explicit wdgTitleBarStatus(QWidget *parent = nullptr);
		~wdgTitleBarStatus() override;

	protected:
		void paintEvent(QPaintEvent *event) override;
};

// ----------------------------------------------------------------------------------------------

class wdgTitleBarWindow : public QWidget, public Ui::wdgTitleBarWindow {
	Q_OBJECT

	public:
		enum OperationType {
			NONE = 0x00,
			CUSTOM_MOVE = 0x01,
			CUSTOM_RESIZE = 0x02,
			SYSTEM_MOVE = 0x04,
			SYSTEM_RESIZE = 0x08,
		};
		Q_DECLARE_FLAGS(OperationTypes, OperationType)

	protected:
		QVBoxLayout *private_layout = nullptr;
		wdgTitleBar *title_bar = nullptr;
		wdgTitleBarStatus *status_bar = nullptr;
		QMainWindow *private_main_window = nullptr;
		hoverWatcher *private_hover_watcher = nullptr;
		QWidget *private_widget = nullptr;
		QSizeGrip *size_grip = nullptr;
		dialogExitCode dialog_exit_code = REJECTED;
		OperationType operation_type = NONE;
		QColor border_color;
		QPoint cursor_position;
		Qt::Edges edges;
		QMargins layout_margins;
		bool force_custom_move = false;
		bool force_custom_resize = false;
		bool enable_custom_events = true;
		bool disabled_resize = false;
		bool loop_in_exec = false;
		bool wm_disabled = false;
		int resize_threshold = 4;

	public:
		QRect geom = QRect(0, 0, 0, 0);

	signals:
		void et_quit_loop(void);

	public:
		explicit wdgTitleBarWindow(QWidget *parent = nullptr, Qt::WindowType window_type = Qt::Window);
		~wdgTitleBarWindow() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void closeEvent(QCloseEvent *event) override;
		void changeEvent(QEvent *event) override;
		void paintEvent(QPaintEvent *event) override;
		void mousePressEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;
		virtual void customMouseMoveEvent(QMouseEvent* event);

	public:
		void init_fullscreen(bool mode) const;
		void set_gui_visible(bool mode) const;
		void init_geom_variable(_last_geometry lg);
		void set_geometry(void);
		static void is_in_desktop(int *x, int *y);
		dialogExitCode exec(void);

	protected:
		void set_border_color(QColor color);
		void add_widget(QWidget *widget);
		void set_buttons(barButtons buttons);
		void set_force_custom_move(bool force);
		void set_force_custom_resize(bool force);
		void set_permit_resize(bool mode);

	private:
		void update_track_mouse(void) const;
		void update_size_grip_visibility(void) const;
		QPainterPath path_rounded(void) const;
		void apply_rounded_mask(void);
		bool is_moving(void) const;
		bool is_resizing(void) const;
		void redefine_cursor(const QPoint &pos);
		bool start_system_move(void) const;
		bool start_system_resize(void) const;

	public slots:
		void s_accept(void);
		void s_reject(void);

	private slots:
		void s_hover_watcher_entered(void);
		void s_title_bar_maximize(void);
		void s_title_bar_start_window_move(const QPoint &start);
		void s_title_bar_stop_window_move(void);
		void s_title_bar_change_window_position(const QPoint &to);
};

// ----------------------------------------------------------------------------------------------

class wdgTitleBarDialog : public wdgTitleBarWindow {
	public:
		explicit wdgTitleBarDialog(QWidget *parent = nullptr);
		~wdgTitleBarDialog() override;
};

#endif /* WDGWINDOW_HPP_ */
