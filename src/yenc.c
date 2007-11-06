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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "crc32.h"
#include "yenc.h"

#ifdef DEBUG
#include "types.h"

#endif

/*
 * Yenc decode the data in segment->data and store it in segment->decoded_data
 *
 * TODO:
 *  - Performance: create hash while decoding instead of at the end. This might
 *    give a small performance boost.
 */
int yenc_decode(segment_t *segment)
{
    char *p, *s = segment->data;
    unsigned char ch;
    uint32_t yenc_checksum;
    uint32_t data_checksum;
    int i = 0;
    int pos = 0;
    int in_data = 0;
    int len = 0;
    
    if (segment->data == NULL)
    {
        fprintf(stderr, "No data to decode!\n");
        return 1;
    }

    
    data_checksum = crc32_init(data_checksum);
    
    while((p = strsep(&s, "\r\n")))
    {
        if (*p == 0)
            continue;
        
        if(strncmp(p, "=ybegin ", 8) == 0)
        {
            yenc_parse_ybegin(p, segment);
            
            // Allocate memory
            segment->decoded_data = malloc(segment->decoded_size + 1);
            
            in_data = 1;
        }
        else if(strncmp(p, "=ypart", 5) == 0)
        {
            yenc_parse_ypart(p, segment);
        }
        else if(strncmp(p, "=yend", 5) == 0)
        {
            yenc_checksum = yenc_parse_yend(p);
            in_data = 0;
            break;
        }
        else if (in_data)
        {
            for (i = 0, len = strlen(p); i < len; i++)
            {
                ch = p[i];

                // Escape double dots at the start of the line 
                if (i == 0 && ch == '.' && p[i+1] == '.')
                    continue;

                if (ch == '=')
                    ch = p[++i] - 64;

                ch -= 42;

                segment->decoded_data[pos++] = ch;
                data_checksum = crc32_add(data_checksum, ch);
            }
        }
    }
    segment->decoded_data[pos] = '\0';

    data_checksum = crc32_finish(data_checksum);
    if (yenc_checksum != data_checksum)
        return YENC_CRC_ERROR;
    
    return YENC_OK;
}

/*
 * Parse the '=ybegin ' line and store all relevant information in the segment
 * structure.
 *  segment->number is the target line length
 *  segment->decoded_size is the size of the file (when decoded)
 *  segment->fileinfo->filename is the filename
 *
 *  TODO: Why segment->fileinfo->filename and not segment->fileinfo->filesize ?
 */
void yenc_parse_ybegin(char *line, segment_t *segment)
{
    char *p = line;
    char *c;
    fileinfo_t * fileinfo;
    
    
    p += 8;
    if((c = strstr(line, "part=")))
    {
        c += 5;
        p = index(c, ' ');
        *p = '\0';
        segment->number = strtol(c, (char **)NULL, 10);
        *p = ' ';
    }
    
    if((c = strstr(line, "size=")))
    {
        c += 5;
        p = index(c, ' ');
        *p = '\0';
        segment->decoded_size = strtol(c, (char **)NULL, 10);
        *p = ' ';
    }

    if((c = strstr(line, "name=")))
    {
        c += 5;
        fileinfo = segment->post->fileinfo;
        
        if(fileinfo->filename == NULL || strcmp(c, fileinfo->filename) != 0)
        {
            // Force the use of the filename in the yenc message 
            if(fileinfo->filename != NULL)
                free(fileinfo->filename);
                
            fileinfo->filename = strdup(c);    
        }
    }
            
}


/*
 * Parse the '=ypart ' line and store all relevant information in the segment
 * structure.
 *
 * The =ypart line is only available in multipart messages.
 *
 * segment->decoded_size is the size of this part
 * segment->decoded_data is pointer to the data
 *
 * TODO: Why realloc and not malloc?
 */
void yenc_parse_ypart(char *line, segment_t *segment)
{
    int ypart_size_begin = 0;
    int ypart_size_end = 0;
    char *p;
    char *value;

    if((value = strstr(line, "begin=")))
    {
        value += 6;
        
        p = index(value, ' ');
        *p = '\0';

        ypart_size_begin = (uint32_t)strtoull(value, (char **)NULL, 10);
        *p = ' ';
    }
    
    if((value = strstr(line, "end=")))
    {
        value += 4;
        
        ypart_size_end = (uint32_t)strtoull(value, (char **)NULL, 10);
        segment->decoded_size = ypart_size_end - (ypart_size_begin -1);
        segment->decoded_data = realloc(segment->decoded_data,
                                        segment->decoded_size + 1);
    }
            
}

/*
 * Parse the '=ypart ' line return the crc32 hash.
 *
 * TODO:
 *  - The ypart line contains more data when it is a multipart message
 *  - The crc32 field is called pcrc32 when it is a mulitpart message,
 *    however this seems to work fine.
 */
uint32_t yenc_parse_yend(char *line)
{
    char *p = NULL;
    
    // if is part
    // p = strstr(p, "pcrc32=");
    // else
    p = strstr(line, "crc32=");
    if(p != NULL)
        return (uint32_t)strtoull(p+6, (char **)NULL, 16);
    else
        return 0;
}





#ifdef DEBUG
int main(int argc, char **argv)
{
    FILE *fp;
    int ret, bytes;
    segment_t *segment;
    post_t *post;
    
    char *data;

    
    int i = 0;
    for(i = 0; i < 1; i++)
    {
        data = malloc(2048576);
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            perror("Error opening file");
            return 1;
        }
        
        bytes = fread(data, sizeof(char), 2048576, fp);
        printf("Read %d bytes\n", bytes);
        fclose(fp);
        post = types_create_post();
        post->fileinfo = types_create_fileinfo();
        segment = types_create_segment();
        segment->data = data;
        segment->bytes = bytes;
        segment->post = post;
        
        ret = yenc_decode(segment);
        printf("yenc_decode() returned %d\n", ret);
        printf("Segment number: %d\n", segment->number);
        
    }
    
    return 0;
}
#endif
