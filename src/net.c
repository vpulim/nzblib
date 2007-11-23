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
#	include "compat/vasprintf.h"
#	include <windows.h>
#	include "compat/win32.h"
#	define herror perror
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netdb.h>
#	include <openssl/ssl.h>
#endif

#include <stdarg.h>

#include "global.h"
#include "server.h"
#include "net.h"

#if HAVE_LIBSSL

static SSL_CTX *ssl_context;


int net_ssl_init()
{
    SSL_load_error_strings();
    SSL_library_init();
    ssl_context = SSL_CTX_new(SSLv2_client_method());
}

#endif //HAVE_LIBSSL


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
connection_t *net_connect(struct sockaddr_in * addr, int ssl)
{
    int ret;
    connection_t *conn = malloc(sizeof(connection_t));
    
    conn->use_ssl = ssl;
    
    conn->sock = (int)socket(PF_INET, SOCK_STREAM, 0);
    
    ret = connect(conn->sock , (struct sockaddr *)addr, sizeof(struct sockaddr));
    
    if (ret != 0)
    {
        perror("Unable to connect");
    }
    
#if HAVE_LIBSSL
    if (conn->use_ssl)
    {
	conn->ssl = SSL_new(ssl_context);
	ret = SSL_set_fd(conn->ssl, conn->sock);
	ret = SSL_connect(conn->ssl);
    }
#endif

    return conn;
}

/*
 * Receive and allocate data in second argument from the sock as first
 * argument. This functions mallocs BUFFER_SIZE stores the information in
 * the data pointer and returns the number of bytes.
 */
int net_recv(connection_t *conn, char **data)
{
    int bytes;

    *data = malloc(sizeof(char) * BUFFER_SIZE);

#if HAVE_LIBSSL 
    if (conn->use_ssl)
	bytes = SSL_read(conn->ssl, *data, BUFFER_SIZE);
    else
#else
	bytes = recv(conn->sock, *data, BUFFER_SIZE, 0);
#endif
    
    if (bytes == -1)
    {
        perror("Unable to receive");
        free(data);
        data = NULL;
        return -1;
    }
    
    (*data)[bytes] = '\0';
    return bytes;
}

/*
 * Send data to the NTTP server. This function uses va_start/va_end to let it
 * behave like printf().
 */
int net_send(connection_t *conn, char *format, ...)
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

#if HAVE_LIBSSL
    if (conn->use_ssl)
	ret = SSL_write(conn->ssl, buffer, strlen(buffer));
    else
#else
#   ifdef WIN32
	ret = send(conn->sock, buffer, (int)strlen(buffer), 0);
#   else
	ret = send(conn->sock, buffer, strlen(buffer), 0);
#   endif // WIN32
# endif // HAVE_LIBSSL
    
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
void net_disconnect(connection_t *conn)
{
#ifdef WIN32
    closesocket((SOCKET)conn->sock);
#else
    close(conn->sock);
#endif
    //free(conn);
}


