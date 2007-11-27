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
#	include <process.h>
#else
#	include <pthread.h>
#	include <unistd.h>
#endif


#include "yenc.h"
#include "process.h"

#include "segment.h"
#include "file.h"
#include "nzb_fetch.h"

void *process_data_queue(void *arg)
{
    nzb_fetch *fetcher = (nzb_fetch *)arg;
    queue_item_t *queue_item;
    queue_list_t *queue = fetcher->data_queue;
    segment_t *segment;
    
    int ret;
    
    //printf("Starting process_data_queue thread\n");
        
    while(1)
    {
        // This function blocks if there is no data
        queue_item = queue_list_shift(queue, NULL);
        
        segment = queue_item->segment;
        
        assert(segment->data != NULL);

        //printf("processing\n");
        

        //file_write_raw(segment, fetcher->file);
        ret = process_yenc_data(segment);

        if (ret < 0)
        {
            printf("SEGMENT_ERROR\n");
            segment_status_set(segment, SEGMENT_ERROR);
            
            // TODO: Free queue_item
            queue_item_destroy(queue_item);
            continue;
        }
        
        
        ret = file_write_chunk(segment, fetcher->file);
        
        free(segment->data);
        segment->data = NULL;
        segment->bytes = 0;
        
        if (ret < 0)
            printf("Unable to store file\n");
        
        segment_status_set(segment, SEGMENT_COMPLETE);
        
        ret = process_check_post_status(segment->post);

        if (ret == 0)
        {
            file_combine(segment->post, fetcher->file);
            nzb_fetch_file_complete(fetcher, segment->post);
            //post_free(segment->post);
        }
        free(segment->decoded_data);
        segment->decoded_data = NULL;
        segment->decoded_size = 0;

        queue_item_destroy(queue_item);
    }   
#ifdef WIN32
	_endthread();
#else
    pthread_exit(NULL);
#endif
}


int process_yenc_data(segment_t *segment)
{
    int ret;
    
    ret = yenc_decode(segment->data, &segment->decoded_data,
                      &segment->post->filename, &segment->post->filesize,
                      &segment->number);
    
    if (ret > 0)
        segment->decoded_size = ret;
        
    return ret;

    
    
    
}

int process_check_post_status(post_t *post)
{
    int i;
    
    // Return -1 when we have not tried to download all segments
    for (i = 0; i < post->num_segments; i++)
        if (post->segments_status[i] == SEGMENT_NEW)
            return -1;
    // Tried all posts    
    return 0;
}
