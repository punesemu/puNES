/*
 * menu_video_effect.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#include "menu_video_effect.h"
#include "opengl.h"
#include "gfx.h"
#include "input.h"
#include "cfg_file.h"
#include "openGL/no_effect.h"
#include "openGL/cube3d.h"

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
  /* length: header (24) + pixel_data (717) */
  "\0\0\2\345"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\243\377\377\377\0\12\341\341\341<\344\337\330\265\345\326\306\366\345"
  "\320\274\377\346\316\270\377\346\313\264\377\346\311\261\377\347\312"
  "\264\370\347\325\307\304\345\345\345<\205\377\377\377\0\14\343\343\343"
  "\241\346\332\315\371\350\323\272\377\356\325\276\377\351\317\266\377"
  "\331\276\245\377\325\270\237\377\334\273\237\377\361\310\252\377\356"
  "\303\243\377\352\301\242\376\351\334\323\262\203\377\377\377\0v\343\343"
  "\343\271\346\323\300\377\354\323\273\377\320\271\241\377\263\240\217"
  "\377\253\241\233\377\246\250\263\377\241\245\264\377\233\241\262\377"
  "\246\235\233\377\300\235\201\377\354\272\224\377\356\270\221\377\354"
  "\322\277\322\377\377\377\0\340\340\340\216\346\326\305\377\354\321\267"
  "\377\277\253\227\377\305\312\326\377\251\304\357\377\244\300\357\377"
  "\235\273\356\377\225\266\355\377\216\261\353\377\205\253\351\377\261"
  "\307\357\377\331\272\244\377\344\254\203\377\360\261\206\377\343\323"
  "\307\240\344\334\323\350\345\313\255\377\326\312\275\377\373\373\373"
  "\377\262\311\361\377\242\277\357\377\233\272\356\377\20\25\35\377\17"
  "\24\33\377\204\252\352\377|\245\350\377\204\252\351\377\373\373\373\377"
  "\347\322\304\377\345\246{\377\321\232w\373\337\312\261\376\350\342\334"
  "\377\367\367\367\377\373\373\373\377\253\304\360\377\230\270\354\377"
  "\221\264\354\377\17\23\33\377\16\22\31\377z\244\347\377s\235\345\377"
  "}\244\347\377\373\373\373\377\367\367\367\377\354\334\322\377\314\217"
  "h\377\344\340\335\374\356\356\356\377\364\364\364\377\371\371\371\377"
  "\276\321\360\377\216\261\351\377\206\253\347\377~\246\345\377w\240\344"
  "\377p\233\343\377h\226\341\377\236\273\353\377\371\371\371\377\364\364"
  "\364\377\356\356\356\377\335\261\224\376\343\322\302\356\351\340\327"
  "\377\360\360\360\377\364\364\364\377\334\344\362\377\232\267\347\377"
  "{\241\342\377s\235\341\377l\230\337\377d\223\336\377|\242\342\377\317"
  "\333\357\377\364\364\364\377\360\360\360\377\347\325\313\377\337\241"
  "y\371\341\333\325\231\350\310\255\377\352\323\300\377\355\346\340\377"
  "\357\357\357\377\321\332\351\377\241\271\341\377w\234\332\377q\230\331"
  "\377\225\260\337\377\311\324\350\377\360\360\360\377\356\351\346\377"
  "\351\311\265\377\342\226f\377\306\233\201\306\377\377\377\0\331\317\307"
  "\306\343\274\234\377\354\300\235\377\353\321\276\356\353\334\323\346"
  "\353\347\344\340\203\353\353\353\336\6\352\352\352\336\351\325\311\346"
  "\347\263\221\377\341\226f\377\272\216r\346\260rJ\1\202\377\377\377\0"
  "\15\313\301\272\261\305\251\224\373\335\252\205\377\351\264\215\377\351"
  "\273\233\377\347\300\245\377\346\277\245\377\347\275\241\377\343\261"
  "\220\377\343\232l\377\303\206`\376\266\226\202\312\257qI\4\204\377\377"
  "\377\0\12\305\225r\7\263\206fW\254\177_\250\260\200_\351\266\201_\365"
  "\271\201\\\366\267}Y\364\256uQ\302\245mIr\246kG)\243\377\377\377\0"};

enum {
	MEFFECT,
	MCUBE,
	NUMCHKS
};

static GtkWidget *check[NUMCHKS];

void menu_video_effect(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu;

	/* Settings/Video/Effect */
	menu = gtk_menu_new();
	check[MEFFECT] = gtk_image_menu_item_new_with_mnemonic("_Effect");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(check[MEFFECT]), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), check[MEFFECT]);

	gw_image_from_inline(check[MEFFECT], icon_inline);

	check[MCUBE] = gtk_check_menu_item_new_with_mnemonic("_Cube");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MCUBE]);

	gtk_widget_add_accelerator(check[MCUBE], "activate", accel_group, GDK_r, (GdkModifierType) 0,
			GTK_ACCEL_VISIBLE);

	g_signal_connect_swapped(G_OBJECT(check[MCUBE]), "activate",
	        G_CALLBACK(menu_video_effect_set), GINT_TO_POINTER(FALSE));
}
void menu_video_effect_check(void) {
	if (gfx.opengl && (input_zapper_is_connected((_port *) &port) == FALSE)) {
		gtk_widget_set_sensitive(check[MEFFECT], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MEFFECT], FALSE);
	}

	/* Effect */
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MCUBE]), FALSE);
	if (opengl.rotation) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[MCUBE]), TRUE);
	}
}
void menu_video_effect_set(void) {
	if (gui_in_update) {
		return;
	}

	if (input_zapper_is_connected((_port *) &port) == TRUE) {
		return;
	}

	opengl_unset_effect();

	opengl.rotation = !opengl.rotation;

	if (opengl.rotation) {
		opengl_init_effect = opengl_init_cube3d;
		opengl_set_effect = opengl_set_cube3d;
		opengl_unset_effect = opengl_unset_cube3d;
		opengl_draw_scene = opengl_draw_scene_cube3d;

		opengl.factor_distance = opengl.x_rotate = opengl.y_rotate = 0;
		if (cfg->fullscreen == FULLSCR) {
			SDL_ShowCursor(SDL_ENABLE);
		}

	} else {
		opengl_init_effect = opengl_init_no_effect;
		opengl_set_effect = opengl_set_no_effect;
		opengl_unset_effect = opengl_unset_no_effect;
		opengl_draw_scene = opengl_draw_scene_no_effect;

		if (cfg->fullscreen == FULLSCR) {
			SDL_ShowCursor(SDL_DISABLE);
		}
	}

	opengl_init_effect();

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE);
}
