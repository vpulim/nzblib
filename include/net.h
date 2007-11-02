#ifndef _NET_H
#define _NET_H

#include "types.h"

int net_prepare_connection(server_t *server);
int net_connect(struct sockaddr_in * addr);
char *net_recv(int sock);
int net_send(int sock, char *format, ...);
void net_disconnect(int sock);

#endif
