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

#ifndef _SERVER_H
#define _SERVER_H

#ifdef WIN32
#   include <windows.h>
#   include <string.h>
#   include <conio.h>
#   include <process.h>
#   define THREAD_ID int
#else
#   include <netinet/in.h>
#   include <pthread.h>
#   include <sys/time.h>
#   define THREAD_ID pthread_t
#endif

#include "nzb_fetch.h"
#include "queue.h"


/*  Forward declaration, this is defined in nzb_fetch.h but we get included
    from nzb_fetch.h ... */
struct nzb_fetch_s;

typedef struct server_s
{
    char *username;         //!< Username to authenticate with
    char *password;         //!< Password to authenticate with
    
    int priority;           //!< Priority of the server, the lower the better
    int user_priority;      //!< Priority as specified by client
    int num_threads;        //!< Number of connection threads
    
    int ssl;                //!< Enable SSL
    int port;               //!< NTTP server port
    char *address;          //!< NTTP server address
    
    queue_list_t *queue;                /*!< Reference to the queue used for
                                         this server */
    struct server_s *next;              //!< Reference to the next server
    struct server_s *prev;              //!< Referecen to the previous server
    
    struct sockaddr_in server_addr;     //!< sockaddr_in structure
    struct connection_thread *threads;  /*< Array to the connection_threads
                                            used by the connections */
} server_t;

struct connection_thread
{
    int thread_num;
    int ready;

    struct connection_s *connection;    //!< TCP Connection
    
    int prev_recv_bytes;
    struct timeval prev_time;

    struct queue_list_s **queues;     //!< Array of queues with empy segments
    struct queue_list_s *data_queue;  //!< Pointer to queue with full segments
    THREAD_ID thread_id;
    struct server_s *server;
    //postlist_t *postlist;
};

void swap(int *a, int *b);
void server_sort(server_t * arr[], int beg, int end);
int server_calculate_priorities(struct nzb_fetch_s *fetcher);
server_t *server_create(char *address, int port, char *username,
                        char *password, int threads, int ssl, int priority);

#endif