/*
 * menu_audio_stereo_delay.c
 *
 *  Created on: 12/nov/2013
 *      Author: fhorse
 */

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
  /* length: header (24) + pixel_data (658) */
  "\0\0\2\252"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\233\377\377\377\0\2W\242N\270R\235J\6\215\377\377\377\0\3V\241M8Q\234"
  "I\367M\226E\331\203\377\377\377\0\203f\265Z\377\13d\262X\377b\257V\377"
  "_\253T\306]\250R@]\252T\220Y\245P\330U\240L\377`\246X\377{\270s\377g"
  "\250_\377C\212<\317\202\377\377\377\0.f\265Z\377\302\343\272\377\301"
  "\343\271\377\300\342\270\377\276\341\267\377\231\313\221\377n\257e\377"
  "j\255a\377\221\311\211\377\231\317\220\377\225\314\216\377\222\313\213"
  "\377\217\311\207\377^\237W\377:~4\304\377\377\377\0e\264Y\377\300\343"
  "\271\377\277\342\267\377\276\341\266\377\274\340\264\377\272\337\263"
  "\377\243\317\232\377Y\235P\377\226\315\216\377\224\313\214\377\220\312"
  "\211\377\215\310\206\377\212\306\203\377Y\232S\3775x/\311\377\377\377"
  "\0c\260W\377`\255U\377]\251R\377_\247U\377\204\274{\377\257\327\247\377"
  "\264\334\255\377]\234U\377s\261l\377[\236S\377A\206:\377[\235U\377X\231"
  "Q\3774w.\321\205\377\377\377\0\12V\236L\14R\231I\205{\263s\377\261\332"
  "\252\377\201\266y\377C\206;\374@\2059K\377\377\377\0""6r0\3753v.\334"
  "\206\377\377\377\0\12U\240L\14P\232H\205L\216C\377\247\323\240\377\250"
  "\324\242\377=z6\3749t2K\377\377\377\0""0h*\375,b'\334\203\377\377\377"
  "\0\16\\\251S\377X\244O\377T\237K\377S\233J\377k\255c\377]\234U\377\203"
  "\266|\377\246\324\240\377\204\267}\377S\213N\377/g*\377Z\216T\377V\212"
  "R\377#V\37\321\202\377\377\377\0.W\243N\377\225\314\216\377\222\313\213"
  "\377\217\311\207\377\213\307\204\377w\264q\377T\216M\377\242\322\234"
  "\377\236\317\230\377\235\317\230\377\233\316\226\377\230\314\223\377"
  "\226\314\221\377O\202K\377\34K\31\311\377\377\377\0R\235J\377\220\312"
  "\211\377\215\310\206\377\212\306\203\377\207\304\200\377i\252c\377<y"
  "6\377b\230]\377\213\276\205\377\231\316\224\377\227\314\222\377\224\313"
  "\220\377\222\312\216\377K~H\377\30F\26\304\377\377\377\0M\226E\377I\221"
  "A\377D\213=\377@\2059\377;\2005\3777{1\3063t.@-c(\220)^$\330%X!\377:"
  "m6\377g\234c\377K}G\377\30E\25\317\214\377\377\377\0\3\35M\32""8\32I"
  "\27\367\27E\25\331\216\377\377\377\0\2\27D\24\270\24@\22\6\223\377\377"
  "\377\0"};

void set_stereo_delay(int );

enum {
	M5,
	M10,
	M15,
	M20,
	M25,
	M30,
	M35,
	M40,
	M45,
	M50,
	M55,
	M60,
	M65,
	M70,
	M75,
	M80,
	M85,
	M90,
	M95,
	M100,
	NUMCHKS
};

static GtkWidget *stereo_delay, *check[NUMCHKS];

void menu_audio_stereo_delay(GtkWidget *channels, GtkAccelGroup *accel_group) {
	GtkWidget *menu;

	menu = gtk_menu_new();
	stereo_delay = gtk_image_menu_item_new_with_mnemonic("_Stereo delay");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(stereo_delay), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(channels), stereo_delay);

	gw_image_from_inline(stereo_delay, icon_inline);

	{
		char description[30];
		int i;

		for (i = 5; i <= 100; i += 5) {
			int index = (i / 5) - 1;

			if (i == 30) {
				sprintf(description, "%d%% (Default)", i);
			} else {
				sprintf(description, "%d%%", i);
			}

			check[index] = gtk_check_menu_item_new_with_label(description);

			gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[index]);

			g_signal_connect_swapped(G_OBJECT(check[index]), "activate",
					G_CALLBACK(set_stereo_delay), GINT_TO_POINTER(i));
		}
	}
}
void menu_audio_stereo_delay_check(void) {
	int index;

	for (index = M5; index < NUMCHKS; index++) {
		int delay = cfg->stereo_delay * 100;

		if (delay == ((index + 1) * 5)) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}
	}

	if (cfg->channels == STEREO) {
		gtk_widget_set_sensitive(stereo_delay, TRUE);
	} else {
		gtk_widget_set_sensitive(stereo_delay, FALSE);
	}
}
void set_stereo_delay(gint stereo_delay) {
	double delay = ((double) stereo_delay) / 100.f;

	if (gui_in_update) {
		return;
	}


	if (cfg->stereo_delay == delay) {
		gui_update();
		return;
	}

	cfg->stereo_delay = delay;
	snd_stereo_delay();
	gui_update();
}
