/*
 * menu_video_fullscreen.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_fullscreen.h"
#include "menu_video_rendering.h"
#include "menu_video_vsync.h"
#include "menu_video_effect.h"
#include "opengl.h"
#include "sdl_gfx.h"
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
  /* length: header (24) + pixel_data (489) */
  "\0\0\2\1"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\207\377\377\377\0\2\0U\252\3\27F\213\13\216\377\377\377\0\2#L\212|#"
  "N\211\247\215\377\377\377\0\4\36G\210+#N\212\365(Q\216\365\"J\206L\213"
  "\377\377\377\0\2@@\200\4\40J\207\316\202\40J\207\377\2!K\210\350\34G"
  "\216\22\227\377\377\377\0\4\0U\252\3\377\377\377\0\210\214\205I\216\220"
  "\214\342\204\210\212\205\377\4\216\220\214\342\210\214\205I\377\377\377"
  "\0""33\231\5\203\377\377\377\0\5\33Q\206\23\40J\207\262\377\377\377\0"
  "\216\220\214\342\330\331\327\377\203\374\374\374\377\15\306\310\302\377"
  "\310\312\305\377\216\220\214\342\377\377\377\0!J\210|\37H\206J\377\377"
  "\377\0\"J\206&$N\212\345\"L\211\366\377\377\377\0\210\212\205\377\360"
  "\362\357\377\203\353\355\351\377\15\300\303\274\377\314\317\311\377\210"
  "\212\205\377\377\377\377\0#M\211\214\40J\210\376\40L\207h\40I\206v#M"
  "\211\373\"L\210\367\377\377\377\0\210\212\205\377\354\356\353\377\204"
  "\377\377\377\377\14\354\356\353\377\210\212\205\377\377\377\377\0#M\211"
  "\214#M\212\376#M\212\323\377\377\377\0\"L\211[!K\210\352\377\377\377"
  "\0\210\212\205\377\361\363\360\377\204\377\377\377\377\6\361\363\360"
  "\377\210\212\205\377\377\377\377\0\"J\210\211%O\212\274\27F\213\13\202"
  "\377\377\377\0\3!L\210/\377\377\377\0\210\212\205\377\206\364\365\363"
  "\377\4\210\212\205\377\377\377\377\0\37I\2071\0\200\200\2\205\377\377"
  "\377\0\210\210\212\205\377\211\377\377\377\0\1\0\200\200\2\204\40P\200"
  "\20\1@@\200\4\212\377\377\377\0\6\0U\252\3!K\210\314\40K\210\375!K\210"
  "\375!K\210\336\32M\200\12\213\377\377\377\0\4\37K\211)$N\212\365%O\213"
  "\365\37L\2069\215\377\377\377\0\2\"L\211y#M\212\214\207\377\377\377\0"};

enum {
	MFULLSCR,
	MSTRETCHFSCR,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_video_fullscreen(GtkWidget *video, GtkAccelGroup *accel_group) {
	/* Fullscreen and Stretching */
	check[MFULLSCR] = gtk_image_menu_item_new_with_mnemonic("F_ullscreen");
	check[MSTRETCHFSCR] = gtk_check_menu_item_new_with_mnemonic("S_tretch in fullscreen");

	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MFULLSCR]);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MSTRETCHFSCR]);

	icon_inline(check[MFULLSCR], icon_inline)

	gtk_widget_add_accelerator(check[MFULLSCR], "activate", accel_group, GDK_Return, GDK_MOD1_MASK,
	        GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MSTRETCHFSCR], "activate", accel_group, GDK_p, 0,
	        GTK_ACCEL_VISIBLE);

	g_signal_connect(G_OBJECT(check[MFULLSCR]), "activate",
	        G_CALLBACK(guiFullscreen), NULL);
	g_signal_connect(G_OBJECT(check[MSTRETCHFSCR]), "activate",
	        G_CALLBACK(menu_video_fullscreen_switch_stretch), NULL);
}
void menu_video_fullscreen_check(void) {
	if (gfx.opengl) {
		gtk_widget_set_sensitive(check[MFULLSCR], TRUE);
		gtk_widget_set_sensitive(check[MSTRETCHFSCR], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MFULLSCR], FALSE);
		gtk_widget_set_sensitive(check[MSTRETCHFSCR], FALSE);
	}

	/* Stretch */
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSTRETCHFSCR]), FALSE);
	if (gfx.opengl && !cfg->aspect_ratio) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSTRETCHFSCR]), TRUE);
	}
}
void menu_video_fullscreen_switch_stretch(void) {
	if (guiupdate) {
		return;
	}

	cfg->aspect_ratio = !cfg->aspect_ratio;

	if (cfg->fullscreen == FULLSCR) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
	}
}
