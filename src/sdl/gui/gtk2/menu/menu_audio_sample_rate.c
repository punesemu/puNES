/*
 * menu_audio_sample_rate.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_audio_sample_rate.h"
#include "param.h"
#include "cfg_file.h"
#include "snd.h"

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
  /* length: header (24) + pixel_data (723) */
  "\0\0\2\353"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\213\377\377\377\0\5VXSkWYU\356Y[W\377WYU\367TWTg\213\377\377\377\0\6"
  "UWS\355\306\311\304\377\320\322\316\377\267\271\265\377UWS\327;@\0\356"
  "\212;\77\0\373\7UWQ\377\324\326\322\377UWS\377UWS\356TWTd:>\0\377\225"
  "\2379\377\203\225\240:\377\202\226\240;\377\203\226\241<\377\11\227\241"
  "=\377WYS\377\324\326\322\377UWS\377```\10\377\377\377\0:>\0\376\230\242"
  "=\377@H0\377\2044:8\377\15""397\3778>3\377DcP\377EeQ\377WYS\377\324\326"
  "\322\377UWS\377[]Z\361^_\\\323:\77\0\376\230\242=\377175\377non\377\203"
  "oqn\377\15prn\3774;7\377:[`\377<]_\377WYS\377\324\326\322\377UWS\377"
  "\353\353\353\377UWS\377:\77\0\376\230\242=\377175\377prn\377\203UWS\377"
  "\15pqn\3775;7\377q~\5\377x\206\0\377WYS\377\324\326\322\377UWS\377^`"
  "]\366Y[W\360:>\0\375\230\242<\377286\377prn\377\203pqn\377\14rsp\377"
  "5;7\377EbO\377=_^\377WYS\377\324\326\322\377UWS\377ZZZ\37NNN\15:>\0\375"
  "\227\242<\3779@2\377\205188\377\21""174\377DbP\377;\\a\377WYS\377\324"
  "\326\322\377UWS\377Z\\X\367\\_[\332;\77\0\374\227\241<\377y\205\2\377"
  "u\204\2\377u\205\2\377u\204\2\377u\205\2\377u\204\2\377w\205\1\377\202"
  "x\206\0\377\22WYS\377\324\326\322\377UWS\377\357\357\357\377UWS\377;"
  "\77\0\374\226\241<\377y\206\1\377|\213\2\377\252\273\31\377x\206\0\377"
  "\254\275\32\377}\212\2\377\255\276\32\377\227\242;\377\230\243=\377W"
  "YS\377\324\326\322\377\202UWS\377\"[\\Y\350;@\0\374\226\240;\377{\211"
  "\6\377\203\222\11\377\333\3562\377z\210\4\377\336\3612\377\203\222\11"
  "\377\341\3644\377\226\241;\377=A\4\377UWQ\377\324\326\322\377UWS\377"
  "```\10\377\377\377\0;\77\0\373\220\2337\377\223\2369\377\233\2479\377"
  "\346\3719\377\223\2369\377\351\3749\377\233\2479\377\353\3779\377\223"
  "\2369\377\77D\6\375UWS\370\324\326\322\377UWS\377```\10\377\377\377\0"
  "<@\0\342\211<\77\0\367\5<\77\0\351UWS\362\307\312\305\377UWS\376fff\5"
  "\214\377\377\377\0\3TVS\216VXT\376UWS\234\216\377\377\377\0\1PPP\20\203"
  "\377\377\377\0"};

enum {
	M44100,
	M22050,
	M11025,
	NUMCHKS
};

void set_sample_rate(int newsamplerate);

static GtkWidget *check[NUMCHKS];

void menu_audio_sample_rate(GtkWidget *audio, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *sample_rate;

	menu = gtk_menu_new();
	sample_rate = gtk_image_menu_item_new_with_mnemonic("_Sample rate");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(sample_rate), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(audio), sample_rate);

	icon_inline(sample_rate, icon_inline)

	check[M44100] = gtk_check_menu_item_new_with_mnemonic("_44100");
	check[M22050] = gtk_check_menu_item_new_with_mnemonic("_22050");
	check[M11025] = gtk_check_menu_item_new_with_mnemonic("_11025");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[M44100]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[M22050]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[M11025]);

	g_signal_connect_swapped(G_OBJECT(check[M44100]), "activate", G_CALLBACK(set_sample_rate),
	        GINT_TO_POINTER(0));
	g_signal_connect_swapped(G_OBJECT(check[M22050]), "activate", G_CALLBACK(set_sample_rate),
	        GINT_TO_POINTER(1));
	g_signal_connect_swapped(G_OBJECT(check[M11025]), "activate", G_CALLBACK(set_sample_rate),
	        GINT_TO_POINTER(2));
}
void menu_audio_sample_rate_check(void) {
	int index;

	for (index = M44100; index < NUMCHKS; index++) {
		if (cfg->samplerate == index) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}
	}
}
void set_sample_rate(int newsamplerate) {
	if (guiupdate) {
		return;
	}

	if (cfg->samplerate == newsamplerate) {
		guiUpdate();
		return;
	}

	emu_pause(TRUE);

	cfg->samplerate = newsamplerate;

	snd_start();

	guiUpdate();

	emu_pause(FALSE);
}
