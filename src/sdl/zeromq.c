/*
 * zeromq2.c
 *
 *  Created on: 06/dic/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zmq.h>
#include "zeromq.h"
#include "gui.h"
#include "version.h"

#define chat(...) netplay_display_message(CHAT, 0, __VA_ARGS__)
#define chat_size(sz, ...) netplay_display_message(CHAT, sz, __VA_ARGS__)
#define info(...) netplay_display_message(INFO, 0, __VA_ARGS__)
#define info_size(sz, ...) netplay_display_message(INFO, sz, __VA_ARGS__)

typedef struct {
	void *context;
	void *socket;
	zmq_msg_t in;
	zmq_msg_t out;
	uint8_t in_send;
	uint32_t error;
	_exchange_info *exchange;
} _zmq;

void net_close_socket(void *data);

void *net_init(_exchange_info *exchange) {
	_zmq *zmq = 0;

	zmq = malloc(sizeof(_zmq));

	memset(zmq, 0, sizeof(_zmq));

	if ((zmq->context = zmq_init(2)) == NULL) {
		info("zmq_init %s", zmq_strerror(errno));
		free(zmq);
		return (NULL);
	}

	zmq->exchange = exchange;

	return (zmq);
}

uint8_t net_server(void *data) {
	_zmq *zmq = data;

	if (zmq->exchange->active && !zmq->exchange->want_disconnect) {
		info("server is alredy active");
		return (EXIT_ERROR);
	}

	zmq->error = 0;
	zmq->exchange->accept_peer = SERVER_WAIT;
	zmq->exchange->peer_connected = PEER_NO_CONNECTED;

	if ((zmq->socket = zmq_socket(zmq->context, ZMQ_REP)) == NULL) {
		info("zmq_socket %s", zmq_strerror(errno));
		goto net_server_error;
	}

	{
		int linger = 0;
		zmq_setsockopt (zmq->socket, ZMQ_LINGER, &linger, sizeof(linger));
	}

	/* diamo il tempo ai thread zmq di fare cio' che devono */
	gui_sleep(10);

	if (zmq_bind(zmq->socket, "tcp://*:5555") == -1) {
		info("zmq_bind %s", zmq_strerror(errno));
		goto net_server_error;
	}

	/* diamo il tempo ai thread zmq di fare cio' che devono */
	gui_sleep(10);

	zmq->exchange->active = TRUE;
	return (EXIT_OK);

	net_server_error: zmq->error = errno;
	net_close_socket(data);
	return (EXIT_ERROR);
}
uint8_t net_client(void *data, char *ip) {
	_zmq *zmq = data;
	char server_service[61];

	if (zmq->exchange->peer_connected) {
		info("client is alredy connected");
		return (EXIT_ERROR);
	}

	if (ip == NULL) {
		info("ip NULL");
		return (EXIT_ERROR);
	}

	zmq->error = 0;

	if ((zmq->socket = zmq_socket(zmq->context, ZMQ_REQ)) == NULL) {
		info("zmq_socket %s", zmq_strerror(errno));
		goto net_client_error;
	}

	{
		int linger = 0;
		zmq_setsockopt (zmq->socket, ZMQ_LINGER, &linger, sizeof(linger));
	}

	/* diamo il tempo ai thread zmq di fare cio' che devono */
	gui_sleep(10);

	snprintf(server_service, 60, "tcp://%s:5555", ip);

	if (zmq_connect(zmq->socket, server_service) == -1) {
		info("zmq_connect %s", zmq_strerror(errno));
		goto net_client_error;
	}

	/* diamo il tempo ai thread zmq di fare cio' che devono */
	gui_sleep(10);
	return (EXIT_OK);

	net_client_error: zmq->error = errno;
	net_close_socket(data);
	return (EXIT_ERROR);
}

void net_server_thread(void *data) {
	_zmq *zmq = data;
	uint32_t repeat = 0;
	net_req request = 0;

#define net_server_thread_control_error\
	switch(zmq->error) {\
		case EAGAIN:\
			goto net_server_thread_step;\
		case EFSM:\
		case ENOTSUP:\
		case ENOTSOCK:\
		case EINTR:\
		case EFAULT:\
		case ETERM:\
			info("zmq_recv %s", zmq_strerror(errno));\
			goto net_server_thread_stop;\
	}
#define net_server_reset_window()\
	netplay_disab_widget(ID_SERVER_CONNECT);\
	netplay_disab_widget(ID_SERVER_DISCONNECT);\
	netplay_disab_widget(ID_SERVER_PORT);\
	netplay_disab_widget(ID_CHAT_FRAME);\
	netplay_enab_widget(ID_NICKNAME)
#define net_server_wait_window()\
	netplay_enab_widget(ID_SERVER_CONNECT);\
	netplay_enab_widget(ID_SERVER_DISCONNECT);\
	netplay_disab_widget(ID_NICKNAME)
#define net_server_accept_client_window()\
	netplay_disab_widget(ID_SERVER_CONNECT);\
	netplay_enab_widget(ID_SERVER_DISCONNECT);\
	netplay_enab_widget(ID_CHAT_FRAME);\
	netplay_enab_widget(ID_NICKNAME)

	if (net_server(data) != EXIT_OK) {
		goto net_server_thread_stop;
	}

	while (TRUE) {
		char *pointer;
		size_t size;
		zmq_msg_t in;

		/*
		 * per poter uscire dal thread devono essere vere queste tre condizioni:
		 * 1) zmq->exchange->want_quit = TRUE
		 * 2) zmq->exchange->want_disconnect = FALSE
		 * 3) zmq->exchange->peer_connected = PEER_NO_CONNECTED.
		 */
		if (zmq->exchange->want_quit && !zmq->exchange->peer_connected) {
			goto net_server_thread_stop;
		}

		zmq->error = 0;

		if (zmq_msg_init(&in) == -1) {
			info("zmq_msg_init %s", zmq_strerror(errno));
			zmq->error = errno;
			goto net_server_thread_stop;
		}

		if (zmq_recv(zmq->socket, &in, ZMQ_NOBLOCK) == -1) {
			zmq->error = errno;
			net_server_thread_control_error
			goto net_server_thread_step;
		}

		pointer = (char *) zmq_msg_data(&in);
		size = zmq_msg_size(&in);

		if (!repeat) {
			memcpy(&request, pointer, SIZE_REQUEST);
			pointer += SIZE_REQUEST;
			size -= SIZE_REQUEST;
		}

		/* ricevo la richiesta */
		switch(request) {
			case NET_REQUEST_CONNECTION: {
				_peer_info *pinfo;

				net_server_wait_window();

				pinfo = (void *) pointer;

				zmq->exchange->peer_info = (*pinfo);

				info("request from : %s", zmq->exchange->peer_info.nickname);

				/* se e' gia' attiva una richiesta, rifiuto */
				if (zmq->exchange->peer_connected) {
					zmq->exchange->accept_peer = SERVER_REFUSE;
					info("one connection are alredy active");
				}

				zmq->exchange->peer_connected = PEER_WAIT_TO_REQUEST;

				break;
			}
			case NET_CHAT:
				chat_size(size, pointer);
				break;
			case NET_CHANGENICK:
				strncpy(zmq->exchange->peer_info.nickname, pointer, size);

				zmq->exchange->peer_info.nickname[size] = 0;

				info("connected to : %s", zmq->exchange->peer_info.nickname);

				chat("the client have changed nickname in : %s\n",
						zmq->exchange->peer_info.nickname);

				break;
			case NET_DISCONNECT:
				net_server_reset_window();

				if (strlen(zmq->exchange->peer_info.nickname)) {
					info("connection closed by %s", zmq->exchange->peer_info.nickname);
				} else {
					info("connection closed");
				}

				_net_send(data, NET_RECEIVED, NULL, 0);

				goto net_server_thread_reset;
			case NET_ALIVE:
				/* non devo fare proprio niente */
				break;
		}

		/* trasmetto la risposta */
		if (zmq->exchange->want_disconnect) {
			net_server_reset_window();

			/* se ho una richiesta di disconnessione la trasmetto al client */
			_net_send(data, NET_DISCONNECT, NULL, 0);

			goto net_server_thread_reset;

		} else if (zmq->exchange->accept_peer != SERVER_WAIT) {
			/*
			 * in caso richiesta di connessione da parte del client
			 * trasmetto l'accept o il refuse.
			 */
			switch (zmq->exchange->accept_peer) {
				case SERVER_ACCEPT: {
					_peer_info pinfo;

					strncpy(pinfo.nickname, zmq->exchange->nickname, sizeof(pinfo.nickname));
					strncpy(pinfo.version, VERSION, sizeof(pinfo.version));
					pinfo.cfg = (*cfg);

					if (_net_send(data, NET_SERVER_ACCEPT, (void *) &pinfo, sizeof(_peer_info))
					        == EXIT_OK) {
						net_server_accept_client_window();

						zmq->exchange->peer_connected = PEER_CONNECTED;

						info("connected to : %s", zmq->exchange->peer_info.nickname);
					}

					break;
				}
				case SERVER_REFUSE:
					net_server_reset_window();

					zmq->exchange->peer_connected = PEER_NO_CONNECTED;

					if (_net_send(data, NET_SERVER_REFUSE, NULL, 0) == EXIT_OK) {
						info(" ");
					}

					break;
			}

			zmq->exchange->accept_peer = SERVER_WAIT;

		} else if (zmq->exchange->changed_nickname) {
			/* se ho cambiato il nickname lo trasmetto */
			_net_send(data, NET_CHANGENICK, zmq->exchange->nickname,
					strlen(zmq->exchange->nickname));

			zmq->exchange->changed_nickname = FALSE;

		} else if (zmq->exchange->chat_available) {
			/* se ho un messaggio di chat da trasmettere, trasmetto */
			_net_send(data, NET_CHAT, zmq->exchange->chat, strlen(zmq->exchange->chat));

			zmq->exchange->chat_available = FALSE;

		} else {
			/* di default trasmetto un NET_RECEIVED */
			if (_net_send(data, NET_RECEIVED, NULL, 0) != EXIT_OK) {
				net_server_thread_control_error
			}
		}

		{
			int64_t more;
			size_t more_size = sizeof(more);

			zmq_getsockopt(zmq->socket, ZMQ_RCVMORE, &more, &more_size);

			if (more) {
				repeat++;
			} else {
				repeat = 0;
			}
		}

		net_server_thread_step: zmq_msg_close(&in);
		gui_sleep(1);
		continue;

		net_server_thread_reset: zmq->exchange->want_disconnect = TRUE;
		net_close_socket(data);
		net_server(data);
		zmq->exchange->want_disconnect = FALSE;
		continue;

		net_server_thread_stop: zmq_msg_close(&in);
		net_close_socket(data);
		zmq->exchange->want_disconnect = FALSE;
		zmq->exchange->want_quit = FALSE;
		zmq->exchange->active = FALSE;
		break;
	}

	exit_thread(NULL);
}
void net_client_thread(void *data) {
	_zmq *zmq = data;

	/*
	 * a differenza della modalita' server, zmq->exchange->active
	 * devo impostarlo a TRUE qui, all'inizio del thread perche'
	 * altrimenti potrei trovarmi nella situazione in cui, non avendo
	 * premuto connect e premendo close, non uscirei dal thread.
	 */
	zmq->exchange->active = TRUE;

	while (TRUE) {
		/*
		 * se non e' piu' attiva una connessione posso uscire
		 * immediatamente dal thread.
		 */
		if (zmq->exchange->want_quit && !zmq->exchange->peer_connected) {
			goto net_client_thread_stop;
		}

		gui_sleep(500);

		if (!zmq->exchange->peer_connected) {
			continue;
		}

		//if (!zmq->exchange->sync) {
		//	continue;
		//}

		if (!zmq->in_send) {
			net_client_send(data, NET_ALIVE, NULL, 0);
		}
		continue;

		net_client_thread_stop: zmq->exchange->want_disconnect = FALSE;
		zmq->exchange->want_quit = FALSE;
		zmq->exchange->active = FALSE;
		break;
	}

	exit_thread(NULL);
}

void net_client_send(void *data, net_req request, char *buffer, uint32_t size) {
	_zmq *zmq = data;

#define net_client_reset_window_no_clean_info()\
	netplay_enab_widget(ID_CLIENT_IP);\
	netplay_enab_widget(ID_CLIENT_CONNECT);\
	netplay_disab_widget(ID_CLIENT_DISCONNECT);\
	netplay_disab_widget(ID_CHAT_FRAME)
#define net_client_reset_window()\
	net_client_reset_window_no_clean_info();\
	info(" ")
#define net_client_wait_window()\
	netplay_enab_widget(ID_CLIENT_DISCONNECT);\
	netplay_disab_widget(ID_CLIENT_CONNECT);\
	netplay_disab_widget(ID_CLIENT_IP);\
	netplay_disab_widget(ID_NICKNAME)
#define net_client_accepted_window()\
	netplay_enab_widget(ID_NICKNAME);\
	netplay_enab_widget(ID_CHAT_FRAME)

	/*  se sto gia' trasmettendo un messaggio aspetto */
	while (zmq->in_send) {
		gui_sleep(1);
	}

	zmq->in_send = TRUE;

	switch(request) {
		case NET_REQUEST_CONNECTION: {
			_peer_info pinfo;

			net_client_wait_window();

			if (zmq->exchange->peer_connected != PEER_NO_CONNECTED) {
				info("client is alredy connected with a server");
				goto net_client_send_exit;
			}

			strncpy(pinfo.nickname, zmq->exchange->nickname, sizeof(pinfo.nickname));
			strncpy(pinfo.version, VERSION, sizeof(pinfo.version));
			pinfo.cfg = (*cfg);

			buffer = (void *) &pinfo;

			size = sizeof(_peer_info);

			info(" ");

			break;
		}
		default:
			if (zmq->exchange->peer_connected == PEER_NO_CONNECTED) {
				info("client is not connected to server");

				if (zmq->exchange->want_disconnect) {
					goto net_client_send_stop;
				}

				goto net_client_send_exit;
			} else if (zmq->exchange->want_disconnect) {
				net_client_reset_window();

				request = NET_DISCONNECT;
				buffer = NULL;
				size = 0;

				break;
			} else if (zmq->exchange->sync) {
				switch (zmq->exchange->sync) {
					case SYNC_REQUEST: {
						//char *version[10];

						request = NET_SYNC_REQUEST;

						//strcpy(version, VERSION, sizeof(VERSION));
						//zmq->exchange->peer_sync_info.cfg = cfg_from_file;

						//buffer = &zmq->exchange->peer_sync_info;
						//size = sizeof(_sync_info);

						//buffer = &zmq->exchange->peer_sync_info;
						//size = sizeof(_sync_info);

						break;
					}
				}

			}
			break;
	}

	if (_net_send(data, request, buffer, size) != EXIT_OK) {
		goto net_client_send_stop;
	}

	{
		char *pointer;
		size_t size;
		zmq_msg_t in;
		net_req response = 0;

		if (_net_recv(data, &response, &in) != EXIT_OK) {
			goto net_client_send_stop;
		}

		pointer = zmq_msg_data(&in) + SIZE_REQUEST;
		size = zmq_msg_size(&in) - SIZE_REQUEST;

		/* se ricevo una risposta vuol dire che e' avvenuto un collegamento */
		if (!zmq->exchange->peer_connected) {
			zmq->exchange->peer_connected = PEER_WAIT_TO_REQUEST;
		}

		switch(response) {
			case NET_SERVER_ACCEPT: {
				_peer_info *pinfo;

				net_client_accepted_window();

				pinfo = (void *) pointer;

				zmq->exchange->peer_info = (*pinfo);

				info("connected to : %s", zmq->exchange->peer_info.nickname);

				zmq->exchange->peer_connected = PEER_CONNECTED;

				//zmq->exchange->sync = SYNC_REQUEST;

				break;
			}
			case NET_SERVER_REFUSE:
				net_client_reset_window();

				info("the server rejects the connection");

				goto net_client_send_stop;
				break;
			case NET_CHAT:
				chat_size(size, pointer);
				break;
			case NET_CHANGENICK:
				strncpy(zmq->exchange->peer_info.nickname, pointer, size);
				zmq->exchange->peer_info.nickname[size] = 0;
				info("connected to : %s", zmq->exchange->peer_info.nickname);
				chat("the server have changed nickname in : %s\n",
						zmq->exchange->peer_info.nickname);
				break;
			case NET_SYNC_REQUEST_RESPONSE:
				break;
			case NET_DISCONNECT:
				net_client_reset_window();

				if (strlen(zmq->exchange->peer_info.nickname)) {
					info("connection closed by %s", zmq->exchange->peer_info.nickname);
				} else {
					info("connection closed");
				}

				goto net_client_send_stop;
				break;
		}

		if (zmq->exchange->want_disconnect) {
			goto net_client_send_stop;
		}

		zmq_msg_close(&in);
		goto net_client_send_exit;

		net_client_send_stop: net_client_reset_window_no_clean_info();
		zmq_msg_close(&in);
		net_close_socket(data);
		zmq->exchange->want_disconnect = FALSE;
		zmq->exchange->peer_connected = PEER_NO_CONNECTED;
		goto net_client_send_exit;
	}

	net_client_send_exit:
	zmq->in_send = FALSE;
	return;
}

uint8_t _net_send(void *data, net_req request, char *buffer, uint32_t size) {
	_zmq *zmq = data;
	zmq_msg_t out;
	uint32_t repeat = 0;

	zmq->error = 0;

	while (TRUE) {
		uint8_t flags;
		uint32_t size_msg;
		char *pointer_out;

		size_msg = size;
		flags = 0;

		if (size > NET_MAX_MSG_SIZE) {
			size_msg = NET_MAX_MSG_SIZE;
			flags = ZMQ_SNDMORE;
		}

		if (!repeat) {
			size_msg += SIZE_REQUEST;
		}

		if (zmq_msg_init_size(&out, size_msg) == -1) {
			info("zmq_msg_init_size %s", zmq_strerror(errno));
			goto _net_send_error;
		}

		pointer_out = zmq_msg_data(&out);

		if (!repeat) {
			memcpy(pointer_out, &request, SIZE_REQUEST);
			pointer_out += SIZE_REQUEST;
		}

		if (buffer != NULL) {
			memcpy(pointer_out, buffer, size);
		}

		if (zmq_send(zmq->socket, &out, flags) == -1) {
			info("zmq_send %s", zmq_strerror(errno));
			goto _net_send_error;
		}

		zmq_msg_close(&out);

		if (!repeat) {
			size_msg -= SIZE_REQUEST;
		}

		if ((size -= size_msg) <= 0) {
			break;
		}

		buffer += size_msg;
		repeat++;
	}

	return (EXIT_OK);

	_net_send_error: zmq->error = errno;
	zmq_msg_close(&out);
	return (EXIT_ERROR);
}
uint8_t _net_recv(void *data, net_req *request, void *buffer) {
	_zmq *zmq = data;
	zmq_msg_t *in = buffer;
	int counter = 0;

	zmq->error = 0;

	if (zmq_msg_init(in) == -1) {
		info("zmq_msg_init %s", zmq_strerror(errno));
		goto _net_receive_error;
	}

	/* rimane bloccato fino a quando non riceve un messaggio */
	while(TRUE) {
		if (zmq_recv(zmq->socket, in, ZMQ_NOBLOCK) == -1) {
			switch(errno) {
				case EAGAIN:
					if ((counter += 5) > 500) {
						info("the server is not responding");
						goto _net_receive_error;
					}
					gui_sleep(5);
					continue;
				case EFSM:
				case ENOTSUP:
				case ENOTSOCK:
				case EINTR:
				case EFAULT:
				case ETERM:
					info("zmq_recv %s", zmq_strerror(errno));
					goto _net_receive_error;
			}
		}
		break;
	}

	if (request != NULL) {
		memcpy(request, zmq_msg_data(in), SIZE_REQUEST);
	}
	return (EXIT_OK);

	_net_receive_error: zmq->error = errno;
	return (EXIT_ERROR);
}

void net_close_socket(void *data) {
	_zmq *zmq = data;

	if (zmq->socket != NULL) {
		if (zmq_close(zmq->socket) == -1) {
			info("zmq_close %s", zmq_strerror(errno));
		}
	}

	zmq->socket = NULL;

	/* aspetto qualche millisecondo che l'operazione venga conclusa */
	gui_sleep(5);
}
void net_close(void *data) {
	_zmq *zmq = data;

	if (zmq != NULL) {
		/*
		 * sia in modalita' server che client, il rispettivo thread e' attivo
		 * finche' zmq->exchange->active e' a TRUE. Avvisando che voglio
		 * disconnettermi inviero' il messaggio NET_DISCONNECT al peer
		 * al prossimo scambio di informazioni quindi, chiudero' il socket.
		 * Con zmq->exchange->want_quit avviso che voglio uscire dal thread.
		 */
		zmq->exchange->want_disconnect = TRUE;
		zmq->exchange->want_quit = TRUE;

		while (zmq->exchange->active && zmq->exchange->want_quit) {
			gui_sleep(5);
		}

		/*
		 * in entrambe le modalita', a questo punto il socket
		 * doverebbe essere gia' chiuso. Questo controllo lo
		 * faccio solo perche' sono un pignolo.
		 */
		if ((zmq->socket != NULL)) {
			net_close_socket(data);
			gui_sleep(5);
		}

		if (zmq->context) {
			zmq_term(zmq->context);
		}

		zmq->exchange->active = FALSE;
		zmq->exchange->peer_connected = PEER_NO_CONNECTED;
		zmq->exchange->want_disconnect = FALSE;
		zmq->exchange->want_quit = FALSE;

		gui_sleep(5);

		free(zmq);
	}
}

void net_create_random_name(char *dst, const int len) {
	int i;

	for (i = 0; i < len; ++i) {
		int randomChar = rand() % (26 + 26 + 10);
		if (randomChar < 26) {
			dst[i] = 'a' + randomChar;
		} else if (randomChar < 26 + 26) {
			dst[i] = 'A' + randomChar - 26;
		} else {
			dst[i] = '0' + randomChar - 26 - 26;
		}
	}
	dst[len] = 0;
}
