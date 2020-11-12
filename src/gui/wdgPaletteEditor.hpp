/*  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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
		wdgPaletteWall(QWidget *parent = 0);
		~wdgPaletteWall();

	public:
		QSize sizeHint(void) const;

	private:
		void resizeEvent(QResizeEvent *event);
		void paintEvent(QPaintEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void focusInEvent(QFocusEvent *event);
		void focusOutEvent(QFocusEvent *event);

	public:
		int count(void);
		QColor color_at(int index);
		void update_cell(int row, int col);
		void update_cell_color(int index, QColor color);
		int color_index(int row, int col);
		int current_palette_index(void);
		void color_reset(int index);
		void colors_reset(void);
		void palette_changed(void);
		virtual int palette_index(int row, int col);
		virtual void print_in_cell(QPainter *p, int row, int col, const QRect &rect);
		virtual void set_current(int row, int col);

	private:
		int row_at(int y);
		int column_at(int x);
		int row_y(int row);
		int column_x(int col);
		QSize grid_size(void) const;
		QRect cell_geometry(int row, int col);
		void set_selected(int row, int col);
		void paint_cell(QPainter *p, int row, int col, const QRect &rect);
		void paint_cell_contents(QPainter *p, int row, int col, const QRect &rect);

	signals:
		void first_paint(void);
		void selected(int row, int col);
		void current_changed(int row, int col);
};
class wdgPalettePPU : public wdgPaletteWall {
	public:
		wdgPalettePPU(QWidget *parent = 0);
		~wdgPalettePPU();

	public:
		int palette_index(int row, int col);
		void print_in_cell(QPainter *p, int row, int col, const QRect &rect);
};
class wdgColorToChange : public wdgPaletteWall {
	public:
		int color;

	public:
		wdgColorToChange(QWidget *parent = 0);
		~wdgColorToChange();

	private:
		void resizeEvent(QResizeEvent *event);

	public:
		int palette_index(int row, int col);
		void set_current(int row, int col);
};
class wdgHtmlName : public QLineEdit {
	Q_OBJECT

	public:
		wdgHtmlName(QWidget *parent = 0);
		~wdgHtmlName();

	signals:
		void focus_out(void);

	private:
		void focusOutEvent(QFocusEvent *event);
};

#include "wdgPaletteEditor.hh"

class wdgPaletteEditor : public QWidget, public Ui::wdgPaletteEditor {
		Q_OBJECT

	public:
		wdgPaletteEditor(QWidget *parent = 0);
		~wdgPaletteEditor();

	public:
		void palette_changed(void);

	private:
		void set_slider(QSlider *slider, int value);
		void set_spin(QSpinBox *spin, int value);
		void set_sliders_spins_lineedit(void);
		void set_internal_color(int index, QColor qrgb);

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
