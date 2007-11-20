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

#ifndef _LIBNZB_FETCH_H
#define _LIBNZB_FETCH_H

#include <pthread.h>

#include "post.h"
#include "server.h"
#include "queue.h"


#define FILE_COMPLETE   1

struct nzb_file_info_s;


typedef struct nzb_file_s
{
    char *target;
    char *filename;
    char *storage_path;
    char *temporary_path;
    struct post_s *posts;
    
} nzb_file;

typedef struct nzb_fetch_s
{
    struct server_s *servers;
    struct queue_list_s *queue;
    struct queue_list_s *data_queue;
    struct queue_list_s **priority_queues;
    struct nzb_file_s *file;
    pthread_t process_thread_id;
    
    void (*callback_file_complete)(struct nzb_file_info_s*);
} nzb_fetch;


typedef struct nzb_file_info_s
{
    char *filename;
    
    
    nzb_file *file;         /*!< Internal: Reference to nzb file object */
    struct post_s *post;    /*!< Internal: Reference to the post object*/
} nzb_file_info;



int nzb_fetch_add_callback(nzb_fetch *fetcher, int type, void *file_complete);
nzb_fetch * nzb_fetch_init(void);
int nzb_fetch_add_server(nzb_fetch *fetcher, char *address, int port,
                         char *username, char *password, int threads,
                         int priority);

nzb_file *nzb_fetch_parse(char *filename);
int nzb_fetch_connect(nzb_fetch *fetcher);
int nzb_fetch_storage_path(nzb_file *file, char *path);
int nzb_fetch_temporary_path(nzb_file *file, char *path);
int nzb_fetch_download(nzb_fetch *fetcher, nzb_file_info *file_info);
int nzb_fetch_list_files(nzb_file *file, nzb_file_info ***files);

#endif
