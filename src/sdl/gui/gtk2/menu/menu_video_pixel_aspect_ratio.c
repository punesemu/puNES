/*
 * menu_video_pixel_aspect_ratio.c
 *
 *  Created on: 16/mar/2014
 *      Author: fhorse
 */

#include "menu_video_pixel_aspect_ratio.h"
#include "cfg_file.h"
#include "opengl.h"
#include "gfx.h"

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
  /* length: header (24) + pixel_data (527) */
  "\0\0\2'"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\245\0\0\0\0\1\35|\314\337\204\0\0\0\0\1\20R\254\337\211\0\0\0\0\2H\234"
  "\336\336e\273\353\377\204\0\0\0\0\2,\214\325\377\10Q\245\336\207\0\0"
  "\0\0\3>\234\336\323\324\377\377\377Y\276\355\377\204\0\0\0\0\3'\220\334"
  "\3778\260\355\377\5H\241\323\205\0\0\0\0\4=\233\335\332\310\366\377\377"
  "}\333\374\377\203\321\366\377\2047\245\344\377\4[\262\346\377#\233\343"
  "\377\32\221\336\377\2G\240\332\203\0\0\0\0>I\241\340\323\306\362\377"
  "\377\206\335\376\377e\314\367\377w\321\372\377|\330\371\377m\314\367"
  "\377b\305\363\377U\276\360\3777\245\344\377\34\214\332\377\20\214\333"
  "\377\4\177\325\377\0G\242\323\0\0\0\0V\272\356\327\327\377\377\377{\332"
  "\376\377p\322\372\377n\317\367\377`\304\362\377Q\272\357\377D\263\353"
  "\3777\247\347\377,\235\342\377\"\222\336\377\27\211\330\377\7}\322\377"
  "\0z\323\377\1\207\336\377\0G\236\327R\264\353\264\211\322\365\377\217"
  "\340\377\377j\317\370\377h\312\366\377\\\301\362\377O\270\355\377E\260"
  "\352\3776\245\345\377,\231\340\377\34\217\333\377\16\203\325\377\1x\316"
  "\377\0\203\331\377\1f\275\377\0H\2427\0\0\0\0(\226\334\223\177\313\361"
  "\377\214\342\377\377W\303\364\377W\275\361\377J\270\356\377>\261\353"
  "\3771\245\346\377#\227\337\377\23\212\330\377\5|\321\377\0\203\331\377"
  "\1`\267\377\0""5\221\201\203\0\0\0\0\4\11h\303\271l\266\347\377|\330"
  "\375\377/\232\340\377\205\13n\305\377\3\0\206\334\377\3[\263\377\5""8"
  "\226\207\205\0\0\0\0\3\11h\303\307g\261\344\377Z\267\352\377\204\0\0"
  "\0\0\3\5q\311\377\3_\267\377\15D\235\220\207\0\0\0\0\3\26g\276\324)z"
  "\310\377,\204\317\1\202\0\0\0\0\3\13T\256\1\13P\252\377\15D\235\231\211"
  "\0\0\0\0\2\26g\276\263\34m\301\14\202\0\0\0\0\2\15K\244\14\15I\242\253"
  "\245\0\0\0\0"};

enum {
	MPAR,
	MPAR11,
	MPAR54,
	MPAR87,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_video_pixel_aspect_ratio(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu;

	menu = gtk_menu_new();
	check[MPAR] = gtk_image_menu_item_new_with_mnemonic("Pixel Aspect Ra_tio");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(check[MPAR]), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MPAR]);

	gw_image_from_inline(check[MPAR], icon_inline);

	check[MPAR11] = gtk_check_menu_item_new_with_mnemonic("_1:1");
	check[MPAR54] = gtk_check_menu_item_new_with_mnemonic("_5:4");
	check[MPAR87] = gtk_check_menu_item_new_with_mnemonic("_8:7 (NTSC TV)");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPAR11]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPAR54]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPAR87]);

	g_signal_connect_swapped(G_OBJECT(check[MPAR11]), "activate",
	        G_CALLBACK(menu_video_pixel_aspect_ratio_set), GINT_TO_POINTER(PAR11));
	g_signal_connect_swapped(G_OBJECT(check[MPAR54]), "activate",
	        G_CALLBACK(menu_video_pixel_aspect_ratio_set), GINT_TO_POINTER(PAR54));
	g_signal_connect_swapped(G_OBJECT(check[MPAR87]), "activate",
	        G_CALLBACK(menu_video_pixel_aspect_ratio_set), GINT_TO_POINTER(PAR87));
}
void menu_video_pixel_aspect_ratio_check(void) {
	int index = 0;

	if (gfx.opengl) {
		gtk_widget_set_sensitive(check[MPAR], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MPAR], FALSE);
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPAR11]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPAR54]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPAR87]), FALSE);

	switch (cfg->pixel_aspect_ratio) {
		case PAR11:
			index = MPAR11;
			break;
		case PAR54:
			index = MPAR54;
			break;
		case PAR87:
			index = MPAR87;
			break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
}
void menu_video_pixel_aspect_ratio_set(int par) {
	if (gui_in_update) {
		return;
	}

	if (cfg->pixel_aspect_ratio == par) {
		return;
	}

	cfg->pixel_aspect_ratio = par;

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);
}
