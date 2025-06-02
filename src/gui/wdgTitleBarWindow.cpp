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

#include <QtGui/QWindow>
#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QScreen>
#include <QtWidgets/QStyleOption>
#include "wdgTitleBarWindow.hpp"
#include "mainWindow.hpp"
#include "gui.h"

#ifndef EV_GLOBAL_MACRO
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define EV_GLOBAL_X(event) event->globalX()
#define EV_GLOBAL_Y(event) event->globalY()
#define EV_GLOBAL_POS(event) event->globalPos()
#else
#define EV_GLOBAL_X(event) event->globalPosition().toPoint().x()
#define EV_GLOBAL_Y(event) event->globalPosition().toPoint().y()
#define EV_GLOBAL_POS(event) event->globalPosition().toPoint()
#endif
#define EV_GLOBAL_MACRO
#endif

// ----------------------------------------------------------------------------------------------

hoverWatcher::hoverWatcher(QWidget *parent) : QWidget(parent) {
	setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
	installEventFilter(this);
}
hoverWatcher::~hoverWatcher() = default;

bool hoverWatcher::eventFilter(UNUSED(QObject *obj), QEvent *event) {
	switch (event->type()) {
		case QEvent::Enter:
		case QEvent::HoverEnter:
			emit et_entered();
			break;
		default:
			break;
	}
	return false;
}
void hoverWatcher::paintEvent(QPaintEvent *event) {
	QStyleOption opt;
	QPainter p(this);

	opt.initFrom(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}

QSize hoverWatcher::sizeHint(void) const {
	return (QSize(0, 0));
}
QSize hoverWatcher::minimumSizeHint(void) const {
	return (sizeHint());
}

// ----------------------------------------------------------------------------------------------

wdgTitleBar::wdgTitleBar(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	is_maximized = false;
	stylesheet_update();

	setAttribute(Qt::WA_StyledBackground, true);

	set_buttons(barButton::Fullscreen | barButton::Minimize | barButton::Maximize | barButton::Close);

	connect(pushButton_fullscreen, SIGNAL(clicked()), this, SIGNAL(et_fullscreen()));
	connect(pushButton_minimize, SIGNAL(clicked()), this, SIGNAL(et_minimize()));
	connect(pushButton_maximize, SIGNAL(clicked()), this, SIGNAL(et_maximize()));
	connect(pushButton_close, SIGNAL(clicked()), this, SIGNAL(et_close()));

	connect(this, SIGNAL(windowTitleChanged(QString)), label_title, SLOT(setText(QString)));
	connect(this, SIGNAL(windowIconChanged(QIcon)), this, SLOT(s_window_icon_changed(QIcon)));
}
wdgTitleBar::~wdgTitleBar() = default;

void wdgTitleBar::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else if (event->type() == QEvent::PaletteChange) {
		stylesheet_update();
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgTitleBar::paintEvent(QPaintEvent *event){
	QStyleOption opt;
	QPainter p(this);
	QRect r = rect();

	opt.initFrom(this);
	r.adjust(1, 0, -1, 0);
	opt.rect = r;
	style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &opt, &p, this);
	QWidget::paintEvent(event);
}
void wdgTitleBar::mouseMoveEvent(QMouseEvent *event){
	if (event->buttons() & Qt::LeftButton) {
		emit et_change_window_position(EV_GLOBAL_POS(event));
	}
	QWidget::mouseMoveEvent(event);
}
void wdgTitleBar::mousePressEvent(QMouseEvent *event){
	if (event->button() & Qt::LeftButton) {
		emit et_start_window_move(EV_GLOBAL_POS(event));
	}
	QWidget::mousePressEvent(event);
}
void wdgTitleBar::mouseReleaseEvent(QMouseEvent *event){
	if (event->button() & Qt::LeftButton) {
		emit et_stop_window_move();
	}
	QWidget::mouseReleaseEvent(event);
}
void wdgTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
	if ((usedButtons & barButton::Maximize) &&
		pushButton_maximize->isEnabled() &&
		(event->buttons() & Qt::LeftButton)) {
		emit et_maximize();
	}
	QWidget::mouseDoubleClickEvent(event);
}

void wdgTitleBar::stylesheet_update(void) const {
	if (theme::is_dark_theme()) {
		pushButton_fullscreen->setIcon(QIcon(":/icon/icons/fullscreen_white.svgz"));
		pushButton_minimize->setIcon(QIcon(":/icon/icons/minimize_white.svgz"));
		pushButton_close->setIcon(QIcon(":/icon/icons/close_white.svgz"));
	} else {
		pushButton_fullscreen->setIcon(QIcon(":/icon/icons/fullscreen_black.svgz"));
		pushButton_minimize->setIcon(QIcon(":/icon/icons/minimize_black.svgz"));
		pushButton_close->setIcon(QIcon(":/icon/icons/close_black.svgz"));
	}
	set_maximized_button_icon();
}

void wdgTitleBar::set_maximized_button_icon(void) const {
	if (theme::is_dark_theme()) {
		pushButton_maximize->setIcon(QIcon(is_maximized
			? ":/icon/icons/maximize_minimize_white.svgz"
			: ":/icon/icons/maximize_white.svgz"));
	} else {
		pushButton_maximize->setIcon(QIcon(is_maximized
			? ":/icon/icons/maximize_minimize_black.svgz"
			: ":/icon/icons/maximize_black.svgz"));
	}
}
void wdgTitleBar::set_buttons(const barButtons buttons) {
	usedButtons = buttons;
	pushButton_fullscreen->setVisible(buttons & barButton::Fullscreen);
	pushButton_minimize->setVisible(buttons & barButton::Minimize);
	pushButton_maximize->setVisible(buttons & barButton::Maximize);
	pushButton_close->setVisible(buttons & barButton::Close);
}
void wdgTitleBar::set_button_text(const barButton button, const QString &text) const {
	switch (button) {
		case barButton::Fullscreen:
			pushButton_fullscreen->setText(text);
			break;
		case barButton::Minimize:
			pushButton_minimize->setText(text);
			break;
		case barButton::Maximize:
			pushButton_maximize->setText(text);
			break;
		default:
		case barButton::Close:
			pushButton_close->setText(text);
			break;
	}
}
void wdgTitleBar::set_button_enabled(const barButton button, const bool enabled) const {
	switch (button) {
		case barButton::Fullscreen:
			pushButton_fullscreen->setEnabled(enabled);
			break;
		case barButton::Minimize:
			pushButton_minimize->setEnabled(enabled);
			break;
		case barButton::Maximize:
			pushButton_maximize->setEnabled(enabled);
			break;
		default:
		case barButton::Close:
			pushButton_close->setEnabled(enabled);
			break;
	}
}

void wdgTitleBar::s_window_icon_changed(const QIcon &icon) const {
	label_icon->setPixmap(icon.pixmap(label_icon->size()));
	label_icon->setVisible(!icon.isNull());
}

// ----------------------------------------------------------------------------------------------

wdgTitleBarStatus::wdgTitleBarStatus(QWidget *parent) : QStatusBar(parent) {
	setAttribute(Qt::WA_StyledBackground, true);
}
wdgTitleBarStatus::~wdgTitleBarStatus() = default;

void wdgTitleBarStatus::paintEvent(UNUSED(QPaintEvent *event)) {
	QPainter painter(this);
	QStyleOption opt;
	QRectF r = rect();
	QColor background;
	QColor border;

	if (theme::is_dark_theme()) {
		background = palette().color(QPalette::Dark);
		border = palette().color(QPalette::Light);
	} else {
		background = palette().color(QPalette::Light);
		border = palette().color(QPalette::Dark);
	}
	opt.initFrom(this);
	painter.setRenderHint(QPainter::Antialiasing);
	// background
	painter.fillRect(r, background);
	// bordo
	painter.setPen(QPen(border, 1));
	painter.drawLine(r.left(), r.top(), r.right(), r.top());
	// contenuto della statusbar
	painter.save();
	// imposto un clip per assicurarMi che il contenuto non esca dai bordi arrotondati
	painter.setClipRect(r);
	// gli elementi della statusbar
	for (QObject *child : children()) {
		if (QWidget *widget = qobject_cast<QWidget*>(child)) {
			if (widget->isVisible()) {
				painter.save();
				painter.translate(widget->pos());
				widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
				painter.restore();
			}
		}
	}
	painter.restore();
}

// ----------------------------------------------------------------------------------------------

wdgTitleBarWindow::wdgTitleBarWindow(QWidget *parent, Qt::WindowType window_type) : QWidget(parent) {
	setupUi(this);

	// if (!gfx.wayland.enabled) {
	// 	setWindowFlags(window_type);
	// 	return;
	// }

	//setAttribute(Qt::WA_NativeWindow);
	// abilita la trasparenza della finestra
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags(window_type | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint);
	setMouseTracking(true);

	border_color = palette().color(QPalette::Window);
	private_hover_watcher = new hoverWatcher(this);
	private_main_window = new QMainWindow();
	private_main_window->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
	private_main_window->setMaximumSize(0, 0);
	title_bar = new wdgTitleBar(this);
	title_bar->setWindowIcon(windowIcon());
	title_bar->setWindowTitle(windowTitle());
	status_bar = new wdgTitleBarStatus(this);
	status_bar->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
	status_bar->setSizeGripEnabled(true);
	size_grip = status_bar->findChild<QSizeGrip *>();

	if (size_grip) {
		// Assicuro che il sizegrip sia visibile
		size_grip->setVisible(true);
		// Imposto un tooltip per il sizegrip
		//size_grip->setToolTip(tr("Drag to resize the window"));
	}

	verticalLayout->addWidget(private_hover_watcher);
	private_layout = new QVBoxLayout(private_hover_watcher);
	private_layout->setContentsMargins(0, 0, 0, 0);
	private_layout->setSizeConstraint(QLayout::SetDefaultConstraint);
	private_layout->setSpacing(4);
	private_layout->addWidget(title_bar);
	private_layout->addWidget(private_main_window);
	verticalLayout->addWidget(private_hover_watcher);
	verticalLayout->addWidget(status_bar);
	private_hover_watcher->setLayout(private_layout);
	private_hover_watcher->setFixedHeight(private_hover_watcher->height());

	connect(private_hover_watcher, SIGNAL(et_entered(void)), this, SLOT(s_hover_watcher_entered(void)));
	connect(this, &QMainWindow::windowIconChanged, title_bar, &QWidget::setWindowIcon);
	connect(this, SIGNAL(windowTitleChanged(QString)), title_bar, SLOT(setWindowTitle(QString)));

	connect(title_bar, SIGNAL(et_minimize(void)), this, SLOT(showMinimized(void)));
	connect(title_bar, SIGNAL(et_maximize(void)), this, SLOT(s_title_bar_maximize(void)));
	connect(title_bar, SIGNAL(et_close(void)), this, SLOT(close(void)));
	connect(title_bar, SIGNAL(et_start_window_move(QPoint)), this, SLOT(s_title_bar_start_window_move(QPoint)));
	connect(title_bar, SIGNAL(et_stop_window_move(void)), this, SLOT(s_title_bar_stop_window_move(void)));
	connect(title_bar, SIGNAL(et_change_window_position(QPoint)), this, SLOT(s_title_bar_change_window_position(QPoint)));

	installEventFilter(this);

	// applico la maschera arrotondata dopo che la finestra è stata mostrata
	QTimer::singleShot(0, this, &wdgTitleBarWindow::apply_rounded_mask);
	update_size_grip_visibility();
}
wdgTitleBarWindow::~wdgTitleBarWindow() = default;

bool wdgTitleBarWindow::eventFilter(UNUSED(QObject *obj), QEvent *event) {
	switch (event->type()) {
		case QEvent::Leave:
		case QEvent::HoverLeave:
			if (!(operation_type & OperationType::CUSTOM_RESIZE)) {
				operation_type = OperationType::NONE;
				unsetCursor();
			}
			break;
		case QEvent::MouseMove:
			if (operation_type & OperationType::CUSTOM_RESIZE) {
				customMouseMoveEvent(static_cast<QMouseEvent*>(event));
			} else if (!is_moving()) {
				redefine_cursor(EV_GLOBAL_POS(static_cast<QMouseEvent*>(event)));
			}
			break;
		case QEvent::Resize:
			// riapplico la maschera quando la finestra viene ridimensionata
			QTimer::singleShot(0, this, &wdgTitleBarWindow::apply_rounded_mask);
			break;
		default:
			break;
	}
	return (false);
}
void wdgTitleBarWindow::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgTitleBarWindow::closeEvent(QCloseEvent *event) {
	if (loop_in_exec) {
		emit et_quit_loop();
		event->ignore();
	} else {
		QWidget::closeEvent(event);
	}
}
void wdgTitleBarWindow::paintEvent(QPaintEvent *event) {
	QStyleOption opt;
	QPainter p(this);

	opt.initFrom(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	if (title_bar != nullptr) {
		QPainterPath path = path_rounded();

		p.setRenderHint(QPainter::Antialiasing);
		p.fillPath(path, palette().color(QPalette::Window));
		p.setPen(QPen(border_color, 1));
		p.drawPath(path);
	}

	QWidget::paintEvent(event);
}
void wdgTitleBarWindow::mousePressEvent(QMouseEvent *event) {
	redefine_cursor(EV_GLOBAL_POS(event));
	if ((event->button() & Qt::LeftButton) && edges) {
		if (!force_custom_resize && start_system_resize()) {
			operation_type = OperationType::SYSTEM_RESIZE;
		} else {
			QPoint position = EV_GLOBAL_POS(event);

			if (edges & Qt::TopEdge) {
				position.ry() -= y();
			}
			if (edges & Qt::LeftEdge) {
				position.rx() -= x();
			}
			if (edges & Qt::RightEdge) {
				position.rx() -= (x() + width());
			}
			if (edges & Qt::BottomEdge) {
				position.ry() -= (y() + height());
			}
			operation_type = OperationType::CUSTOM_RESIZE;
		}
	}
	QWidget::mousePressEvent(event);
}
void wdgTitleBarWindow::mouseReleaseEvent(QMouseEvent *event) {
	operation_type = OperationType::NONE;
	redefine_cursor(EV_GLOBAL_POS(event));
	QWidget::mouseReleaseEvent(event);
}
void wdgTitleBarWindow::customMouseMoveEvent(QMouseEvent *event) {
	QPoint tl = geometry().topLeft(), br = geometry().bottomRight();
	int gx = EV_GLOBAL_X(event), gy = EV_GLOBAL_Y(event);
	bool crh = br.x() - tl.x() > minimumWidth();
	bool crv = br.y() - tl.y() > minimumHeight();

	if ((edges & Qt::TopEdge) && (crv || (gy < tl.y()))) {
		tl.ry() = gy;
	}
	if ((edges & Qt::BottomEdge) && (crv || (gy > br.y()))) {
		br.ry() = gy;
	}
	if ((edges & Qt::LeftEdge) && (crh || (gx < tl.x()))) {
		tl.rx() = gx;
	}
	if ((edges & Qt::RightEdge) && (crh || (gx > br.x()))) {
		br.rx() = gx;
	}
	setGeometry(QRect(tl, br));
}

void wdgTitleBarWindow::set_gui_visible(const bool mode) const {
	private_hover_watcher->setVisible(mode);
	if (!disabled_resize) {
		status_bar->setVisible(mode);
	}
}
void wdgTitleBarWindow::init_geom_variable(const _last_geometry lg) {
	geom.setRect(lg.x, lg.y, lg.w, lg.h);
	set_geometry();
}
void wdgTitleBarWindow::set_geometry(void) {
	if (geom == QRect(0, 0, 0, 0)) {
		QPoint center = mainwin->geometry().center();

		center -= QPoint(size().width() / 2, size().height() / 2);
		geom.setRect(center.x(), center.y(), sizeHint().width(), sizeHint().height());
	}
	if (gfx.wayland.enabled) {
		resize(geom.width(), geom.height());
	} else {
		setGeometry(geom);
	}
}
void wdgTitleBarWindow::is_in_desktop(int *x, int *y) {
	QList<QScreen *> screens = QGuiApplication::screens();
	int i, x_min = 0, x_max = 0, y_min = 0, y_max = 0;

	for (i = 0; i < screens.count(); i++) {
		QRect g = screens[i]->availableGeometry();

		if (g.x() < x_min) {
			x_min = g.x();
		}
		if ((g.x() + g.width()) > x_max) {
			x_max = g.x() + g.width();
		}
		if (g.y() < y_min) {
			y_min = g.y();
		}
		if ((g.y() + g.height()) > y_max) {
			y_max = g.y() + g.height();
		}
	}
	if (((*x) == 0) || ((*x) < x_min) || ((*x) > x_max)) {
		(*x) = 80;
	}
	if (((*y) == 0) || ((*y) < y_min) || ((*y) > y_max)) {
		(*y) = 80;
	}
}
dialogExitCode wdgTitleBarWindow::exec(void) {
	QEventLoop loop;

	connect(this, SIGNAL(et_quit_loop(void)), &loop, SLOT(quit(void)));
	loop_in_exec = TRUE;
	show();
	loop.exec();
	loop_in_exec = FALSE;
	hide();
	QTimer::singleShot(0, this, &wdgTitleBarWindow::close);
	return dialog_exit_code;
}

void wdgTitleBarWindow::set_border_color(const QColor color) {
	border_color = color;
}
void wdgTitleBarWindow::add_widget(QWidget *widget) {
	const int w = widget->size().width() + layout()->contentsMargins().left() + layout()->contentsMargins().right();
	const QSize new_size(qMax(size().width(), w), widget->height() + size().height());

	verticalLayout->insertWidget(verticalLayout->count() - 1, widget);
	resize(new_size);
	private_widget = widget;
	update_track_mouse();
}
void wdgTitleBarWindow::set_buttons(const barButtons buttons) const {
	if (title_bar == nullptr) {
		Qt::WindowFlags flags = windowFlags();

		if (buttons & barButton::Minimize) {
			flags |= Qt::WindowMinimizeButtonHint;
		} else {
			flags &= ~Qt::WindowMinimizeButtonHint;
		}
		if (buttons & barButton::Maximize) {
			flags |= Qt::WindowMaximizeButtonHint;
		} else {
			flags &= ~Qt::WindowMaximizeButtonHint;
		}
		if (buttons & barButton::Close) {
			flags |= Qt::WindowCloseButtonHint;
		} else {
			flags &= ~Qt::WindowCloseButtonHint;
		}
	} else {
		title_bar->set_buttons(buttons);
	}
}
void wdgTitleBarWindow::set_force_custom_move(const bool force) {
	force_custom_move = force;
}
void wdgTitleBarWindow::set_force_custom_resize(const bool force) {
	force_custom_resize = force;
}
void wdgTitleBarWindow::set_permit_resize(const bool mode) {
	disabled_resize = !mode;
	if (disabled_resize && (title_bar == nullptr)) {
		layout()->setSizeConstraint(QLayout::SetFixedSize);
	}
	update_size_grip_visibility();
	// mi serve per la corretta gestione del redefine_cursor();
	update_track_mouse();
}

void wdgTitleBarWindow::update_track_mouse(void) const {
	// mi serve per la corretta gestione del redefine_cursor();
	if (private_widget) {
		private_widget->setMouseTracking(hasMouseTracking() & !disabled_resize);
	}
}
void wdgTitleBarWindow::update_size_grip_visibility(void) const {
	if (size_grip) {
		size_grip->setVisible(!disabled_resize);
	}
	if (status_bar) {
		bool other_widgets = false;

		for (QObject* child : status_bar->children()) {
			QWidget* widget = qobject_cast<QWidget*>(child);

			if (widget && (widget != size_grip) && widget->isVisible()) {
				other_widgets = true;
				break;
			}
		}
		status_bar->setVisible(other_widgets || !disabled_resize);
	}
}
QPainterPath wdgTitleBarWindow::path_rounded(void) const {
	const QRectF r = rect();
	constexpr qreal radius = 4;
	QPainterPath path;

	path.moveTo(r.left() + radius, r.top());
	path.arcTo(r.left(), r.top(), radius * 2, radius * 2, 90, 90);
	path.lineTo(r.left(), r.bottom());
	path.lineTo(r.right(), r.bottom());
	path.lineTo(r.right(), r.top() + radius);
	path.arcTo(r.right() - radius * 2, r.top(), radius * 2, radius * 2, 0, 90);
	path.closeSubpath();
	return (path);
}
void wdgTitleBarWindow::apply_rounded_mask(void) {
	const QPainterPath path = path_rounded();
	const QRegion region(path.toFillPolygon().toPolygon());

	setMask(region);
}
bool wdgTitleBarWindow::is_moving(void) const {
	return (operation_type & (OperationType::CUSTOM_MOVE | OperationType::SYSTEM_MOVE));
}
bool wdgTitleBarWindow::is_resizing(void) const {
	return (operation_type & (OperationType::CUSTOM_RESIZE | OperationType::SYSTEM_RESIZE));
}
void wdgTitleBarWindow::redefine_cursor(const QPoint &pos) {
	if (!disabled_resize) {
		const int x = pos.x() - this->x(), y = pos.y() - this->y();
		const int bottom = height() - resize_threshold;
		const int right = width() - resize_threshold;
		Qt::Edges new_edges = {};

		if (x < resize_threshold) {
			new_edges |= Qt::LeftEdge;
		}
		if (y < resize_threshold) {
			new_edges |= Qt::TopEdge;
		}
		if (x > right) {
			new_edges |= Qt::RightEdge;
		}
		if (y > bottom) {
			new_edges |= Qt::BottomEdge;
		}
		switch (new_edges) {
			case Qt::LeftEdge:
			case Qt::RightEdge:
				setCursor(QCursor(Qt::SizeHorCursor));
				break;
			case Qt::TopEdge:
			case Qt::BottomEdge:
				setCursor(QCursor(Qt::SizeVerCursor));
				break;
			case Qt::TopEdge | Qt::LeftEdge:
			case Qt::BottomEdge | Qt::RightEdge:
				setCursor(QCursor(Qt::SizeFDiagCursor));
				break;
			case Qt::TopEdge | Qt::RightEdge:
			case Qt::BottomEdge | Qt::LeftEdge:
				setCursor(QCursor(Qt::SizeBDiagCursor));
				break;
			default:
				unsetCursor();
				break;
		}
		edges = new_edges;
	}
}
bool wdgTitleBarWindow::start_system_move(void) const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	return (windowHandle()->startSystemMove());
#else
	return false;
#endif
}
bool wdgTitleBarWindow::start_system_resize(void) const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	return (windowHandle()->startSystemResize(edges));
#else
	return false;
#endif
}

void wdgTitleBarWindow::s_accept(void) {
	dialog_exit_code = dialogExitCode::ACCEPTED;
	close();
}
void wdgTitleBarWindow::s_reject(void) {
	dialog_exit_code = dialogExitCode::REJECTED;
	close();
}

void wdgTitleBarWindow::s_hover_watcher_entered(void) {
	if (!(operation_type & OperationType::CUSTOM_RESIZE)) {
		operation_type = OperationType::NONE;
		unsetCursor();
	}
}
void wdgTitleBarWindow::s_title_bar_maximize(void) {
	title_bar->is_maximized = !isMaximized();
	if (isMaximized()) {
		showNormal();
	} else {
		showMaximized();
	}
	title_bar->set_maximized_button_icon();
}
void wdgTitleBarWindow::s_title_bar_start_window_move(const QPoint &start) {
	cursor_position = start - geometry().topLeft();
}
void wdgTitleBarWindow::s_title_bar_stop_window_move(void) {
	if (operation_type & OperationType::CUSTOM_MOVE) {
		unsetCursor();
	}
	operation_type = OperationType::NONE;
}
void wdgTitleBarWindow::s_title_bar_change_window_position(const QPoint &to) {
	if (operation_type & OperationType::CUSTOM_MOVE) {
		move(to - cursor_position);
	} else if (!is_moving()) {
		if (!force_custom_move && start_system_move()) {
			operation_type = OperationType::SYSTEM_MOVE;
		} else {
			operation_type = OperationType::CUSTOM_MOVE;
			setCursor(QCursor(Qt::SizeAllCursor));
		}
	}
}

// ----------------------------------------------------------------------------------------------

wdgTitleBarDialog::wdgTitleBarDialog(QWidget *parent) : wdgTitleBarWindow(parent, Qt::Dialog) { }
wdgTitleBarDialog::~wdgTitleBarDialog() = default;