/*
 * gtk2.h
 *
 *  Created on: 21/lug/2011
 *      Author: fhorse
 */

#ifndef NETPLAY_H_
#define NETPLAY_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define netplay_enab_widget(wdg) netplay_enable_widget(wdg, TRUE)
#define netplay_disab_widget(wdg) netplay_enable_widget(wdg, FALSE)

enum { INFO, CHAT };
enum {
	NETPLAY,
	ID_NICKNAME,
	ID_SERVER_ENABLE,
	ID_CLIENT_ENABLE,
	ID_SERVER_FRAME,
	ID_SERVER_IP,
	ID_SERVER_PORT,
	ID_SERVER_CONNECT,
	ID_SERVER_DISCONNECT,
	ID_SERVER_INFO,
	ID_CLIENT_FRAME,
	ID_CLIENT_IP,
	ID_CLIENT_PORT,
	ID_CLIENT_CONNECT,
	ID_CLIENT_DISCONNECT,
	ID_CLIENT_INFO,
	ID_CHAT_FRAME,
	ID_CHAT_ENTRY,
	ID_CHAT_WINDOW,
	WINDGETS
};

#endif /* NETPLAY_H_ */

void netplay_init(void);
void netplay_create(void);
void netplay_display_message(uint8_t mode, uint32_t size, const char *fmt, ...);
void netplay_enable_widget(uint32_t widget, uint8_t mode);


