/*
 * netplay.c
 *
 *  Created on: 27/nov/2011
 *      Author: fhorse
 */

#include <string.h>
#include <ctype.h>
#include "gtk2.h"
#include "netplay.h"
#include "zeromq.h"
#include "sdlsnd.h"
#include "sdlgfx.h"

struct _net {
	GtkWidget *widget[WINDGETS];
	uint8_t findrun;
	char external_ip[20];
	void *protocol_data;
	_exchange_info exchange;
} net;

void active_server_mode(void);
void active_client_mode(void);

gboolean nickname_focus_out_event(GtkEntry *entry, GdkEvent *event, gpointer user_data);
void nickname_activate(GtkEntry *entry, gpointer user_data);

void radiobutton_enable_toggled(GtkToggleButton *togglebutton, int type);
void cancel_clicked(GtkButton *button, gpointer user_data);

void server_connect_clicked(GtkButton *button, gpointer user_data);
void server_disconnect_clicked(GtkButton *button, gpointer user_data);

void client_connect_clicked(GtkButton *button, gpointer user_data);
void client_disconnect_clicked(GtkButton *button, gpointer user_data);

void chat_entry_activated(GtkEntry *entry, gpointer user_data);

uint8_t chat_display_message(void *guievent);
uint8_t info_display_message(void *guievent);
uint8_t enable_widget(void *guievent);

void find_external_ip(void);


void netplay_init(void) {
	memset(&net, 0, sizeof(net));
}
void netplay_create(void) {
	GtkWidget *mainbox, *alignment, *entry;
	GtkWidget *vbox, *hbox, *frame, *label;
	GtkWidget *cancel;

	/* se e' gia' aperta la finestra, non posso aprirne un'altra */
	if (net.widget[NETPLAY]) {
		return;
	}

	emuPause(TRUE, SNDNOSYNC);

	net.widget[NETPLAY] = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_container_set_border_width(GTK_CONTAINER(net.widget[NETPLAY]), 10);
	gtk_window_set_resizable(GTK_WINDOW(net.widget[NETPLAY]), FALSE);

	gtk_window_set_title(GTK_WINDOW(net.widget[NETPLAY]), "Net play");

	mainbox = gtk_vbox_new(FALSE, SPACING);
	gtk_container_add(GTK_CONTAINER(net.widget[NETPLAY]), mainbox);

	/* rules frame */
	{
		GSList *netplay_enable_group = NULL;

		frame = gtk_frame_new(" Rules ");
		gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

		alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
		gtk_container_add(GTK_CONTAINER(frame), alignment);
		gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 7, 7, 7, 7);

		vbox = gtk_vbox_new(FALSE, SPACING);
		gtk_container_add(GTK_CONTAINER(alignment), vbox);

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		net.widget[ID_SERVER_ENABLE] = gtk_radio_button_new_with_label(NULL, "server");
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_SERVER_ENABLE], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_SERVER_ENABLE], 100, -1);
		gtk_radio_button_set_group(GTK_RADIO_BUTTON(net.widget[ID_SERVER_ENABLE]),
				netplay_enable_group);
		netplay_enable_group = gtk_radio_button_get_group(
				GTK_RADIO_BUTTON(net.widget[ID_SERVER_ENABLE]));

		g_signal_connect(G_OBJECT(net.widget[ID_SERVER_ENABLE]), "toggled",
				G_CALLBACK(radiobutton_enable_toggled), GINT_TO_POINTER(SERVER));

		net.widget[ID_CLIENT_ENABLE] = gtk_radio_button_new_with_label(NULL, "client");
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_CLIENT_ENABLE], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_CLIENT_ENABLE], 100, -1);
		gtk_radio_button_set_group(GTK_RADIO_BUTTON(net.widget[ID_CLIENT_ENABLE]),
		        netplay_enable_group);

		g_signal_connect(G_OBJECT(net.widget[ID_CLIENT_ENABLE]), "toggled",
				G_CALLBACK(radiobutton_enable_toggled), GINT_TO_POINTER(CLIENT));

		net.widget[ID_NICKNAME] = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_NICKNAME], TRUE, TRUE, 0);
		//gtk_entry_set_alignment(GTK_ENTRY(net.widget[ID_NICKNAME]), TRUE);
		gtk_entry_set_max_length(GTK_ENTRY(net.widget[ID_NICKNAME]), NET_MAX_NAME_SIZE - 1);

		g_signal_connect(G_OBJECT(net.widget[ID_NICKNAME]), "activate",
		        G_CALLBACK(nickname_activate), NULL);
		g_signal_connect_after(G_OBJECT(net.widget[ID_NICKNAME]), "focus-out-event",
		        G_CALLBACK(nickname_focus_out_event), NULL);

		net_create_random_name(net.exchange.nickname, 10);

		gtk_entry_set_text(GTK_ENTRY(net.widget[ID_NICKNAME]), net.exchange.nickname);
	}

	/* server frame */
	{
		net.widget[ID_SERVER_FRAME] = gtk_frame_new(" Server mode ");
		gtk_box_pack_start(GTK_BOX(mainbox), net.widget[ID_SERVER_FRAME], FALSE, FALSE, 0);

		alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
		gtk_container_add(GTK_CONTAINER(net.widget[ID_SERVER_FRAME]), alignment);
		gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 7, 7, 7, 7);

		vbox = gtk_vbox_new(FALSE, SPACING);
		gtk_container_add(GTK_CONTAINER(alignment), vbox);

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		label = gtk_label_new("external IP :");
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_widget_set_size_request(label, 100, -1);

		net.widget[ID_SERVER_IP] = gtk_entry_new();
		gtk_box_pack_end(GTK_BOX(hbox), net.widget[ID_SERVER_IP], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_SERVER_IP], 204, -1);
		gtk_editable_set_editable(GTK_EDITABLE(net.widget[ID_SERVER_IP]), FALSE);
		gtk_entry_set_alignment(GTK_ENTRY(net.widget[ID_SERVER_IP]), TRUE);
		gtk_entry_set_text(GTK_ENTRY(net.widget[ID_SERVER_IP]), "127.0.0.1");

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		net.widget[ID_SERVER_PORT] = gtk_combo_box_entry_new_text();
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_SERVER_PORT], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_SERVER_PORT], 100, -1);

		net.widget[ID_SERVER_DISCONNECT] = gtk_button_new_with_label("Disconnect");
		gtk_box_pack_end(GTK_BOX(hbox), net.widget[ID_SERVER_DISCONNECT], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_SERVER_DISCONNECT], 100, -1);

		g_signal_connect(G_OBJECT(net.widget[ID_SERVER_DISCONNECT]), "clicked",
				G_CALLBACK(server_disconnect_clicked), NULL);

		net.widget[ID_SERVER_CONNECT] = gtk_button_new_with_label("Connect");
		gtk_box_pack_end(GTK_BOX(hbox), net.widget[ID_SERVER_CONNECT], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_SERVER_CONNECT], 100, -1);

		g_signal_connect(G_OBJECT(net.widget[ID_SERVER_CONNECT]), "clicked",
				G_CALLBACK(server_connect_clicked), NULL);

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		net.widget[ID_SERVER_INFO] = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_SERVER_INFO], TRUE, TRUE, 0);
		gtk_editable_set_editable(GTK_EDITABLE(net.widget[ID_SERVER_INFO]), FALSE);
		//gtk_entry_set_alignment(GTK_ENTRY(net.widget[ID_SERVER_INFO]), TRUE);
	}

	/* client Frame */
	{
		net.widget[ID_CLIENT_FRAME] = gtk_frame_new(" Client mode ");
		gtk_box_pack_start(GTK_BOX(mainbox), net.widget[ID_CLIENT_FRAME], FALSE, FALSE, 0);

		alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
		gtk_container_add(GTK_CONTAINER(net.widget[ID_CLIENT_FRAME]), alignment);
		gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 7, 7, 7, 7);

		vbox = gtk_vbox_new(FALSE, SPACING);
		gtk_container_add(GTK_CONTAINER(alignment), vbox);

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		label = gtk_label_new("connect to :");
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_widget_set_size_request(label, 100, -1);

		net.widget[ID_CLIENT_IP] = gtk_combo_box_text_new_with_entry();
		gtk_box_pack_end(GTK_BOX(hbox), net.widget[ID_CLIENT_IP], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_CLIENT_IP], 204, 28);

		entry = gtk_bin_get_child(GTK_BIN(net.widget[ID_CLIENT_IP]));
		gtk_entry_set_alignment(GTK_ENTRY(entry), TRUE);

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		net.widget[ID_CLIENT_PORT] = gtk_combo_box_entry_new_text();
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_CLIENT_PORT], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_CLIENT_PORT], 100, -1);

		net.widget[ID_CLIENT_DISCONNECT] = gtk_button_new_with_label("Disconnect");
		gtk_box_pack_end(GTK_BOX (hbox), net.widget[ID_CLIENT_DISCONNECT], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_CLIENT_DISCONNECT], 100, -1);

		g_signal_connect(G_OBJECT(net.widget[ID_CLIENT_DISCONNECT]), "clicked",
				G_CALLBACK(client_disconnect_clicked), NULL);

		net.widget[ID_CLIENT_CONNECT] = gtk_button_new_with_label("Connect");
		gtk_box_pack_end(GTK_BOX (hbox), net.widget[ID_CLIENT_CONNECT], FALSE, FALSE, 0);
		gtk_widget_set_size_request(net.widget[ID_CLIENT_CONNECT], 100, -1);

		g_signal_connect(G_OBJECT(net.widget[ID_CLIENT_CONNECT]), "clicked",
				G_CALLBACK(client_connect_clicked), NULL);

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		net.widget[ID_CLIENT_INFO] = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_CLIENT_INFO], TRUE, TRUE, 0);
		gtk_editable_set_editable(GTK_EDITABLE(net.widget[ID_CLIENT_INFO]), FALSE);
		//gtk_entry_set_alignment(GTK_ENTRY(net.widget[ID_CLIENT_INFO]), TRUE);
	}

	/* chat frame */
	{
		GtkWidget *scrolledwindow;

		net.widget[ID_CHAT_FRAME] = gtk_frame_new(" Chat ");
		gtk_box_pack_start(GTK_BOX(mainbox), net.widget[ID_CHAT_FRAME], FALSE, FALSE, 0);

		alignment = gtk_alignment_new(0.5, 0.5, 1, 1);
		gtk_container_add(GTK_CONTAINER(net.widget[ID_CHAT_FRAME]), alignment);
		gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 7, 7, 7, 7);

		vbox = gtk_vbox_new(FALSE, SPACING);
		gtk_container_add(GTK_CONTAINER(alignment), vbox);

		scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
		gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, FALSE, FALSE, 0);
		gtk_widget_set_size_request(scrolledwindow, -1, 230);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_SHADOW_IN);

		net.widget[ID_CHAT_WINDOW] = gtk_text_view_new();
		gtk_text_view_set_editable(GTK_TEXT_VIEW(net.widget[ID_CHAT_WINDOW]), FALSE);
		gtk_container_add(GTK_CONTAINER(scrolledwindow), net.widget[ID_CHAT_WINDOW]);

		hbox = gtk_hbox_new(FALSE, SPACING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

		net.widget[ID_CHAT_ENTRY] = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), net.widget[ID_CHAT_ENTRY], TRUE, TRUE, 0);

		g_signal_connect(G_OBJECT(net.widget[ID_CHAT_ENTRY]), "activate",
				G_CALLBACK(chat_entry_activated), NULL);
	}

	cancel = gtk_button_new_with_label("Close");
	gtk_box_pack_end(GTK_BOX(mainbox), cancel, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(cancel), "clicked", G_CALLBACK (cancel_clicked), NULL);

	gtk_widget_show_all(net.widget[NETPLAY]);

	/* TODO: devo aggiungere la lettura del file di configurazione */

	g_thread_create((GThreadFunc) find_external_ip, NULL, FALSE, NULL);

	active_server_mode();
}
void netplay_display_message(uint8_t mode, uint32_t size, const char *fmt, ...) {
	_guievent *event;
	void *funct;
	va_list ap;

	event = malloc(sizeof(_guievent));
	event->arg[0] = malloc(sizeof(uint32_t));

	if (mode == INFO) {
		funct = info_display_message;
		if (net.exchange.rule == SERVER) {
			(*(uint32_t *)event->arg[0]) = ID_SERVER_INFO;
		} else {
			(*(uint32_t *)event->arg[0]) = ID_CLIENT_INFO;
		}
	} else {
		funct = chat_display_message;
	}

	if (!size || (size > NET_MAX_MSG_SIZE))  {
		size = NET_MAX_MSG_SIZE + 1;
	} else {
		/* aggiungo lo 0 della terminazione */
		size += 1;
	}

	event->arg[1] = malloc(size);

	va_start(ap, fmt);
	vsnprintf(event->arg[1], size, fmt, ap);
	va_end(ap);

	guiAddEvent(funct, event);
}
void netplay_enable_widget(uint32_t widget, uint8_t mode) {
	_guievent *event;

	event = malloc(sizeof(_guievent));
	event->arg[0] = malloc(sizeof(uint32_t));
	event->arg[1] = malloc(sizeof(uint8_t));

	(*(uint32_t *)event->arg[0]) = widget;
	(*(uint8_t *)event->arg[1]) = mode;

	guiAddEvent(enable_widget, event);
}

void active_server_mode(void) {
	/* se attiva gia' un'altra connessione la chiudo */
	net_close(net.protocol_data);

	/* assegno il nuovo ruolo */
	net.exchange.rule = SERVER;

	/* disabilito il frames client e chat */
	netplay_disab_widget(ID_CLIENT_FRAME);
	netplay_disab_widget(ID_CHAT_FRAME);

	/* abilito il frame server disabilitando PORT, CONNECT e DISCONNECT */
	netplay_enab_widget(ID_SERVER_FRAME);
	netplay_disab_widget(ID_SERVER_PORT);
	netplay_disab_widget(ID_SERVER_CONNECT);
	netplay_disab_widget(ID_SERVER_DISCONNECT);

	/* reinizializzo il net framework */
	if ((net.protocol_data = net_init(&net.exchange)) == NULL) {
		netplay_display_message(INFO, 0, "init error, impossible to establish connections!\n");
		netplay_disab_widget(ID_SERVER_FRAME);
		return;
	}

	/* avvio il thread del server */
	g_thread_create((GThreadFunc) net_server_thread, net.protocol_data, FALSE, NULL);
}
void active_client_mode(void) {
	/* se attiva gia' un'altra connessione la chiudo */
	net_close(net.protocol_data);

	/* assegno il nuovo ruolo */
	net.exchange.rule = CLIENT;

	/* disabilito i frames server e chat */
	netplay_disab_widget(ID_SERVER_FRAME);
	netplay_disab_widget(ID_CHAT_FRAME);

	/* abilito il frame client disabilitano PORT e DISCONNECT */
	netplay_enab_widget(ID_CLIENT_FRAME);
	netplay_disab_widget(ID_CLIENT_PORT);
	netplay_disab_widget(ID_CLIENT_DISCONNECT);

	/* reinizializzo il net framework */
	if ((net.protocol_data = net_init(&net.exchange)) == NULL) {
		netplay_display_message(INFO, 0, "init error, impossible to establish connections!\n");
		netplay_disab_widget(ID_CLIENT_FRAME);
		return;
	}

	/* avvio il thread del client */
	g_thread_create((GThreadFunc) net_client_thread, net.protocol_data, FALSE, NULL);
}

gboolean nickname_focus_out_event(GtkEntry *entry, GdkEvent *event, gpointer user_data) {
	nickname_activate(entry, NULL);
	return (FALSE);
}
void nickname_activate(GtkEntry *entry, gpointer user_data) {
	char *string = (char *) gtk_entry_get_text(entry);

	if (strcmp(string, net.exchange.nickname)) {
		strncpy(net.exchange.nickname, string, NET_MAX_NAME_SIZE);

		if (net.exchange.peer_connected == PEER_CONNECTED) {
			if (net.exchange.rule == CLIENT) {
				net_client_send(net.protocol_data, NET_CHANGENICK, net.exchange.nickname,
						strlen(net.exchange.nickname));
			} else {
				net.exchange.changed_nickname = TRUE;
			}
		}
	}
}

void radiobutton_enable_toggled(GtkToggleButton *togglebutton, int type) {
	if (gtk_toggle_button_get_active(togglebutton)) {
		if (type == SERVER) {
			active_server_mode();
		} else {
			active_client_mode();
		}
	}
}
void cancel_clicked(GtkButton *button, gpointer user_data) {
	/* chiudo qualunque connessione */
	net_close(net.protocol_data);

	/* se ci sono eventi gtk pendenti li eseguo */
	guiFlush();

	/* ditruggo la finestra */
	gtk_widget_destroy(net.widget[NETPLAY]);

	/* riazzero tutto (e' importante che sia l'ultima istruzione) */
	netplay_init();

	emuPause(FALSE, 2000);
}

void server_connect_clicked(GtkButton *button, gpointer user_data) {
	netplay_enab_widget(ID_SERVER_PORT);
	netplay_disab_widget(ID_SERVER_CONNECT);

	net.exchange.accept_peer = SERVER_ACCEPT;
}
void server_disconnect_clicked(GtkButton *button, gpointer user_data) {
	if (net.exchange.peer_connected) {
		net.exchange.want_disconnect = TRUE;
	} else if (net.exchange.accept_peer == SERVER_WAIT) {
		net.exchange.accept_peer = SERVER_REFUSE;
	}
}

void client_connect_clicked(GtkButton *button, gpointer user_data) {
	char *ip = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(net.widget[ID_CLIENT_IP]));

	if (net_client(net.protocol_data, ip) == EXIT_OK) {
		net_client_send(net.protocol_data, NET_REQUEST_CONNECTION, NULL, 0);
	}
}
void client_disconnect_clicked(GtkButton *button, gpointer user_data) {
	net.exchange.want_disconnect = TRUE;
}

void chat_entry_activated(GtkEntry *entry, gpointer user_data) {
	GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
	char *string, formatted[NET_MAX_MSG_SIZE + NET_MAX_NAME_SIZE + 1];

	string = (char *) gtk_entry_get_text(entry);

	sprintf(formatted, "%s: %s\n", net.exchange.nickname, string);

	netplay_display_message(CHAT, 0, formatted);

	if (net.exchange.rule == CLIENT) {
		net_client_send(net.protocol_data, NET_CHAT, formatted, strlen(formatted));
	} else {
		strcpy(net.exchange.chat, formatted);
		net.exchange.chat_available = TRUE;
	}

	gtk_entry_buffer_delete_text(buffer, 0, -1);
}

uint8_t chat_display_message(void *guievent) {
	_guievent *event = guievent;
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(net.widget[ID_CHAT_WINDOW]));
	gtk_text_buffer_insert_at_cursor(buffer, event->arg[1], -1);

	free(event->arg[0]);
	free(event->arg[1]);
	free(guievent);

	return (FALSE);
}
uint8_t info_display_message(void *guievent) {
	_guievent *event = guievent;

	gtk_entry_set_text(GTK_ENTRY(net.widget[(*(uint32_t *)event->arg[0])]), event->arg[1]);

	free(event->arg[0]);
	free(event->arg[1]);
	free(guievent);

	return (FALSE);
}
uint8_t enable_widget(void *guievent) {
	_guievent *event = guievent;

	gtk_widget_set_sensitive(GTK_WIDGET(net.widget[(*(uint32_t *)event->arg[0])]),
	        (*(uint8_t *)event->arg[1]));

	free(event->arg[0]);
	free(event->arg[1]);
	free(guievent);

	return (FALSE);
}

void find_external_ip(void) {
	char checkip[] = "checkip.dyndns.org";
	struct addrinfo hints, *server;
	int s, sock = 0;

	if (net.findrun) {
		g_thread_exit(NULL);
	}

	net.findrun = TRUE;

	strcpy(net.external_ip, "127.0.0.1");

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((s = getaddrinfo(checkip, "80", &hints, &server)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		goto exit;
	}
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("soket");
		goto exit;
	}
	if (connect(sock, server->ai_addr, server->ai_addrlen) != 0) {
		perror("connect");
		goto exit;
	}
	{
		char get_request[512];

		sprintf(get_request, "GET http://%s/ HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
				checkip, checkip);

		if (send(sock, get_request, strlen(get_request) + 1, 0) < 0) {
			goto exit;
		}
	}
	while (TRUE) {
		struct timeval tv;
		fd_set fdset;
		char response[1024], *p;

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);

		select(sock + 1, &fdset, NULL, NULL, &tv);

		if (FD_ISSET(sock, &fdset)) {
			if (recv(sock, response, sizeof(response) - 1, 0) > 0) {
				if ((p = strstr(response, "Current IP Address: "))) {
					int i;

					p += strlen("Current IP Address: ");

					for (i = 0; ((isdigit((*p)) || (*p) == '.') && i < 15); p++, i++) {
						(*(net.external_ip + i)) = (*p);
					}
					(*(net.external_ip + i)) = 0;
					break;
				}
			}
		}
	}

	exit: gdk_threads_enter();
	gtk_entry_set_text(GTK_ENTRY(net.widget[ID_SERVER_IP]), net.external_ip);
	gdk_threads_leave();

	if (sock) {
		close(sock);
	}

	net.findrun = FALSE;

	g_thread_exit(NULL);
}
