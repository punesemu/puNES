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

#ifndef THEME_HPP_
#define THEME_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>

class theme {
	public:
		static QString stylesheet_wdgroupbox(void);
		static QString stylesheet_wdgtoolgroupbox(void);
		static QString stylesheet_wdgbutton(void);
		static QString stylesheet_wdgtoolbutton(void);
		static QString stylesheet_pixmapbutton(void);

	public:
		static float calculate_brightness(const QColor &color);
		static bool is_dark_theme(void);
		static QColor get_focus_color(void);
		static QColor get_foreground_color(const QColor &background_color);
		static QColor get_theme_color(const QColor &base_color);
		static QColor get_theme_aware_color(const QColor &base_color);
		static QColor get_theme_adaptive_color(const QColor &base_color);
		static QColor get_grayed_color(const QColor &base_color);
};
class themeGroupBox : public QGroupBox {
	Q_OBJECT

	public:
		explicit themeGroupBox(QWidget *parent = nullptr);
		~themeGroupBox() override;
};
class themeToolGroupBox : public QGroupBox {
	Q_OBJECT

	public:
		explicit themeToolGroupBox(QWidget *parent = nullptr);
		~themeToolGroupBox() override;
};
class themePushButton : public QPushButton {
	Q_OBJECT

	public:
		explicit themePushButton(QWidget *parent = nullptr);
		~themePushButton() override;
};
class themeToolButton : public QToolButton {
	Q_OBJECT

	public:
		explicit themeToolButton(QWidget *parent = nullptr);
		~themeToolButton() override;
};
class pixmapPushButton: public QPushButton {
	Q_OBJECT

	private:
		QPixmap pixmap;

	public:
		explicit pixmapPushButton(QWidget *parent = nullptr);
		~pixmapPushButton() override;

	protected:
		void paintEvent(QPaintEvent *e) override;

	public:
		void setPixmap(const QPixmap &pixmap);
};

#endif //THEME_HPP_
