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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdlib.h>
     
#include "nzb_fetch.h"


char *file_get_path(char *path)
{
    char resolved_path[PATH_MAX];
    char *result;
    char *err_string;
    
    result = realpath(path, resolved_path);
    
    if (result == NULL)
    {
        asprintf(&err_string, "Unable to write to '%s'", path);
        perror(err_string);
        free(err_string);
        return NULL;
    }
    
    return result;
}

/*
 * Write data to the given filename. Return 0 on success < 0 on error
 */
int file_write_data(char *filename, char *data, int size)
{
    FILE *fp;
    fp = fopen(filename, "w");
    if(fp != NULL)
    {
        fwrite(data, 1, size, fp);
        fclose(fp);
    }
    else
    {
        perror("Error while saving file");
        return -1;
    }
    
    return 0;
}

int file_write_chunk(segment_t *segment, nzb_file *file)
{

    char *path;
    char *filename;
    int ret;
    
    path = file_get_path(file->temporary_path);
    
    if (path == NULL)
        return -1;
    
    asprintf(&filename, "%s/%s.segment.%03d", path,
             segment->fileinfo->filename, segment->number);
    
    ret = file_write_data(filename, segment->decoded_data,
                          segment->decoded_size);
    free(filename);
    
    if (ret < 0)
        return -1;
    
    return 0;
}


#ifdef FILE_DEBUG

int main(int argc, char **argv)
{
    nzb_file *file = malloc(sizeof(nzb_file));
    FILE *fp;
    int ret, bytes;
    segment_t *segment;
    
    char *data;

   
    data = malloc(2048576);
    fp = fopen(argv[1], "r");
    if (fp == NULL)
        return -1;
    
    bytes = fread(data, sizeof(char), 2048576, fp);
    fclose(fp);

    segment = types_create_segment();
    segment->data = data;
    segment->bytes = bytes;
    segment->number = 1;
    
    file->storage_path = strdup("tmp/complete/");
    file->temporary_path = strdup("tmp/chunks/");
    
    yenc_decode(segment);
    file_write_chunk(segment, file);
    
    return 0;
}

#endif