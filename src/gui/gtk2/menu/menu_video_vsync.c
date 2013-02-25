/*
 * menu_video_vsync.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_vsync.h"
#include "sdlgfx.h"
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
  /* length: header (24) + pixel_data (709) */
  "\0\0\2\335"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\221\377\377\377\0\3\211\211\211\342\210\210\210\377\207\207\207\326"
  "\205\377\377\377\0\3xxx\277sss\377qqq\326\205\377\377\377\0\13\211\211"
  "\211\377\323\323\323\377\204\204\204\377\256\313\365\377\254\310\365"
  "\377\253\310\365\377\251\305\365\377\247\305\365\377ooo\377\311\311\311"
  "\377ggg\377\205\377\377\377\0\13\206\206\206\315\203\203\203\377\214"
  "\221\231\377\300\325\367\377\276\325\367\377\275\323\367\377\273\322"
  "\367\377\272\321\367\377}\203\215\377ccc\377___\304\206\377\377\377\0"
  "\11\254\311\365\377\277\325\367\377\276\324\367\377\274\323\367\377\272"
  "\322\367\377\270\321\367\377\267\320\367\377\265\316\367\377\233\276"
  "\365\377\207\377\377\377\0\15\253\310\365\377\275\323\367\377\203\207"
  "\215\377qqq\377uy~\377\257\306\353\377\264\316\367\377\263\315\367\377"
  "\231\273\363\377\377\377\377\0BBB\314===\377888\314\203\377\377\377\0"
  "\15\251\305\365\377\273\322\367\377ooo\377\311\311\311\377bbb\377\204"
  "\251\327\377\206\257\345\377\205\254\343\377\201\252\343\377\177\246"
  "\341\377:::\377\267\267\267\377///\377\203\377\377\377\0\15\245\304\365"
  "\377\270\321\367\377tx\177\377___\377bej\377\232\266\333\377\241\277"
  "\352\377\236\275\351\377\234\272\350\377\231\270\347\377EKT\377,,,\377"
  "'''\315\202\377\377\377\0\15ppp\314kkk\377v{\203\377\264\316\367\377"
  "\210\257\345\377\242\300\353\377\240\276\352\377\236\274\350\377\233"
  "\272\350\377\230\267\346\377\225\264\345\377\222\261\344\377l\226\332"
  "\377\203\377\377\377\0\15ggg\377\306\306\306\377]]]\377\232\274\363\377"
  "\205\254\343\377\237\276\352\377\235\273\350\377\232\270\350\377\227"
  "\266\346\377\224\263\345\377\221\261\344\377\217\256\343\377g\221\331"
  "\377\203\377\377\377\0\15___\315YYY\377UUU\315\377\377\377\0\200\251"
  "\342\377\234\272\350\377\231\270\347\377\226\265\346\377\223\263\344"
  "\377\221\257\343\377\215\255\343\377\213\252\341\377c\215\330\377\207"
  "\377\377\377\0\11|\245\341\377\230\267\346\377\225\264\345\377\222\261"
  "\344\377\217\257\343\377\215\254\342\377\212\251\341\377\207\247\340"
  "\377_\211\325\377\206\377\377\377\0\13""333\342---\377:\77G\377\221\261"
  "\344\377\217\256\343\377\213\253\342\377\211\251\341\377\206\247\337"
  "\377,4B\377\12\12\12\377\7\7\7\326\205\377\377\377\0\13+++\377\262\262"
  "\262\377!!\"\377j\224\332\377g\220\330\377c\215\330\377_\211\325\377"
  "]\206\325\377\11\11\11\377\246\246\246\377\3\3\3\377\205\377\377\377"
  "\0\3###\315\37\37\37\376\32\32\32\307\205\377\377\377\0\3\4\4\4\273\1"
  "\1\1\377\0\0\0\304\221\377\377\377\0"};

enum {
	MVSYNC,
	MVSYNCON,
	MVSYNCOFF,
	NUMCHKS
};

void vsync_set(int bool);

static GtkWidget *check[NUMCHKS];

void menu_video_vsync(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu;

	/* Settings/Video/VSync */
	menu = gtk_menu_new();
	check[MVSYNC] = gtk_image_menu_item_new_with_mnemonic("_VSync");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(check[MVSYNC]), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MVSYNC]);

	icon_inline(check[MVSYNC], icon_inline)

	check[MVSYNCON] = gtk_check_menu_item_new_with_mnemonic("_On");
	check[MVSYNCOFF] = gtk_check_menu_item_new_with_mnemonic("O_ff");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MVSYNCON]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MVSYNCOFF]);

	g_signal_connect_swapped(G_OBJECT(check[MVSYNCON]), "activate", G_CALLBACK(vsync_set),
	        GINT_TO_POINTER(TRUE));
	g_signal_connect_swapped(G_OBJECT(check[MVSYNCOFF]), "activate", G_CALLBACK(vsync_set),
	        GINT_TO_POINTER(FALSE));
}
void menu_video_vsync_check(void) {
	if (gfx.opengl) {
		gtk_widget_set_sensitive(check[MVSYNC], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MVSYNC], FALSE);
	}

	/* VSync */
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MVSYNCON]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MVSYNCOFF]), FALSE);
	if (cfg->vsync) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MVSYNCON]), TRUE);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MVSYNCOFF]), TRUE);
	}
}
void vsync_set(int bool) {
	gint x, y;

	if (guiupdate) {
		return;
	}

	if (cfg->vsync == bool) {
		guiUpdate();
		return;
	}

	/* salvo la posizione */
	gtk_window_get_position(GTK_WINDOW(mainWin), &x, &y);

	/*
	 * se non nascondo la finestra, al momento del
	 * SDL_QuitSubSystem e del SDL_InitSubSystem
	 * l'applicazione crasha.
	 */
	gtk_widget_hide(mainWin);

	/* switch vsync */
	cfg->vsync = bool;

	gfxResetVideo();
	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);

	/* rispristino la posizione */
	gtk_window_move(GTK_WINDOW(mainWin), x, y);

	gtk_widget_show(mainWin);
}
