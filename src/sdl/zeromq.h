/*
 * zeromq.h
 *
 *  Created on: 01/dic/2011
 *      Author: fhorse
 */

#ifndef ZEROMQ_H_
#define ZEROMQ_H_

#include "common.h"
#include "cfg_file.h"

typedef uint8_t net_req;

#define NET_MAX_MSG_SIZE 256
#define NET_MAX_NAME_SIZE 20
#define NET_MAX_VERSION_SIZE 30
#define SIZE_REQUEST sizeof(net_req)

enum { SERVER, CLIENT };
enum protocol {
	NET_REQUEST_CONNECTION = 1,
	NET_SERVER_ACCEPT,
	NET_SERVER_REFUSE,
	NET_ALIVE,
	NET_RECEIVED,
	NET_CHAT,
	NET_CHANGENICK,
	NET_DISCONNECT,
	NET_SYNC_REQUEST,
	NET_SYNC_REQUEST_RESPONSE,
};
enum server {
	SERVER_ACCEPT,
	SERVER_REFUSE,
	SERVER_WAIT,
	SERVER_WANT_DISCONNECT
};
enum peer {
	PEER_NO_CONNECTED,
	PEER_WAIT_TO_REQUEST,
	PEER_CONNECTED
};
enum sync {
	SYNC_REQUEST = 1,
};

typedef struct {
	char version[NET_MAX_VERSION_SIZE];
	char nickname[NET_MAX_NAME_SIZE];
	_config cfg;
} _peer_info;
typedef struct {
	uint8_t active;
	uint8_t rule;
	uint8_t peer_connected;
	uint8_t accept_peer;
	uint8_t want_disconnect;
	uint8_t want_quit;
	uint8_t chat_available;
	uint8_t sync;
	uint8_t changed_nickname;
	char chat[NET_MAX_MSG_SIZE + 1];
	char nickname[NET_MAX_NAME_SIZE];
	_peer_info peer_info;
} _exchange_info;

void *net_init(_exchange_info *gui);

uint8_t net_server(void *data);
uint8_t net_client(void *data, char *ip);

void net_server_thread(void *data);
void net_client_thread(void *data);

void net_client_send(void *data, net_req request, char *buffer, uint32_t size);

uint8_t _net_send(void *data, net_req request, char *buffer, uint32_t size);
uint8_t _net_recv(void *data, net_req *request, void *buffer);

void net_close(void *data);

void net_create_random_name(char *dst, const int len);

#endif /* ZEROMQ_H_ */
