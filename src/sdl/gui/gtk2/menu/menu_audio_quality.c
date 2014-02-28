/*
 * menu_audio_quality.c
 *
 *  Created on: 11/lug/2012
 *      Author: fhorse
 */

#include "menu_audio_quality.h"
#include "param.h"
#include "cfg_file.h"
#include "audio_quality.h"

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
  /* length: header (24) + pixel_data (497) */
  "\0\0\2\11"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\202>\207\266\0\1>\207\266a\202>\207\266\377\1>\207\266\335\213>\207"
  "\266\0\6>\207\266\16>\207\266\3778{\246\272\37CZU>\207\266\377>\207\266"
  "\307\212>\207\266\0\7>\206\265\261>\207\266\377\0\0\0\"\0\0\0\10\"Ke"
  "\\>\207\266\377>\207\266\17\211>\207\266\0\2>\207\266\377.e\210~\202"
  ">\207\266\0\3\0\0\0\12=\205\264\363>\206\265\213\210>\207\266\0\3>\207"
  "\266`>\207\266\377\0\0\0\23\203>\207\266\0\2""7x\242\257=\204\262\337"
  "\210>\207\266\0\2=\204\262\3139|\247\301\204>\207\266\0\2""0h\215a>\207"
  "\266\377\210>\207\266\0\2>\207\266\3773n\224x\204>\207\266\0\3\0\0\0"
  "\17>\207\266\377>\207\2668\207>\207\266\0\2>\207\266\377\"Jd(\205>\207"
  "\266\0\2=\205\264\363;\201\255{\205>\207\266\0\4>\207\266\2>\207\266"
  "\377\0\0\0""3\0\0\0\4\205>\207\266\0\2""9}\250\300=\204\262\313\205>"
  "\207\266\0\2>\207\266N>\207\266\377\207>\207\266\0\2""3n\224w>\207\266"
  "\377\205>\207\266\0\2<\202\260\243;\200\255\323\207>\207\266\0\3\0\0"
  "\0\23>\207\266\377>\207\266K\204>\207\266\0\2>\207\266\3771k\221{\210"
  ">\207\266\0\2=\205\264\362<\203\261\264\203>\207\266\0\3>\207\266K>\207"
  "\266\377\0\0\0\24\210>\207\266\0\3/g\212}>\207\266\377>\207\266\11\202"
  ">\207\266\0\2>\206\265\3349|\250\301\211>\207\266\0\7\0\0\0\23>\207\266"
  "\377>\207\266\353>\207\266\1>\207\266\254>\207\266\377\24+;2\212>\207"
  "\266\0\1\34=RP\203>\207\266\377\3*\\|q\0\0\0\3>\207\266\0\211\0\0\0\0"
  "\1\0\0\0\7\203\0\0\0""3\1\0\0\0\17\202\0\0\0\0"};

enum {
	MAQLOW,
	MAQHIGH,
	NUMCHKS
};

void set_audio_quality(int quality);

static GtkWidget *check[NUMCHKS];

void menu_audio_quality(GtkWidget *audio, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *quality;

	menu = gtk_menu_new();
	quality = gtk_image_menu_item_new_with_mnemonic("_Quality");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(quality), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(audio), quality);

	gw_image_from_inline(quality, icon_inline);

	check[MAQLOW] = gtk_check_menu_item_new_with_mnemonic("_Low");
	check[MAQHIGH] = gtk_check_menu_item_new_with_mnemonic("_High");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MAQLOW]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MAQHIGH]);

	g_signal_connect_swapped(G_OBJECT(check[MAQLOW]), "activate", G_CALLBACK(set_audio_quality),
	        GINT_TO_POINTER(AQ_LOW));
	g_signal_connect_swapped(G_OBJECT(check[MAQHIGH]), "activate", G_CALLBACK(set_audio_quality),
	        GINT_TO_POINTER(AQ_HIGH));
}
void menu_audio_quality_check(void) {
	int index;

	for (index = MAQLOW; index < NUMCHKS; index++) {
		if (cfg->audio_quality == index) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}
	}
}
void set_audio_quality(int quality) {
	if (gui_in_update) {
		return;
	}

	if (cfg->audio_quality == quality) {
		gui_update();
		return;
	}

	emu_pause(TRUE);
	cfg->audio_quality = quality;
	audio_quality(cfg->audio_quality);
	gui_update();
	emu_pause(FALSE);
}
