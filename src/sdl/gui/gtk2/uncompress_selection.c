/*
 * uncompress_selection.c
 *
 *  Created on: 24/dic/2013
 *      Author: fhorse
 */

#include <libgen.h>
#include <string.h>
#define __GUI_BASE__
#include "gui.h"
#undef __GUI_BASE__
#include "uncompress.h"

void uncompress_selection_treeview_init(void);
void uncompress_selection_ok_clicked(GtkWidget *widget, gpointer user_data);
void uncompress_selection_cancel_clicked(GtkWidget *widget, gpointer user_data);
void uncompress_selection_window_destroy(GtkWidget *widget, gpointer user_data);

enum uncompress_selection_columns {
	UNC_SEL_COLUMN_COUNT,
	UNC_SEL_COLUMN_ROM,
	UNC_SEL_NUM_COLUMNS
};

struct _uncompress_selection_data {
	GtkBuilder *builder;
	GtkWidget *window;
	BYTE on_selec;
	int selected;
} uncompress_selection;

int uncompress_selection_dialog(void) {
	memset(&uncompress_selection, 0x00, sizeof(uncompress_selection));

	dg_create_gtkbuilder(&uncompress_selection.builder, UNCOMPRESS_SELECTION_DIALOG);

	uncompress_selection.window = GTK_WIDGET(
	        gtk_builder_get_object(uncompress_selection.builder, "uncompress_selection_dialog"));

	uncompress_selection_treeview_init();

	gtk_builder_connect_signals(uncompress_selection.builder, NULL);

	/* ora collego i miei segnali */
	dg_signal_connect(uncompress_selection.builder, "uncompress_ok_button", "clicked",
			G_CALLBACK(uncompress_selection_ok_clicked), NULL);
	dg_signal_connect(uncompress_selection.builder, "uncompress_cancel_button", "clicked",
			G_CALLBACK(uncompress_selection_cancel_clicked), NULL);
	g_signal_connect(G_OBJECT(uncompress_selection.window), "destroy",
	        G_CALLBACK(uncompress_selection_window_destroy), NULL);

	/* disabilito la gestiore del docus della finestra principale */
	gui.main_win_lfp = FALSE;

	/*
	 * se l'archivio compresso e' caricato da riga di comando,
	 * la gui non e' ancora stata avviata.
	 */
	if (gui.start == TRUE) {
		emu_pause(TRUE);
		/* ridisegno lo screen sdl ogni tot millisecondi */
		//g_timeout_redraw_start();
	}

	gtk_widget_show(uncompress_selection.window);

	uncompress_selection.selected = UNCOMP_NO_FILE_SELECTED;
	uncompress_selection.on_selec = TRUE;

	while (uncompress_selection.on_selec == TRUE) {
		gui_flush();
		gui_sleep(30);
	}

	return (uncompress_selection.selected);
}

void uncompress_selection_treeview_init(void) {
	GtkTreeView *treeview;
	GtkListStore *liststore;

	treeview = _gw_get_treeview(uncompress_selection.builder, "uncompress_treeview");
	liststore = gtk_list_store_new(UNC_SEL_NUM_COLUMNS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);

	{
		int i;

		for (i = 0; i < uncomp.files_founded; i++) {
			GtkTreeIter iter;

			gtk_list_store_append(liststore, &iter);

			if (uncomp_name_file(&uncomp.file[i]) == EXIT_OK) {
				gtk_list_store_set(liststore, &iter, UNC_SEL_COLUMN_COUNT, i, UNC_SEL_COLUMN_ROM,
				        basename(uncomp.buffer), -1);
			}
		}
	}

	gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(liststore));
	g_object_unref(liststore);

	{
		GtkTreeViewColumn *column;
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

		column = _gw_get_treeviewcolumn(uncompress_selection.builder,
		        "uncompress_id_treeviewcolumn");

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), renderer, TRUE);
		gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(column), renderer, "text",
		        UNC_SEL_COLUMN_COUNT);

		column = _gw_get_treeviewcolumn(uncompress_selection.builder,
		        "uncompress_rom_treeviewcolumn");

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), renderer, TRUE);
		gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(column), renderer, "text",
		        UNC_SEL_COLUMN_ROM);
	}
}
void uncompress_selection_ok_clicked(GtkWidget *widget, gpointer user_data) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeView *treeview = _gw_get_treeview(uncompress_selection.builder, "uncompress_treeview");

	model = gtk_tree_view_get_model(treeview);

	if (gtk_tree_model_get_iter_first(model, &iter) == FALSE) {
		return;
	}

	if (gtk_tree_selection_get_selected(gtk_tree_view_get_selection(treeview), &model, &iter)) {
		gtk_tree_model_get(model, &iter, UNC_SEL_COLUMN_COUNT, &uncompress_selection.selected, -1);
	}

	gtk_widget_destroy(uncompress_selection.window);
}
void uncompress_selection_cancel_clicked(GtkWidget *widget, gpointer user_data) {
	gtk_widget_destroy(uncompress_selection.window);
}
void uncompress_selection_window_destroy(GtkWidget *widget, gpointer user_data) {
	uncompress_selection.window = NULL;

	g_object_unref(G_OBJECT(uncompress_selection.builder));

	uncompress_selection.on_selec = FALSE;

	if (gui.start == TRUE) {
		//g_timeout_redraw_stop();
		emu_pause(FALSE);
	}

	/* restituisco alla finestra principale la gestione del focus */
	gui.main_win_lfp = TRUE;
}
