/*
 * menu_video_rendering.c
 *
 *  Created on: 16/dic/2011
 *      Author: fhorse
 */

#ifdef OPENGL
#include "menu_video_rendering.h"
#include "opengl.h"
#include "sdlgfx.h"

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
  /* length: header (24) + pixel_data (493) */
  "\0\0\2\5"
  /* pixdata_type (0x2010002) */
  "\2\1\0\2"
  /* rowstride (64) */
  "\0\0\0@"
  /* width (16) */
  "\0\0\0\20"
  /* height (16) */
  "\0\0\0\20"
  /* pixel_data: */
  "\232\377\377\377\0\6\344\272/\11\333\252O\352\312\223P\375\310\220N\377"
  "\322\235/\337\336\2445\31\212\377\377\377\0\6\331\250P\377\330\245j\377"
  "\367\341\311\377\370\343\313\377\312\225B\377\302\2021\256\211\377\377"
  "\377\0\3\344\272/\377\364\324\247\377\370\341\305\377\202\371\343\314"
  "\377\2\367\333\275\377\311\220O\375\210\377\377\377\0\4\344\272/\377"
  "\370\355\303\377\370\342\250\377\335\256l\377\202\364\317\245\377\2\367"
  "\333\275\377\313\223S\367\207\377\377\377\0\10\344\272/\377\370\355\303"
  "\377\374\364\263\377\365\331h\377\363\317o\377\320\235Y\377\335\253s"
  "\377\311\221O\377\207\377\377\377\0\10\344\272/\377\370\355\303\377\374"
  "\364\263\377\365\334`\377\364\326D\377\372\356\216\377\346\264]\377\331"
  "\217;\377\207\377\377\377\0\10\344\272/\377\370\355\303\377\374\364\263"
  "\377\365\334`\377\364\326D\377\372\356\216\377\346\264]\377\331\217;"
  "\377\207\377\377\377\0\10\277\220'\377\370\355\303\377\374\364\263\377"
  "\365\334`\377\364\326D\377\372\356\216\377\346\264]\377\331\217;\377"
  "\207\377\377\377\0\10\271\211&\377\362\346\276\377\374\364\263\377\365"
  "\334`\377\364\326D\377\372\356\216\377\346\264]\377\331\217;\377\206"
  "\377\377\377\0\11\230c\40\40\271\211&\377\341\313\260\377\310\251g\377"
  "\365\334`\377\364\326D\377\372\356\216\377\346\264]\377\331\217;\377"
  "\207\377\377\377\0\10\223m\36\377\362\341\310\377\372\347\321\377\265"
  "}4\377\303\2311\377\334\304m\377\317\234J\377\307\2034\377\210\377\377"
  "\377\0\7\223m\36\377\371\343\313\377\354\252a\377\350\230@\377\302g\25"
  "\377\252`\26\377\264v,\377\211\377\377\377\0\6YB\22\377\324\234]\377"
  "\365\317\246\377\354\317\251\377\301\213H\377\264v,\377\212\377\377\377"
  "\0\5X@\23\377^B\25\377\234i%\377\264v,\377\272\213;\255\232\377\377\377"
  "\0"};

enum {
	MSOFTWARE,
	MOPENGL,
	MOPENGLSL,
	NUMCHKS
};

void rendering_set(int newrendering);

static GtkWidget *check[NUMCHKS];

void menu_video_rendering(GtkWidget *video, GtkAccelGroup *accel_group) {
	GtkWidget *menu, *rendering;

	menu = gtk_menu_new();
	rendering = gtk_image_menu_item_new_with_mnemonic("_Rendering");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(rendering), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(video), rendering);

	icon_inline(rendering, icon_inline)

	check[MSOFTWARE] = gtk_check_menu_item_new_with_mnemonic("_Software");
	check[MOPENGL] = gtk_check_menu_item_new_with_mnemonic("_OpenGL");
	check[MOPENGLSL] = gtk_check_menu_item_new_with_mnemonic("OpenGL _GLSL");

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MSOFTWARE]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MOPENGL]);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), check[MOPENGLSL]);

	g_signal_connect_swapped(G_OBJECT(check[MSOFTWARE]), "activate", G_CALLBACK(rendering_set),
	        GINT_TO_POINTER(MSOFTWARE));
	g_signal_connect_swapped(G_OBJECT(check[MOPENGL]), "activate", G_CALLBACK(rendering_set),
	        GINT_TO_POINTER(MOPENGL));
	g_signal_connect_swapped(G_OBJECT(check[MOPENGLSL]), "activate", G_CALLBACK(rendering_set),
	        GINT_TO_POINTER(MOPENGLSL));
}
void menu_video_rendering_check(void) {
	int index;

	if (opengl.glsl.compliant) {
		gtk_widget_set_sensitive(check[MOPENGLSL], TRUE);
	} else {
		gtk_widget_set_sensitive(check[MOPENGLSL], FALSE);
	}

	for (index = MSOFTWARE; index < NUMCHKS; index++) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), FALSE);
	}

	if (!gfx.opengl) {
		index = MSOFTWARE;
	} else {
		if (!opengl.glsl.compliant) {
			index = MOPENGL;
		} else if (!opengl.glsl.enabled) {
			index = MOPENGL;
		} else {
			index = MOPENGLSL;
		}
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check[index]), TRUE);
}
void rendering_set(int newrendering) {
	gint x, y;

	if (guiupdate) {
		return;
	}

	if ((gfx.opengl + opengl.glsl.enabled) == newrendering) {
		guiUpdate();
		return;
	}

	/* salvo la posizione */
	gtk_window_get_position(GTK_WINDOW(mainWin), &x, &y);

	/*
	 * se non nascondo la finestra, al momento del
	 * SDL_QuitSubSystem e del SDL_InitSubSystem
	 * l'applicazione crasha.
	 */
	gtk_widget_hide(mainWin);

	if (gfx.opengl && !newrendering) {
		opengl.rotation = FALSE;
	}

	/* switch opengl/software render */
	switch (newrendering) {
		case MSOFTWARE:
			gfx.opengl = FALSE;
			opengl.glsl.enabled = FALSE;
			break;
		case MOPENGL:
			gfx.opengl = TRUE;
			opengl.glsl.enabled = FALSE;
			break;
		case MOPENGLSL:
			gfx.opengl = TRUE;
			opengl.glsl.enabled = TRUE;
			break;
	}

	gfxResetVideo();
	gfxSetScreen(NOCHANGE, NOCHANGE, NOCHANGE, NOCHANGE, TRUE);

	/* rispristino la posizione */
	gtk_window_move(GTK_WINDOW(mainWin), x, y);

	gtk_widget_show(mainWin);
}
#endif
