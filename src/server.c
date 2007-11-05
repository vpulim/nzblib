/*
 * Copyright (c) 2007 Michael van Tellingen.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *    
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "nzb_fetch.h"

/*!
 * Swap two pointer values
 */
void swap(int *a, int *b)
{
    int t;
    
    t = *a;
    *a = *b;
    *b = t;
}

/*!
 * Sort the servers on user_priority
 */
void server_sort(server_t * arr[], int beg, int end)
{
    int piv, l, r;

    if (end > beg + 1)
    {
        piv = arr[beg]->user_priority;
        l = beg + 1;
        r = end;
        
        while (l < r)
        {
            if (arr[l]->user_priority <= piv)
                l++;
            else
                swap((int *)&arr[l], (int *)&arr[--r]);
        }
        swap((int *)&arr[--l], (int *)&arr[beg]);
        server_sort(arr, beg, l);
        server_sort(arr, r, end);
    }
}


server_t *server_create(char *address, int port, char *username,
                        char *password, int threads, int priority)
{
    server_t * server;
    
    server = malloc(sizeof(server_t));
    server->address = strdup(address);
    server->port = port;
    server->username = strdup(username);
    server->password = strdup(password);
    server->num_threads = threads;
    server->user_priority = priority;
    server->priority = -1;
    server->prev = NULL;
    server->next = NULL;
    server->queue = NULL;
    
    return server;
}

/*!
 * Calculate the real server priorities based on the user_priorites.
 * Thus priorities 0, 2, 40, 40 become priorities 0, 1, 2, 2
 */
int server_calculate_priorities(nzb_fetch *fetcher)
{
    int num_servers = 0;
    int i;
    int prev_priority;
    int prev_user_priority;
    int unique_priorities = 0;
    
    server_t *server;
    server_t **servers;
    
    // Calculate the number of servers    
    for(server = fetcher->servers; server != NULL; server = server->next)
        num_servers++;
    
    // Create an array of all servers
    servers = malloc(sizeof(server_t *) * num_servers);
    i = 0;
    for(server = fetcher->servers; server != NULL; server = server->next)
        servers[i++] = server;
    
    server_sort(servers, 0, num_servers );
    
    prev_priority = -1;
    prev_user_priority = -1;
    
    for (i = 0; i < num_servers; i++)
    {
        if (servers[i]->user_priority > prev_user_priority)
        {
            servers[i]->priority = prev_priority + 1;
            unique_priorities += 1;
        }
        else
        {
            servers[i]->priority = prev_priority;
        }
        prev_user_priority = servers[i]->user_priority;
        prev_priority = servers[i]->priority;
        
    }
    
    free(servers);
    
    return unique_priorities;
}