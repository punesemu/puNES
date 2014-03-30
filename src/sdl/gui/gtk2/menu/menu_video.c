/*
 * menu_video.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video.h"
#include "menu_video_fps.h"
#include "menu_video_frame_skip.h"
#include "menu_video_size.h"
#include "menu_video_overscan.h"
#include "menu_video_filter.h"
#include "menu_video_palette.h"
#include "menu_video_rendering.h"
#include "menu_video_vsync.h"
#include "menu_video_effect.h"
#include "menu_video_tv_aspect_ratio.h"
#include "menu_video_interpolation.h"
#include "menu_video_txt_on_screen.h"
#include "menu_video_fullscreen.h"

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
  /* length: header (24) + pixel_data (758) */
  "\0\0\3\16"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\14~~{]|~z\361y{v\377x{v\377xzu\377wzu\377wyt\377vyt\377vxs\377uxs\377"
  "uwr\377twr\377\202tvq\377\24uxs\361vys]z|w\361\275\277\272\377\326\327"
  "\323\377\334\335\332\377\342\343\340\377\351\351\346\377\355\355\353"
  "\377\347\347\345\377\341\342\336\377\333\334\330\377\325\326\322\377"
  "\316\320\313\377\310\312\305\377\302\305\276\377\254\257\250\377rtp\361"
  "twr\377\316\320\313\377\214\11,Z\377T\270\273\267\377mpk\377rto\377\314"
  "\316\311\377\11,Z\377(Y\233\377(Z\233\3777f\243\377Rz\260\3770a\240\377"
  "$W\232\377Co\251\377g\213\272\377b\207\267\377]\203\264\377\13.\\\377"
  "\271\275\270\377kmh\377oqm\377\312\314\307\377\11,Z\3770a\242\3770b\243"
  "\377Ht\257\377Q{\263\377/b\244\3773e\246\377^\206\270\377]\205\270\377"
  "Z\202\266\377V\177\264\377\13.\\\377\272\276\272\377hjf\377loj\377\310"
  "\313\305\377\11,Z\3776h\251\3778i\252\377O{\265\377Dt\262\3777k\255\377"
  "P|\267\377W\202\271\377U\177\270\377Q}\266\377Ly\263\377\13.\\\377\274"
  "\277\273\377ehc\377jlg\377\306\311\303\377\11,Z\377=o\257\377@q\262\377"
  "N}\271\377Bt\265\377Hy\270\377S\201\275\377Q\177\273\377N}\272\377Iy"
  "\267\377Cs\263\377\13.\\\377\275\300\274\377ce`\377gie\377\304\306\300"
  "\377\11,Z\377As\264\377Ev\267\377Hz\272\377J|\275\377O\200\300\377P\200"
  "\300\377L~\276\377Gz\274\377Bu\270\377<p\264\377\13.\\\377\276\301\275"
  "\377`b^\377dgb\377\302\304\276\377\214\11,Z\377\24\277\302\276\377]_"
  "[\377fhb\361\253\256\250\377\305\307\302\377\313\315\307\377\320\322"
  "\315\377\325\326\322\377\331\332\326\377\334\335\331\377\335\336\332"
  "\377\334\335\331\377\331\332\326\377\325\326\322\377\320\322\315\377"
  "\313\315\310\377\257\261\254\377^`]\361ce`]be_\361\202^`\\\377\202]_"
  "[\377\202\\^Z\377\202[]Y\377\202Z\\X\377\4Z\\W\377Y[W\377]_Z\361]]Z]"
  "\205\377\377\377\0\6\211\213\206\377\304\305\301\377\273\274\270\377"
  "\267\270\265\377\273\274\271\377\211\213\206\377\210\377\377\377\0\5"
  "\214\214\211_\224\224\220\344\225\227\222\370\351\351\347\377\335\336"
  "\333\377\202\325\326\323\377\3\222\224\217\370\221\223\216\344\211\214"
  "\206_\206\377\377\377\0\12\211\214\206\363\363\363\362\377\361\361\360"
  "\377\357\360\357\377\356\356\355\377\355\355\355\377\355\355\354\377"
  "\354\354\354\377\361\361\361\377\215\217\211\365\206\377\377\377\0\2"
  "Y[W\277Z\\X\377\206\\^Z\377\2Z\\X\377XZV\314\223\377\377\377\0"};

void menu_video(GtkWidget *settings, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *video;

	menu = gtk_menu_new();

	video = gtk_image_menu_item_new_with_mnemonic("_Video");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(video), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(settings), video);

	gw_image_from_inline(video, icon_inline);
	menu_video_rendering(menu, NULL);
	menu_video_fps(menu, NULL);
	menu_video_frame_skip(menu, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	menu_video_size(menu, accel_group);
	menu_video_overscan(menu, NULL);
	menu_video_filter(menu, NULL);
	menu_video_palette(menu, NULL);
	menu_video_effect(menu, accel_group);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	menu_video_vsync(menu, NULL);
	menu_video_tv_aspect_ratio(menu, accel_group);
	menu_video_interpolation(menu, accel_group);
	menu_video_txt_on_screen(menu, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
	menu_video_fullscreen(menu, accel_group);
}
void menu_video_check(void) {
	menu_video_rendering_check();
	menu_video_fps_check();
	menu_video_frame_skip_check();
	menu_video_size_check();
	menu_video_overscan_check();
	menu_video_vsync_check();
	menu_video_tv_aspect_ratio_check();
	menu_video_interpolation_check();
	menu_video_txt_on_screen_check();
	menu_video_filter_check();
	menu_video_palette_check();
	menu_video_effect_check();
	menu_video_fullscreen_check();
}
