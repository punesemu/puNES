/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#define _DOS_STATIC_
#include "draw_on_screen.h"
#undef _DOS_STATIC_
#include "draw_on_screen_font.h"
#include "ppu.h"

static char dos_tags[][10] = {
	"[normal]",  "[red]",	  "[yellow]",  "[green]",
	"[cyan]",    "[brown]",	  "[blue]",    "[gray]",
	"[black]",
	"[bck]",
	"[left]",    "[right]",	  "[up]",      "[down]",
	"[select1]", "[select2]", "[select3]",
	"[start1]",  "[start2]",  "[start3]",
	"[a]",       "[b]",
	"[floppy]",
	"[play]",    "[next]",    "[prev]",    "[pause]",
	"[stop]",
	"[rmouse]",  "[lmouse]",
	"[vrc7a]",   "[vrc7b]",   "[vrc6a]",   "[vrc6b]",
	"[mmc5a]",   "[mmc5b]",   "[n163a]",   "[n163b]",
	"[s5b1]",    "[s5b2]",    "[fds1]",    "[fds2]"
};

void _dos_text(int x, int y, int l, int r, int b, int t, const char *fmt, ...) {
	va_list ap;
	unsigned int i = 0, tlength = 0;
	int w = 0, pixels = 0, dlength = 0;
	char text[1024];
	WORD color = doscolor(DOS_NORMAL), background = doscolor(DOS_BLACK);
	BYTE first_char = TRUE, last_char = FALSE, is_bck_color = FALSE;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	if (l < 0) {
		l = 0;
	}
	if (r < 0) {
		r = 0;
	}

	tlength = strlen(text);
	dlength = dos_strlen(text);
	w = dlength * 8 - l - r;

	if (x >= DOS_CENTER) {
		if (x == DOS_CENTER) {
			x = (SCR_COLUMNS - w) >> 1;
		} else if (x == DOS_LEFT) {
			x = 0;
		} else if (x == DOS_RIGHT) {
			x = SCR_COLUMNS - w;
		}
		if (x < 0) {
			x = 0;
		}
	}
	if (y >= DOS_CENTER) {
		if (y == DOS_CENTER) {
			y = SCR_ROWS >> 1;
		} else if (y == DOS_UP) {
			y = 0;
		} else if (y == DOS_DOWN) {
			y = SCR_ROWS - 8;
		}
		if (y < 0) {
			y = 0;
		}
	}

	i = 0;

	for (pixels = x; pixels < SCR_COLUMNS;) {
		unsigned int font_x = 0, font_y = 0;
		int xl = 0, xr = 0;
		unsigned char ch = 0;

		if (i < tlength) {
			ch = text[i];
		} else {
			break;
		}

		if (ch == '[') {
			unsigned int tag = 0, found = FALSE;

			for (tag = 0; tag < LENGTH(dos_tags); tag++) {
				size_t len = strlen(dos_tags[tag]);

				if (strncmp(text + i, dos_tags[tag], len) == 0) {
					if (tag <= DOS_BLACK) {
						if (is_bck_color) {
							is_bck_color = FALSE;
							background = doscolor(tag);
						} else {
							color = doscolor(tag);
						}
						i += len;
						found = TRUE;
					} else if (tag == DOS_BACKGROUND_COLOR) {
						is_bck_color = TRUE;
						i += len;
						found = TRUE;
					} else if (tag <= DOS_TAGS) {
						ch = tag - DOS_BUTTON_LEFT;
						i += len - 1;
						found = FALSE;
					}
					break;
				}
			}

			if (found) {
				continue;
			}
		}

		if (i == (tlength - 1)) {
			last_char = TRUE;
		}
		if (first_char) {
			xl = l;
		}
		if (last_char) {
			xr = r;
		}
		pixels += (8 - xl - xr);
		if (pixels > SCR_COLUMNS) {
			break;
		}

		switch (ch) {
/* riga 0 */
			case ' ':
				font_x = dospf(0);
				font_y = dospf(0);
				break;
			case '!':
				font_x = dospf(1);
				font_y = dospf(0);
				break;
			case '"':
				font_x = dospf(2);
				font_y = dospf(0);
				break;
			case '#':
				font_x = dospf(3);
				font_y = dospf(0);
				break;
			case '$':
				font_x = dospf(4);
				font_y = dospf(0);
				break;
			case '%':
				font_x = dospf(5);
				font_y = dospf(0);
				break;
			case '&':
				font_x = dospf(6);
				font_y = dospf(0);
				break;
			case 0x27: // '
				font_x = dospf(7);
				font_y = dospf(0);
				break;
			case '(':
				font_x = dospf(8);
				font_y = dospf(0);
				break;
			case ')':
				font_x = dospf(9);
				font_y = dospf(0);
				break;
			case '*':
				font_x = dospf(10);
				font_y = dospf(0);
				break;
			case '+':
				font_x = dospf(11);
				font_y = dospf(0);
				break;
			case ',':
				font_x = dospf(12);
				font_y = dospf(0);
				break;
			case '-':
				font_x = dospf(13);
				font_y = dospf(0);
				break;
			case '.':
				font_x = dospf(14);
				font_y = dospf(0);
				break;
			case '/':
				font_x = dospf(15);
				font_y = dospf(0);
				break;
/* riga 1 */
			case '0':
				font_x = dospf(0);
				font_y = dospf(1);
				break;
			case '1':
				font_x = dospf(1);
				font_y = dospf(1);
				break;
			case '2':
				font_x = dospf(2);
				font_y = dospf(1);
				break;
			case '3':
				font_x = dospf(3);
				font_y = dospf(1);
				break;
			case '4':
				font_x = dospf(4);
				font_y = dospf(1);
				break;
			case '5':
				font_x = dospf(5);
				font_y = dospf(1);
				break;
			case '6':
				font_x = dospf(6);
				font_y = dospf(1);
				break;
			case '7':
				font_x = dospf(7);
				font_y = dospf(1);
				break;
			case '8':
				font_x = dospf(8);
				font_y = dospf(1);
				break;
			case '9':
				font_x = dospf(9);
				font_y = dospf(1);
				break;
			case ':':
				font_x = dospf(10);
				font_y = dospf(1);
				break;
			case ';':
				font_x = dospf(11);
				font_y = dospf(1);
				break;
			case '<':
				font_x = dospf(12);
				font_y = dospf(1);
				break;
			case '=':
				font_x = dospf(13);
				font_y = dospf(1);
				break;
			case '>':
				font_x = dospf(14);
				font_y = dospf(1);
				break;
			case '?':
				font_x = dospf(15);
				font_y = dospf(1);
				break;
/* riga 2 */
			case '@':
				font_x = dospf(0);
				font_y = dospf(2);
				break;
			case 'A':
				font_x = dospf(1);
				font_y = dospf(2);
				break;
			case 'B':
				font_x = dospf(2);
				font_y = dospf(2);
				break;
			case 'C':
				font_x = dospf(3);
				font_y = dospf(2);
				break;
			case 'D':
				font_x = dospf(4);
				font_y = dospf(2);
				break;
			case 'E':
				font_x = dospf(5);
				font_y = dospf(2);
				break;
			case 'F':
				font_x = dospf(6);
				font_y = dospf(2);
				break;
			case 'G':
				font_x = dospf(7);
				font_y = dospf(2);
				break;
			case 'H':
				font_x = dospf(8);
				font_y = dospf(2);
				break;
			case 'I':
				font_x = dospf(9);
				font_y = dospf(2);
				break;
			case 'J':
				font_x = dospf(10);
				font_y = dospf(2);
				break;
			case 'K':
				font_x = dospf(11);
				font_y = dospf(2);
				break;
			case 'L':
				font_x = dospf(12);
				font_y = dospf(2);
				break;
			case 'M':
				font_x = dospf(13);
				font_y = dospf(2);
				break;
			case 'N':
				font_x = dospf(14);
				font_y = dospf(2);
				break;
			case 'O':
				font_x = dospf(15);
				font_y = dospf(2);
				break;
/* riga 3 */
			case 'P':
				font_x = dospf(0);
				font_y = dospf(3);
				break;
			case 'Q':
				font_x = dospf(1);
				font_y = dospf(3);
				break;
			case 'R':
				font_x = dospf(2);
				font_y = dospf(3);
				break;
			case 'S':
				font_x = dospf(3);
				font_y = dospf(3);
				break;
			case 'T':
				font_x = dospf(4);
				font_y = dospf(3);
				break;
			case 'U':
				font_x = dospf(5);
				font_y = dospf(3);
				break;
			case 'V':
				font_x = dospf(6);
				font_y = dospf(3);
				break;
			case 'W':
				font_x = dospf(7);
				font_y = dospf(3);
				break;
			case 'X':
				font_x = dospf(8);
				font_y = dospf(3);
				break;
			case 'Y':
				font_x = dospf(9);
				font_y = dospf(3);
				break;
			case 'Z':
				font_x = dospf(10);
				font_y = dospf(3);
				break;
			case '[':
				font_x = dospf(11);
				font_y = dospf(3);
				break;
			case '\\':
				font_x = dospf(12);
				font_y = dospf(3);
				break;
			case ']':
				font_x = dospf(13);
				font_y = dospf(3);
				break;
			case '^':
				font_x = dospf(14);
				font_y = dospf(3);
				break;
			case '_':
				font_x = dospf(15);
				font_y = dospf(3);
				break;
/* riga 4 */
			case '`':
				font_x = dospf(0);
				font_y = dospf(4);
				break;
			case 'a':
				font_x = dospf(1);
				font_y = dospf(4);
				break;
			case 'b':
				font_x = dospf(2);
				font_y = dospf(4);
				break;
			case 'c':
				font_x = dospf(3);
				font_y = dospf(4);
				break;
			case 'd':
				font_x = dospf(4);
				font_y = dospf(4);
				break;
			case 'e':
				font_x = dospf(5);
				font_y = dospf(4);
				break;
			case 'f':
				font_x = dospf(6);
				font_y = dospf(4);
				break;
			case 'g':
				font_x = dospf(7);
				font_y = dospf(4);
				break;
			case 'h':
				font_x = dospf(8);
				font_y = dospf(4);
				break;
			case 'i':
				font_x = dospf(9);
				font_y = dospf(4);
				break;
			case 'j':
				font_x = dospf(10);
				font_y = dospf(4);
				break;
			case 'k':
				font_x = dospf(11);
				font_y = dospf(4);
				break;
			case 'l':
				font_x = dospf(12);
				font_y = dospf(4);
				break;
			case 'm':
				font_x = dospf(13);
				font_y = dospf(4);
				break;
			case 'n':
				font_x = dospf(14);
				font_y = dospf(4);
				break;
			case 'o':
				font_x = dospf(15);
				font_y = dospf(4);
				break;
/* riga 5 */
			case 'p':
				font_x = dospf(0);
				font_y = dospf(5);
				break;
			case 'q':
				font_x = dospf(1);
				font_y = dospf(5);
				break;
			case 'r':
				font_x = dospf(2);
				font_y = dospf(5);
				break;
			case 's':
				font_x = dospf(3);
				font_y = dospf(5);
				break;
			case 't':
				font_x = dospf(4);
				font_y = dospf(5);
				break;
			case 'u':
				font_x = dospf(5);
				font_y = dospf(5);
				break;
			case 'v':
				font_x = dospf(6);
				font_y = dospf(5);
				break;
			case 'w':
				font_x = dospf(7);
				font_y = dospf(5);
				break;
			case 'x':
				font_x = dospf(8);
				font_y = dospf(5);
				break;
			case 'y':
				font_x = dospf(9);
				font_y = dospf(5);
				break;
			case 'z':
				font_x = dospf(10);
				font_y = dospf(5);
				break;
			case '{':
				font_x = dospf(11);
				font_y = dospf(5);
				break;
			case '|':
				font_x = dospf(12);
				font_y = dospf(5);
				break;
			case '}':
				font_x = dospf(13);
				font_y = dospf(5);
				break;
			case '~':
				font_x = dospf(14);
				font_y = dospf(5);
				break;
/* altro */
			case 0: // DOS_BUTTON_LEFT
				font_x = dospf(5);
				font_y = dospf(21);
				break;
			case 1: // DOS_BUTTON_RIGHT
				font_x = dospf(6);
				font_y = dospf(21);
				break;
			case 2: // DOS_BUTTON_UP
				font_x = dospf(3);
				font_y = dospf(21);
				break;
			case 3: // DOS_BUTTON_DOWN
				font_x = dospf(4);
				font_y = dospf(21);
				break;
			case 4: // DOS_BUTTON_SELECT1
				font_x = dospf(9);
				font_y = dospf(21);
				break;
			case 5: // DOS_BUTTON_SELECT2
				font_x = dospf(10);
				font_y = dospf(21);
				break;
			case 6: // DOS_BUTTON_SELECT3
				font_x = dospf(11);
				font_y = dospf(21);
				break;
			case 7: // DOS_BUTTON_START1
				font_x = dospf(12);
				font_y = dospf(21);
				break;
			case 8: // DOS_BUTTON_START2
				font_x = dospf(13);
				font_y = dospf(21);
				break;
			case 9: // DOS_BUTTON_START3
				font_x = dospf(14);
				font_y = dospf(21);
				break;
			case 10: // DOS_BUTTON_A
				font_x = dospf(8);
				font_y = dospf(21);
				break;
			case 11: // DOS_BUTTON_B
				font_x = dospf(7);
				font_y = dospf(21);
				break;
			case 12: // DOS_FLOPPY
				font_x = dospf(1);
				font_y = dospf(19);
				break;
			case 13: // DOS_PLAY
				font_x = dospf(15);
				font_y = dospf(15);
				break;
			case 14: // DOS_NEXT
				font_x = dospf(11);
				font_y = dospf(9);
				break;
			case 15: // DOS_PREV
				font_x = dospf(11);
				font_y = dospf(8);
				break;
			case 16: // DOS_PAUSE
				font_x = dospf(2);
				font_y = dospf(21);
				break;
			case 17: // DOS_STOP
				font_x = dospf(7);
				font_y = dospf(18);
				break;
			case 18: // DOS_MOUSE_RIGHT
				font_x = dospf(15);
				font_y = dospf(21);
				break;
			case 19: // DOS_MOUSE_LEFT
				font_x = dospf(0);
				font_y = dospf(22);
				break;
			case 20: // DOS_MOUSE_VRC7A
				font_x = dospf(1);
				font_y = dospf(22);
				break;
			case 21: // DOS_MOUSE_VRC7B
				font_x = dospf(2);
				font_y = dospf(22);
				break;
			case 22: // DOS_MOUSE_VRC6A
				font_x = dospf(5);
				font_y = dospf(22);
				break;
			case 23: // DOS_MOUSE_VRC6B
				font_x = dospf(6);
				font_y = dospf(22);
				break;
			case 24: // DOS_MOUSE_MMC5A
				font_x = dospf(3);
				font_y = dospf(22);
				break;
			case 25: // DOS_MOUSE_MMC5B
				font_x = dospf(4);
				font_y = dospf(22);
				break;
			case 26: // DOS_MOUSE_N163A
				font_x = dospf(7);
				font_y = dospf(22);
				break;
			case 27: // DOS_MOUSE_N163B
				font_x = dospf(8);
				font_y = dospf(22);
				break;
			case 28: // DOS_MOUSE_S5B1
				font_x = dospf(9);
				font_y = dospf(22);
				break;
			case 29: // DOS_MOUSE_S5B2
				font_x = dospf(10);
				font_y = dospf(22);
				break;
			case 30: // DOS_MOUSE_FDS1
				font_x = dospf(11);
				font_y = dospf(22);
				break;
			case 31: // DOS_MOUSE_FDS2
				font_x = dospf(12);
				font_y = dospf(22);
				break;
			default:
				break;
		}

		{
			int x1 = 0, y1 = 0;

			for (y1 = 0; y1 < 8; y1++) {
				char *list = font_8x8[font_y] + font_x + xl;

				if ((y + y1) >= SCR_ROWS) {
					break;
				} else if ((t >= 0) && ((y + y1) < (y + t))) {
					font_y++;
					continue;
				} else if ((b >= 0) && ((y + y1) > (y + b))) {
					font_y++;
					continue;
				}

				for (x1 = 0; x1 < (8 - xl - xr); x1++) {
					if ((x + x1) >= SCR_COLUMNS) {
						break;
					}

					if (list[x1] == '@') {
						nes[0].p.ppu_screen.wr->line[y + y1][x + x1] = color;
					} else if (list[x1] == ',') {
						nes[0].p.ppu_screen.wr->line[y + y1][x + x1] = 0x0010;
					} else if (list[x1] == '.') {
						nes[0].p.ppu_screen.wr->line[y + y1][x + x1] = 0x002D;
					} else {
						nes[0].p.ppu_screen.wr->line[y + y1][x + x1] = background;
					}
				}
				font_y++;
			}
		}
		x += (8 - xl - xr);
		i++;
		first_char = FALSE;
	}
}
int dos_strlen(const char *fmt, ...) {
	va_list ap;
	unsigned int i = 0;
	int tmp = 0, tag = 0, length = 0;
	char text[1024];

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	for (i = 0; i < strlen(text); i++) {
		if ((text[i] == '[') && ((tmp = dos_is_tag(text + i, &tag)) > 0)) {
			i += (tmp - 1);
			if (tag <= DOS_BACKGROUND_COLOR) {
				continue;
			}
		}
		length++;
	}

	return (length);
}
int dos_is_tag(const char *text, int *tag_founded) {
	int len = 0;

	if (text[0] == '[') {
		BYTE found = FALSE;
		int tag = 0;

		for (tag = 0; tag < (int)LENGTH(dos_tags); tag++) {
			len = (int)strlen(dos_tags[tag]);

			if (strncmp(text, dos_tags[tag], len) == 0) {
				(*tag_founded) = tag;
				found = TRUE;
				break;
			}
		}
		if (!found) {
			len = 0;
			(*tag_founded) = -1;
		}
	}
	return (len);
}

void dos_vline(int x, int y, int h, WORD color) {
	int y1 = 0;

	if (x >= DOS_CENTER) {
		if (x == DOS_CENTER) {
			x = (SCR_COLUMNS - 1) >> 1;
		} else if (x == DOS_LEFT) {
			x = 0;
		} else if (x == DOS_RIGHT) {
			x = SCR_COLUMNS - 1;
		}
		if (x < 0) {
			x = 0;
		}
	}
	if (y >= DOS_CENTER) {
		if (y == DOS_CENTER) {
			y = (SCR_ROWS - h) >> 1;
		} else if (y == DOS_UP) {
			y = 0;
		} else if (y == DOS_DOWN) {
			y = SCR_ROWS - h;
		}
		if (y < 0) {
			y = 0;
		}
	}

	for (y1 = 0; y1 < h; y1++) {
		if ((y + y1) >= SCR_ROWS) {
			break;
		}
		if (x >= SCR_COLUMNS) {
			break;
		}
		nes[0].p.ppu_screen.wr->line[y + y1][x] = color;
	}
}
void dos_hline(int x, int y, int w, WORD color) {
	int x1 = 0;

	if (x >= DOS_CENTER) {
		if (x == DOS_CENTER) {
			x = (SCR_COLUMNS - w) >> 1;
		} else if (x == DOS_LEFT) {
			x = 0;
		} else if (x == DOS_RIGHT) {
			x = SCR_COLUMNS - w;
		}
		if (x < 0) {
			x = 0;
		}
	}
	if (y >= DOS_CENTER) {
		if (y == DOS_CENTER) {
			y = (SCR_ROWS - 1) >> 1;
		} else if (y == DOS_UP) {
			y = 0;
		} else if (y == DOS_DOWN) {
			y = SCR_ROWS - 1;
		}
		if (y < 0) {
			y = 0;
		}
	}

	for (x1 = 0; x1 < w; x1++) {
		if (y >= SCR_ROWS) {
			break;
		}
		if ((x + x1) >= SCR_COLUMNS) {
			break;
		}
		nes[0].p.ppu_screen.wr->line[y][x + x1] = color;
	}
}

void dos_box(int x, int y, int w, int h, WORD color1, WORD color2, WORD bck) {
	int y1 = 0;

	if (x >= DOS_CENTER) {
		if (x == DOS_CENTER) {
			x = (SCR_COLUMNS - w) >> 1;
		} else if (x == DOS_LEFT) {
			x = 0;
		} else if (x == DOS_RIGHT) {
			x = SCR_COLUMNS - w;
		}
		if (x < 0) {
			x = 0;
		}
	}
	if (y >= DOS_CENTER) {
		if (y == DOS_CENTER) {
			y = (SCR_ROWS - h) >> 1;
		} else if (y == DOS_UP) {
			y = 0;
		} else if (y == DOS_DOWN) {
			y = SCR_ROWS - h;
		}
		if (y < 0) {
			y = 0;
		}
	}

	dos_vline(x          , y          , h, color2);
	dos_hline(x          , y          , w, color1);
	dos_vline(x + (w - 1), y          , h, color1);
	dos_hline(x          , y + (h - 1), w, color2);

	for (y1 = 1; y1 < (h - 1); y1++) {
		dos_hline(x + 1, y + y1, w - 2, bck);
	}
}
