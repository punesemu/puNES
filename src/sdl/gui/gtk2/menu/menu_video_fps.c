/*
 * menu_video_fps.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_fps.h"
#include "param.h"
#include "fps.h"
#include "cfg_file.h"
#include "gui_snd.h"

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
  /* length: header (24) + pixel_data (725) */
  "\0\0\2\355"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\222\377\377\377\0\3qqq\324ooo\363mmm\335\206\377\377\377\0\3RRR\326"
  "MMM\371III\327\204\377\377\377\0\3ooo\370lll\233iii\377\206WWW\377\3"
  "MMM\377HHH\215CCC\325\204\377\377\377\0\14lll\377iii\355eee\372l\272"
  "\345\366t\274\354\366n\270\353\366h\266\353\366\315\313\200\366\352\327"
  "\200\366GGG\372BBB\314===\377\204\377\377\377\0\14hhh\321eee\204aaa\324"
  "6\277\177\377i\305\335\377p\302\377\377g\277\377\377\254\313\256\377"
  "\371\340h\377AAA\326<<<l777\320\204\377\377\377\0\14ddd\337```\303\\"
  "\\\\\320E\304O\377,\272K\377`\275\330\377b\272\377\377e\271\377\377}"
  "\273\333\377;;;\322666\303111\337\204\377\377\377\0\14```\332\\\\\\\242"
  "WWW\317\\\313O\377z\327W\377j\321B\377^\307p\377P\270\266\377\77\256"
  "\313\377666\320111\242,,,\332\204\377\377\377\0\14[[[\324WWW\201RRR\315"
  "\236\342r\377\244\343|\377\240\343x\377\225\340c\377\225\341Z\377\223"
  "\342U\377000\320+++\201&&&\324\204\377\377\377\0\3VVV\342QQQ\320hhh\377"
  "\206WWW\377\3***\366%%%\320\40\40\40\342\204\377\377\377\0\14QQQ\324"
  "LLL\201GGG\314n\306\370\377w\310\377\377r\305\377\377l\304\377\377\317"
  "\330\230\377\353\344\231\377$$$\316\40\40\40\201\33\33\33\324\204\377"
  "\377\377\0\14KKK\332FFF\242AAA\3176\277\177\377i\305\335\377p\302\377"
  "\377g\277\377\377\254\313\256\377\371\340h\377\37\37\37\320\32\32\32"
  "\242\26\26\26\332\204\377\377\377\0\14EEE\337@@@\303;;;\320E\304O\377"
  ",\272K\377`\275\330\377b\272\377\377e\271\377\377}\273\333\377\32\32"
  "\32\322\25\25\25\303\21\21\21\337\204\377\377\377\0\14@@@\320;;;l666"
  "\325\\\313O\377z\327W\377j\321B\377^\307p\377P\270\266\377\77\256\313"
  "\377\25\25\25\327\21\21\21l\15\15\15\320\204\377\377\377\0\14:::\377"
  "555\306000\365\235\337n\376\243\340w\376\236\340s\376\223\335^\376\223"
  "\336U\376\221\336P\376\20\20\20\364\14\14\14\244\11\11\11\377\204\377"
  "\377\377\0\3""444\372///a555\377\206WWW\377\3\32\32\32\377\10\10\10O"
  "\5\5\5\357\204\377\377\377\0\3...\330)))\372$$$\314\206\377\377\377\0"
  "\3\10\10\10\331\5\5\5\377\2\2\2\334\202\377\377\377\0"};

enum {
	MFPSDEFAULT,
	MFPS60,
	MFPS59,
	MFPS58,
	MFPS57,
	MFPS56,
	MFPS55,
	MFPS54,
	MFPS53,
	MFPS52,
	MFPS51,
	MFPS50,
	MFPS49,
	MFPS48,
	MFPS47,
	MFPS46,
	MFPS45,
	MFPS44,
	NUMCHKS
};

void set_fps(int fps);

static GtkWidget *check[NUMCHKS];

void menu_video_fps(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *fps;
	int index;

	menu = gtk_menu_new();
	fps = gtk_image_menu_item_new_with_mnemonic("_FPS");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fps), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), fps);

	icon_inline(fps, icon_inline)

	for (index = MFPSDEFAULT; index < NUMCHKS; index++) {
		check[index] = gtk_check_menu_item_new_with_mnemonic(param_fps[index].lname);

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[index]);

		if (index == MFPSDEFAULT) {
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
		}

		g_signal_connect_swapped(G_OBJECT(check[index]), "activate", G_CALLBACK(set_fps),
		        GINT_TO_POINTER(index));
	}
}
void menu_video_fps_check(void) {
	int index;

	for (index = MFPSDEFAULT; index < NUMCHKS; index++) {
		if (cfg->fps == index) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}
	}
}
void set_fps(int fps) {
	if (gui_in_update) {
		return;
	}
	if (cfg->fps == fps) {
		gui_update();
		return;
	}
	cfg->fps = fps;
	emu_pause(TRUE);
	fps_init();
	snd_start();
	gui_update();
	emu_pause(FALSE);
}
