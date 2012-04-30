/*
 * menu_settings.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_settings.h"
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
  /* length: header (24) + pixel_data (559) */
  "\0\0\2G"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\206\377\377\377\0\4M\231\4<O\231\6\323N\232\6\305N\230\4>\214\377\377"
  "\377\0\4N\231\6\335\256\337\177\377r\305!\377N\232\6\310\214\377\377"
  "\377\0\4N\232\6\377\320\363\256\377\212\3424\377N\232\6\377\214\377\377"
  "\377\0\4N\232\6\377\320\363\256\377\212\3424\377N\232\6\377\214\377\377"
  "\377\0\4N\232\6\377\320\363\256\377\212\3424\377N\232\6\377\214\377\377"
  "\377\0\4N\232\6\377\320\363\256\377\212\3424\377N\232\6\377\211\377\377"
  "\377\0\12L\230\5/N\232\6\327N\233\6\313N\232\6\377\320\363\256\377\212"
  "\3424\377N\232\6\377N\232\6\341N\232\6\314N\230\4>\206\377\377\377\0"
  "\12N\232\6\312\256\342{\377\256\345y\377u\2706\377\320\363\256\377\212"
  "\3424\377z\273<\377\273\352\216\377\234\332`\377N\232\5\276\206\377\377"
  "\377\0\12N\231\6\316\227\332W\377\237\347X\377\307\361\237\377\320\363"
  "\255\377\212\3424\377\235\347V\377\234\346S\377x\315'\377N\232\6\305"
  "\206\377\377\377\0\12N\230\4>O\234\7\360m\300\36\377\214\3418\377\237"
  "\347X\377\212\3424\377\211\3403\377m\276\35\377P\233\10\355N\233\6.\205"
  "\377\377\377\0\5\277\301\273\377\202\205\206\377@L\77\377E}\23\377Z\251"
  "\20\377\202{\320)\377\5Z\250\17\377E}\23\3771\77""1\377.46\377\272\275"
  "\266\377\204\377\377\377\0\14\323\324\320\377\377\377\377\377\366\367"
  "\365\377\365\366\364\377\373\373\372\377\376\376\376\377\371\372\370"
  "\377\364\365\363\377\357\360\355\377\351\353\347\377\343\346\342\377"
  "\265\270\261\377\204\377\377\377\0\4\244\246\241\377\357\360\357\377"
  "\344\345\342\377\341\341\337\377\203\377\377\377\377\5\355\355\354\377"
  "\315\316\312\377\342\343\340\377\345\346\344\377\235\240\232\377\204"
  "\377\377\377\0\14\206\211\203\377\353\354\352\377\342\343\340\377\320"
  "\321\316\377\271\272\267\377\220\222\215\377\210\212\205\377\215\217"
  "\213\377\315\316\312\377\342\343\340\377\353\354\352\377\206\211\203"
  "\377\204\377\377\377\0\1oqm\377\212\361\362\361\377\1oqm\377\204\377"
  "\377\377\0\214XZV\377\202\377\377\377\0"};

enum {
	MSAVEONEXIT,
	NUMCHKS
};

void switch_saveOnExit(void);

static GtkWidget *check[NUMCHKS];

void menu_settings(GtkWidget *mainmenu, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *settings, *savenow;

	menu = gtk_menu_new();
	settings = gtk_menu_item_new_with_mnemonic("Sett_ings");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mainmenu), settings);

	/* mode */
	menu_mode(menu, accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	/* video */
	menu_video(menu, accel_group);
	/* audio */
	menu_audio(menu, accel_group);
	/* input */
	menu_input(menu, accel_group);
#ifdef __NETPLAY__
	/* netplay */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	menu_netplay(menu, accel_group);
#endif
	/* game genie */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	menu_gamegenie(menu, accel_group);

	/* Save Settings */
	savenow = gtk_image_menu_item_new_with_mnemonic("Save se_ttings");
	check[MSAVEONEXIT] = gtk_check_menu_item_new_with_mnemonic("Save settings on e_xit");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), savenow);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSAVEONEXIT]);

	icon_inline(savenow, icon_inline)

	gtk_widget_add_accelerator(savenow, "activate", accel_group, GDK_w, GDK_CONTROL_MASK,
			GTK_ACCEL_VISIBLE);

	g_signal_connect(G_OBJECT(savenow), "activate", G_CALLBACK(cfgfileSave), NULL);
	g_signal_connect(G_OBJECT(check[MSAVEONEXIT]), "activate", G_CALLBACK(switch_saveOnExit), NULL);

}
void menu_settings_check(void) {
	menu_mode_check();
	menu_video_check();
	menu_audio_check();
	/* game genie */
	menu_gamegenie_check();
	/* Save on Exit*/
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSAVEONEXIT]), FALSE);
	if (cfg->saveOnExit) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSAVEONEXIT]), TRUE);
	}
}
void switch_saveOnExit(void) {
	if (guiupdate) {
		return;
	}

	cfg->saveOnExit = !cfg->saveOnExit;
}
