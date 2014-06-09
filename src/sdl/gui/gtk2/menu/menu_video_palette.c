/*
 * menu_video_palette.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_palette.h"
#include "param.h"
#include "palette.h"
#include "gfx.h"
#include "cfg_file.h"
#include "gui.h"

#if defined (__SUNPRO_C)
#pragma align 4 (icon_inline)
#endif
#if defined (__GNUC__)
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

#if defined (__SUNPRO_C)
#pragma align 4 (savepalette_icon_inline)
#endif
#if defined (__GNUC__)
static const guint8 savepalette_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 savepalette_icon_inline[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (858) */
  "\0\0\3r"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\7""6k\274q6k\274\2206k\274\3146k\274\3566k\274\3726k\273\3766k\273\377"
  "\2026j\273\377*9l\274\377;n\275\377:m\273\3778k\273\357>p\273\3135i\266"
  "T\377\377\377\0""6k\274\233\321\340\366\377\321\340\367\377\370\373\376"
  "\377\367\373\376\377\366\371\375\377\360\365\374\377\352\360\372\377"
  "\355\362\373\377\366\371\375\377\367\372\375\377\353\361\373\377\337"
  "\351\370\376\275\320\354\373^\211\311\3545i\265c6k\274\330\321\337\366"
  "\377\200\252\351\377\366\372\376\377\366\372\375\377d\214\310\377\356"
  "\363\373\377\352\361\373\377\362\366\374\377\370\373\376\377\361\366"
  "\374\377\342\354\371\377\333\347\370\377\272\320\356\377\275\320\354"
  "\377>p\273\3706k\274\360\320\337\366\377~\250\350\377\202\361\366\374"
  "\377\16d\214\310\377\351\361\372\377\356\364\373\377\367\372\375\377"
  "\366\371\375\377\350\360\372\377\335\350\370\377\333\346\367\377z\243"
  "\341\377\303\325\357\3775i\267\3766k\274\372\314\335\365\377~\250\347"
  "\377\202\350\360\372\377\16f\215\311\377\351\360\372\377\363\370\375"
  "\377\370\372\376\377\357\364\374\377\337\351\371\377\333\347\367\377"
  "\331\345\367\377x\242\340\377\251\302\347\3775h\266\3776k\273\376\311"
  "\334\364\377}\247\347\377\202\341\354\371\377\25\343\355\371\377\356"
  "\364\374\377\367\372\375\377\363\367\375\377\345\355\372\377\333\347"
  "\367\377\331\345\367\377\330\345\366\377w\240\336\377\244\276\344\377"
  "4g\264\3776k\273\377\307\331\364\377}\246\346\377d\214\310\377e\215\311"
  "\377g\216\311\377l\222\313\377m\222\313\377i\220\312\377e\214\310\377"
  "\203d\214\310\377\25t\234\332\377\237\272\341\3774f\263\3776j\273\377"
  "\305\330\362\377{\244\343\377z\243\343\377z\244\343\377{\244\342\377"
  "{\243\342\377{\243\341\377y\242\341\377w\240\337\377v\237\336\377t\236"
  "\335\377r\234\333\377t\235\334\377\232\265\335\3774e\261\3776j\273\377"
  "\302\325\362\377\202z\243\343\377\202{\243\342\377\14{\244\342\377y\242"
  "\341\377x\241\340\377w\240\336\377u\236\336\377t\235\334\377s\233\332"
  "\377s\233\331\377\225\260\332\3773d\257\3776j\273\377\276\322\360\377"
  "\202z\243\342\377\17z\243\341\377{\243\342\377{\243\341\377x\241\340"
  "\377w\237\336\377v\237\335\377t\235\334\377r\233\331\377q\231\330\377"
  "p\231\326\377\216\253\325\3773c\255\3776j\272\377\273\320\357\377z\242"
  "\342\377\212\370\373\376\377\7m\226\323\377\212\247\322\3772b\253\377"
  "8k\273\377\270\316\357\377y\242\341\377\367\372\376\377\210\210\300b"
  "\377\10\366\371\375\377j\223\317\377\204\243\316\3772a\252\3778l\273"
  "\377\266\314\356\377z\242\341\377\367\372\376\377\210\302\334\277\377"
  "\10\366\371\375\377h\220\315\377\201\236\314\3772a\250\3777k\272\376"
  "\263\312\355\377z\242\340\377\367\372\376\377\210\210\300b\377\7\366"
  "\371\375\377e\215\312\377|\233\311\3771`\247\3765j\272\336\255\306\353"
  "\377\255\305\352\377\212\370\373\376\377\23|\232\310\377y\230\307\377"
  "1`\247\3556j\272\2175i\271\2655i\270\3565h\267\3775h\265\3774g\264\377"
  "4f\262\3773e\260\3773d\256\3772c\254\3772b\252\3772a\251\3771`\250\377"
  "1`\247\3761`\246\3611a\250\304"};

enum {
	MPALETTEPAL,
	MPALETTENTSC,
	MPALETTESONY,
	MPALETTEMONO,
	MPALETTEGREEN,
	MPALETTEFILE,
	NUMCHKS
};

void set_palette(int palette);
void load_palette(void);
void save_palette(void);

static GtkWidget *check[NUMCHKS];

void menu_video_palette(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *palette, *pal_save;

	menu = gtk_menu_new();
	palette = gtk_image_menu_item_new_with_mnemonic("_Palette");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(palette), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), palette);

	gw_image_from_inline(palette, icon_inline);

	check[MPALETTEPAL] = gtk_check_menu_item_new_with_mnemonic("_PAL");
	check[MPALETTENTSC] = gtk_check_menu_item_new_with_mnemonic("_NTSC");
	check[MPALETTESONY] = gtk_check_menu_item_new_with_mnemonic("S_ony CXA2025AS US");
	check[MPALETTEMONO] = gtk_check_menu_item_new_with_mnemonic("_Monochrome");
	check[MPALETTEGREEN] = gtk_check_menu_item_new_with_mnemonic("_Green");
	check[MPALETTEFILE] = gtk_check_menu_item_new_with_mnemonic("_Load from file");
	pal_save = gtk_image_menu_item_new_with_mnemonic("_Save to file");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTEPAL]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTENTSC]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTESONY]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTEMONO]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTEGREEN]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPALETTEFILE]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), pal_save);

	gw_image_from_inline(pal_save, savepalette_icon_inline);

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
	g_signal_connect_swapped(G_OBJECT(check[MPALETTEFILE]), "activate", G_CALLBACK(load_palette),
			NULL);
	g_signal_connect(G_OBJECT(pal_save), "activate", G_CALLBACK(save_palette), NULL);
}
void menu_video_palette_check(void) {
	BYTE pf = FALSE;
	int index;

	if (strlen(cfg->palette_file) != 0) {
		pf = TRUE;
	}

	for (index = MPALETTEPAL; index < NUMCHKS; index++) {
		if ((cfg->palette == index) && (pf == FALSE)) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}

		if (pf == TRUE) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPALETTEFILE]), TRUE);
		}
	}
}

void set_palette(int palette) {
	if (gui_in_update) {
		return;
	}

	memset(cfg->palette_file, 0x00, sizeof(cfg->palette_file));
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, palette, FALSE, TRUE);
}
void load_palette(void) {
	GtkWidget *dialog;

	if (gui_in_update) {
		return;
	}

	emu_pause(TRUE);

	dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(main_win),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);

	file_open_filter_add(dialog, "PAL files", "*.pal;*.PAL");
	file_open_filter_add(dialog, "All files", "*.*;");

	//gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), gui.last_state_path);

	g_timeout_redraw_start();

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if (emu_file_exist(file_name) == EXIT_OK) {
			memset(cfg->palette_file, 0x00, sizeof(cfg->palette_file));
			strncpy(cfg->palette_file, file_name, sizeof(cfg->palette_file) - 1);
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, TRUE);
		} else {
			text_add_line_info(1, "[red]error on palette file");
		}
		g_free(file_name);
	}

	gui_update();

	gtk_widget_destroy(dialog);

	g_timeout_redraw_stop();

	emu_pause(FALSE);
}
void save_palette(void) {
	GtkWidget *dialog;

	emu_pause(TRUE);

	dialog = gtk_file_chooser_dialog_new("Save palette", GTK_WINDOW(main_win),
	        GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
	        GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	file_open_filter_add(dialog, "PAL files", "*.pal;*.PAL");
	file_open_filter_add(dialog, "All files", "*.*;");

	g_timeout_redraw_start();

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;

		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		palette_save_on_file(filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);

	g_timeout_redraw_stop();

	emu_pause(FALSE);
}
