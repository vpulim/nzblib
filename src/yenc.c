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
#include <stdio.h>
#include <assert.h>

#if !HAVE_REALLOCF
#   include "compat/reallocf.h"
#endif

#ifdef WIN32
#	include "compat/win32.h"
#	include "compat/stdint.h"
#	include "compat/strsep.h"
#else
#	include <stdint.h>
#endif

#include "crc32.h"
#include "yenc.h"

#ifdef DEBUG
#   include <sys/types.h>
#   include <sys/stat.h>
#endif


/*
 * Yenc decode the data in segment->data and store it in segment->decoded_data
 */
int yenc_decode(char *src, char **dst, char **filename, int *filesize, int *partnum)
{
    char *p, *s = strdup(src);
    unsigned char ch;
    uint32_t yenc_checksum = 0;
    uint32_t data_checksum;
    int i = 0;
    int pos = 0;
    int in_data = 0;
    int len = 0;
    int datasize = 0;
    int allocated = 0;
    
    if (src == NULL)
    {
        fprintf(stderr, "No data to decode!\n");
        return 1;
    }
    
    data_checksum = crc32_init();
    
    while((p = strsep(&s, "\r\n")))
    {
        if (*p == 0)
            continue;
        
        
        if(strncmp(p, "=ybegin ", 8) == 0)
        {
            yenc_parse_ybegin(p, filename, filesize, partnum);
            in_data = 1;
        }
        else if(strncmp(p, "=ypart", 5) == 0)
        {
            datasize = yenc_parse_ypart(p);
        }
        else if(strncmp(p, "=yend", 5) == 0)
        {
            yenc_checksum = yenc_parse_yend(p);
            in_data = 0;
            break;
        }
        else if (in_data)
        {
            // When this is not a multipart, the datasize is the filesize
            if (datasize == 0)
                datasize = *filesize;
            
            // Allocate memory 
            if (!allocated)
            {
                (*dst) = malloc(datasize + 1);
                allocated = 1;
            }
            
            for (i = 0, len = (int)strlen(p); i < len; i++)
            {
                ch = p[i];

                // Escape double dots at the start of the line 
                if (i == 0 && ch == '.' && p[i+1] == '.')
                    continue;

                if (ch == '=')
                    ch = p[++i] - 64;

                ch -= 42;

                (*dst)[pos++] = ch;
                data_checksum = crc32_add(data_checksum, ch);
            }
        }
    }
    (*dst)[pos] = '\0';

    data_checksum = crc32_finish(data_checksum);
    if (yenc_checksum != data_checksum || yenc_checksum == 0)
        return -1;
    
    return datasize;
}

/*
 * Parse the '=ybegin ' line and store all relevant information in the segment
 * structure.
 *  segment->number is the target line length
 *  segment->decoded_size is the size of the file (when decoded)
 *  segment->filename is the filename
 *
 *  TODO: Why segment->filename and not segment->filesize
 */
void yenc_parse_ybegin(char *line, char **filename, int *filesize, int *partnum)
{
    char *p = line;
    char *c;
    
    
    p += 8;
    if((c = strstr(line, "part=")))
    {
        c += 5;
        p = index(c, ' ');
        *p = '\0';
        (*partnum) = strtol(c, (char **)NULL, 10);
        *p = ' ';
    }
    
    if((c = strstr(line, "size=")))
    {
        c += 5;
        p = index(c, ' ');
        *p = '\0';
        (*filesize) = strtol(c, (char **)NULL, 10);
        *p = ' ';
    }

    if((c = strstr(line, "name=")))
    {
        c += 5;
        (*filename) = strdup(c);    
    }
            
}


/*
 * Parse the '=ypart ' line and return the size of the contained data when
 * decoded.
 *
 * The =ypart line is only available in multipart messages.
 *
 */
int yenc_parse_ypart(char *line)
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
        return ypart_size_end - (ypart_size_begin -1);
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
    char *decoded_data;
    char *data;
    int bytes;
    int ret;
    char *filename;
    int filesize;
    int partnumber;
    int i;
    struct stat stat_buf;
    int iterations = 10000;
    
    if (argc < 2)
    {
        fprintf(stderr, "No filename given\n");
        return EXIT_FAILURE;
    }
    
    
    ret = stat(argv[1], &stat_buf);
    if (ret < 0)
    {
        fprintf(stderr, "No such file or directory\n");
        return EXIT_FAILURE;
    }
    
    data = malloc(stat_buf.st_size + 1);
    

    fp = fopen(argv[1], "r");
    
    bytes = fread(data, sizeof(char), stat_buf.st_size, fp);
    printf("Read %d bytes (stat = %d)\n", bytes, stat_buf.st_size);
    fclose(fp);
    
    printf("Doing %d iterations\n", iterations);
    for(i = 0; i < iterations; i++)
    {
        ret = yenc_decode(data, &decoded_data, &filename, &filesize, &partnumber);
        //printf("yenc_decode() returned %d\n", ret);
    }
    
    printf("Segment number: %d\n", partnumber);
    
    fp = fopen("out", "w");
    fwrite(decoded_data, ret, sizeof(char), fp);
    fclose(fp);
    
    return 0;
}
#endif
