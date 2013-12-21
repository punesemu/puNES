/*
 * menu_mode.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_mode.h"
#include "cfg_file.h"
#include "clock.h"
#include "text.h"
#include "param.h"

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
  /* length: header (24) + pixel_data (836) */
  "\0\0\3\\"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\204\377\377\377\0\7\207\214\2075\207\212\204W\252\252\252\3\377\377"
  "\377\0\211\211\205E\213\215\210\344\207\213\203@\211\377\377\377\0\7"
  "\213\214\210\362\224\226\222\364\214\215\211\250\207\213\203@\214\215"
  "\211\364\253\254\251\377\217\220\214\272\211\377\377\377\0\24\211\213"
  "\207\371\327\327\327\377\237\240\235\364\213\215\210\372\311\311\310"
  "\377\315\315\314\377\215\216\211\350\210\210\204<\215\217\212\235\215"
  "\220\213\245\210\210\210\17\377\377\377\0\377\377\377\1\213\213\206e"
  "\213\214\207\212\207\211\204h\213\215\210\367\332\332\332\377\336\336"
  "\336\377\334\334\334\377\202\336\336\336\377\15\250\252\247\365\225\226"
  "\222\361\263\264\261\374\220\222\216\366\211\211\205A\377\377\377\0\212"
  "\212\212\30\212\214\207\374\264\265\263\376\250\251\245\365\301\302\300"
  "\377\335\335\335\377\334\334\334\377\202\312\313\312\377\202\333\333"
  "\333\377\10\332\332\332\377\307\310\307\377\214\215\211\353\222\222\222"
  "\7\377\377\377\0\200\200\200\4\215\217\212\314\260\261\256\371\202\332"
  "\332\332\377\2\304\304\303\377\225\226\221\362\202\215\216\211\346\6"
  "\224\226\221\362\302\303\301\377\327\327\327\377\261\262\260\377\211"
  "\214\206\253\222\222\222\7\202\377\377\377\0\6\206\206\206\25\211\213"
  "\206\374\324\324\324\377\326\326\326\377\222\225\220\362\211\211\206"
  "]\202\377\377\377\1\15\211\211\206]\222\224\217\362\323\323\323\377\312"
  "\313\312\377\230\231\226\362\212\213\207\372\215\216\211\234\206\213"
  "\2067\215\217\212\311\230\232\225\364\322\322\322\377\302\302\301\377"
  "\214\216\211\343\204\377\377\377\0\2\213\216\211\343\277\277\276\377"
  "\202\317\317\317\377\10\266\267\265\377\212\214\207\370\212\214\207\364"
  "\264\265\263\377\317\317\317\377\322\322\322\377\310\310\307\377\214"
  "\216\211\337\204\377\377\377\0\15\214\216\211\337\306\306\305\377\316"
  "\316\316\377\234\236\232\365\214\216\211\337\206\212\206J\213\216\211"
  "\265\213\214\210\367\235\240\233\363\310\310\310\377\341\341\341\377"
  "\224\225\221\363\210\213\204M\202\377\377\377\0\6\210\213\204M\224\225"
  "\221\363\337\337\337\377\306\306\306\377\211\213\206\374\217\217\200"
  "\20\202\377\377\377\0\6\205\217\205\31\212\213\207\265\253\254\251\377"
  "\333\333\333\377\332\332\331\376\223\224\220\364\202\214\217\212\323"
  "\16\222\224\220\364\331\331\330\376\331\331\331\377\304\304\304\377\240"
  "\241\236\366\213\215\210\270\377\377\377\1\377\377\377\0\200\200\200"
  "\4\213\216\210\343\264\265\263\377\306\306\306\377\343\343\343\377\365"
  "\365\365\377\202\341\342\341\377\16\365\365\365\377\342\342\342\377\264"
  "\264\264\377\243\244\241\375\251\252\247\377\210\212\205\375\214\214"
  "\200\24\377\377\377\0\207\213\203@\216\217\213\366\253\254\252\377\227"
  "\231\225\362\244\245\243\374\310\310\310\377\202\326\326\326\377\23\307"
  "\307\307\377\274\274\274\377\212\214\207\367\210\212\206\201\214\217"
  "\212\252\211\213\207{\200\200\200\2\377\377\377\0\206\206\206\23\213"
  "\215\211\300\213\216\210\271\211\211\206T\213\214\210\355\264\264\263"
  "\377\253\254\252\375\215\216\212\370\233\234\231\365\270\270\267\377"
  "\211\213\206\367\211\377\377\377\0\7\214\215\211\276\244\245\242\377"
  "\212\215\207\365\212\212\206=\212\215\210\272\222\224\220\365\211\213"
  "\207\366\211\377\377\377\0\7\210\213\204O\212\213\207\360\213\215\207"
  "w\377\377\377\0\231\231\231\5\207\212\205f\207\213\203@\204\377\377\377"
  "\0"};

enum {
	MPAL,
	MNTSC,
	MDENDY,
	MAUTO,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_mode(GtkWidget *settings, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *mode;

	menu = gtk_menu_new();
	mode = gtk_image_menu_item_new_with_mnemonic("_Mode");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mode), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(settings), mode);

	gw_image_from_inline(mode, icon_inline);

	check[MPAL] = gtk_check_menu_item_new_with_mnemonic("_PAL");
	check[MNTSC] = gtk_check_menu_item_new_with_mnemonic("_NTSC");
	check[MDENDY] = gtk_check_menu_item_new_with_mnemonic("_Dendy");
	check[MAUTO] = gtk_check_menu_item_new_with_mnemonic("_Auto");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MPAL]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MNTSC]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MDENDY]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MAUTO]);

	gtk_widget_add_accelerator(check[MPAL], "activate", accel_group, GDK_F6, (GdkModifierType) 0,
			GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MNTSC], "activate", accel_group, GDK_F7, (GdkModifierType) 0,
			GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MDENDY], "activate", accel_group, GDK_F8, (GdkModifierType) 0,
			GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MAUTO], "activate", accel_group, GDK_F9, (GdkModifierType) 0,
			GTK_ACCEL_VISIBLE);

	g_signal_connect_swapped(G_OBJECT(check[MPAL]), "activate", G_CALLBACK(menu_mode_set_mode),
			GINT_TO_POINTER(PAL));
	g_signal_connect_swapped(G_OBJECT(check[MNTSC]), "activate", G_CALLBACK(menu_mode_set_mode),
			GINT_TO_POINTER(NTSC));
	g_signal_connect_swapped(G_OBJECT(check[MDENDY]), "activate", G_CALLBACK(menu_mode_set_mode),
			GINT_TO_POINTER(DENDY));
	g_signal_connect_swapped(G_OBJECT(check[MAUTO]), "activate", G_CALLBACK(menu_mode_set_mode),
			GINT_TO_POINTER(AUTO));
}
void menu_mode_check(void) {
	int index = 0;

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MPAL]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MNTSC]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MDENDY]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MAUTO]), FALSE);
	if (cfg->mode == AUTO) {
		index = MAUTO;
	} else if (machine.type == PAL) {
		index = MPAL;
	} else if (machine.type == NTSC) {
		index = MNTSC;
	} else if (machine.type == DENDY) {
		index = MDENDY;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
}
void menu_mode_set_mode(int mode) {
	BYTE reset = TRUE;

	if (gui_in_update) {
		return;
	}

	if (mode == cfg->mode) {
		gui_update();
		return;
	}

	switch (mode) {
		case PAL:
		case NTSC:
		case DENDY:
			if ((cfg->mode == AUTO) && (mode == machine.type)) {
				reset = FALSE;
			}
			cfg->mode = mode;
			machine = machinedb[mode - 1];
			break;
		case AUTO:
			if (info.machine_db == PAL) {
				machine = machinedb[PAL - 1];
			} else if (info.machine_db == DENDY) {
				machine = machinedb[DENDY - 1];
			} else {
				machine = machinedb[NTSC - 1];
			}
			/*
			 * se la modalita' in cui mi trovo e' gia' quella del database oppure
			 * se ne database la modalita' e' a 0 o impostata su default ed
			 * io sono gia' nella modalita' NTSC (appunto quella di default), allora
			 * non devo fare nessun reset.
			 */
			if ((cfg->mode == info.machine_db) || ((cfg->mode == NTSC) &&
					((info.machine_db < NTSC) || (info.machine_db > DENDY)))){
				reset = FALSE;
			}
			cfg->mode = AUTO;
			break;
	}

	gui_update();

	if (reset) {
		text_add_line_info(1, "switched to [green]%s", param_mode[machine.type].lname);
		make_reset(CHANGE_MODE);
	}
}
