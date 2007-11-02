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
#include <pthread.h>
#include <unistd.h>

#include "nzb_fetch.h"
#include "yenc.h"
#include "file.h"


void *process_data_queue(void *arg)
{
    nzb_fetch *fetcher = (nzb_fetch *)arg;
    queue_item_t *queue_item;
    queue_list_t *queue = fetcher->data_queue;
    int ret;
    
    printf("Starting process_data_queue thread\n");
        
    while(1)
    {
        // This function blocks if there is no data
        queue_item = queue_list_shift(queue, NULL);
    
        printf("Found queue_item\n");
    
        ret = yenc_decode(queue_item->segment);


        if (ret != 0)
        {
            printf("yEnc decode error..\n");
            
            // TODO: Free queue_item
            queue_item_destroy(queue_item);
            continue;
        }
        
        printf("yEnc OK\n");
        

        ret = file_write_chunk(queue_item->segment, fetcher->file);
        
        if (ret < 0)
            printf("Unable to store file\n");
        
        queue_item_destroy(queue_item);
        
    }   
    pthread_exit(NULL);
}
