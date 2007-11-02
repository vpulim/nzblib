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
#include <assert.h>

#include <stdio.h>

#include "global.h"
#include "types.h"
#include "queue.h"

/*
 * Create a queue item
 */
queue_item_t * queue_item_create(segment_t *segment)
{
    queue_item_t *queue_item;
    
    queue_item = malloc(sizeof(queue_item_t));
    queue_item->segment = segment;
    queue_item->next = NULL;
    queue_item->prev = NULL;
    queue_item->num_failed_servers = 0;
    queue_item->failed_servers = NULL;
    return queue_item;
}

void queue_item_destroy(queue_item_t *queue_item)
{
    types_free_segment(queue_item->segment);
    
    if (queue_item->failed_servers != NULL)
        free(queue_item->failed_servers);
        
    free(queue_item);
}

/*
 * Mark that the segment could not be downloaded from the given server
 */
void queue_item_set_failed(queue_item_t *queue_item, server_t *server)
{
    queue_item->failed_servers = realloc(queue_item->failed_servers,
                                         sizeof(server_t *) *
                                         queue_item->num_failed_servers + 1);
    
    queue_item->failed_servers[queue_item->num_failed_servers] = server;
    
    queue_item->num_failed_servers++;
}


/*
 * Check if the given queue_item has failed on the given server before
 */
int queue_item_is_failed(queue_item_t *queue_item, server_t *server)
{
    int i;
    assert(queue_item != NULL);
    assert(server != NULL);
    
    if (queue_item->num_failed_servers == 0)
        return 0;
    
    // Check if this queue_item failed on this server
    for(i = 0; i < queue_item->num_failed_servers; i++)
        if (queue_item->failed_servers[i] == server)
            return 1;

    // queue_item has failed before, but not on this server
    return 0;
}

void queue_item_move(queue_list_t *queue, queue_item_t *queue_item)
{
    free(queue_item->failed_servers);
    queue_item->num_failed_servers = 0;
}

/*
 * Create a queue list
 */
queue_list_t * queue_list_create()
{
    queue_list_t *queue_list;
    queue_list = malloc(sizeof(queue_list_t));
    pthread_mutex_init(&queue_list->mtx_queue, NULL);
    pthread_mutex_init(&queue_list->mtx_cond, NULL);
    pthread_cond_init(&queue_list->cond_item, NULL);
    
    queue_list->first = NULL;
    queue_list->last = NULL;
    
    return queue_list;
}


void queue_list_sleep(queue_list_t *queue)
{
    MTX_LOCK(&queue->mtx_cond);
    pthread_cond_wait(&queue->cond_item, &queue->mtx_cond);
    MTX_UNLOCK(&queue->mtx_cond);
}

/*
 * Shift an item from the begin of the queue list which is not failed on this server.
 */
queue_item_t * queue_list_shift(queue_list_t  *queue_list, server_t *server)
{
    queue_item_t *queue_item;
    assert(queue_list != NULL);
    
    MTX_LOCK(&queue_list->mtx_queue);
    
    queue_item = queue_list->first;
    
    while (queue_item == NULL)
    {
        MTX_UNLOCK(&queue_list->mtx_queue);
        
        // Sleep until we are woken that we have new data
        queue_list_sleep(queue_list);
        
        // There is new data however another thread might be quicker, thus we
        // loop here to make sure that we really have an item from the queue
        MTX_LOCK(&queue_list->mtx_queue);
        queue_item = queue_list->first;
    }
    
    assert(queue_item != NULL);
    
    
    //if(server != NULL)
    //{
    //    do
    //    {
    //        if(queue_item_is_failed(queue_item, server) == 0)
    //            break;
    //    }
    //    while((queue_item = queue_item->next));
    //}
    
    // Break out the queue item from the linked list
    if(queue_item->prev != NULL)
        queue_item->prev->next = queue_item->next;
    
    if(queue_item->next != NULL)
        queue_item->next->prev = queue_item->prev;
        
    queue_list->first = queue_item->next;
    assert(queue_item != NULL);
    MTX_UNLOCK(&queue_list->mtx_queue);
    return queue_item;    
}


/* THIS FUNCTION IS CURRENTLY BROKEN!!!!
 * Pop an item from end of the queue list which is not failed on this server.
 */
queue_item_t * queue_list_pop(queue_list_t  *queue_list, server_t *server)
{
    assert(0); // THis function is not workig atm
    queue_item_t *queue_item;
    
    MTX_LOCK(&queue_list->mtx_queue);
    queue_item = queue_list->last;

    do
    {
        if(queue_item_is_failed(queue_item, server) == 0)
            break;
    }
    while((queue_item = queue_item->prev));

    // Break out the queue item from the linked list    
    queue_item->prev->next = queue_item->next;
    queue_item->next->prev = queue_item->prev;

    
    MTX_UNLOCK(&queue_list->mtx_queue);
    return queue_item;    
}



/*
 * Insert an item at the end of the queue list
 */
void queue_list_append(queue_list_t *queue_list, queue_item_t *queue_item)
{
    // Clean previous linked list vars
    queue_item->next = NULL;
    queue_item->prev = NULL;
    
    MTX_LOCK(&queue_list->mtx_queue);
    if (queue_list->first == NULL)
        queue_list->first = queue_item;
    
    
    if(queue_list->last)
        queue_list->last->next = queue_item;
    
    queue_item->prev = queue_list->last;
    queue_list->last = queue_item;
    
    MTX_UNLOCK(&queue_list->mtx_queue);
    

    MTX_LOCK(&queue_list->mtx_cond);
    pthread_cond_signal(&queue_list->cond_item);
    MTX_UNLOCK(&queue_list->mtx_cond);
        
}

/*
 * Insert an item at the start of the queue list
 */
void queue_list_prepend(queue_list_t *queue_list, queue_item_t *queue_item)
{
    assert(0); // THis function is not workig atm
    MTX_LOCK(&queue_list->mtx_queue);
    queue_item->next = queue_list->first;
    queue_list->first = queue_item;
    MTX_UNLOCK(&queue_list->mtx_queue);

    MTX_LOCK(&queue_list->mtx_cond); 
    pthread_cond_signal(&queue_list->cond_item);
    MTX_UNLOCK(&queue_list->mtx_cond);
}
