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

#ifndef _SEGMENT_H
#define _SEGMENT_H

#include <stdlib.h>

/*!
 * A segment represents one article on the NTTP server.
 *
 * TODO: decoded_size and bytes should be of type size_t;
 */

#include "types.h"

typedef struct segment_s
{
    int already_exists;     //!< Flag TODO BITMASK
    
    int number;             //!< Segment number as in the nzb
    int index;              //!< Internal segment_id 
    int yenc_part;          //!< Segment part as in the yEnc header
    int nzb_bytes;          //!< Bytes according to the nzb file.    
    char *messageid;        //!< The message id 
    
    int complete;           //!< Set to 1 is complete 0 otherwise

    int bytes;              //!< Size of the raw data 
    char *data;             //!< Raw data
    size_t decoded_size;    //!< Size of the decoded data
    char *decoded_data;     //!< Decoded data

    struct post_s *post;    //!< Reference to post which segment belongs to
} segment_t;

segment_t * segment_create();
void segment_free(segment_t *segment);

inline void segment_status_set(segment_t *segment, int flag);
inline int segment_status_get(segment_t *segment);

#endif


