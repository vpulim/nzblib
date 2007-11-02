/*
 * Copyright (c) 2004-2007 Michael van Tellingen.  All rights reserved.
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
#include <stdio.h>
#include <assert.h>

#include "net.h"
#include "nttp_connection.h"
#include "types.h"
#include "queue.h"
#include "parse_nzb.h"
#include "nzb_fetch.h"
#include "process.h"

nzb_fetch *nzb_fetch_init()
{
    nzb_fetch *fetcher;
    
    fetcher = malloc(sizeof(nzb_fetch));
    
    fetcher->queue = queue_list_create();
    fetcher->data_queue = queue_list_create();
    
    fetcher->servers = NULL;
    
    pthread_create( &fetcher->process_thread_id,
                    NULL,
                    &process_data_queue,
                    (void *)fetcher
                   );
            
    return fetcher;
}

int nzb_fetch_add_server(nzb_fetch *fetcher, char *address, int port,
                         char *username, char *password, int threads,
                         int priority)
{
    server_t *new_server, *server;

    new_server = malloc(sizeof(server_t));
    
    new_server->address = strdup(address);
    new_server->port = port;
    new_server->username = strdup(username);
    new_server->password = strdup(password);
    new_server->num_threads = threads;
    new_server->priority = priority;
    new_server->prev = NULL;
    new_server->next = NULL;
    new_server->queue = NULL;
    
    // Insert into server linked list
    server = fetcher->servers;
    
    if(fetcher->servers == NULL)
        fetcher->servers = new_server;
    else
    {
        // Append server at the end
        while(server->next != NULL)
        {
            server = server->next;
        }
        server->next = new_server;
        new_server->prev = server;
        
    }
    
    return 0;
}

int nzb_fetch_connect(nzb_fetch *fetcher)
{
    server_t *server;
    int connections = 0;
    int i;
    
    for (server = fetcher->servers; server != NULL; server = server->next)
    {
        if (net_prepare_connection(server) != 0)
        {
            printf("Unable to resolve hostname %s\n", server->address);
            continue;
        }

        server->threads = malloc(sizeof(struct connection_thread) *
                                 server->num_threads);
        
        // Create a thread for each connection, see nttp_connection.c
        for(i = 0; i < server->num_threads; i++)
        {
            server->threads[i].thread_num = i + connections;
            server->threads[i].server = server;
            
            server->threads[i].queues = malloc(sizeof(queue_list_t *));
            server->threads[i].queues[0] = fetcher->queue;
            server->threads[i].data_queue = fetcher->data_queue;
            
            pthread_create( &server->threads[i].thread_id,
                            NULL,
                            &nttp_connection,
                            (void *)&server->threads[i]
                           );
        }
        
        connections += server->num_threads;
        
    }
    
    return 0;
}

int nzb_fetch_storage_path(nzb_file *file, char *path)
{
    file->storage_path = strdup(path);
    return 0;
}

int nzb_fetch_temporary_path(nzb_file *file, char *path)
{
    file->temporary_path = strdup(path);
    
    return 0;
}

nzb_file *nzb_fetch_parse(char *filename)
{
    nzb_file *file;
    file = malloc(sizeof(nzb_file));
    file->posts = parse_nzb(filename);
    file->filename = filename;
    
    return file;
}

int nzb_fetch_download(nzb_fetch *fetcher, nzb_file *file)
{
    post_t *post_item;
    queue_item_t  *queue_item;
    
    printf("Parsing %s\n", file->filename);
    
    printf("Adding post to queue\n");
    
    
    fetcher->file = file;
    
    
    // First add all the first segments of each post
    for(post_item = file->posts; post_item != NULL; post_item = post_item->next)
    {
        printf("msgid: %s\n", post_item->segments[0]->messageid);
        queue_item = queue_item_create(post_item->segments[0]);
        queue_list_append(fetcher->queue, queue_item);
    }
    printf("Done\n");
    
    // Then add the rest
    // TODO
    
    return 0;
}



