#ifndef _NTTP_CONNECT_H
#define _NTTP_CONNECT_H

void *nttp_connection(void *arg);
int nttp_get_status_code(char *buffer);
int nttp_handle_retrieve_error(struct connection_thread *ct,
                               queue_item_t *queue_item);
void nttp_process_queue(struct connection_thread *ct);
void nttp_handle_segment(struct connection_thread *ct,
                         queue_item_t * queue_item);
#endif
