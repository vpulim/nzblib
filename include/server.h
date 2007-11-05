#ifndef _SERVER_H
#define _SERVER_H

#include "types.h"
#include "nzb_fetch.h"

void swap(int *a, int *b);
void server_sort(server_t * arr[], int beg, int end);
int server_calculate_priorities(nzb_fetch *fetcher);
server_t *server_create(char *address, int port, char *username,
                        char *password, int threads, int priority);

#endif