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

#ifndef _QUEUE_H
#define _QUEUE_H

#include "post.h"
#include "server.h"


#include <pthread.h>

/*!
 * A structure which contains a reference to a segment. This item is referenced
 * in the queue_list type
 */
typedef struct queue_item_s
{
    struct segment_s *segment;        //!< Reference to segment
    struct server_s **failed_servers; //!< Array with servers which failed
    int num_failed_servers;           //!< Number of failed servers
    struct queue_item_s *next;        //!< Reference to the next queue_item
    struct queue_item_s *prev;        //!< Reference to the previous queue_item
} queue_item_t;

/*!
 * A queue with queue_item's.
 */
typedef struct queue_list_s
{
    pthread_mutex_t mtx_queue;
    pthread_mutex_t mtx_data;
    pthread_mutex_t mtx_cond;

    pthread_cond_t cond_item;       // Protected by mtx_cond
    
    
    struct queue_item_s *last;
    struct queue_item_s *first;
    
} queue_list_t;


queue_item_t * queue_item_create();
void queue_item_set_failed(queue_item_t *queue_item, struct server_s *server);
int queue_item_is_failed(queue_item_t *queue_item, struct server_s *server);
queue_list_t * queue_list_create();
queue_item_t * queue_list_shift(queue_list_t  *queue_list, struct server_s *server);
queue_item_t * queue_list_pop(queue_list_t  *queue_list, struct server_s *server);
void queue_list_prepend(queue_list_t *queue_list, queue_item_t *queue_item);
void queue_list_append(queue_list_t *queue_list, queue_item_t *queue_item);
void queue_list_sleep(queue_list_t *queue);
void queue_item_move(queue_list_t *queue, queue_item_t *queue_item);
void queue_item_destroy(queue_item_t *queue_item);
#endif