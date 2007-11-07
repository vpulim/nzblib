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

#include "post.h"
#include "segment.h"


post_t * types_create_post()
{
    post_t * post;
    
    post = malloc(sizeof(post_t));

    // Set default values
    post->segments = NULL;
    post->num_segments = 0;
    post->num_groups    = 0;
    post->groups = NULL;
    
    post->filename = NULL;
    post->segments_status = NULL;
    post->next = NULL;
    post->prev = NULL;
    return post;
    
}

    
void types_free_post(post_t *post)
{
    int i;
    
    if(post == NULL)
        return;

    for (i = 0; i < post->num_groups; i++)
        free(post->groups[i]);

    for (i = 0; i < post->num_segments; i++)
        segment_free(post->segments[i]);
        
    free(post->subject);
    free(post);
}

void types_post_insert(post_t *target, post_t *post)
{
    if(target == NULL)
    {
        target = post;
        post->next = NULL;
        post->prev = NULL;
    }
    else
    {
        target->prev = post;
        post->next = target;
        post->prev = NULL;
        target = post;
    }
    
}

void types_post_remove(post_t *target, post_t *post)
{
    if(post->prev != NULL)
        post->prev->next = post->next;
                
    if(post->next != NULL)
        post->next->prev = post->prev;
                
    if(target == post)
        target = NULL;
}

