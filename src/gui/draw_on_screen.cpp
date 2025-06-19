/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#include <QtGui/QImageReader>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>
#include <QtCore/QTextStream>
#include <math.h>
#include "draw_on_screen.h"
#include "mainWindow.hpp"
#include "nes.h"
#include "gui.h"
#include "unicode_def.h"

#define not_in_ppu() ((px < 0) || (px >= SCR_COLUMNS) || (py < 0) || (py >= SCR_ROWS))
#define not_in_qimage() ((ix < 0) || (ix >= rect_w) || (iy < 0) || (iy >= rect_h))

static INLINE int dos_ctr_x(int x, int w, int max_w);
static INLINE int dos_ctr_y(int y, int h, int max_h);
static INLINE int dos_ctr_w(int w, int min_w, int max_w);
static INLINE int dos_ctr_h(int h, int min_h, int max_h);
static INLINE int dos_round(double d0, double d1);
static INLINE QSize dos_image_dim(const uTCHAR *resource);
static WORD *_dos_text_to_ppu_image(int rect_x, int rect_y, int rect_w, int rect_h,
	const WORD fg_def, const WORD bg_def, const uTCHAR *font_family, const int font_size, const uTCHAR *text);

typedef struct _dos_tag_ele {
	QString desc;
	WORD value;
	long long opt;
} _dos_tag_ele;

#define gdt(desc, value, opt) { QStringLiteral(desc), (value), (opt) }
static QList<_dos_tag_ele> dos_tags = {
	// gray
	gdt("[gy01]", DOS_GY01, 0xFF424242), gdt("[gy02]", DOS_GY02, 0xFF585858),
	gdt("[gy03]", DOS_GY03, 0xFFA1A1A1), gdt("[gy04]", DOS_GY04, 0xFFACACAC),
	// blue
	gdt("[bl01]", DOS_BL01, 0xFF00237C), gdt("[bl02]", DOS_BL02, 0xFF0B53D7),
	gdt("[bl03]", DOS_BL03, 0xFF51A5FE), gdt("[bl04]", DOS_BL04, 0xFFB5D9FE),
	// royal blue
	gdt("[rb01]", DOS_RB01, 0xFF0D1099), gdt("[rb02]", DOS_RB02, 0xFF3337FE),
	gdt("[rb03]", DOS_RB03, 0xFF8084FE), gdt("[rb04]", DOS_RB04, 0xFFCACAFE),
	// indigo
	gdt("[in01]", DOS_IN01, 0xFF300092), gdt("[in02]", DOS_IN02, 0xFF6621F7),
	gdt("[in03]", DOS_IN03, 0xFFBC6AFE), gdt("[in04]", DOS_IN04, 0xFFE3BEFE),
	// purple
	gdt("[pu01]", DOS_PU01, 0xFF4F006C), gdt("[pu02]", DOS_PU02, 0xFF9515BE),
	gdt("[pu03]", DOS_PU03, 0xFFF15BFE), gdt("[pu04]", DOS_PU04, 0xFFF9B8FE),
	// magenta
	gdt("[ma01]", DOS_MA01, 0xFF600035), gdt("[ma02]", DOS_MA02, 0xFFAC166E),
	gdt("[ma03]", DOS_MA03, 0xFFFE5EC4), gdt("[ma04]", DOS_MA04, 0xFFFEBAE7),
	// pink
	gdt("[pi01]", DOS_PI01, 0xFF5C0500), gdt("[pi02]", DOS_PI02, 0xFFA62721),
	gdt("[pi03]", DOS_PI03, 0xFFFE7269), gdt("[pi04]", DOS_PI04, 0xFFFEC3BC),
	// brown
	gdt("[br01]", DOS_BR01, 0xFF461800), gdt("[br02]", DOS_BR02, 0xFF864300),
	gdt("[br03]", DOS_BR03, 0xFFE19321), gdt("[br04]", DOS_BR04, 0xFFF4D199),
	// olive
	gdt("[ol01]", DOS_OL01, 0xFF272D00), gdt("[ol02]", DOS_OL02, 0xFF596200),
	gdt("[ol03]", DOS_OL03, 0xFFADB600), gdt("[ol04]", DOS_OL04, 0xFFDEE086),
	// pea
	gdt("[pe01]", DOS_PE01, 0xFF093E00), gdt("[pe02]", DOS_PE02, 0xFF2D7A00),
	gdt("[pe03]", DOS_PE03, 0xFF79D300), gdt("[pe04]", DOS_PE04, 0xFFC6EC87),
	// green
	gdt("[gr01]", DOS_GR01, 0xFF004500), gdt("[gr02]", DOS_GR02, 0xFF0C8500),
	gdt("[gr03]", DOS_GR03, 0xFF51DF21), gdt("[gr04]", DOS_GR04, 0xFFB2F29D),
	// teal
	gdt("[tl01]", DOS_TL01, 0xFF004106), gdt("[tl02]", DOS_TL02, 0xFF007F2A),
	gdt("[tl03]", DOS_TL03, 0xFF3AD974), gdt("[tl04]", DOS_TL04, 0xFFA7F0C3),
	// cyan
	gdt("[cy01]", DOS_CY01, 0xFF003545), gdt("[cy02]", DOS_CY02, 0xFF006D85),
	gdt("[cy03]", DOS_CY03, 0xFF39C3DF), gdt("[cy04]", DOS_CY04, 0xFFA8E7F0),
	// white
	gdt("[white]", DOS_WHITE, 0xFFFFFFFF),
	// black
	gdt("[black]", DOS_BLACK, 0xFF000000),
	//  retrocompatibilita'
	gdt("[normal]", DOS_NORMAL, 0xFFFFFFFF),
	gdt("[red]", DOS_RED, 0xFFFE7269),
	gdt("[yellow]", DOS_YELLOW, 0xFFDEE086),
	gdt("[green]", DOS_GREEN, 0xFF51DF21),
	gdt("[cyan]", DOS_CYAN, 0xFF39C3DF),
	gdt("[brown]", DOS_BROWN, 0xFFE19321),
	gdt("[blue]", DOS_BLUE, 0xFF0B53D7),
	gdt("[gray]", DOS_GRAY, 0xFF585858),
	gdt("[transparent]", DOS_TRASPARENT, 0x00000000),
	//  altri
	gdt("[none]", DOS_NONE, -1),
	gdt("[bck]", DOS_BCK, -1),
	gdt("[image]", DOS_IMAGE, -1),
	gdt("[endimage]", DOS_ENDIMAGE, -1),
	gdt("[top]", DOS_ALIGNTOP, -1),
	gdt("[bottom]", DOS_ALIGNBOTTOM, -1),
	gdt("[vcenter]", DOS_ALIGNVCENTER, -1),
	gdt("[left]", DOS_ALIGNLEFT, -1),
	gdt("[right]", DOS_ALIGNRIGHT, -1),
	gdt("[hcenter]", DOS_ALIGNHCENTER, -1)
};
#undef gdt

void dos_text(BYTE nidx, int ppu_x, int ppu_y, int rect_x, int rect_y, int rect_w, int rect_h,
	const WORD fg_def, const WORD bg_def, const uTCHAR *font_family, const int font_size, const uTCHAR *fmt, ...) {
	static uint mask0 = qRgb(255, 255, 255), mask1 = qRgb(0, 0, 0);
	QFont font(uQString(font_family));
	WORD fg = fg_def, bg = bg_def;
	QString line;
	uTCHAR text[1024];
	va_list ap;

	va_start(ap, fmt);
	uvsnprintf(text, usizeof(text), fmt, ap);
	va_end(ap);

	line = uQString(text);

	// minimo 11 per i caratteri unicode
	font.setPixelSize(font_size);
	font.setStretch(QFont::Unstretched);

	{
		int char_x = 0, wpixels = 0, hpixels = 0;
		QFontMetrics fontMetrics(font);
		QString subimage = "";
		bool is_bck_tag = false;
		bool is_subimage = false;
		bool is_name_subimage = false;
		int alignv = DOS_ALIGNVCENTER;
		int alignh = DOS_ALIGNLEFT;

		dos_text_pixels_size(&wpixels, &hpixels, font_family, font_size, &text[0]);

		rect_w = dos_ctr_w(rect_w, wpixels, SCR_COLUMNS);
		rect_h = dos_ctr_h(rect_h, hpixels, SCR_ROWS);
		rect_x = dos_ctr_x(rect_x, wpixels, rect_w);
		rect_y = dos_ctr_y(rect_y, hpixels, rect_h);
		ppu_x = dos_ctr_x(ppu_x, rect_w, SCR_COLUMNS);
		ppu_y = dos_ctr_y(ppu_y, rect_h, SCR_ROWS);

		if (!rect_w || !rect_h) {
			return;
		}

		QImage mask(rect_w, rect_h, QImage::Format_Mono);
		mask.setColorTable(QVector<QRgb>{ mask0, mask1 });
		// nero
		mask.fill(1);

		QPainter painter(&mask);
		painter.setFont(font);
		painter.setPen(mask0);

		// pulisco l'intera zona
		for (int iy = 0; iy < rect_h; iy++) {
			for (int ix = 0; ix < rect_w; ix++) {
				int px = ix + ppu_x, py = iy + ppu_y;

				if (not_in_ppu() || (bg_def == DOS_TRASPARENT)) {
					continue;
				}
				nes[nidx].p.ppu_screen.wr->line[py][px] = bg_def;
			}
		}

		char_x = rect_x;

		// disegno la stringa
		for (int i = 0; i < line.length(); i++) {
			QChar ch = line[i];
			int chw = 0;

			if (ch == '[') {
				bool is_tag = false;

				for (const _dos_tag_ele &ele : dos_tags) {
					int len = ele.desc.length();

					if ((i + len) < line.length()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
						QStringView tag = QStringView{line}.mid(i, len);
#else
						QStringRef tag(&line, i, len);
#endif

						if (ele.desc == tag) {
							if ((tag == QStringLiteral("[image]")) && !is_name_subimage) {
								is_name_subimage = true;
							} else if ((tag == QStringLiteral("[endimage]")) && is_name_subimage) {
								is_name_subimage = false;
								is_subimage = true;
							} else  if (tag == QStringLiteral("[bck]")) {
								is_bck_tag = true;
							} else if (tag == QStringLiteral("[top]")) {
								alignv = DOS_ALIGNTOP;
							} else if (tag == QStringLiteral("[bottom]")) {
								alignv = DOS_ALIGNBOTTOM;
							} else if (tag == QStringLiteral("[vcenter]")) {
								alignv = DOS_ALIGNVCENTER;
							} else if (tag == QStringLiteral("[left]")) {
								alignh = DOS_ALIGNLEFT;
							} else if (tag == QStringLiteral("[right]")) {
								alignh = DOS_ALIGNRIGHT;
							} else if (tag == QStringLiteral("[hcenter]")) {
								alignh = DOS_ALIGNHCENTER;
							} else {
								bool is_normal = tag == QStringLiteral("[normal]");

								if (is_bck_tag) {
									bg = is_normal ? bg_def : ele.value;
								} else {
									fg = is_normal ? fg_def : ele.value;
								}
								is_bck_tag = false;
							}
							is_tag = !is_subimage;
							i += (len - 1);
							break;
						}
					}
				}
				if (is_tag) {
					continue;
				}
			} else if (is_name_subimage) {
				subimage.append(ch);
				continue;
			}
			if (is_subimage) {
				QSize dim = dos_image_dim(uQStringCD(subimage));

				chw = dim.width();
				char_x = rect_x;
				rect_x += chw;
				if (char_x >= rect_w) {
					break;
				}
				dos_image(nidx, ppu_x + char_x, ppu_y, alignh, alignv, chw, rect_h, uQStringCD(subimage), NULL, 0);
				subimage = "";
				is_subimage = false;
			} else {
				chw = fontMetrics.size(0, ch).width();
				char_x = rect_x;
				rect_x += chw;
				if (char_x >= rect_w) {
					break;
				}
				// Disegno della lettera
				painter.drawText(char_x, rect_y, rect_w, rect_h, Qt::AlignLeft | Qt::AlignTop, ch);
				// disegno sullo schermo PPU
				for (int iy = 0; iy < rect_h; iy++) {
					for (int ix = char_x; ix < (char_x + chw); ix++) {
						int px = ix + ppu_x, py = iy + ppu_y;
						WORD ppu_pixel = DOS_TRASPARENT;

						if (not_in_ppu() || not_in_qimage()) {
							continue;
						}
						ppu_pixel = static_cast<uint>(mask.pixel(ix, iy)) == mask0 ? fg : bg;
						if (ppu_pixel != DOS_TRASPARENT) {
							nes[nidx].p.ppu_screen.wr->line[py][px] = ppu_pixel;
						}
					}
				}
			}
		}
	}
}

WORD *dos_text_to_ppu_image(int rect_x, int rect_y, int rect_w, int rect_h, const WORD fg_def, const WORD bg_def,
	const uTCHAR *font_family, const int font_size, const uTCHAR *fmt, ...) {
	uTCHAR text[1024];
	va_list ap;

	va_start(ap, fmt);
	uvsnprintf(text, usizeof(text), fmt, ap);
	va_end(ap);

	return (_dos_text_to_ppu_image(rect_x, rect_y, rect_w, rect_h, fg_def, bg_def, font_family, font_size, &text[0]));
}

void dos_text_scroll_tick(BYTE nidx, int ppu_x, int ppu_y, const WORD fg_def, const WORD bg_def,
	const uTCHAR *font_family, const int font_size, _dos_text_scroll *scroll, const uTCHAR *fmt, ...) {
	uTCHAR text[1024];
	QString line;
	va_list ap;

	va_start(ap, fmt);
	uvsnprintf(text, usizeof(text), fmt, ap);
	va_end(ap);

	if (!scroll->pimage.data) {
		dos_text_pixels_size(&scroll->pimage.w, &scroll->pimage.h, font_family, font_size, &text[0]);
		if (!scroll->pimage.w || !scroll->pimage.h) {
			return;
		}
		scroll->x = -(scroll->pimage.w - scroll->velocity);
		scroll->pimage.w += scroll->rect.w;
		scroll->pimage.data = _dos_text_to_ppu_image(0, 0, scroll->pimage.w, scroll->pimage.h,
			fg_def, bg_def, font_family, font_size, &text[0]);
	}
	if (scroll->pimage.data) {
		scroll->timer -= 0.34f;
		if (scroll->timer < 0) {
			scroll->timer = scroll->reload;
			scroll->x -= scroll->velocity;
			dos_draw_ppu_image(nidx, ppu_x, ppu_y,
				scroll->rect.x, scroll->rect.y, scroll->rect.w, scroll->rect.h,
				scroll->pimage.w, scroll->pimage.h,
				scroll->x, 0, scroll->pimage.data);
			if ((scroll->x + ((scroll->pimage.w - scroll->rect.w))) < 0) {
				scroll->x = scroll->rect.w;
			}
		}
	}
}

void dos_text_curtain(BYTE nidx, int ppu_x, int ppu_y, _dos_text_curtain *curtain, BYTE mode) {
	if (mode == DOS_TEXT_CURTAIN_INIT) {
		dos_text_curtain(nidx, ppu_x, ppu_y, curtain, DOS_TEXT_CURTAIN_QUIT);
		curtain->count = 0;
		curtain->index = 0;
		curtain->pause = FALSE;
		curtain->timer = curtain->reload.r1;
		curtain->h = 0;
	} else if (mode == DOS_TEXT_CURTAIN_TICK) {
		if (curtain->redraw.all) {
			curtain->redraw.all = FALSE;
			dos_draw_ppu_image(nidx, ppu_x, ppu_y,
				0, 0, curtain->image.w, curtain->image.h,
				curtain->image.w, curtain->image.h,
				0, 0, curtain->line[curtain->index].data);
		}
		if (curtain->timer <= 0) {
			if (!curtain->pause) {
				dos_draw_ppu_image(nidx, ppu_x, ppu_y,
					0, curtain->h, curtain->image.w, 1,
					curtain->image.w, 1,
					0, 0, curtain->line[curtain->index].data);
				curtain->h++;
				if (curtain->h == curtain->image.h) {
					curtain->h = 0;
					curtain->timer = curtain->reload.r2;
					curtain->pause = TRUE;
				} else {
					curtain->timer = curtain->reload.r1;
				}
			} else {
				curtain->pause = FALSE;
				curtain->timer = curtain->reload.r1;
				if (++curtain->index == curtain->count) {
					curtain->index = 0;
				}
			}
		} else {
			curtain->timer -= nsf.timers.diff;
		}
	} else if (mode == DOS_TEXT_CURTAIN_QUIT) {
		if (curtain->line) {
			for (int i = 0; i < curtain->count; i++) {
				if (curtain->line[i].data) {
					free(curtain->line[i].data);
				}
			}
			free(curtain->line);
			curtain->line = NULL;
		}
	}
}
void dos_text_curtain_add_line(_dos_text_curtain *curtain, const WORD fg_def, const WORD bg_def,
	const uTCHAR *font_family, const int font_size, const uTCHAR *fmt, ...) {
	WORD *ppu_image = NULL;
	uTCHAR text[1024];
	va_list ap;

	va_start(ap, fmt);
	uvsnprintf(text, usizeof(text), fmt, ap);
	va_end(ap);

	if (curtain->image.w < 0) {
		curtain->image.w = dos_text_pixels_w(font_family, font_size, text);
	}
	if (curtain->image.h < 0) {
		curtain->image.h = dos_text_pixels_h(font_family, font_size, text);
	}
	ppu_image = _dos_text_to_ppu_image(curtain->image.x, curtain->image.y, curtain->image.w, curtain->image.h,
		fg_def, bg_def, font_family, font_size, &text[0]);
	if (ppu_image) {
		_dos_text_ppu_image *line = (_dos_text_ppu_image *)realloc(curtain->line,
			(curtain->count + 1) * sizeof(_dos_text_ppu_image));

		if (line) {
			int index = curtain->count;

			curtain->line = line;
			curtain->line[index].data = ppu_image;
			curtain->count++;
		}
	}
}

void dos_text_pixels_size(int *w, int *h, const uTCHAR *font_family, int font_size, const uTCHAR *txt) {
	QFont font(uQString(font_family));
	QString line, subimage = "";
	bool is_name_subimage = false;
	bool is_subimage = false;
	int tw = 0, th = 0;
	QSize size(0, 0);

	line = uQString(txt);

	font.setPixelSize(font_size);
	font.setStretch(QFont::Unstretched);

	QFontMetrics fontMetrics(font);

	for (int i = 0; i < line.length(); i++) {
		QChar ch = line[i];

		if (ch == '[') {
			bool is_tag = false;

			for (const _dos_tag_ele &ele: dos_tags) {
				int len = ele.desc.length();

				if ((i + len) < line.length()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
					QStringView tag = QStringView{line}.mid(i, len);
#else
					QStringRef tag(&line, i, len);
#endif

					if (ele.desc == tag) {
						if ((tag == QStringLiteral("[image]")) && !is_name_subimage) {
							is_name_subimage = true;
						} else if ((tag == QStringLiteral("[endimage]")) && is_name_subimage) {
							is_name_subimage = false;
							is_subimage = true;
						}
						is_tag = !is_subimage;
						i += (len - 1);
						break;
					}
				}
			}
			if (is_tag) {
				continue;
			}
		} else if (is_name_subimage) {
			subimage.append(ch);
			continue;
		}
		if (is_subimage) {
			size = dos_image_dim(uQStringCD(subimage));
			subimage = "";
			is_subimage = false;
		} else {
			size = fontMetrics.size(0, ch);
		}
		tw += size.width();
		if (th < size.height()) {
			th = size.height();
		}
	}
	if (w) {
		(*w) = tw;
	}
	if (h) {
		(*h) = th;
	}
}
int dos_text_pixels_w(const uTCHAR *font_family, const int font_size, const uTCHAR *txt) {
	int w = 0;

	dos_text_pixels_size(&w, NULL, font_family, font_size, txt);
	return (w);
}
int dos_text_pixels_h(const uTCHAR *font_family, const int font_size, const uTCHAR *txt) {
	int h = 0;

	dos_text_pixels_size(NULL, &h, font_family, font_size, txt);
	return (h);
}

void dos_vline(BYTE nidx, int ppu_x, int ppu_y, int h, WORD color) {
	ppu_x = dos_ctr_x(ppu_x, 1, SCR_COLUMNS);
	ppu_y = dos_ctr_y(ppu_y, h, SCR_ROWS);

	for (int y1 = 0; y1 < h; y1++) {
		int px = ppu_x, py = ppu_y + y1;

		if (not_in_ppu() || (color == DOS_TRASPARENT)) {
			continue;
		}
		nes[nidx].p.ppu_screen.wr->line[py][px] = color;
	}
}
void dos_hline(BYTE nidx, int ppu_x, int ppu_y, int w, WORD color) {
	ppu_x = dos_ctr_x(ppu_x, w, SCR_COLUMNS);
	ppu_y = dos_ctr_y(ppu_y, 1, SCR_ROWS);

	for (int x1 = 0; x1 < w; x1++) {
		int px = ppu_x + x1, py = ppu_y;

		if (not_in_ppu() || (color == DOS_TRASPARENT)) {
			continue;
		}
		nes[nidx].p.ppu_screen.wr->line[py][px] = color;
	}
}
void dos_box(BYTE nidx, int ppu_x, int ppu_y, int w, int h, WORD color1, WORD color2, WORD bck) {
	ppu_x = dos_ctr_x(ppu_x, w, SCR_COLUMNS);
	ppu_y = dos_ctr_y(ppu_y, h, SCR_ROWS);

	dos_vline(nidx, ppu_x          , ppu_y          , h, color2);
	dos_hline(nidx, ppu_x          , ppu_y          , w, color1);
	dos_vline(nidx, ppu_x + (w - 1), ppu_y          , h, color1);
	dos_hline(nidx, ppu_x          , ppu_y + (h - 1), w, color2);

	for (int y1 = 1; y1 < (h - 1); y1++) {
		for (int x1 = 1; x1 < (w - 1); x1++) {
			int px = x1 + ppu_x, py = y1 + ppu_y;

			if (not_in_ppu() || (bck == DOS_TRASPARENT)) {
				continue;
			}
			nes[nidx].p.ppu_screen.wr->line[py][px] = bck;
		}
	}
}

void dos_image(BYTE nidx, int ppu_x, int ppu_y, int rect_x, int rect_y, int rect_w, int rect_h,
	const uTCHAR *resource, WORD *ppu_image, uint32_t ppu_image_pitch) {
	QImage src(uQString(resource));
	int max_w = rect_w, max_h = rect_h;

	rect_w = dos_ctr_w(rect_w, src.width(), rect_w);
	rect_h = dos_ctr_h(rect_h, src.height(), rect_h);
	rect_x = dos_ctr_x(rect_x, src.width(), max_w);
	rect_y = dos_ctr_y(rect_y, src.height(), max_h);
	if (!ppu_image) {
		ppu_x = dos_ctr_x(ppu_x, rect_w, SCR_COLUMNS);
		ppu_y = dos_ctr_y(ppu_y, rect_h, SCR_ROWS);
	}

	if (!rect_w || !rect_h) {
		return;
	}

	{
		QImage dst(rect_w, rect_h, src.format());
		QPainter painter(&dst);

		dst.fill(Qt::transparent);
		painter.drawImage(rect_x, rect_y, src);

		for (int iy = 0; iy < rect_h; iy++) {
			for (int ix = 0; ix < rect_w; ix++) {
				int px = ix + ppu_x, py = iy + ppu_y;
				long long pixel = 0;

				if (not_in_ppu() || not_in_qimage()) {
					continue;
				}
				pixel = static_cast<long long >(dst.pixel(ix, iy));
				if (!(pixel & 0xFF00000)) {
					continue;
				}
				if (ppu_image) {
					ppu_image[px + (py * ppu_image_pitch)] = dos_tag_value_from_opt(pixel);
				} else {
					nes[nidx].p.ppu_screen.wr->line[py][px] = dos_tag_value_from_opt(pixel);
				}
			}
		}
	}
}

void dos_draw_ppu_image(BYTE nidx, int ppu_x, int ppu_y, int rect_x, int rect_y, int rect_w, int rect_h,
	int img_w, int img_h, int scroll_x, int scroll_y, WORD *ppu_image) {
	if (ppu_image) {
		int ppux0 = (rect_x + ppu_x) < 0 ? 0 : rect_x + ppu_x;
		int ppuy0 = (rect_y + ppu_y) < 0 ? 0 : rect_y + ppu_y;
		int ppux1 = (ppux0 + rect_w) >= SCR_COLUMNS ? SCR_COLUMNS : ppux0 + rect_w;
		int ppuy1 = (ppuy0 + rect_h) >= SCR_ROWS ? SCR_ROWS : ppuy0 + rect_h;

		for (int iy = 0; iy < img_h; iy++) {
			const WORD *src_line = ppu_image + ((iy + rect_y) * img_w);
			int py = (ppuy0 + iy) + scroll_y;

			if ((py < ppuy0) || (py >= ppuy1)) {
				continue;
			}
			{
				WORD *dst_line = nes[nidx].p.ppu_screen.wr->line[py];

				for (int ix = 0; ix < img_w; ix++) {
					int px = (ppux0 + ix) + scroll_x;

					if ((px < ppux0) || (px >= ppux1)) {
						continue;
					}
					dst_line[px] = src_line[ix];
				}
			}
		}
	}
}

int dos_resource_w(const uTCHAR *resource) {
	return (QImageReader(uQString(resource)).size().width());
}
int dos_resource_h(const uTCHAR *resource) {
	return (QImageReader(uQString(resource)).size().height());
}
void dos_resource_size(int *w, int *h, const uTCHAR *resource) {
	QSize size = QImageReader(uQString(resource)).size();

	if (w) {
		(*w) = size.width();
	}
	if (h) {
		(*h) = size.height();
	}
}

uTCHAR *dos_tag_desc_from_value(const WORD value) {
	for (const _dos_tag_ele &ele : dos_tags) {
		if (ele.value == value) {
			return (uQStringCD(ele.desc));
		}
	}
	return (dos_tag_desc_from_value(DOS_NONE));
}
WORD dos_tag_value_from_desc(const uTCHAR *desc) {
	for (const _dos_tag_ele &ele : dos_tags) {
		if (ele.desc == uQString(desc)) {
			return (ele.value);
		}
	}
	return (DOS_NONE);
}
WORD dos_tag_value_from_opt(const long long opt) {
	for (const _dos_tag_ele &ele : dos_tags) {
		if (ele.opt == opt) {
			return (ele.value);
		}
	}
	return (DOS_NONE);
}

static INLINE int dos_ctr_x(int x, int w, int max_w) {
	if (x == DOS_ALIGNHCENTER) {
		x = dos_round((double)(max_w - w), 2.0f);
	} else if (x == DOS_ALIGNLEFT) {
		x = 0;
	} else if (x == DOS_ALIGNRIGHT) {
		x = max_w - w;
	}
	return (x);
}
static INLINE int dos_ctr_y(int y, int h, int max_h) {
	if (y == DOS_ALIGNVCENTER) {
		y = dos_round((double)(max_h - h), 2.0f);
	} else if (y == DOS_ALIGNTOP) {
		y = 0;
	} else if (y == DOS_ALIGNBOTTOM) {
		y = (max_h - h);
	}
	return (y);
}
static INLINE int dos_ctr_w(int w, int min_w, int max_w) {
	if (w < 0) {
		w = min_w;
	}
	if (max_w < 0) {
		max_w = w;
	}
	if (w > max_w) {
		w = max_w;
	}
	return (w);
}
static INLINE int dos_ctr_h(int h, int min_h, int max_h) {
	if (h < 0) {
		h = min_h;
	}
	if (max_h < 0) {
		max_h = h;
	}
	if (h > max_h) {
		h = max_h;
	}
	return (h);
}
static INLINE int dos_round(double d0, double d1) {
	double result = d0 / d1;

	if (remainder(d0, d1) >= 0.5) {
		result = round(result);
	}
	return (result);
}
static INLINE QSize dos_image_dim(const uTCHAR *resource) {
	return (QImageReader(uQString(resource)).size());
}
static WORD *_dos_text_to_ppu_image(int rect_x, int rect_y, int rect_w, int rect_h,
	const WORD fg_def, const WORD bg_def, const uTCHAR *font_family, const int font_size, const uTCHAR *text) {
	static uint mask0 = qRgb(255, 255, 255), mask1 = qRgb(0, 0, 0);
	QFont font(uQString(font_family));
	WORD fg = fg_def, bg = bg_def;
	WORD *ppu_image = NULL;
	QString line;

	line = uQString(text);

	// minimo 11 per i caratteri unicode
	font.setPixelSize(font_size);
	font.setStretch(QFont::Unstretched);

	{
		int char_x = 0, wpixels = 0, hpixels = 0;
		QFontMetrics fontMetrics(font);
		QString subimage = "";
		bool is_bck_tag = false;
		bool is_subimage = false;
		bool is_name_subimage = false;
		int alignv = DOS_ALIGNVCENTER;
		int alignh = DOS_ALIGNLEFT;
		int max_w = rect_w, max_h = rect_h;

		dos_text_pixels_size(&wpixels, &hpixels, font_family, font_size, &text[0]);

		if (!wpixels || !hpixels) {
			return (NULL);
		}

		rect_w = dos_ctr_w(rect_w, wpixels, rect_w);
		rect_h = dos_ctr_h(rect_h, hpixels, rect_h);
		rect_x = dos_ctr_x(rect_x, wpixels, max_w);
		rect_y = dos_ctr_y(rect_y, hpixels, max_h);

		ppu_image = (WORD *)malloc(rect_w * rect_h * sizeof(WORD));

		QImage mask(rect_w, rect_h, QImage::Format_Mono);
		mask.setColorTable(QVector<QRgb>{ mask0, mask1 });
		// nero
		mask.fill(1);

		QPainter painter(&mask);
		painter.setFont(font);
		painter.setPen(mask0);

		// pulisco l'intera zona
		for (int iy = 0; iy < rect_h; iy++) {
			for (int ix = 0; ix < rect_w; ix++) {
				ppu_image[ix + (iy * rect_w)] = bg_def;
			}
		}

		char_x = rect_x;

		// disegno la stringa
		for (int i = 0; i < line.length(); i++) {
			QChar ch = line[i];
			int chw = 0;

			if (ch == '[') {
				bool is_tag = false;

				for (const _dos_tag_ele &ele : dos_tags) {
					int len = ele.desc.length();

					if ((i + len) < line.length()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
						QStringView tag = QStringView{line}.mid(i, len);
#else
						QStringRef tag(&line, i, len);
#endif

						if (ele.desc == tag) {
							if ((tag == QStringLiteral("[image]")) && !is_name_subimage) {
								is_name_subimage = true;
							} else if ((tag == QStringLiteral("[endimage]")) && is_name_subimage) {
								is_name_subimage = false;
								is_subimage = true;
							} else  if (tag == QStringLiteral("[bck]")) {
								is_bck_tag = true;
							} else if (tag == QStringLiteral("[top]")) {
								alignv = DOS_ALIGNTOP;
							} else if (tag == QStringLiteral("[bottom]")) {
								alignv = DOS_ALIGNBOTTOM;
							} else if (tag == QStringLiteral("[vcenter]")) {
								alignv = DOS_ALIGNVCENTER;
							} else if (tag == QStringLiteral("[left]")) {
								alignh = DOS_ALIGNLEFT;
							} else if (tag == QStringLiteral("[right]")) {
								alignh = DOS_ALIGNRIGHT;
							} else if (tag == QStringLiteral("[hcenter]")) {
								alignh = DOS_ALIGNHCENTER;
							} else {
								bool is_normal = tag == QStringLiteral("[normal]");

								if (is_bck_tag) {
									bg = is_normal ? bg_def : ele.value;
								} else {
									fg = is_normal ? fg_def : ele.value;
								}
								is_bck_tag = false;
							}
							is_tag = !is_subimage;
							i += (len - 1);
							break;
						}
					}
				}
				if (is_tag) {
					continue;
				}
			} else if (is_name_subimage) {
				subimage.append(ch);
				continue;
			}
			if (is_subimage) {
				QImage src(subimage);

				chw = src.width();
				char_x = rect_x;
				rect_x += chw;
				if (char_x >= rect_w) {
					break;
				}
				dos_image(0, char_x, 0, alignh, alignv, chw, rect_h, uQStringCD(subimage), ppu_image, rect_w);
				subimage = "";
				is_subimage = false;
			} else {
				chw = fontMetrics.size(0, ch).width();
				char_x = rect_x;
				rect_x += chw;
				if (char_x >= rect_w) {
					break;
				}
				// Disegno della lettera
				painter.drawText(char_x, rect_y, rect_w, rect_h, Qt::AlignLeft | Qt::AlignTop, ch);
				// disegno sull'immagine PPU
				for (int iy = 0; iy < rect_h; iy++) {
					for (int ix = char_x; ix < (char_x + chw); ix++) {
						WORD ppu_pixel = DOS_TRASPARENT;

						if (not_in_qimage()) {
							continue;
						}
						ppu_pixel = static_cast<uint>(mask.pixel(ix, iy)) == mask0 ? fg : bg;
						if (ppu_pixel != DOS_TRASPARENT) {
							ppu_image[ix + (iy * rect_w)] = ppu_pixel;
						}
					}
				}
			}
		}
	}
	return (ppu_image);
}
