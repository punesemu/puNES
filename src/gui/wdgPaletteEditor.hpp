/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef WDGPALETTEEDITOR_HPP_
#define WDGPALETTEEDITOR_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>

class wdgPaletteWall : public QWidget {
	Q_OBJECT

	protected:
		QList<QColor> colors;
		QList<QColor> defaults;
		int nrows;
		int ncols;
		int cellw;
		int cellh;
		int curRow;
		int curCol;
		int selRow;
		int selCol;
		int margin;

	public:
		explicit wdgPaletteWall(QWidget *parent = nullptr);
		~wdgPaletteWall() override;

	signals:
		void et_first_paint(void);
		void et_selected(int row, int col);
		void et_current_changed(int row, int col);

	public:
		QSize sizeHint(void) const override;

	private:
		void resizeEvent(QResizeEvent *event) override;
		void paintEvent(QPaintEvent *event) override;
		void mousePressEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;
		void keyPressEvent(QKeyEvent *event) override;
		void focusInEvent(QFocusEvent *event) override;
		void focusOutEvent(QFocusEvent *event) override;

	public:
		int count(void);
		QColor color_at(int index);
		void update_cell(int row, int col);
		void update_cell_color(int index, const QColor &color);
		int color_index(int row, int col) const;
		int current_palette_index(void);
		void color_reset(int index);
		void colors_reset(void);
		void palette_changed(void);
		virtual int palette_index(int row, int col);
		virtual void print_in_cell(QPainter *p, int row, int col, const QRect &rect);
		virtual void set_current(int row, int col);

	private:
		int row_at(int y) const;
		int column_at(int x) const;
		int row_y(int row) const;
		int column_x(int col) const;
		QSize grid_size(void) const;
		QRect cell_geometry(int row, int col);
		void set_selected(int row, int col);
		void paint_cell(QPainter *p, int row, int col, const QRect &rect);
		void paint_cell_contents(QPainter *p, int row, int col, const QRect &rect);
};
class wdgPalettePPU : public wdgPaletteWall {
	public:
		explicit wdgPalettePPU(QWidget *parent = nullptr);
		~wdgPalettePPU() override;

	public:
		int palette_index(int row, int col) override;
		void print_in_cell(QPainter *p, int row, int col, const QRect &rect) override;
};
class wdgColorToChange : public wdgPaletteWall {
	public:
		int color;

	public:
		explicit wdgColorToChange(QWidget *parent = nullptr);
		~wdgColorToChange() override;

	private:
		void resizeEvent(QResizeEvent *event) override;

	public:
		int palette_index(int row, int col) override;
		void set_current(int row, int col) override;
};
class wdgHtmlName : public QLineEdit {
	Q_OBJECT

	public:
		explicit wdgHtmlName(QWidget *parent = nullptr);
		~wdgHtmlName() override;

	signals:
		void et_focus_out(void);

	private:
		void focusOutEvent(QFocusEvent *event) override;
};

#include "ui_wdgPaletteEditor.h"

class wdgPaletteEditor : public QWidget, public Ui::wdgPaletteEditor {
	Q_OBJECT

	public:
		explicit wdgPaletteEditor(QWidget *parent = nullptr);
		~wdgPaletteEditor() override;

	protected:
		void changeEvent(QEvent *event) override;

	public:
		void palette_changed(void);

	private:
		void set_sliders_spins_lineedit(void);
		static void set_internal_color(int index, const QColor &qrgb, bool update_palette);

	private slots:
		void s_first_paint(void);
		void s_palette_wall(int row, int col);
		void s_palette_ppu(int row, int col);
		void s_slider_and_spin(int i);
		void s_html(void);
		void s_color_reset(bool checked);
		void s_palette_save(bool checked);
		void s_palette_reset(bool checked);
};

#endif /* WDGPALETTEEDITOR_HPP_ */
