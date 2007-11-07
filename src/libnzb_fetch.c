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
#include "post.h"
#include "queue.h"
#include "file.h"
#include "parse_nzb.h"
#include "nzb_fetch.h"
#include "process.h"
#include "server.h"


/*!
 * Initialize nzb_fetch
 */
nzb_fetch *nzb_fetch_init()
{
    nzb_fetch *fetcher;
    
    fetcher = malloc(sizeof(nzb_fetch));
    
    fetcher->queue = queue_list_create();
    fetcher->queue->id = strdup("main queue");
    
    fetcher->data_queue = queue_list_create();
    
    
    fetcher->data_queue->id = strdup("data queue");
    
    fetcher->priority_queues = malloc(sizeof(queue_list_t *));
    fetcher->priority_queues[0] = fetcher->queue;
    
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
    int required_queues;
    int current_queues;
    
    if (priority < 0)
    {
        printf("Error: priority should be > 0\n");
        return -1;
    }

    

    new_server = server_create(address, port, username, password,
                               threads, priority);
    
    
    // Insert into server linked list
    server = fetcher->servers;
    
    if(server == NULL)
        fetcher->servers = new_server;
    else
    {
        // Append server at the end
        while(server->next != NULL)
            server = server->next;
        
        server->next = new_server;
        new_server->prev = server;
        
    }
    
    required_queues = server_calculate_priorities(fetcher);
    
    if (required_queues > 1)
    {
        printf("Require an extra priority queues\n");
        
        current_queues = sizeof(fetcher->priority_queues) /
                            sizeof(queue_list_t *);
        
        printf("Required queues: %d Current queues %d\n", required_queues,
               current_queues);
        
        if (required_queues > current_queues)
        {
            printf("Creating extra priority queues: %d\n", required_queues);
            fetcher->priority_queues = reallocf(fetcher->priority_queues,
                                                sizeof(queue_list_t *) *
                                                required_queues);
            
            printf("%d\n", new_server->priority);
            fetcher->priority_queues[new_server->priority] = queue_list_create();
            fetcher->priority_queues[new_server->priority]->id = strdup("priority queue");
        }   
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
            
            server->threads[i].queues = fetcher->priority_queues;
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


int nzb_fetch_list_files(nzb_file *file, nzb_file_info ***files)
{
    post_t *post;
    int num_posts = 0;
    int i = 0;
    
    // First add all the first segments of each post
    for(post = file->posts; post != NULL; post = post->next)
        num_posts++;
    
    *files = malloc(sizeof(nzb_file_info *) * num_posts);
    
    for(post = file->posts, i = 0; post != NULL; post = post->next, i++)
    {
        (*files)[i] = malloc(sizeof(nzb_file_info));
        (*files)[i]->filename = strdup(post->filename);
        (*files)[i]->post = post;
        (*files)[i]->file = file;
    }
    

        
    return num_posts;
}

int nzb_fetch_download(nzb_fetch *fetcher, nzb_file_info *filelist)
{
    post_t *post = filelist->post;
    queue_item_t  *queue_item;
    int i;
    
    fetcher->file = filelist->file;
    
    printf("Putting file %s on the queue\n", filelist->filename);
    for (i = 0; i < post->num_segments; i++)
    {
        if (file_chunk_exists(post->segments[i], filelist->file))
        {
            segment_status_set(post->segments[i], SEGMENT_COMPLETE);
            printf("Segment is complete\n");
            continue;
        }
        queue_item = queue_item_create(post->segments[i]);
        queue_list_append(fetcher->queue, queue_item);
    }
 
    return 0;
}



