/*
 * menu_audio.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_audio.h"
#include "menu_audio_sample_rate.h"
#include "menu_audio_channels.h"
#include "menu_audio_quality.h"
#include "../cfg_apu_channels.h"
#include "param.h"
#include "cfg_file.h"
#include "gui_snd.h"

#ifdef __SUNPRO_C
#pragma align 4 (audio_icon_inline)
#endif
#ifdef __GNUC__
static const guint8 audio_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 audio_icon_inline[] =
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

#ifdef __SUNPRO_C
#pragma align 4 (volumes_icon_inline)
#endif
#ifdef __GNUC__
static const guint8 volumes_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 volumes_icon_inline[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (683) */
  "\0\0\2\303"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\221\0\0\0\0\1e\300\352\243\214e\300\352\377\4e\301\352\227\0\0\0\0X"
  "\267\346\255\226\333\365\377\215\227\333\365\377\6O\262\345\237W\257"
  "\342\377s\313\361\377A\230\324\377\232\336\370\377r\315\363\377\202t"
  "\312\357\377\2A\230\324\377\226\330\362\377\202t\312\357\377\10v\314"
  "\360\377A\230\324\377r\311\357\377t\312\357\377J\251\340\377T\252\337"
  "\377[\234\304\377l\230\265\377\202a\210\250\377\4D\235\322\377l\304\355"
  "\377\13b\265\377\230\332\366\377\202l\304\355\377\27q\311\361\377\13"
  "b\265\377i\303\355\377l\304\355\377F\244\335\377M\241\331\377H\220\274"
  "\377\350\352\352\377\337\345\350\377\350\352\352\377D\235\322\377[\273"
  "\350\377\20f\267\377\226\331\363\377[\273\350\377D\235\322\377-\212\305"
  "\377\13b\265\377-\212\305\377D\235\322\377\77\231\327\377H\233\327\377"
  "B\217\276\377\202k\216\237\377\21o\220\241\377D\235\322\377S\266\347"
  "\377\20f\267\377\222\327\363\377S\266\347\377D\235\322\377\377\375\367"
  "\377\366\365\364\377\377\375\370\377f\206\230\377:\224\325\377B\221\317"
  "\377C\253\343\377\26Y\252\377\213\323\361\377@\252\342\377\202D\254\343"
  "\377\37\22g\270\377\213\323\361\377D\254\343\377D\235\322\377=t\230\377"
  "\32U\214\377:q\226\377=\222\311\3774\211\315\377\77\214\313\377=\247"
  "\341\377\26Y\252\377\206\320\357\377:\245\340\377>\250\341\3771n\226"
  "\377\32U\214\377l\230\265\377<\251\343\377>\250\341\377@\257\350\377"
  "\26Y\252\377:\251\344\377>\251\343\3770\202\310\3778\203\306\377.\237"
  "\335\377\25X\247\377z\316\366\377*\235\333\377\37s\252\377\202\374\367"
  "\362\377-\371\364\360\377\37s\252\377.\242\341\3770\244\340\377\25X\247"
  "\377*\235\333\377/\240\335\377)x\301\3776\177\302\377(\233\334\377\25"
  "U\246\377b\303\361\377%\231\333\3771n\226\377l\230\265\377\32U\214\377"
  "l\230\265\3771n\226\377)\236\337\377*\240\337\377\25U\246\377%\231\333"
  "\377)\234\334\377(t\276\377/w\275\377\37\225\331\377\25U\246\377O\265"
  "\346\377\35\224\330\377\37\226\331\377\37\227\332\377\30j\271\377P\270"
  "\351\377\37\226\332\377\37\226\331\377\40\230\333\377\25U\246\377\35"
  "\224\330\377\40\226\331\377\"l\270\377,s\271\265\35\223\330\377\36\224"
  "\330\377\"\216\323\377\35\224\330\377\202\36\224\330\377\2\36\225\331"
  "\377\35\223\327\377\203\36\224\330\377\6\36\226\332\377\35\223\330\377"
  "\36\224\330\377\37h\263\252\0\0\0\0\30]\255\261\214\31]\255\377\1\31"
  "]\255\245\241\0\0\0\0"};

enum {
	MSNDSWAPDUTY,
	MSNDENABLE,
	NUMCHKS
};

void menu_audio_set_audio_swap_duty(void);

static GtkWidget *check[NUMCHKS];

void menu_audio(GtkWidget *settings, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *audio, *volumes;

	menu = gtk_menu_new();

	audio = gtk_image_menu_item_new_with_mnemonic("_Audio");
	volumes = gtk_image_menu_item_new_with_mnemonic("A_PU channels");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(audio), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(settings), audio);

	icon_inline(audio, audio_icon_inline)
	icon_inline(volumes, volumes_icon_inline)

	menu_audio_sample_rate(menu, NULL);
	menu_audio_channels(menu, NULL);
	menu_audio_quality(menu, NULL);

	check[MSNDSWAPDUTY] = gtk_check_menu_item_new_with_mnemonic("S_wap Duty Cycles");
	check[MSNDENABLE] = gtk_check_menu_item_new_with_mnemonic("_Enable");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), volumes);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSNDSWAPDUTY]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSNDENABLE]);

	gtk_widget_add_accelerator(check[MSNDENABLE], "activate", accel_group, GDK_a,
	        GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	g_signal_connect(G_OBJECT(volumes), "activate",
	        G_CALLBACK(apu_channels_dialog), NULL);
	g_signal_connect_swapped(G_OBJECT(check[MSNDSWAPDUTY]), "activate",
	        G_CALLBACK(menu_audio_set_audio_swap_duty), NULL);
	g_signal_connect_swapped(G_OBJECT(check[MSNDENABLE]), "activate",
	        G_CALLBACK(menu_audio_set_audio_enable), NULL);

}
void menu_audio_check(void) {
	menu_audio_sample_rate_check();
	menu_audio_channels_check();
	menu_audio_quality_check();

	/* Swap Duty Cycles */
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSNDSWAPDUTY]), FALSE);
	if (cfg->swap_duty) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSNDSWAPDUTY]), TRUE);
	}

	/* Audio Enable */
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSNDENABLE]), FALSE);
	if (cfg->apu.channel[APU_MASTER]) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSNDENABLE]), TRUE);
	}
}
void menu_audio_set_audio_swap_duty(void) {
	if (gui_in_update) {
		return;
	}
	emu_pause(TRUE);
	cfg->swap_duty = !cfg->swap_duty;
	gui_update();
	emu_pause(FALSE);
}
void menu_audio_set_audio_enable(void) {
	if (gui_in_update) {
		return;
	}
	emu_pause(TRUE);
	cfg->apu.channel[APU_MASTER] = !cfg->apu.channel[APU_MASTER];
	if (cfg->apu.channel[APU_MASTER]) {
		snd_start();
	} else {
		snd_stop();
	}
	gui_update();
	emu_pause(FALSE);
}
