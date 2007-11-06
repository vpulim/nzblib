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


#include "segment.h"

/*!
 * Initialize a segment
 */
segment_t * segment_create()
{
    segment_t * segment;
    segment = malloc(sizeof(segment_t));
    segment->data = NULL;
    segment->bytes = 0;
    segment->nzb_bytes = 0;
    segment->decoded_data = NULL;
    segment->decoded_size = 0;
    segment->messageid = NULL;
    segment->complete = 0;
    return segment;
}


/*!
 * Free segment
 */
void segment_free(segment_t *segment)
{
    if(segment->data != NULL)
        free(segment->data);

    if(segment->decoded_data != NULL)
        free(segment->decoded_data);

    free(segment);
}


inline void segment_status_set(segment_t *segment, int flag)
{
    segment->post->segments_status[segment->index] = flag;
}

inline int segment_status_get(segment_t *segment)
{
    return segment->post->segments_status[segment->index];
}
