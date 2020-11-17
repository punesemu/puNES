/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QStyleOption>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QMenu>
#include "wdgPaletteEditor.moc"
#include "common.h"
#include "palette.h"
#include "mem_map.h"
#include "emu.h"
#include "conf.h"
#include "settings.h"
#include "video/gfx.h"

// ----------------------------------------------------------------------------------------------

wdgPaletteWall::wdgPaletteWall(QWidget *parent) : QWidget(parent) {
	setFocusPolicy(Qt::StrongFocus);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	colors.clear();
	defaults.clear();
	margin = 0;
	nrows = 4;
	ncols = 16;
	cellw = 22;
	cellh = 22;
	curCol = 0;
	curRow = 0;
	selCol = -1;
	selRow = -1;

	//setStyleSheet("background-color:black;");
}
wdgPaletteWall::~wdgPaletteWall() {};

QSize wdgPaletteWall::sizeHint() const {
	ensurePolished();
	return (grid_size().boundedTo(QSize(640, 480)));
}

void wdgPaletteWall::resizeEvent(QResizeEvent *event) {
	QSize s = event->size();

	cellw = s.width() / ncols;
	cellh = s.height() / nrows;
}
void wdgPaletteWall::paintEvent(QPaintEvent *event) {
	QRect r = event->rect();
	int cx = r.x();
	int cy = r.y();
	int ch = r.height();
	int cw = r.width();
	int colfirst = column_at(cx);
	int collast = column_at(cx + cw);
	int rowfirst = row_at(cy);
	int rowlast = row_at(cy + ch);
	QPainter painter(this);
	QPainter *p = &painter;
	QRect rect(0, 0, cellw, cellh);
	QStyleOption opt;

	opt.init(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, p, this);

	if (collast < 0 || collast >= ncols) {
		collast = ncols - 1;
	}

	if (rowlast < 0 || rowlast >= nrows) {
		rowlast = nrows - 1;
	}

	// Go through the rows
	for (int r = rowfirst; r <= rowlast; ++r) {
		// get row position and height
		int rowp = row_y(r);

		// Go through the columns in the row r
		// if we know from where to where, go through [colfirst, collast],
		// else go through all of them
		for (int c = colfirst; c <= collast; ++c) {
			// get position and width of column c
			int colp = column_x(c);

			// Translate painter and draw the cell
			rect.translate(colp, rowp);
			paint_cell(p, r, c, rect);
			rect.translate(-colp, -rowp);
		}
	}
}
void wdgPaletteWall::mousePressEvent(QMouseEvent *event) {
	// The current cell marker is set to the cell the mouse is pressed in
	QPoint pos = event->pos();

	set_current(row_at(pos.y()), column_at(pos.x()));
}
void wdgPaletteWall::mouseReleaseEvent(UNUSED(QMouseEvent *event)) {
	// The current cell marker is set to the cell the mouse is clicked in
	set_selected(curRow, curCol);
}
void wdgPaletteWall::keyPressEvent(QKeyEvent* e) {
	switch (e->key()) {
		case Qt::Key_Left:
			if (curCol > 0) {
				set_current(curRow, curCol - 1);
			}
			break;
		case Qt::Key_Right:
			if (curCol < (ncols - 1)) {
				set_current(curRow, curCol + 1);
			}
			break;
		case Qt::Key_Up:
			if (curRow > 0) {
				set_current(curRow - 1, curCol);
			}
			break;
		case Qt::Key_Down:
			if (curRow < (nrows - 1)) {
				set_current(curRow + 1, curCol);
			}
			break;
		case Qt::Key_Space:
			set_selected(curRow, curCol);
			break;
		default:
			e->ignore();
			return;
	}
}
void wdgPaletteWall::focusInEvent(UNUSED(QFocusEvent *event)) {
	update_cell(curRow, curCol);
	emit current_changed(curRow, curCol);
}
void wdgPaletteWall::focusOutEvent(UNUSED(QFocusEvent *event)) {
	update_cell(curRow, curCol);
}

int wdgPaletteWall::count(void) {
	return (colors.count());
}
QColor wdgPaletteWall::color_at(int index) {
	return (colors.at(index));
}
void wdgPaletteWall::update_cell(int row, int col) {
	update(cell_geometry(row, col));
}
void wdgPaletteWall::update_cell_color(int index, QColor color) {
	int row = (index / ncols);
	int col = index - (row * ncols);

	colors.replace(index, color);
	update(cell_geometry(row, col));
}
int wdgPaletteWall::color_index(int row, int col) {
	return ((row * ncols) + col);
}
int wdgPaletteWall::current_palette_index(void) {
	return (palette_index(selRow, selCol));
}
void wdgPaletteWall::color_reset(int index) {
	update_cell_color(index, defaults.at(index));
}
void wdgPaletteWall::colors_reset(void) {
	int i;

	for (i = 0; i < colors.count(); i++) {
		color_reset(i);
	}
}
void wdgPaletteWall::palette_changed(void) {
	colors.clear();
	defaults.clear();
}
int wdgPaletteWall::palette_index(int row, int col) {
	return (color_index(row, col));
}
void wdgPaletteWall::print_in_cell(QPainter *p, int row, int col, const QRect &rect) {
	int pindex = palette_index(row, col);

	p->drawText(rect, Qt::AlignVCenter | Qt::AlignHCenter, QString("%1").arg(pindex, 2, 16, QChar('0')).toUpper());
}
void wdgPaletteWall::set_current(int row, int col) {
	int oldRow = curRow;
	int oldCol = curCol;

	if ((curRow == row) && (curCol == col)) {
		return;
	}

	if (row < 0 || col < 0) {
		row = col = -1;
	}

	curRow = row;
	curCol = col;
	update_cell(oldRow, oldCol);
	update_cell(curRow, curCol);
	emit current_changed(curRow, curCol);
}

int wdgPaletteWall::row_at(int y) {
	return (y / cellh);
}
int wdgPaletteWall::column_at(int x) {
	return (x / cellw);
}
int wdgPaletteWall::row_y(int row) {
	return (cellh * row);
}
int wdgPaletteWall::column_x(int col) {
	return (cellw * col);
}
QSize wdgPaletteWall::grid_size(void) const {
	return (QSize(ncols * cellw, nrows * cellh));
}
QRect wdgPaletteWall::cell_geometry(int row, int col) {
	QRect r;

	if (row >= 0 && row < nrows && col >= 0 && col < ncols) {
		r.setRect(column_x(col), row_y(row), cellw, cellh);
	}

	return (r);
}
void wdgPaletteWall::set_selected(int row, int col) {
	int oldRow = selRow;
	int oldCol = selCol;

	selCol = col;
	selRow = row;
	update_cell(oldRow, oldCol);
	update_cell(selRow, selCol);

	if (row >= 0) {
		emit selected(row, col);
	}

	if (isVisible() && qobject_cast<QMenu*>(parentWidget())) {
		parentWidget()->close();
	}
}
void wdgPaletteWall::paint_cell(QPainter *p, int row, int col, const QRect &rect) {
	const QPalette &g = palette();
	int dfw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	QStyleOptionFrame opt;
	int b = margin;

	opt.lineWidth = dfw;
	opt.midLineWidth = 1;
	opt.rect = rect.adjusted(b, b, -b, -b);
	opt.palette = g;
	opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
	style()->drawPrimitive(QStyle::PE_Frame, &opt, p, this);
	b += dfw;

	if ((row == curRow) && (col == curCol)) {
		if (hasFocus()) {
			QStyleOptionFocusRect opt;

			opt.palette = g;
			opt.rect = rect;
			opt.state = QStyle::State_None | QStyle::State_KeyboardFocusChange;
			style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, p, this);
		}
	}
	paint_cell_contents(p, row, col, opt.rect.adjusted(dfw, dfw, -dfw, -dfw));
}
void wdgPaletteWall::paint_cell_contents(QPainter *p, int row, int col, const QRect &rect) {
	int pindex = palette_index(row, col);
	int index = color_index(row, col);
	double brightness;
	_color_RGB *rgb = &palette_RGB.noswap[pindex];
	QColor color = QColor(rgb->r, rgb->g, rgb->b);
	QColor bg;

	if (colors.count() <= index) {
		colors.append(color);
		defaults.append(color);

		if (colors.count() == (nrows * ncols)) {
			emit first_paint();
		}
	}

	bg = colors.at(index);

	if (bg != color) {
		colors.replace(index, color);
		bg = color;
	}

	brightness = bg.redF() * 0.299f + bg.greenF() * 0.587f + bg.blueF() * 0.114f;
	p->fillRect(rect, bg);
	p->setPen((brightness > 0.5f) ? Qt::black : Qt::white);
	print_in_cell(p, row, col, rect);
}

// ----------------------------------------------------------------------------------------------

wdgPalettePPU::wdgPalettePPU(QWidget *parent) : wdgPaletteWall(parent) {
	margin = 1;
	nrows = 2;
	ncols = 16;
	cellh = 32;
}
wdgPalettePPU::~wdgPalettePPU() {};

int wdgPalettePPU::palette_index(int row, int col) {
	return (mmap_palette.color[(row * ncols) + col]);
}
void wdgPalettePPU::print_in_cell(QPainter *p, int row, int col, const QRect &rect) {
	int pindex = palette_index(row, col);
	int index = color_index(row, col);

	p->drawText(rect, Qt::AlignBottom | Qt::AlignHCenter, QString("%1").arg(pindex, 2, 16, QChar('0')).toUpper());
	p->drawText(rect, Qt::AlignTop | Qt::AlignHCenter, QString("%1").arg(index, 2, 10, QChar('0')).toUpper());
}

// ----------------------------------------------------------------------------------------------

wdgColorToChange::wdgColorToChange(QWidget *parent) : wdgPaletteWall(parent) {
	setFocusPolicy(Qt::NoFocus);
	margin = 0;
	nrows = 1;
	ncols = 1;
	color = 0;
}
wdgColorToChange::~wdgColorToChange() {};

void wdgColorToChange::resizeEvent(QResizeEvent *event) {
	QSize s = event->size();

	cellw = s.width();
	cellh = s.height();
}

int wdgColorToChange::palette_index(UNUSED(int row), UNUSED(int col)) {
	return (color);
}
void wdgColorToChange::set_current(int row, int col) {
	selCol = col;
	selRow = row;

	update_cell(selRow, selCol);

	if (row >= 0) {
		emit selected(row, col);
	}

	if (isVisible() && qobject_cast<QMenu*>(parentWidget())) {
		parentWidget()->close();
	}
}

// ----------------------------------------------------------------------------------------------

wdgHtmlName::wdgHtmlName(QWidget *parent) : QLineEdit(parent) {}
wdgHtmlName::~wdgHtmlName() {}

void wdgHtmlName::focusOutEvent(QFocusEvent *event) {
	QLineEdit::focusOutEvent(event);
	emit focus_out();
}

// ----------------------------------------------------------------------------------------------

wdgPaletteEditor::wdgPaletteEditor(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	connect(widget_Palette_Wall, SIGNAL(selected(int, int)), this, SLOT(s_palette_wall(int, int)));
	connect(widget_Palette_Wall, SIGNAL(first_paint(void)), this, SLOT(s_first_paint(void)));
	connect(widget_Palette_PPU, SIGNAL(selected(int, int)), this, SLOT(s_palette_ppu(int, int)));

	connect(horizontalSlider_Red, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(horizontalSlider_Green, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(horizontalSlider_Blue, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(horizontalSlider_Hue, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(horizontalSlider_Sat, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(horizontalSlider_Val, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));

	connect(spinBox_Red, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(spinBox_Green, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(spinBox_Blue, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(spinBox_Hue, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(spinBox_Sat, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));
	connect(spinBox_Val, SIGNAL(valueChanged(int)), this, SLOT(s_slider_and_spin(int)));

	connect(lineEdit_Html_Name, SIGNAL(focus_out(void)), this, SLOT(s_html(void)));
	connect(lineEdit_Html_Name, SIGNAL(returnPressed(void)), this, SLOT(s_html(void)));

	connect(pushButton_Color_reset, SIGNAL(clicked(bool)), this, SLOT(s_color_reset(bool)));
	connect(pushButton_Palette_save, SIGNAL(clicked(bool)), this, SLOT(s_palette_save(bool)));
	connect(pushButton_Palette_reset, SIGNAL(clicked(bool)), this, SLOT(s_palette_reset(bool)));
}
wdgPaletteEditor::~wdgPaletteEditor() {}

void wdgPaletteEditor::palette_changed(void) {
	widget_Palette_Wall->palette_changed();
	update();
}

void wdgPaletteEditor::set_slider(QSlider *slider, int value) {
	slider->blockSignals(true);
	slider->setValue(value);
	slider->blockSignals(false);
}
void wdgPaletteEditor::set_spin(QSpinBox *spin, int value) {
	spin->blockSignals(true);
	spin->setValue(value);
	spin->blockSignals(false);
}
void wdgPaletteEditor::set_sliders_spins_lineedit(void) {
	QColor qrgb = widget_Palette_Wall->color_at(widget_Color_Selected->color);

	set_slider(horizontalSlider_Hue, qrgb.hue());
	set_slider(horizontalSlider_Sat, qrgb.saturation());
	set_slider(horizontalSlider_Val, qrgb.value());

	set_spin(spinBox_Hue, qrgb.hue());
	set_spin(spinBox_Sat, qrgb.saturation());
	set_spin(spinBox_Val, qrgb.value());

	set_slider(horizontalSlider_Red, qrgb.red());
	set_slider(horizontalSlider_Green, qrgb.green());
	set_slider(horizontalSlider_Blue, qrgb.blue());

	set_spin(spinBox_Red, qrgb.red());
	set_spin(spinBox_Green, qrgb.green());
	set_spin(spinBox_Blue, qrgb.blue());

	lineEdit_Html_Name->blockSignals(true);
	lineEdit_Html_Name->setText(qrgb.name().toUpper());
	lineEdit_Html_Name->blockSignals(false);
}
void wdgPaletteEditor::set_internal_color(int index, QColor qrgb, bool update_palette) {
	_color_RGB *rgb = &palette_RGB.noswap[index];

	rgb->r = qrgb.red();
	rgb->g = qrgb.green();
	rgb->b = qrgb.blue();

	if (update_palette) {
		ntsc_set(NULL, cfg->ntsc_format, 0, (BYTE *)palette_RGB.noswap, 0, (BYTE *)palette_RGB.noswap);
		gfx_palette_update();
	}
}

void wdgPaletteEditor::s_first_paint(void) {
	set_sliders_spins_lineedit();
}
void wdgPaletteEditor::s_palette_wall(UNUSED(int row), UNUSED(int col)) {
	widget_Color_Selected->color = widget_Palette_Wall->current_palette_index();
	widget_Color_Selected->update_cell(0, 0);
	set_sliders_spins_lineedit();
}
void wdgPaletteEditor::s_palette_ppu(UNUSED(int row), UNUSED(int col)) {
	widget_Color_Selected->color = widget_Palette_PPU->current_palette_index();
	widget_Color_Selected->update_cell(0, 0);
	set_sliders_spins_lineedit();
}
void wdgPaletteEditor::s_slider_and_spin(int i) {
	int index = widget_Color_Selected->color;
	QColor qrgb = widget_Palette_Wall->color_at(index);
	int h, s, v;

	qrgb.getHsv(&h, &s, &v);

	if (((QObject *)sender())->objectName().contains("_Red", Qt::CaseSensitive)) {
		qrgb.setRed(i);
	} else if (((QObject *)sender())->objectName().contains("_Green", Qt::CaseSensitive)) {
		qrgb.setGreen(i);
	} else if (((QObject *)sender())->objectName().contains("_Blue", Qt::CaseSensitive)) {
		qrgb.setBlue(i);
	} else if (((QObject *)sender())->objectName().contains("_Hue", Qt::CaseSensitive)) {
		qrgb.setHsv(i, s, v);
	} else if (((QObject *)sender())->objectName().contains("_Sat", Qt::CaseSensitive)) {
		qrgb.setHsv(h, i, v);
	} else if (((QObject *)sender())->objectName().contains("_Val", Qt::CaseSensitive)) {
		qrgb.setHsv(h, s, i);
	}

	set_internal_color(index, qrgb, true);

	widget_Palette_Wall->update_cell_color(index, qrgb);
	widget_Color_Selected->update_cell_color(0, qrgb);
	set_sliders_spins_lineedit();
}
void wdgPaletteEditor::s_html(void) {
	int index = widget_Color_Selected->color;
	QColor qrgb = widget_Palette_Wall->color_at(index);

	if (QColor::isValidColor(lineEdit_Html_Name->text()) == false) {
		set_sliders_spins_lineedit();
		return;
	}

	qrgb.setNamedColor(lineEdit_Html_Name->text());

	set_internal_color(index, qrgb, true);

	widget_Palette_Wall->update_cell_color(index, qrgb);
	widget_Color_Selected->update_cell_color(0, qrgb);
	set_sliders_spins_lineedit();
}
void wdgPaletteEditor::s_color_reset(UNUSED(bool checked)) {
	int index = widget_Color_Selected->color;
	QColor qrgb;

	widget_Palette_Wall->color_reset(index);
	qrgb = widget_Palette_Wall->color_at(index);

	set_internal_color(index, qrgb, true);

	widget_Color_Selected->update_cell(0, 0);
	set_sliders_spins_lineedit();
}
void wdgPaletteEditor::s_palette_save(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Palette files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.pal *.PAL)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getSaveFileName(this, tr("Save palette on file"),
		uQString(opt_palette[cfg->palette].lname).replace(" ", "_"),
		filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(QString(file) + ".pal");
		}

		palette_save_on_file(uQStringCD(fileinfo.absoluteFilePath()));
	}

	emu_pause(FALSE);
}
void wdgPaletteEditor::s_palette_reset(UNUSED(bool checked)) {
	int i;

	widget_Palette_Wall->colors_reset();

	for (i = 0; i < widget_Palette_Wall->count(); i++) {
		QColor qrgb = widget_Palette_Wall->color_at(i);

		set_internal_color(i, qrgb, false);
	}

	// forzo la'aggiornamento dell'intera paletta.
	set_internal_color(0, widget_Palette_Wall->color_at(0), true);

	widget_Color_Selected->update_cell(0, 0);
	set_sliders_spins_lineedit();
}
