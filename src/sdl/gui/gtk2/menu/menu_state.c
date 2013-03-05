/*
 * menu_state.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_state.h"
#include "snd.h"
#include "save_slot.h"
#include "cfg_file.h"

#ifdef __SUNPRO_C
#pragma align 4 (savestate_icon_inline)
#endif
#ifdef __GNUC__
static const guint8 savestate_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 savestate_icon_inline[] =
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

#ifdef __SUNPRO_C
#pragma align 4 (loadstate_icon_inline)
#endif
#ifdef __GNUC__
static const guint8 loadstate_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 loadstate_icon_inline[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (621) */
  "\0\0\2\205"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\242\377\377\377\0\13r\304f\377p\302d\377n\277b\377k\274`\377h\270]\377"
  "e\265[\377b\261X\375_\254U\352[\250R\267X\243OpT\237K\35\205\377\377"
  "\377\0\14p\301d\377\260\333\246\377\257\332\246\377\255\331\243\377\253"
  "\330\242\377\250\327\237\377\245\325\234\377\235\320\224\377\214\305"
  "\203\377t\264l\377P\231G\260L\224D9\204\377\377\377\0\15m\276b\377\256"
  "\332\245\377\254\330\242\377\252\330\241\377\247\326\236\377\245\325"
  "\234\377\242\324\231\377\237\322\227\377\225\315\214\377\231\317\221"
  "\377{\270s\377G\217@\260C\212<\35\203\377\377\377\0\15j\272_\377g\266"
  "\\\377d\263Y\377`\256V\377]\252S\377Y\246P\377V\241M\377p\261h\377\220"
  "\310\210\377\225\314\215\377\222\313\213\377d\245]\377\77\2058p\212\377"
  "\377\377\0\6N\227F\217J\222B\342\210\303\201\377\205\304}\377u\263n\377"
  ";\1775\267\213\377\377\377\0\5E\215>\217k\253d\377\206\304\177\377\201"
  "\276y\3776z1\352\204\377\377\377\0\2[\250R\3X\243O{\205\377\377\377\0"
  "\5A\207:\217g\250a\377\202\302{\377|\274v\3772u-\352\204\377\377\377"
  "\0\2W\243N\237S\236K\322\204\377\377\377\0\6@\206:\217<\2016\342{\273"
  "u\377w\275p\377i\253c\377.p*\267\203\377\377\377\0\15V\242M\223o\261"
  "g\377k\255d\377K\224C\377G\217@\377C\212<\377@\2059\377Z\235T\377z\272"
  "t\377\177\301y\377}\277w\377O\221J\377+k&p\202\377\377\377\0\16V\241"
  "M\212n\260f\377\222\312\212\377\220\312\211\377\215\310\206\377\212\306"
  "\203\377\207\305\200\377\204\303~\377\201\302{\377t\273m\377|\276v\377"
  "]\240Y\377*k&\260'g#\35\202\377\377\377\0\15Q\234I\204j\254b\377\215"
  "\310\205\377\213\307\205\377\210\306\202\377\206\304\177\377\202\302"
  "|\377\200\301y\377x\271q\377e\250_\377N\221I\377*j%\260'f#9\204\377\377"
  "\377\0\13I\221B\220b\244Z\377^\241X\377>\2037\377:~4\3776z1\3773u.\377"
  "/q+\352,m(\267)i%p&f\"\35\206\377\377\377\0\2A\207:\234=\2027\322\216"
  "\377\377\377\0\2<\2016\3""9}3x\213\377\377\377\0"};

#ifdef __SUNPRO_C
#pragma align 4 (inc_icon_inline)
#endif
#ifdef __GNUC__
static const guint8 inc_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 inc_icon_inline[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (648) */
  "\0\0\2\240"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\1""7g\246E\202>m\251\341\1""7g\246E\205\377\377\377\0\3\271\276\267"
  "n\272\275\266\337\271\276\267j\204\377\377\377\0\4>m\251\341\377\377"
  "\377\377\273\314\341\377>m\251\341\205\377\377\377\0\3\272\275\266\370"
  "\364\365\364\377\272\275\266\360\204\377\377\377\0\1>m\251\341\202\273"
  "\314\341\377\1>m\251\341\205\377\377\377\0\3\272\275\266\377\377\377"
  "\377\377\272\275\266\377\204\377\377\377\0\1""7g\246E\202>m\251\341\1"
  "7g\246E\205\377\377\377\0\3\265\270\261\377\376\376\376\377\265\270\261"
  "\377\215\377\377\377\0\3\251\254\246\377\374\374\374\377\251\254\246"
  "\377\215\377\377\377\0\3\236\241\233\377\373\373\373\377\236\241\233"
  "\377\204\377\377\377\0\1""7g\246E\202>m\251\341\1""7g\246E\205\377\377"
  "\377\0\3\223\225\220\377\372\372\371\377\223\225\220\377\204\377\377"
  "\377\0\4>m\251\341\377\377\377\377\273\314\341\377>m\251\341\205\377"
  "\377\377\0\3\207\212\204\377\371\371\370\377\207\212\204\377\204\377"
  "\377\377\0\1>m\251\341\202\273\314\341\377\1>m\251\341\205\377\377\377"
  "\0\3|\177y\377\367\367\367\377|\177y\377\204\377\377\377\0\1""7g\246"
  "E\202>m\251\341\1""7g\246E\205\377\377\377\0\3qsn\377\366\366\365\377"
  "qsn\377\212\377\377\377\0\11fidkfhc\342gic\206fhc\377\365\365\364\377"
  "fhc\377fhb\221fic\336fidk\207\377\377\377\0\13Z\\X\336\237\241\236\377"
  "~\177|\377Z\\X\377\364\364\362\377Z\\X\377~\177|\377\237\241\235\377"
  "Z]X\342\377\377\377\0""7g\246E\202>m\251\341\1""7g\246E\202\377\377\377"
  "\0\16TVS\221z|x\377\344\344\342\377z|x\377\362\362\361\377z|x\377\343"
  "\343\341\377z|w\377VXT\206\377\377\377\0>m\251\341\377\377\377\377\273"
  "\314\341\377>m\251\341\202\377\377\377\0\13\200\200\200\2VWR\236z|w\377"
  "\342\342\341\377\361\361\357\377\342\342\341\377z|w\377VWR\236\200\200"
  "\200\2\377\377\377\0>m\251\341\202\273\314\341\377\1>m\251\341\203\377"
  "\377\377\0\7\200\200\200\2VWR\236y{w\377\341\341\340\377y{w\377VWR\236"
  "\200\200\200\2\202\377\377\377\0\1""7g\246E\202>m\251\341\1""7g\246E"
  "\204\377\377\377\0\5\200\200\200\2VWR\233[]Y\377VWR\236\200\200\200\2"
  "\203\377\377\377\0"};

#ifdef __SUNPRO_C
#pragma align 4 (dec_icon_inline)
#endif
#ifdef __GNUC__
static const guint8 dec_icon_inline[] __attribute__ ((__aligned__ (4))) =
#else
static const guint8 dec_icon_inline[] =
#endif
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (650) */
  "\0\0\2\242"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\1""7g\246E\202>m\251\341\1""7g\246E\204\377\377\377\0\5\200\200\200"
  "\2\272\275\266\236\275\300\271\377\272\275\267\233\200\200\200\2\203"
  "\377\377\377\0\4>m\251\341\377\377\377\377\273\314\341\377>m\251\341"
  "\203\377\377\377\0\7\200\200\200\2\272\275\266\236\312\315\307\377\371"
  "\371\370\377\312\315\307\377\272\275\266\236\200\200\200\2\202\377\377"
  "\377\0\1>m\251\341\202\273\314\341\377\1>m\251\341\202\377\377\377\0"
  "\13\200\200\200\2\272\275\266\236\312\315\307\377\371\371\370\377\377"
  "\377\377\377\371\371\370\377\312\315\307\377\272\275\266\236\200\200"
  "\200\2\377\377\377\0""7g\246E\202>m\251\341\1""7g\246E\202\377\377\377"
  "\0\11\265\271\261\206\306\311\303\377\367\367\367\377\306\311\303\377"
  "\376\376\376\377\306\311\303\377\366\366\366\377\306\311\303\377\265"
  "\271\262\221\207\377\377\377\0\11\251\254\246\342\353\354\353\377\275"
  "\300\273\377\251\254\246\377\374\374\374\377\251\254\246\377\274\277"
  "\272\377\317\320\314\377\251\254\247\336\207\377\377\377\0\13\235\242"
  "\233k\237\241\233\336\236\242\233\221\236\241\233\377\373\373\373\377"
  "\236\241\233\377\236\242\232\206\236\241\233\342\235\242\233k\377\377"
  "\377\0""7g\246E\202>m\251\341\1""7g\246E\205\377\377\377\0\3\223\225"
  "\220\377\372\372\371\377\223\225\220\377\204\377\377\377\0\4>m\251\341"
  "\377\377\377\377\273\314\341\377>m\251\341\205\377\377\377\0\3\207\212"
  "\204\377\371\371\370\377\207\212\204\377\204\377\377\377\0\1>m\251\341"
  "\202\273\314\341\377\1>m\251\341\205\377\377\377\0\3|\177y\377\367\367"
  "\367\377|\177y\377\204\377\377\377\0\1""7g\246E\202>m\251\341\1""7g\246"
  "E\205\377\377\377\0\3qsn\377\366\366\365\377qsn\377\215\377\377\377\0"
  "\3fhc\377\365\365\364\377fhc\377\215\377\377\377\0\3Z\\X\377\364\364"
  "\362\377Z\\X\377\204\377\377\377\0\1""7g\246E\202>m\251\341\1""7g\246"
  "E\205\377\377\377\0\3UWS\377\362\362\361\377UWS\377\204\377\377\377\0"
  "\4>m\251\341\377\377\377\377\273\314\341\377>m\251\341\205\377\377\377"
  "\0\3UWS\377\361\361\357\377UWS\377\204\377\377\377\0\1>m\251\341\202"
  "\273\314\341\377\1>m\251\341\205\377\377\377\0\3UWS\360\327\330\326\377"
  "UWS\370\204\377\377\377\0\1""7g\246E\202>m\251\341\1""7g\246E\205\377"
  "\377\377\0\3TWTjUWS\337VXSn\204\377\377\377\0"};

enum {
	MLOAD,
	MSSLOT0,
	MSSLOT1,
	MSSLOT2,
	MSSLOT3,
	MSSLOT4,
	MSSLOT5,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_state(GtkWidget *mainmenu, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *sslotsave, *sslotinc, *sslotdec, *sslot;

	menu = gtk_menu_new();
	sslot = gtk_image_menu_item_new_with_mnemonic("S_tate");

	sslotsave = gtk_image_menu_item_new_with_mnemonic("_Save state");
	check[MLOAD] = gtk_image_menu_item_new_with_mnemonic("_Load state");
	sslotinc = gtk_image_menu_item_new_with_mnemonic("_Inc slot");
	sslotdec = gtk_image_menu_item_new_with_mnemonic("_Dec slot");
	check[MSSLOT0] = gtk_check_menu_item_new_with_mnemonic("Slot _0");
	check[MSSLOT1] = gtk_check_menu_item_new_with_mnemonic("Slot _1");
	check[MSSLOT2] = gtk_check_menu_item_new_with_mnemonic("Slot _2");
	check[MSSLOT3] = gtk_check_menu_item_new_with_mnemonic("Slot _3");
	check[MSSLOT4] = gtk_check_menu_item_new_with_mnemonic("Slot _4");
	check[MSSLOT5] = gtk_check_menu_item_new_with_mnemonic("Slot _5");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(sslot), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mainmenu), sslot);

	gtk_widget_add_accelerator(sslotsave, "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(check[MLOAD], "activate", accel_group, GDK_F4, 0, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(sslotinc, "activate", accel_group, GDK_F3, 0, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(sslotdec, "activate", accel_group, GDK_F2, 0, GTK_ACCEL_VISIBLE);

	icon_inline(sslotsave, savestate_icon_inline)
	icon_inline(check[MLOAD], loadstate_icon_inline)
	icon_inline(sslotinc, inc_icon_inline)
	icon_inline(sslotdec, dec_icon_inline)

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), sslotsave);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MLOAD]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), sslotinc);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), sslotdec);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSSLOT0]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSSLOT1]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSSLOT2]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSSLOT3]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSSLOT4]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSSLOT5]);

	g_signal_connect_swapped(G_OBJECT(sslotsave), "activate",
			G_CALLBACK(menu_state_save_slot_action), GINT_TO_POINTER(SAVE));
	g_signal_connect_swapped(G_OBJECT(check[MLOAD]), "activate",
			G_CALLBACK(menu_state_save_slot_action), GINT_TO_POINTER(LOAD));
	g_signal_connect_swapped(G_OBJECT(sslotinc), "activate",
			G_CALLBACK(menu_state_save_slot_incdec), GINT_TO_POINTER(INC));
	g_signal_connect_swapped(G_OBJECT(sslotdec), "activate",
			G_CALLBACK(menu_state_save_slot_incdec), GINT_TO_POINTER(DEC));
	g_signal_connect_swapped(G_OBJECT(check[MSSLOT0]), "activate",
			G_CALLBACK(menu_state_save_slot_set), GINT_TO_POINTER(0));
	g_signal_connect_swapped(G_OBJECT(check[MSSLOT1]), "activate",
			G_CALLBACK(menu_state_save_slot_set), GINT_TO_POINTER(1));
	g_signal_connect_swapped(G_OBJECT(check[MSSLOT2]), "activate",
			G_CALLBACK(menu_state_save_slot_set), GINT_TO_POINTER(2));
	g_signal_connect_swapped(G_OBJECT(check[MSSLOT3]), "activate",
			G_CALLBACK(menu_state_save_slot_set), GINT_TO_POINTER(3));
	g_signal_connect_swapped(G_OBJECT(check[MSSLOT4]), "activate",
			G_CALLBACK(menu_state_save_slot_set), GINT_TO_POINTER(4));
	g_signal_connect_swapped(G_OBJECT(check[MSSLOT5]), "activate",
			G_CALLBACK(menu_state_save_slot_set), GINT_TO_POINTER(5));

}
void menu_state_check(void) {
	int index = 0;

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSSLOT0]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSSLOT1]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSSLOT2]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSSLOT3]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSSLOT4]), FALSE);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MSSLOT5]), FALSE);
	switch (save_slot.slot) {
		case 0:
			index = MSSLOT0;
			break;
		case 1:
			index = MSSLOT1;
			break;
		case 2:
			index = MSSLOT2;
			break;
		case 3:
			index = MSSLOT3;
			break;
		case 4:
			index = MSSLOT4;
			break;
		case 5:
			index = MSSLOT5;
			break;
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
	if (save_slot.state[save_slot.slot]) {
		gtk_widget_set_sensitive(GTK_WIDGET(ssload), TRUE);
		gtk_widget_set_sensitive(check[MLOAD], TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(ssload), FALSE);
		gtk_widget_set_sensitive(check[MLOAD], FALSE);
	}
	gtk_widget_queue_draw(GTK_WIDGET(saveslot));
}
void menu_state_save_slot_incdec(BYTE mode) {
	BYTE new_slot;

	if (mode == INC) {
		new_slot = save_slot.slot + 1;
		if (new_slot >= SAVE_SLOTS) {
			new_slot = 0;
		}
	} else {
		new_slot = save_slot.slot - 1;
		if (new_slot >= SAVE_SLOTS) {
			new_slot = SAVE_SLOTS - 1;
		}
	}
	menu_state_save_slot_set(new_slot);
}
void menu_state_save_slot_action(BYTE mode) {
	emu_pause(TRUE);

	if (mode == SAVE) {
		save_slot_save();
		cfg_file_pgs_save();
	} else {
		save_slot_load();
	}

	gui_update();

	emu_pause(FALSE);
}
void menu_state_save_slot_set(BYTE slot) {
	if (gui_in_update) {
		return;
	}

	save_slot.slot = slot;

	gtk_combo_box_set_active(GTK_COMBO_BOX(saveslot), slot);

	gui_update();
}
