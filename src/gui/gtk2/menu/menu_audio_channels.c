/*
 * menu_audio_channels.c
 *
 *  Created on: 31/dic/2011
 *      Author: fhorse
 */

#include "menu_audio_channels.h"
#include "menu_audio_stereo_delay.h"
#include "param.h"
#include "cfg_file.h"
#include "snd.h"

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
  /* length: header (24) + pixel_data (653) */
  "\0\0\2\245"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\225\377\377\377\0\13\234\234\234\377\231\231\231\377\225\225\225\377"
  "\221\221\221\377\215\215\215\377\210\210\210\377\204\204\204\377\177"
  "\177\177\377zzz\377vvv\377fff\377\205\377\377\377\0\13\223\223\223\377"
  "\321\321\321\377\316\316\316\377\313\313\313\377\310\310\310\377\306"
  "\306\306\377\302\302\302\377\277\277\277\377\274\274\274\377\271\271"
  "\271\377JJJ\377\205\377\377\377\0\13\220\220\220\377\315\315\315\377"
  "\204\204\204\377YYY\377PPP\377MMM\377III\377VVV\377\177\177\177\377\264"
  "\264\264\377FFF\377\205\377\377\377\0\3\212\212\212\377\312\312\312\377"
  "^^^\377\205\377\377\377\0\3[[[\377\257\257\257\377@@@\377\205\377\377"
  "\377\0\3\205\205\205\377\305\305\305\377]]]\377\205\377\377\377\0\3]"
  "]]\377\252\252\252\377;;;\377\205\377\377\377\0\3\177\177\177\377\301"
  "\301\301\377YYY\377\205\377\377\377\0\3VVV\377\245\245\245\377777\377"
  "\205\377\377\377\0\3yyy\377\274\274\274\377TTT\377\205\377\377\377\0"
  "\3PPP\377\241\241\241\377222\377\205\377\377\377\0\3ttt\377\270\270\270"
  "\377NNN\377\205\377\377\377\0\3OOO\377\232\232\232\377---\377\205\377"
  "\377\377\0\3qqq\377\261\261\261\377III\377\202\377\377\377\0\6]]]BYY"
  "Y\343QQQ\377lll\377\207\207\207\377'''\355\202\377\377\377\0""5\177\177"
  "\177Bzzz\343sss\377\211\211\211\377\235\235\235\377HHH\355\377\377\377"
  "\0\\\\\\pkkk\377\205\205\205\377\232\232\232\377\227\227\227\377___\377"
  "\"\"\"\240\377\377\377\0~~~p\211\211\211\377\236\236\236\377\260\260"
  "\260\377\254\254\254\377uuu\377BBB\240OOOjsss\377\221\221\221\377\230"
  "\230\230\377\226\226\226\377vvv\377555\377\37\37\37Azzzj\221\221\221"
  "\377\251\251\251\377\256\256\256\377\253\253\253\377\213\213\213\377"
  "JJJ\377555A<<<mSSS\377nnn\377vvv\377NNN\377&&&\377\33\33\33l\377\377"
  "\377\0hhhmrrr\377\205\205\205\377\212\212\212\377ccc\377;;;\377///l\202"
  "\377\377\377\0\5///B&&&\314\40\40\40\315\34\34\34\231\34\34\34\32\203"
  "\377\377\377\0\5LLLB>>>\314666\315111\231///\32\232\377\377\377\0"};

enum {
	MMONO = 1,
	MSTEREO,
	NUMCHKS
};

void set_channels(int channels);

static GtkWidget *check[NUMCHKS];

void menu_audio_channels(GtkWidget *audio, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *channels;

	menu = gtk_menu_new();
	channels = gtk_image_menu_item_new_with_mnemonic("_Channels");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(channels), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(audio), channels);

	gw_image_from_inline(channels, icon_inline);

	check[MMONO] = gtk_check_menu_item_new_with_mnemonic("_Mono");
	check[MSTEREO] = gtk_check_menu_item_new_with_mnemonic("_Stereo");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MMONO]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSTEREO]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	menu_audio_stereo_delay(menu, accel_group);

	g_signal_connect_swapped(G_OBJECT(check[MMONO]), "activate", G_CALLBACK(set_channels),
	        GINT_TO_POINTER(1));
	g_signal_connect_swapped(G_OBJECT(check[MSTEREO]), "activate", G_CALLBACK(set_channels),
	        GINT_TO_POINTER(2));
}
void menu_audio_channels_check(void) {
	int index;

	for (index = MMONO; index < NUMCHKS; index++) {
		if (cfg->channels == index) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}
	}

	menu_audio_stereo_delay_check();
}
void set_channels(int channels) {
	if (gui_in_update) {
		return;
	}

	if (cfg->channels == channels) {
		gui_update();
		return;
	}

	emu_pause(TRUE);
	cfg->channels = channels;
	snd_start();
	gui_update();
	emu_pause(FALSE);
}
