/*
 * menu_video_overscan.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_overscan.h"
#include "param.h"
#include "sdlgfx.h"
#include "cfgfile.h"

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
  /* length: header (24) + pixel_data (849) */
  "\0\0\3i"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\3\377\377\377\0\214\217\211\303\214\214\210o\212\377\377\377\0\2\211"
  "\213\207y\214\216\210\277\202\377\377\377\0\3\226\230\223\372\265\266"
  "\263\371\213\213\211c\210\377\377\377\0\3\212\212\205o\270\271\265\370"
  "\226\230\222\371\202\377\377\377\0\4\226\227\223\365\373\372\372\377"
  "\262\264\261\370\212\215\207W\206\377\377\377\0\4\212\214\205f\270\271"
  "\266\367\373\373\373\377\223\225\221\363\202\377\377\377\0\5\214\216"
  "\212\347\354\354\351\377\372\372\371\377\261\262\256\370\212\212\206"
  "L\204\377\377\377\0\5\212\212\210^\265\267\264\370\373\373\372\377\353"
  "\353\350\377\216\217\211\343\202\377\377\377\0\6\216\217\212\304\316"
  "\315\311\376\352\350\345\377\371\371\370\377\256\260\254\370\211\211"
  "\205C\202\377\377\377\0\6\210\213\205V\265\266\262\370\372\372\371\377"
  "\351\350\344\377\315\315\311\375\215\216\212\300\202\377\377\377\0\16"
  "\212\212\203#\226\227\222\363\337\336\332\377\345\343\337\377\367\367"
  "\366\377\254\256\252\366\210\210\204:\211\211\206N\264\264\262\370\370"
  "\367\367\377\344\342\336\377\336\334\330\377\223\225\221\364\212\212"
  "\203%\203\377\377\377\0\14\213\213\205\\\242\242\236\364\335\334\331"
  "\377\345\343\337\377\363\363\361\377\230\232\225\372\244\247\242\373"
  "\363\363\362\377\344\342\336\377\333\332\326\377\236\237\232\366\210"
  "\213\205V\205\377\377\377\0\12\210\213\204O\225\226\222\363\321\320\315"
  "\376\244\245\241\377\332\333\331\377\241\242\236\377\251\253\246\377"
  "\315\315\311\374\224\226\221\363\206\212\206J\207\377\377\377\0\10\207"
  "\207\207\"\215\217\213\344\214\216\212\377\337\340\335\377\336\337\334"
  "\377\214\216\212\377\217\221\214\334\207\207\207\40\205\377\377\377\0"
  "\2\245\0\0A\250\2\2\333\202\245\1\1\373\6\246\21\20\376\253\224\221\377"
  "\233\234\227\377\231\232\225\377\232\223\217\377\244\14\14\375\202\244"
  "\0\0\377*\250\2\2\341\244\0\0I\377\377\377\0\245\0\0R\255\13\13\365\335"
  ">>\377\360PP\377\360QQ\377\32300\376\246\13\12\377\217zu\326\214\201"
  "|\340\247\15\15\377\340ee\376\362TT\377\362RR\377\340@@\377\257\16\16"
  "\364\244\0\0W\251\3\3\272\343ll\377\313**\370\251\6\6\366\253\11\11\372"
  "\362SS\377\250\4\4\367\245\0\0\37\242\0\0\36\250\5\4\370\362SS\377\250"
  "\6\6\374\247\3\3\371\310((\367\332<<\377\251\3\3\275\246\1\1\366\354"
  "MM\377\247\5\5\371\244\0\0""5\245\4\4\376\354MM\377\246\1\1\366\202\377"
  "\377\377\0\16\246\1\1\364\353LL\377\245\4\4\376\244\0\0""5\250\5\5\371"
  "\354MM\377\246\1\1\366\252\3\3\311\335>>\377\300\"\"\365\246\6\6\374"
  "\302%%\366\32499\375\251\3\3\244\202\377\377\377\0\16\251\2\2\240\322"
  "99\374\306))\366\251\7\7\371\302%%\365\334==\377\251\4\4\307\246\0\0"
  "r\271\25\25\366\347EE\377\356OO\377\336<<\377\254\12\12\365\244\0\0""5"
  "\202\377\377\377\0\15\243\0\0""2\253\11\11\365\33399\377\354MM\377\347"
  "CC\377\267\24\24\366\245\0\0o\277\0\0\4\250\2\2\223\246\2\2\357\246\1"
  "\1\366\252\2\2\321\243\0\0@\204\377\377\377\0\6\243\0\0=\251\3\3\311"
  "\247\1\1\361\247\2\2\351\250\2\2\212\252\0\0\3"};

#ifdef __SUNPRO_C
#pragma align 4 (default_icon_inline)
#endif
#ifdef __GNUC__
static const guint8 default_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 default_icon_inline[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (769) */
  "\0\0\3\31"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\223\377\377\377\0\4\376\302\0""3\375\301\0\273\374\277\0\377\373\276"
  "\0U\202\377\377\377\0\4\366\270\0f\365\266\0\377\363\264\0\273\361\262"
  "\0\"\205\377\377\377\0\5\376\302\0f\375\311\"\377\376\334g\377\376\340"
  "y\377\371\273\0\273\202\377\377\377\0\5\364\266\0\314\375\335z\377\371"
  "\316Q\377\357\257\0\356\355\254\0U\204\377\377\377\0\14\375\300\0D\374"
  "\314-\377\376\345\207\377\377\351\221\377\370\300\25\377\366\267\0\""
  "\364\265\0D\365\301)\377\377\341\200\377\374\331r\377\355\254\0\356\353"
  "\252\0""3\205\377\377\377\0\12\372\275\0w\373\320D\377\377\350\221\377"
  "\372\323W\377\364\265\0\231\362\263\0\231\371\320V\377\377\340~\377\364"
  "\302:\377\352\251\0f\207\377\377\377\0\10\367\271\0\273\372\320M\377"
  "\376\345\211\377\362\262\0\356\371\320X\377\377\341\201\377\365\307F"
  "\377\352\251\0\231\204\377\377\377\0\2\373\276\0""3\371\274\0""3\202"
  "\377\377\377\0\10\365\266\0\21\363\264\0\356\376\342\203\377\376\336"
  "p\377\377\335q\377\373\325f\377\352\250\0\314\350\246\0\21\202\377\377"
  "\377\0""2\341\236\0""3\337\233\0""3\371\274\0\273\371\304\"\377\366\270"
  "\0\377\365\266\0\356\363\264\0\273\361\262\0\314\374\331g\377\377\333"
  "d\377\377\331`\377\373\323_\377\347\245\0\314\345\243\0\273\343\240\0"
  "\335\341\235\0\377\345\250\32\377\334\227\0\273\370\272\0\273\375\337"
  "z\377\377\354\244\377\375\340\201\377\373\333u\377\373\332r\377\377\340"
  "y\377\377\332a\377\377\330\\\377\377\333k\377\372\325o\377\370\320e\377"
  "\373\327u\377\377\336\205\377\365\313_\377\326\220\0\273\366\267\0\273"
  "\364\265\0\377\362\263\0\377\360\261\0\314\357\256\0\273\355\254\0\273"
  "\372\322]\377\377\331a\377\377\327[\377\372\320\\\377\342\237\0\314\340"
  "\235\0\273\336\232\0\335\333\225\0\377\326\217\0\377\321\211\0\273\204"
  "\377\377\377\0\10\354\254\0\21\352\251\0\356\375\336}\377\374\325c\377"
  "\377\332g\377\371\320^\377\340\234\0\314\336\231\0\21\210\377\377\377"
  "\0\10\352\251\0\273\363\305H\377\372\330r\377\344\241\0\335\363\306R"
  "\377\377\333s\377\355\273A\377\331\223\0\231\207\377\377\377\0\12\352"
  "\250\0\210\361\301>\377\377\342\205\377\356\273:\377\341\236\0f\337\233"
  "\0\231\361\302M\377\377\330l\377\345\2566\377\316\206\0w\205\377\377"
  "\377\0\14\351\250\0U\357\2733\377\376\340\202\377\374\332w\377\341\235"
  "\0\356\337\233\0\21\334\227\0D\342\246$\377\377\327j\377\371\320g\377"
  "\324\222\31\377\305{\0""3\204\377\377\377\0\5\347\245\0U\345\242\0\356"
  "\362\306Q\377\357\276D\377\336\232\0w\202\377\377\377\0\5\321\212\0\314"
  "\362\306`\377\342\252<\377\304z\0\356\301u\0U\205\377\377\377\0\4\342"
  "\237\0\"\340\235\0\252\336\232\0\273\333\225\0\"\202\377\377\377\0\4"
  "\314\203\0U\310~\0\273\304y\0w\300u\0\21\203\377\377\377\0"};

enum {
	MOSCANOFF,
	MOSCANON,
	MOSCANDEF,
	MOSCANDON,
	MOSCANDOFF,
	NUMCHKS
};

void set_overscan(int newoscan);

static GtkWidget *check[NUMCHKS];

void menu_video_overscan(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu[2], *overscan, *ovscan;

	menu[0] = gtk_menu_new();
	overscan = gtk_image_menu_item_new_with_mnemonic("_Overscan");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(overscan), menu[0]);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), overscan);

	icon_inline(overscan, icon_inline)

	check[MOSCANDEF] = gtk_check_menu_item_new_with_mnemonic("_Default");
	check[MOSCANON] = gtk_check_menu_item_new_with_mnemonic("_On");
	check[MOSCANOFF] = gtk_check_menu_item_new_with_mnemonic("O_ff");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu[0]), check[MOSCANDEF]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu[0]), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu[0]), check[MOSCANON]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu[0]), check[MOSCANOFF]);

	g_signal_connect_swapped(G_OBJECT(check[MOSCANDEF]), "activate", G_CALLBACK(set_overscan),
	        GINT_TO_POINTER(MOSCANDEF));
	g_signal_connect_swapped(G_OBJECT(check[MOSCANON]), "activate", G_CALLBACK(set_overscan),
	        GINT_TO_POINTER(MOSCANON));
	g_signal_connect_swapped(G_OBJECT(check[MOSCANOFF]), "activate", G_CALLBACK(set_overscan),
	        GINT_TO_POINTER(MOSCANOFF));

	gtk_menu_shell_append(GTK_MENU_SHELL(menu[0]), gtk_separator_menu_item_new());

	/* Settings/Video/Overscan/Default */
	menu[1] = gtk_menu_new();
	ovscan = gtk_image_menu_item_new_with_mnemonic("De_fault value");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(ovscan), menu[1]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu[0]), ovscan);

	icon_inline(ovscan, default_icon_inline)

	check[MOSCANDON] = gtk_check_menu_item_new_with_mnemonic("_On");
	check[MOSCANDOFF] = gtk_check_menu_item_new_with_mnemonic("O_ff");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu[1]), check[MOSCANDON]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu[1]), check[MOSCANDOFF]);

	g_signal_connect_swapped(G_OBJECT(check[MOSCANDON]), "activate",
			G_CALLBACK(set_overscan), GINT_TO_POINTER(OSCAN_DEFAULT_ON));
	g_signal_connect_swapped(G_OBJECT(check[MOSCANDOFF]), "activate",
			G_CALLBACK(set_overscan), GINT_TO_POINTER(OSCAN_DEFAULT_OFF));

}
void menu_video_overscan_check(void) {
	int index = 0;

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MOSCANON]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MOSCANOFF]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MOSCANDEF]), FALSE);
	switch (cfg->oscan) {
		case OSCAN_ON:
			index = MOSCANON;
			break;
		case OSCAN_OFF:
			index = MOSCANOFF;
			break;
		case OSCAN_DEFAULT:
			index = MOSCANDEF;
			break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MOSCANDON]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MOSCANDOFF]), FALSE);
	switch (cfg->oscan_default) {
		case OSCAN_ON:
			index = MOSCANDON;
			break;
		case OSCAN_OFF:
			index = MOSCANDOFF;
			break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
}
void set_overscan(int newoscan) {
	if (guiupdate) {
		return;
	}

	switch (newoscan) {
		case OSCAN_ON:
		case OSCAN_OFF:
		case OSCAN_DEFAULT:
			cfg->oscan = newoscan;
			cfgfilePgsSave();
			break;
		case OSCAN_DEFAULT_OFF:
			cfg->oscan_default = OSCAN_OFF;
			break;
		case OSCAN_DEFAULT_ON:
			cfg->oscan_default = OSCAN_ON;
			break;
	}

	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);
}
