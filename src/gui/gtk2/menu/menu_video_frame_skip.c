/*
 * menu_video_frame_skip.c
 *
 *  Created on: 17/dic/2011
 *      Author: fhorse
 */

#include "menu_video_frame_skip.h"
#include "param.h"
#include "cfgfile.h"
#include "fps.h"

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
  "\3[{\252\215Xw\253\350Fm\250\360\202:f\250\360\203:d\250\360\2Ck\250"
  "\360Qt\250\217\206\377\377\377\0\12Xy\253\351\334\336\336\376\362\363"
  "\363\377\363\364\367\377\362\364\367\377\355\362\370\377\353\362\370"
  "\377\353\362\367\377\350\356\361\377Jp\252\357\206\377\377\377\0\3Fm"
  "\250\360\361\362\363\377U~K\377\203t\234i\377\5h\212\216\377Xw\243\377"
  "Pu\255\377;f\250\376:f\250\360\203:d\250\360\26Ck\250\360Qt\250\217<"
  "f\250\360\363\364\367\377t\230j\377\235\312\216\377\236\315\220\377\240"
  "\321\222\377^\201\251\377\334\336\336\377\362\363\363\377\363\364\367"
  "\377\362\364\367\377\355\362\370\377\353\362\370\377\353\362\367\377"
  "\350\356\361\377Jp\252\357<f\250\360\361\364\367\377z\237p\377\242\324"
  "\224\377\202\243\323\224\377\3Ks\247\377\361\362\363\377\304nu\377\203"
  "\340\210\220\377*\340\214\225\377\271{\200\377\352\361\367\377Em\253"
  "\360<f\250\360\355\362\367\377K\213E\377_\230\210\377Y}\244\377Hq\244"
  "\377:f\250\377En\255\377Cf\247\377Fi\251\377Fi\252\377Np\252\377\235"
  "\227\267\377\342\220\230\377\350\357\367\377Em\253\360:f\250\360\354"
  "\361\367\377L\220G\377Y\177\244\377\334\336\336\377\362\363\363\377\363"
  "\364\367\377\362\364\367\377\355\362\370\377\353\362\370\377\353\362"
  "\367\377\350\356\361\377Uu\254\377\342\220\230\377\347\357\366\377Em"
  "\252\3609d\246\360\354\360\367\377:l6\377Fo\242\377\361\362\363\377\\"
  "\201\226\377\203\200\246\273\377\36\203\250\274\377e\200\217\377\352"
  "\361\367\377Pn\250\377\326fa\377\346\355\366\377Em\252\360:d\246\360"
  "\354\360\366\377\327\350\375\377En\255\377\363\364\367\377\201\244\266"
  "\377\255\334\362\377\260\336\365\377\262\341\372\377\264\343\374\377"
  "\210\253\276\377\350\357\367\377Po\251\377\336hb\377\345\355\366\377"
  "Em\252\360Hm\250\341\352\354\356\377\360\362\367\377Gn\255\377\361\364"
  "\367\377\210\253\276\377\265\345\376\377\203\265\344\375\377\14\210\253"
  "\276\377\347\357\366\377Nm\246\377\265[X\377\345\354\366\377Ck\252\360"
  "St\252\212Or\253\303Em\252\360<f\250\376\355\362\367\377V\177\246\377"
  "\202s\252\336\377\202r\247\334\377\6U}\244\377\346\355\366\377Nt\257"
  "\377\327\350\375\377\345\354\367\377Ck\252\360\203\377\377\377\0\3:f"
  "\250\360\354\361\367\377X\202\254\377\204u\255\345\377\6X\202\254\377"
  "\345\355\366\377Ou\257\377\356\362\370\377\352\356\361\377Jp\253\357"
  "\203\377\377\377\0\3""9d\246\360\354\360\367\377Bb\201\377\204X\202\254"
  "\377\6Bb\201\377\345\354\366\377Ck\252\376Ck\252\360Jp\252\360St\252"
  "\220\203\377\377\377\0\3:d\246\360\354\360\366\377\327\350\375\377\204"
  "\327\350\376\377\3\327\350\375\377\345\354\367\377Ck\252\360\206\377"
  "\377\377\0\5Hm\250\341\352\354\356\377\360\362\367\377\360\363\370\377"
  "\357\363\370\377\202\356\362\367\377\3\356\362\370\377\352\356\361\377"
  "Jp\253\357\206\377\377\377\0\3St\252\212Or\253\303Em\252\360\205Ck\252"
  "\360\2Jp\252\360St\252\220\203\377\377\377\0"};

enum {
	MFSKDEFAULT,
	MFSK1,
	MFSK2,
	MFSK3,
	MFSK4,
	MFSK5,
	MFSK6,
	MFSK7,
	MFSK8,
	MFSK9,
	NUMCHKS
};

void set_frame_skip(int newframeskip);

static GtkWidget *check[NUMCHKS];

void menu_video_frame_skip(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *frame_skip;
	int index;

	menu = gtk_menu_new();
	frame_skip = gtk_image_menu_item_new_with_mnemonic("Fra_me skip");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(frame_skip), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), frame_skip);

	icon_inline(frame_skip, icon_inline)

	for (index = MFSKDEFAULT; index < NUMCHKS; index++) {
		check[index] = gtk_check_menu_item_new_with_mnemonic(pFsk[index].lname);

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[index]);

		if (index == MFSKDEFAULT) {
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
		}

		g_signal_connect_swapped(G_OBJECT(check[index]), "activate", G_CALLBACK(set_frame_skip),
		        GINT_TO_POINTER(index));
	}
}
void menu_video_frame_skip_check(void) {
	int index;

	for (index = MFSKDEFAULT; index < NUMCHKS; index++) {
		if (cfg->frameskip == index) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
		}
	}
}
void set_frame_skip(int newframeskip) {
	if (guiupdate) {
		return;
	}
	if (cfg->frameskip == newframeskip) {
		guiUpdate();
		return;
	}
	cfg->frameskip = newframeskip;
	if (!fps.fastforward) {
		fpsNormalize();
	}
	guiUpdate();
}
