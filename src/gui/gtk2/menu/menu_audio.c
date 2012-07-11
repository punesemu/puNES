/*
 * menu_audio.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_audio.h"
#include "menu_audio_sample_rate.h"
#include "menu_audio_channels.h"
#include "menu_audio_filter.h"
#include "param.h"
#include "cfgfile.h"
#include "sdlsnd.h"

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
  /* length: header (24) + pixel_data (947) */
  "\0\0\3\313"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\204\377\377\377\0\10""077%.46\205058\305289\363/67\363/56\305.24\205"
  ")00%\206\377\377\377\0\14\0\0\0\2.35t=BC\366psr\374vxw\377jlk\377[]\\"
  "\377KML\3779;;\374(-/\366*03t\0\0\0\2\203\377\377\377\0\6\0\0\0\2.35"
  "\304NRS\366\203\205\203\377]a\\\377:\77>\377\202DD1\377\6""498\377<@"
  ";\377797\377'+,\366*/0\304\0\0\0\2\202\377\377\377\0""3,03tADE\366gh"
  "f\377<@<\377{j\25\377\252\213\1\377\270\227\0\377\254\215\0\377\271\230"
  "\1\377\206s\27\377484\377120\377'++\366(,.t\377\377\377\0)00%,13\366"
  "VXV\377;>;\377\235\202\7\377\273\230\0\377\255\215\0\377\270\226\0\377"
  "\271\227\0\377\250\211\0\377\267\225\0\377\245\210\10\377363\377,/-\377"
  "$'(\366\"))%(.0\2058;;\374BE@\377\202p\26\377\263\222\0\377\267\225\0"
  "\377\266\224\1\377\245\207\3\377\311\246\10\377\247\213\7\377\312\250"
  "\12\377\305\243\10\377\207u\31\37703.\377+-,\374#(*\205)//\305687\377"
  "387\377\261\221\1\377\202\304\240\0\377i\212s\7\377b_O\377>:'\377vd\15"
  "\377\307\252\25\377\322\262\24\377\312\252\20\377/43\377,.-\377$()\305"
  "&*,\362,/-\377EE1\377\260\220\0\377\251\212\0\377\250\211\4\377TP>\377"
  "\311\311\310\377rtq\377NL4\377\322\266\37\377\317\264\35\377\327\270"
  "\32\377FG3\377)+)\377!%'\362%)*\362*-*\377DD0\377\271\227\0\377\304\240"
  "\0\377\300\237\11\377CA,\377\212\213\210\377XZV\377^]G\377\320\270-\377"
  "\331\300)\377\334\300%\377FG5\377131\377\40#%\362$)+\305.0.\377,10\377"
  "\277\234\1\377\273\231\1\377\266\230\14\377\236\210\30\377WV\77\377i"
  "hU\377\257\242Q\377\350\323H\377\345\3168\377\341\310.\3775:9\377898"
  "\377\40$$\305!$&\205+--\374,/+\377\206s\27\377\303\237\2\377\305\245"
  "\16\377\306\252\31\377\331\301/\377\345\322V\377\357\337s\377\361\337"
  "b\377\362\335G\377\234\2216\377CFA\377354\374\35\37!\205\34\"\"%\37#"
  "$\365-/-\377/2/\377\256\221\12\377\315\254\16\377\326\267\32\377\334"
  "\303)\377\347\321F\377\362\340f\377\372\352e\377\330\311K\377@D@\377"
  "PRP\377\33\37\40\365\34\34\34%\377\377\377\0\37!#t#''\365,-+\377030\377"
  "\213z\36\377\322\263\31\377\335\301$\377\346\3144\377\352\325F\377\245"
  "\234E\377FKG\377`b_\377-00\365\32\35\37t\202\377\377\377\0\16\0\0\0\2"
  "\36\"\"\304#%%\365.0.\377572\377487\377IJ9\377LN<\377CGF\377Z^Y\377f"
  "ig\377.11\365\33\36\36\304\0\0\0\2\203\377\377\377\0\14\0\0\0\2\32\37"
  "\37t\33\35\36\365,--\374:<:\377GIH\377RTS\377QSQ\377DFE\374\34\37\37"
  "\365\26\32\32t\0\0\0\2\206\377\377\377\0\10\25\34\34%\27\33\33\205\32"
  "\36\36\305\30\33\33\362\27\32\33\362\32\34\34\305\27\31\31\205\25\25"
  "\25%\204\377\377\377\0"};

enum {
	MSNDENABLE,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_audio(GtkWidget *settings, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *audio;

	menu = gtk_menu_new();

	audio = gtk_image_menu_item_new_with_mnemonic("_Audio");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(audio), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(settings), audio);

	icon_inline(audio, icon_inline)

	menu_audio_sample_rate(menu, NULL);
	menu_audio_channels(menu, NULL);
	menu_audio_filter(menu, NULL);

	/* Settings/Audio/Enable */
	check[MSNDENABLE] = gtk_check_menu_item_new_with_mnemonic("_Enable");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSNDENABLE]);

	gtk_widget_add_accelerator(check[MSNDENABLE], "activate", accel_group, GDK_a,
	        GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	g_signal_connect_swapped(G_OBJECT(check[MSNDENABLE]), "activate",
	        G_CALLBACK(menu_audio_set_audio_enable), NULL);

}
void menu_audio_check(void) {
	menu_audio_sample_rate_check();
	menu_audio_channels_check();
	menu_audio_filter_check();

	/* Audio Enable */
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSNDENABLE]), FALSE);
	if (cfg->audio) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSNDENABLE]), TRUE);
	}
}
void menu_audio_set_audio_enable(void) {
	if (guiupdate) {
		return;
	}
	emuPause(TRUE);
	cfg->audio = !cfg->audio;
	if (cfg->audio) {
		sndStart();
	} else {
		sndStop();
	}
	guiUpdate();
	emuPause(FALSE);
}
