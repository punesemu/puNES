/*
 * menu_video_palette.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_palette.h"
#include "param.h"
#include "palette.h"
#include "sdlgfx.h"

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
  /* length: header (24) + pixel_data (958) */
  "\0\0\3\326"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\203\377\377\377\0\11\0\330\15\3\23\320\4T5\321\14\257U\322\25\332t\324"
  "\30\364\214\326\30\361\242\330\23\324\271\333\15\242\331\344\6E\206\377"
  "\377\377\0\14\4\325:\"\11\320\40\257=\327:\377|\346j\377\252\363\210"
  "\377\304\371\226\377\323\373\227\377\333\367\207\377\340\357h\377\345"
  "\347\32\372\353\325\12\230\364\273\5\22\203\377\377\377\0\177\0\330e"
  "\40\16\325L\317X\341t\377\231\366\241\377\246\377\236\377\236\377{\377"
  "\256\377p\377\307\377p\377\343\377|\377\372\377\245\377\373\364\230\377"
  "\365\330U\377\371\267\15\266\374\215\0\21\377\377\377\0\0\336\200\6\11"
  "\332w\260W\342\224\377\235\371\273\377n\377\212\377v\377y\377\227\377"
  "\202\377\266\377\211\377\321\377\207\377\354\377~\377\376\373m\377\377"
  "\350v\377\374\337\233\377\373\272T\377\366\202\7\216\377\377\377\0\0"
  "\337\226R8\341\240\377\227\366\313\377l\377\256\377z\377\244\377\215"
  "\377\235\377\243\377\233\377\304\377\244\377\337\377\242\377\370\377"
  "\227\377\377\357\205\377\377\323n\377\377\305x\377\374\305\222\377\364"
  "n\24\366\360R\0""2\10\346\262\255g\353\306\377\233\377\335\377o\377\305"
  "\377\214\377\303\377\241\377\276\377\263\377\271\377\317\377\275\377"
  "\356\377\275\377\377\370\260\377\377\336\231\377\377\302~\377\377\243"
  "c\377\377\275\236\377\370\203Y\377\3426\3\212\16\352\315\333\202\365"
  "\344\377o\377\345\377y\377\340\377\225\377\340\377\257\377\337\377\307"
  "\377\334\377\340\377\334\377\372\376\331\377\377\351\305\377\377\312"
  "\247\377\377\252\207\377\377\213i\377\377\216x\377\373\206s\377\335\""
  "\10\276\16\354\352\364\222\372\371\377d\375\372\377{\375\372\377\230"
  "\375\371\377\265\375\372\377\323\376\372\377\357\376\372\377\377\363"
  "\357\377\377\325\320\377\377\263\257\377\377\221\215\377\377ol\377\377"
  "ig\377\373\203\202\377\331\15\11\333\16\332\361\361\220\362\377\377b"
  "\351\377\377v\350\377\377\223\347\377\377\260\347\377\377\312\346\377"
  "\377\345\345\377\377\375\336\374\377\377\306\341\377\377\246\302\377"
  "\377\204\241\377\377f~\377\377dv\377\366{\210\377\325\10\32\330\11\277"
  "\365\325\200\335\376\377l\321\377\377k\307\377\377\205\307\377\377\236"
  "\304\377\377\263\275\377\377\320\277\377\377\361\277\377\377\377\260"
  "\364\377\377\226\326\377\377y\266\377\377^\225\377\377w\231\377\354e"
  "\202\377\320\4*\267/\0\246\373\236`\276\375\377\234\320\377\377[\245"
  "\377\377q\242\377\377\204\232\377\377\233\227\377\377\277\237\377\377"
  "\340\237\377\377\373\222\376\377\377|\347\377\377f\311\377\377O\250\377"
  "\375\225\276\377\335@r\377\314\0""6y\0\202\372>\13|\370\373\216\271\373"
  "\377j\227\377\377[y\377\377kq\377\377\211v\377\377\255|\377\377\315{"
  "\377\377\355q\377\377\377c\367\377\377R\331\377\377|\317\377\354s\263"
  "\377\314\6Y\353\314\0F\"\377\377\377\0\0V\360\222Dw\360\377\222\246\374"
  "\377o{\377\377XQ\377\377yZ\377\377\233^\377\377\274\\\377\377\332W\377"
  "\377\367L\377\377\377{\361\377\361\205\326\377\3174\227\377\302\0ao\202"
  "\377\377\377\0\16\0G\353\16\0*\342\256@R\351\377\204\205\370\377\237"
  "\226\377\377\221t\377\377\232`\377\377\267`\377\377\330x\377\377\357"
  "\222\374\377\352q\350\377\3142\266\377\275\0\211\221\316\0m\5\203\377"
  "\377\377\0\14\0\36\345\14\0\4\323\205\14\4\324\361Y@\341\377\204c\356"
  "\377\241s\363\377\262q\360\377\266Z\344\377\2646\322\377\264\2\276\346"
  "\272\0\255n\314\0\227\2\206\377\377\377\0\10\7\0\320.\34\0\303~1\0\277"
  "\266D\1\276\325Z\0\273\322r\0\271\256\212\0\270r\257\0\301!\204\377\377"
  "\377\0"};

enum {
	MPALETTEPAL,
	MPALETTENTSC,
	MPALETTESONY,
	MPALETTEMONO,
	MPALETTEGREEN,
	NUMCHKS
};

void set_palette(int newpalette);

static GtkWidget *check[NUMCHKS];

void menu_video_palette(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *palette;

	menu = gtk_menu_new();
	palette = gtk_image_menu_item_new_with_mnemonic("_Palette");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(palette), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), palette);

	icon_inline(palette, icon_inline)

	check[MPALETTEPAL] = gtk_check_menu_item_new_with_mnemonic("_PAL");
	check[MPALETTENTSC] = gtk_check_menu_item_new_with_mnemonic("_NTSC");
	check[MPALETTESONY] = gtk_check_menu_item_new_with_mnemonic("_Sony CXA2025AS US");
	check[MPALETTEMONO] = gtk_check_menu_item_new_with_mnemonic("_Monochrome");
	check[MPALETTEGREEN] = gtk_check_menu_item_new_with_mnemonic("_Green");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTEPAL]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTENTSC]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTESONY]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTEMONO]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTEGREEN]);

	g_signal_connect_swapped(G_OBJECT(check[MPALETTEPAL]), "activate", G_CALLBACK(set_palette),
	        GINT_TO_POINTER(0));
	g_signal_connect_swapped(G_OBJECT(check[MPALETTENTSC]), "activate", G_CALLBACK(set_palette),
	        GINT_TO_POINTER(1));
	g_signal_connect_swapped(G_OBJECT(check[MPALETTESONY]), "activate", G_CALLBACK(set_palette),
	        GINT_TO_POINTER(2));
	g_signal_connect_swapped(G_OBJECT(check[MPALETTEMONO]), "activate", G_CALLBACK(set_palette),
	        GINT_TO_POINTER(3));
	g_signal_connect_swapped(G_OBJECT(check[MPALETTEGREEN]), "activate", G_CALLBACK(set_palette),
	        GINT_TO_POINTER(4));
}
void menu_video_palette_check(void) {
	int index;

	for (index = MPALETTEPAL; index < NUMCHKS; index++) {
		if (gfx.palette == index) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}
	}
}

void set_palette(int newpalette) {
	if (guiupdate) {
		return;
	}

	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, newpalette, FALSE);
}
