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

#if !HAVE_REALLOCF
#   include "compat/reallocf.h"
#endif

#ifdef WIN32
#   include <windows.h>
#   include <process.h>
#   include "compat/win32.h"
#else
#   include <unistd.h>

#endif

#include "server.h"
#include "net.h"
#include "nttp_connection.h"
#include "post.h"
#include "queue.h"
#include "file.h"
#include "parse_nzb.h"
#include "nzb_fetch.h"
#include "process.h"



static char const rcsid[] = "$Id: Copyright (c) 2004-2007 Michael van Tellingen.  All rights reserved.$";
/*!
 * Initialize nzb_fetch
 */
nzb_fetch *nzb_fetch_init()
{
    nzb_fetch *fetcher;
#ifdef WIN32
	WSADATA wsaData;
#endif

    fetcher = malloc(sizeof(nzb_fetch));
    
    fetcher->queue = queue_list_create();
    fetcher->queue->id = strdup("main queue");
    
    fetcher->data_queue = queue_list_create();
    
    
    fetcher->data_queue->id = strdup("data queue");
    
    fetcher->priority_queues = malloc(sizeof(queue_list_t *));
    fetcher->priority_queues[0] = fetcher->queue;
    
    fetcher->servers = NULL;

#if HAVE_LIBSSL 
    net_ssl_init();
#endif

#ifdef WIN32

    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        printf("WSAStartup failed: %d\n", ret);
        return NULL;
    }
#endif

#ifdef WIN32
    _beginthread(&process_data_queue,
                             0,
                             (void *)fetcher);
#else
    pthread_create( &fetcher->process_thread_id,
                    NULL,
                    &process_data_queue,
                    (void *)fetcher
                   );
#endif
				 
    return fetcher;
}

int nzb_fetch_add_server(nzb_fetch *fetcher, char *address, int port,
                         char *username, char *password, int threads,
                         int ssl, int priority)
{
    server_t *new_server, *server;
    int required_queues;
    int current_queues;
    
    if (priority < 0)
    {
        fprintf(stderr, "Error: priority should be > 0\n");
        return -1;
    }

#if !HAVE_LIBSSL
    if (ssl > 0)
    {
        fprintf(stderr, "Error: SSL Support is not compiled in\n");
        return -1;
    }
#endif

    new_server = server_create(address, port, username, password,
                               threads, ssl, priority);
    
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
    
    required_queues = server_calc_priorities(fetcher);
    
    if (required_queues > 1)
    {
        current_queues = sizeof(fetcher->priority_queues) /
                            sizeof(queue_list_t *);
        
        if (required_queues > current_queues)
        {
            fetcher->priority_queues = reallocf(fetcher->priority_queues,
                                                sizeof(queue_list_t *) *
                                                required_queues);
            
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
	    printf("Creating connection to %s\n", server->address);
            server->threads[i].thread_num = i + connections;
            server->threads[i].server = server;
            server->threads[i].connected = 0;
            server->threads[i].queues = fetcher->priority_queues;
            server->threads[i].data_queue = fetcher->data_queue;
            //gettimeofday(&server->threads[i].prev_time, NULL);
#ifdef WIN32
            _beginthread(&nttp_connection,
                                     0,
                                     (void *)&server->threads[i]
            );
#else
            pthread_create( &server->threads[i].thread_id,
                            NULL,
                            &nttp_connection,
                            (void *)&server->threads[i]
                           );
#endif
        }
      
        connections += server->num_threads;
        
    }
    
    return 0;
}

int nzb_fetch_storage_path(nzb_file *file, char *path)
{
    file->storage_path = file_get_path(path);

    return access(file->storage_path, 0);
}

int nzb_fetch_temporary_path(nzb_file *file, char *path)
{
    file->temporary_path = file_get_path(path);
	
    return access(file->temporary_path, 0);
}

nzb_file *nzb_fetch_parse(char *filename)
{
    nzb_file *file;
    file = malloc(sizeof(nzb_file));
    file->posts = parse_nzb(filename);
    file->filename = filename;
    
    return file;
}


int nzb_fetch_add_callback(nzb_fetch *fetcher, int type, void (*file_complete)(struct nzb_file_info_s*))
{
    fetcher->callback_file_complete = file_complete;
	return 0;
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
        
        
        post->client_data = (*files)[i];
    }
    

        
    return num_posts;
}

int nzb_fetch_list_connections(nzb_fetch *fetcher, nzb_connections ***connections)
{
    int num_connections;
    int i = 1, j = 0;
    server_t *server;

    // Free previous data
/*    if (*connections != NULL)
    {
        num_connections = (sizeof(*connections) / sizeof(nzb_connections *));
        for(i = 0; i < num_connections; i++)
            free((*connections)[i]);
        free(*connections);
    }   */ 
    num_connections = 0;
    for(server = fetcher->servers; server != NULL; server = server->next)
        num_connections+= server->num_threads;

    *connections = malloc(sizeof(nzb_connections *) * num_connections);
    
    for(server = fetcher->servers; server != NULL; server = server->next)
    {
        for(i = 0; i < server->num_threads; i++, j++)
        {
            (*connections)[j] = malloc(sizeof(nzb_connections));
            (*connections)[j]->address = server->address;
            (*connections)[j]->transfer_rate = server_calc_transfer_rate(
                                                    &server->threads[i]
                                                );
        }
    }
    
    
    return num_connections;
            
}

int nzb_fetch_download(nzb_fetch *fetcher, nzb_file_info *filelist)
{
    post_t *post = filelist->post;
    queue_item_t  *queue_item;
    int i;
    
    fetcher->file = filelist->file;
    
    //printf("Putting file %s on the queue\n", filelist->filename);
    //printf("Paths:\n Complete files: %s\n Temporary files: %s\n",
    //       fetcher->file->storage_path, fetcher->file->temporary_path);
    //
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

int nzb_fetch_file_complete(nzb_fetch *fetcher, post_t *post)
{
    (*fetcher->callback_file_complete)((nzb_file_info *)post->client_data);
	return 0;
}
