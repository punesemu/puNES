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

#include <QtWidgets/QApplication>
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QStyleOptionButton>
#include "theme.hpp"

// ----------------------------------------------------------------------------------------------

QString theme::stylesheet_wdgtitlebarwindow(const bool native_wm_disabled, const QColor &border_color) {
	const QString stylesheet =
		"wdgTitleBarWindow {"\
		"	border: 1px solid %0;"\
		"	border-top-left-radius: 4px;"\
		"	border-top-right-radius: 4px;"\
		"}";

	return (native_wm_disabled ? stylesheet.arg(border_color.name()) : "");
}
QString theme::stylesheet_wdgroupbox(void) {
	const QString stylesheet =
		"themeGroupBox {"\
		"	font-weight: bold;"\
		"}";

	return (stylesheet);
}
QString theme::stylesheet_wdgtoolgroupbox(void) {
	const QColor border_color0 = QApplication::palette().light().color();
	const QColor border_color1 = QApplication::palette().dark().color();
	const QString stylesheet =
		"themeToolGroupBox {"\
		"	border-radius: 10px;"\
		"	border: 1px solid %0;"\
		"	border: 2px groove %1;"\
		"}"\
		"themeToolGroupBox:hover {"\
		"	font-weight: bold;"\
		"}"\
		"themeToolGroupBox::title {"\
		"	subcontrol-origin: margin;"\
		"	subcontrol-position: top center;"\
		"	padding: 0 0px;"\
		"}";

	return (stylesheet
		.arg(border_color0.name())
		.arg(border_color1.name()));
}
QString theme::stylesheet_wdgbutton(void) {
	const QColor base_color = QApplication::palette().dark().color().lighter(is_dark_theme() ? 100 : 120);
	const QColor border_color0 = get_theme_adaptive_color(base_color);
	const QColor border_color1 = border_color0.darker(is_dark_theme() ? 155 : 105);
	const QColor background_normal = border_color0.lighter(is_dark_theme() ? 115 : 190);
	const QColor background_hover = border_color1.lighter(is_dark_theme() ? 190 : 115);
	const QColor background_checked = base_color.lighter(is_dark_theme() ? 80 : 115);
	const QString stylesheet =
		"themePushButton {"\
		"	margin: 0;"\
		"	border: 2px groove %0;"\
		"	border-radius: 4px;"\
		"	padding: 2px;"\
		"	background-color: %2;"\
		"}"\
		"themePushButton:disabled {"\
		"	color: gray;"\
		"}"\
		"themePushButton:focus {"\
		"	border-color: %5;"\
		"}"\
		"themePushButton:hover {"\
		"	border: 2px groove %1;"\
		"	background-color: %3;"\
		"}"\
		"themePushButton:hover:focus {"\
		"	border-color: %5;"\
		"}"\
		"themePushButton:pressed {"\
		"	margin: 0;"\
		"	border: 2px inset %1;"\
		"	border-radius: 4px;"\
		"	padding: 2px;"\
		"	background-color: %4;"\
		"}"\
		"themePushButton:pressed:focus {"\
		"	border-color: %5;"\
		"}"\
		"themePushButton:checked {"\
		"	margin: 0;"\
		"	border: 2px inset %1;"\
		"	border-radius: 4px;"\
		"	padding: 2px;"\
		"	background-color: %4;"\
		"}"\
		"themePushButton:checked:focus {"\
		"	border-color: %5;"\
		"}"\
		"themePushButton:checked:disabled {"\
		"	color: gray;"\
		"}";

	return (stylesheet
		.arg(is_dark_theme() ? border_color1.name() : border_color0.name())
		.arg(is_dark_theme() ? border_color0.name() : border_color1.name())
		.arg(background_normal.name())
		.arg(background_hover.name())
		.arg(background_checked.name())
		.arg(get_focus_color().name()));
}
QString theme::stylesheet_wdgtoolbutton(void) {
	return (stylesheet_wdgbutton().replace("themePushButton", "themeToolButton"));
}
QString theme::stylesheet_pixmapbutton(void) {
	return (stylesheet_wdgbutton().replace("themePushButton", "pixmapPushButton"));
}

float theme::calculate_brightness(const QColor &color) {
	return (color.redF() * 0.299f) + (color.greenF() * 0.587f) + (color.blueF() * 0.114f);
}
bool theme::is_dark_theme(void) {
	return (calculate_brightness(QApplication::palette().window().color()) < 0.5f);
}
QColor theme::get_focus_color(void) {
	return (QApplication::palette().highlight().color());
}
QColor theme::get_foreground_color(const QColor &background_color) {
	return (calculate_brightness(background_color) >= 0.5f ? Qt::black : Qt::white);
}
QColor theme::get_theme_color(const QColor &base_color) {
	if (is_dark_theme()) {
		int h, s, l;

		if (base_color == QApplication::palette().dark().color()) {
			return (QApplication::palette().light().color());
		}
		if (base_color == QApplication::palette().light().color()) {
			return (QApplication::palette().dark().color());
		}
		base_color.getHsl(&h, &s, &l);
		l = 255 - l;
		return (QColor::fromHsl(h, s, l));
	}
	return (base_color);
}
QColor theme::get_theme_aware_color(const QColor &base_color) {
	if (is_dark_theme()) {
		int h, s, l;

		if (base_color == QApplication::palette().dark().color()) {
			return (QApplication::palette().light().color());
		}
		if (base_color == QApplication::palette().light().color()) {
			return (QApplication::palette().dark().color());
		}
		base_color.getHsl(&h, &s, &l);
		// calcolo la luminosità complementare
		// 0.8 per mantenere un po' di contrasto
		const double brightness = 255 - (l * 0.8f);
		// riduco leggermente la saturazione per i temi scuri
		s = s * 0.9f;

		return (QColor::fromHsl(h, s, brightness));
	}
	return (base_color);
}
QColor theme::get_theme_adaptive_color(const QColor &base_color) {
	if (is_dark_theme()) {
		// calcolo la luminosità percepita
		const float brightness = calculate_brightness(base_color);
		int h, s, l;

		if (base_color == QApplication::palette().dark().color()) {
			return (QApplication::palette().light().color());
		}
		if (base_color == QApplication::palette().light().color()) {
			return (QApplication::palette().dark().color());
		}
		base_color.getHsl(&h, &s, &l);
		if (brightness > 0.5f) {
			// per colori chiari, scurisco mantenendo la tonalità
			// riduco la luminosità del 60%
			l = l * 0.4f;
			return (QColor::fromHsl(h, s, l));
		} else {
			// per colori scuri, aumento la luminosità
			l = qMin(255, l * 2);
			return (QColor::fromHsl(h, s, l));
		}
	}
	return (base_color);
}
QColor theme::get_grayed_color(const QColor &base_color) {
	const QColor background = is_dark_theme() ? base_color.darker(190) : base_color;
	const QColor gray(128, 128, 128);

	return (QColor(
		background.red() * 0.7 + gray.red() * 0.5,
		background.green() * 0.7 + gray.green() * 0.5,
		background.blue() * 0.7 + gray.blue() * 0.5
	));
}

// ----------------------------------------------------------------------------------------------

themeGroupBox::themeGroupBox(QWidget *parent) : QGroupBox(parent) {}
themeGroupBox::~themeGroupBox() = default;

// ----------------------------------------------------------------------------------------------

themeToolGroupBox::themeToolGroupBox(QWidget *parent) : QGroupBox(parent) {}
themeToolGroupBox::~themeToolGroupBox() = default;

// ----------------------------------------------------------------------------------------------

themePushButton::themePushButton(QWidget *parent) : QPushButton(parent) {}
themePushButton::~themePushButton() = default;

// ----------------------------------------------------------------------------------------------

themeToolButton::themeToolButton(QWidget *parent) : QToolButton(parent) {}
themeToolButton::~themeToolButton() = default;

// ----------------------------------------------------------------------------------------------

pixmapPushButton::pixmapPushButton(QWidget *parent) : QPushButton(parent) {}
pixmapPushButton::~pixmapPushButton() = default;

void pixmapPushButton::paintEvent(QPaintEvent *e) {
	if (!pixmap.isNull()) {
		const QPixmap img = pixmap.scaled(iconSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		QStylePainter spainter(this);
		QStyleOptionButton option;

		initStyleOption(&option);
		option.text = "";
		option.icon = QIcon(img);
		spainter.drawControl(QStyle::CE_PushButton, option);
	} else {
		QPushButton::paintEvent(e);
	}
}

void pixmapPushButton::setPixmap(const QPixmap &pixmap) {
	this->pixmap = pixmap;
}