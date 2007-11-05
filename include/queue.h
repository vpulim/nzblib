#ifndef _QUEUE_H
#define _QUEUE_H

#include "types.h"

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