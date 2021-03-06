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

#ifdef WIN32
#   include <process.h>
#   include "compat/reallocf.h"
#   include "compat/win32.h"
#else
#   include <unistd.h>
#endif

#include "post.h"
#include "net.h"
#include "queue.h"
#include "nttp_connection.h"
#include "global.h"
#include "net.h"
#include "yenc.h"



/*!
 * Connect to the NTTP server and check for status code 200 or 201. Return 0
 * if successfull.
 */
int nttp_connect(struct connection_thread *ct)
{
    char *data;
    int status_code;
    int bytes;

    ct->connection = net_connect(&ct->server->server_addr, ct->server->ssl);
    bytes = net_recv(ct->connection, &data);
    if (bytes < 0)
        return -1;

    status_code = nttp_get_status_code(data);
    free(data);


    if (status_code == 200 || status_code == 201)
        return 0;
    
    return -1;
}


/*!
 * Authenticate on the NTTP server, except if the username or password are
 * null.
 *
 * TODO: When username is send and password is required server sends 381.
 * but if no password is required it sends 281. This should be handled better.
 */
int nttp_authenticate(connection_t *conn, char *username, char *password)
{
    char *data;
    int status_code;
    int bytes;
    
    if (username == NULL || password == NULL)
        return 0;

    // Send username
    net_send(conn, "AUTHINFO USER %s\n", username);
    
    bytes = net_recv(conn, &data);
    if (data == NULL)
        return -1;
    
    status_code = nttp_get_status_code(data);
    free(data);
    
    if (status_code != 381)
    {
        fprintf(stderr, ":: Command 'authinfo user' return status code %d\n",
               status_code);
        return -2;
    }
    
    // Send password
    net_send(conn, "AUTHINFO PASS %s\n", password);

    bytes = net_recv(conn, &data);
    if (data == NULL)
        return -1;
    
    status_code = nttp_get_status_code(data);
    free(data);
    
    if (status_code != 281)
    {
        fprintf(stderr, ":: Authorization failed..\n");
        return -1;
    }
    
    return 0;
}

/*!
 * Convert the first three characters in the buffer to an integer. This should
 * be the status code for messages from the NTTP server.
 */
int nttp_get_status_code(char *buffer)
{
    char status_code[4];
    int code;
    
    strncpy(status_code, buffer, 3);
    status_code[3] = '\0';

    code = strtol(status_code, (char **)NULL, 10);
    
    return (int)code;
}


/*!
 * Logout and cleanup the socket.
 */
void nttp_disconnect(connection_t *conn)
{
    net_send(conn, "quit\n");
    
    net_disconnect(conn);
}


/*!
 * Select a newsgroup on the NTTP server.
 * TODO: Check if the group is already selected before issueing the command?
 */
int nttp_select_group(connection_t *conn, char *group)
{
    char *data;
    int status_code;
    
    // Send password
    net_send(conn, "GROUP %s\n", group);

    net_recv(conn, &data);
    
    if (data == NULL)
        return -1;
    
    status_code = nttp_get_status_code(data);
    free(data);
    
    // 411 = no such group
    if (status_code == 211)
        return 0;
    
    return -1;
}


/*!
 * Retrieve an segment (message) from the NTTP server with the messageid
 * stored in the segment structure (segment->message). The resulting data will
 * be saved as segment->data and the number of bytes in segment->bytes
 */
int nttp_retrieve_segment(connection_t *conn, segment_t *segment)
{
    char *data;
    int status_code;
    int bytes = 0;
    net_send(conn, "BODY <%s>\n", segment->messageid);
    
    assert(segment->bytes == 0);
    
    bytes = net_recv(conn, &data);
    if (data == NULL)
        return -1;
    
    status_code = nttp_get_status_code(data);

    if (status_code != 222)
    {
        printf(":: %d\n", status_code);
        free(data);
        return -1;
    }
    
    do
    {
        // Fixme. Realloc might fail
        segment->data = reallocf(segment->data,
                                 segment->bytes + bytes + 1);

        memcpy(segment->data + segment->bytes, data, bytes);
        
        free(data);
        segment->bytes += bytes;
        
        // Check if we have recevied all bytes
        if (segment->data[segment->bytes - 1] == '\n' &&
            segment->data[segment->bytes - 2] == '\r' &&
            segment->data[segment->bytes - 3] == '.'  &&
            segment->data[segment->bytes - 4] == '\n')
        {
            break;
        }
        
        bytes = net_recv(conn, &data);
        
    } while (1);

    return 0;
}


/*!
 * The thread function which handles the connection with the NTTP Server.
 * This thread is started from libnzbleech.c.
 */
void *nttp_connection(void *arg)
{
    struct connection_thread *ct = (struct connection_thread *)arg;
    int ret = 0;
    int reconnect_tries = 0;
    
    do
    {
        if (nttp_connect(ct) != 0)
            goto reconnect;
    
        ret = nttp_authenticate(ct->connection,
                                ct->server->username,
                                ct->server->password);
    
        if (ret != 0)
            goto reconnect;
        
        ct->connected = 1;
        
        // At this point we have a connection to the server and are ready to
        // retrieve data.
        ret = nttp_process_queue(ct);
        
        if (ret > 0)
            goto exit;
    
        // When here we have connected successfully and authenticate also
        // Thus we lost the connection due to other reasons, reset the
        // reconnect_tries
        reconnect_tries = 0;
        
    reconnect:
        
        if (reconnect_tries > 3)
            goto exit;

        reconnect_tries++;

        sleep(60);

    } while (1);

exit:

#ifdef WIN32
	_endthread();
	return NULL;
#else
    pthread_exit(NULL);
#endif

}
    
    
/*!
 * Process all items on the queue and wait for new items when the queue is
 * empty. Processed items are put on the queue for the data processor
 */
int nttp_process_queue(struct connection_thread *ct)
{
    queue_list_t *queue = ct->queues[ct->server->priority];
    queue_item_t *queue_item;
    char *current_group = NULL;
    int ret;
    
    while(1)
    {
        assert(ct->queues[ct->server->priority] != NULL);

        // This function blocks if there is no data
        queue_item = queue_list_shift(queue, ct->server);


        // Select group if the correct one isn't selected
        // TODO: We should iterate the groups if the first one isn't avail
        if (current_group == NULL ||
            strcmp(queue_item->segment->post->groups[0], current_group) != 0)
        {
            nttp_select_group(ct->connection,
                              queue_item->segment->post->groups[0]);
            current_group = queue_item->segment->post->groups[0];
        }
        

        ret = nttp_retrieve_segment(ct->connection, queue_item->segment);
        if (ret < 0)
        {
            ret = nttp_handle_retrieve_error(ct, queue_item);
            if (ret < 0)
                segment_status_set(queue_item->segment, SEGMENT_ERROR);
            continue;
        }
        
        assert(queue_item->segment->data != NULL);
        assert(queue != ct->data_queue);

        queue_list_append(ct->data_queue, queue_item);
        //sleep(1);
    }
    
    return 0;
}


/*!
 * Handle retrieve errors by trying to place the queue_item on another queue
 * or trying it on the same priority queue but leave it for another server
 * which is using the same priority queue
 */
int nttp_handle_retrieve_error(struct connection_thread *ct,
                               queue_item_t *queue_item)
{
    server_t *server = ct->server;
    server_t *server_head = server;
    server_t *current_server = server;
    queue_list_t * queue = ct->queues[server->priority];
    
    int same_prio_servers = 0;
    int total_servers = 0;
    
    // Find first server:
    while (server_head->prev != NULL)
        server_head = server_head->prev;

    // How many servers do we have with the same priority
    for(server = server_head; server; server = server->next)
    {
        if (server->priority == current_server->priority)
            same_prio_servers++;        
        total_servers++;
    }
    
    queue_item_set_failed(queue_item, server);
    
    // If all servers failed, return   
    if (queue_item->num_failed_servers == total_servers)
        return -1;


    if (queue_item->num_failed_servers < same_prio_servers)
    {
        // There are more servers with the same priority push it on
        // the queue again.
        queue_list_prepend(queue, queue_item);
        return 1;
    }
    else
    {
        // All servers with the same priority have tried and failed
        // Search for a server with a higher priority and push it on that
        // queue.
        for(server = server_head; server; server = server->next)
        {
            if (server->priority > current_server->priority)
            {
                queue_item_move(ct->queues[server->priority], queue_item);
                return 1;
            }
        }
    }
    
    return -1;
}

