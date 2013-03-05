/*
 * menu_video_size.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_size.h"
#include "param.h"
#include "gfx.h"
#include "cfg_file.h"

#ifdef __SUNPRO_C
#pragma align 4 (icon_inline)
#endif
#ifdef __GNUC__
static const guint8 icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 icon_inline[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (852) */
  "\0\0\3l"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\203\377\377\377\0\3+e\266\16.f\267\1773i\270\303\202/g\267\362\3""3"
  "i\270\303.f\267\177+e\266\16\207\377\377\377\0\12)`\255B/b\256\352h\202"
  "\310\372\227\241\335\377\266\270\352\377\304\306\356\377\274\302\350"
  "\377\224\250\327\373/e\260\352)a\256B\205\377\377\377\0\14'\\\245B2a"
  "\252\365\204\215\322\377t{\272\377.:n\373\27\37J\376\23\32A\376,7c\374"
  "\236\243\303\377\330\334\360\377Co\260\365'\\\245B\203\377\377\377\0"
  "\16$W\235\16(Y\237\352w\201\311\377IU\227\374\"+c\374#0V\266=BS\214e"
  "jt\233*6U\271\31\"O\374^f\226\375\275\302\345\377*Z\237\352%W\235\16"
  "\202\377\377\377\0\16$S\225\177Je\253\371ck\263\377(2q\37435@\201fff"
  "Z\334\334\333\223\372\372\372\347\351\351\351\313PT\\\214!)^\374\213"
  "\220\277\377q\207\276\372%T\226\177\202\377\377\377\0\16&Q\217\303bo"
  "\266\3777C\212\373+8f\266```]\237\233\2338\330\330\323,\372\372\372\237"
  "\361\361\361\252\215\215\215r'3\\\2667D\200\374\221\231\317\377&Q\217"
  "\303\202\377\377\377\0\16!J\205\362jp\273\3775=\210\37637N\216\201\201"
  "~I\321\310\310\34\335\335\314\17\337\337\317\20\324\314\314\36\201\201"
  "~I16J\216-6y\376\227\232\326\377!J\206\362\202\377\377\377\0\6\37E}\362"
  "dk\265\3778A\217\3767=U\212\215\215\211A\333\333\316\25\202\333\333\333"
  "\7\6\333\333\316\25\215\215\211A7;Q\2122:\202\376\211\215\315\377\37"
  "F~\362\202\377\377\377\0\6\37Cw\303Q]\242\377<H\226\3731=q\265\207\207"
  "\203F\325\312\312\30\202\352\352\325\14\6\325\312\312\30\207\207\203"
  "F.:j\265;G\217\373js\266\377\37Cw\303\202\377\377\377\0\6\33>p\1775L"
  "\210\370PZ\244\377<F\230\374Z`qf\252\252\2450\202\323\323\323\27\6\252"
  "\252\2450Z`nf7@\215\375]e\256\377BV\222\374\33>o\177\202\377\377\377"
  "\0\6\31=n\16\34>p\352MW\236\377CO\236\373>H\236\374\77M\207\253\202]"
  "g\213e\6\77L\206\253:E\226\375GR\235\376[d\254\377\35<j\376\34\36!d\203"
  "\377\377\377\0\16\31=nB\36\77r\364JT\234\377LV\243\377@L\241\373AK\245"
  "\376AJ\244\376\77L\236\373PZ\247\377R\\\244\377#Ct\377pt{\377$\40\34"
  "\366\31\26\23F\203\377\377\377\0\16\31=nB\33>p\3521G\204\370DQ\226\377"
  "PW\242\377QX\244\377GS\232\3772H\205\374\33;i\3753BT\373qnj\377\206\177"
  "u\377\40\34\31\363\31\26\23""5\203\377\377\377\0\3\31=n\16\32=o\177\35"
  ">q\303\202\33>p\362\10\35>q\303\32=o\177\31\33\37m\32\27\24\354233\357"
  "nke\377tne\375\34\30\25\355\213\377\377\377\0\5\31\26\23H\31\26\23\353"
  "+(#\341D>7\377\31\26\23\377\214\377\377\377\0\4\31\26\23""0\31\26\23"
  "\343\31\26\23\377\31\26\23\340"};

enum {
	MX1 = 1,
	MX2,
	MX3,
	MX4,
	NUMCHKS
};

void set_scale(int scale);

static GtkWidget *check[NUMCHKS];

void menu_video_size(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *size;

	menu = gtk_menu_new();
	size = gtk_image_menu_item_new_with_mnemonic("_Size");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(size), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), size);

	icon_inline(size, icon_inline)

	check[MX1] = gtk_check_menu_item_new_with_mnemonic("_1x");
	check[MX2] = gtk_check_menu_item_new_with_mnemonic("_2x");
	check[MX3] = gtk_check_menu_item_new_with_mnemonic("_3x");
	check[MX4] = gtk_check_menu_item_new_with_mnemonic("_4x");

	gtk_widget_add_accelerator(check[MX1], "activate", accel_group, GDK_1, 0,
			GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MX2], "activate", accel_group, GDK_2, 0,
			GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MX3], "activate", accel_group, GDK_3, 0,
			GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MX4], "activate", accel_group, GDK_4, 0,
			GTK_ACCEL_VISIBLE);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MX1]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MX2]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MX3]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MX4]);

	g_signal_connect_swapped(G_OBJECT(check[MX1]), "activate", G_CALLBACK(set_scale),
	        GINT_TO_POINTER(1));
	g_signal_connect_swapped(G_OBJECT(check[MX2]), "activate", G_CALLBACK(set_scale),
	        GINT_TO_POINTER(2));
	g_signal_connect_swapped(G_OBJECT(check[MX3]), "activate", G_CALLBACK(set_scale),
	        GINT_TO_POINTER(3));
	g_signal_connect_swapped(G_OBJECT(check[MX4]), "activate", G_CALLBACK(set_scale),
	        GINT_TO_POINTER(4));
}
void menu_video_size_check(void) {
	int index;

	if (cfg->filter != NO_FILTER) {
		gtk_widget_set_sensitive(check[MX1], FALSE);
	} else {
		gtk_widget_set_sensitive(check[MX1], TRUE);
	}

	for (index = MX1; index < NUMCHKS; index++) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
	}

	if (cfg->fullscreen == NO_FULLSCR) {
		switch (cfg->scale) {
			case X1:
				index = MX1;
				break;
			case X2:
				index = MX2;
				break;
			case X3:
				index = MX3;
				break;
			case X4:
				index = MX4;
				break;
		}
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
	}
}
void set_scale(int scale) {
	if (gui_in_update) {
		return;
	}

	switch (scale) {
		case X1:
			gfx_set_screen(X1, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			return;
		case X2:
			gfx_set_screen(X2, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			return;
		case X3:
			gfx_set_screen(X3, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			return;
		case X4:
			gfx_set_screen(X4, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
			return;
	}
}
