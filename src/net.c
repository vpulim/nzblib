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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <netdb.h>
#include <stdarg.h>

#include "global.h"
#include "types.h"
#include "net.h"



/*
 * Prepare the server structure to create a connection to the NTTP server.
 * This means that we do a lookup of the hostname, and put all relevant data
 * in the server_addr structure.
 */
int net_prepare_connection(server_t *server)
{
    struct hostent *hp;
    hp = gethostbyname(server->address);

    if (hp == NULL)
    {
        herror("Error");
        return -1;
    }
    
    server->server_addr.sin_family = AF_INET;
    bcopy(hp->h_addr, &(server->server_addr.sin_addr.s_addr), hp->h_length);
    server->server_addr.sin_port = htons( server->port );
    
    return 0;
}

/*
 * Connect to the NTTP server and return the created socket
 */
int net_connect(struct sockaddr_in * addr)
{
    int sock, ret;
    
    sock = socket(PF_INET,SOCK_STREAM, 0);
    ret = connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr));
    if (ret != 0)
    {
        perror("Unable to connect");
    }
    
    return sock;
}

/*
 * Receive data from the socket give as argument. This functions mallocs
 * BUFFER_SIZE stores the information in it and then null terminates the data.
 * The data is then returned as a pointer.
 *
 * TODO: This gives some problems when we want to know the size of the data.
 * Most of the times we don't and this makes it easy, but for example in
 * nttp_retrieve_segment we need to know the length of the bytes. Thus
 * requiring an extra strlen() which has an performance impact.
 *
 * This should be done better? or perhaps it isn't worth it.
 */
char * net_recv(int sock)
{
    char *data = malloc(sizeof(char) * BUFFER_SIZE);
    int bytes;
    
    bytes = recv(sock, data, BUFFER_SIZE, 0);
    
    if (bytes == -1)
    {
        perror("Unable to receive");
        free(data);
        return NULL;
    }
    data[bytes] = '\0';
    return data;
}

/*
 * Send data to the NTTP server. This function uses va_start/va_end to let it
 * behave like printf().
 */
int net_send(int sock, char *format, ...)
{
    va_list argp;
    int ret;
    char *buffer;
    
    // Create string
    va_start(argp, format);
    ret = vasprintf(&buffer, format, argp);
    va_end(argp);

    assert (buffer != NULL);
    assert (ret > 0);
        

    ret = send(sock, buffer, strlen(buffer), 0);
    
    if (ret == -1)
    {
        perror("Unable to send");
        free(buffer);
        return -1;
    }
    
    free(buffer);
    
    return ret;
}

/*
 * Disconnect from the server
 */
void net_disconnect(int sock)
{
    close(sock);
}


