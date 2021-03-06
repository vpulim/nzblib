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

#ifdef WIN32
#	include <io.h>
#	include "compat/asprintf.h"
#	include "compat/win32.h"
#	define PATH_MAX _MAX_PATH
#else
#include <unistd.h>


#include <sys/stat.h>
#include <sys/param.h>
#include <stdlib.h>
#endif
#include <assert.h>

#include "file.h"

#include "nzb_fetch.h"
#include "global.h"


char *file_get_path(char *path)
{
    char resolved_path[PATH_MAX];
    char *result;
    char *err_string;

    //printf("path = %s\n", path);
#ifdef WIN32
    result = _fullpath(resolved_path, path, PATH_MAX);
#else
    result = realpath(path, resolved_path);
#endif
    if (result == NULL)
    {
        asprintf(&err_string, "Unable to get realpath '%s'", path);
        perror(err_string);
        free(err_string);
        return NULL;
    }
#ifdef WIN32
	return strdup(resolved_path);
#else
    return strdup(resolved_path);
#endif

}

/*
 * Write the chunk to the temporary directory, appended with the segment
 * number. Return 0 on success < 0 on error
 */
int file_write_chunk(segment_t *segment, nzb_file *file)
{
    char *filename;
    FILE *fp;
    
    filename = file_get_chunk_filename(segment, file);
    
#ifdef WIN32
    fp = fopen(filename, "wbT"); // add T
#else
    fp = fopen(filename, "w");
#endif

    if(fp != NULL)
    {
        fwrite(segment->decoded_data, 1, segment->decoded_size, fp);
        fclose(fp);
    }
    else
    {
        perror("Error while saving file");
		printf("To %s\n", filename);
        free(filename);
        return -1;
    }
    
    free(filename);
    

    
    return 0;
}

int file_write_raw(segment_t *segment, nzb_file *file)
{
    char *filename;
    FILE *fp;

    
    asprintf(&filename, "%s%s%s.segment.%03d", file->temporary_path, PATH_SEP,
             segment->post->filename, segment->number);

    fp = fopen(filename, "w");
    if(fp != NULL)
    {
        fwrite(segment->data, (size_t)1, (size_t)segment->bytes, fp);
        fclose(fp);
    }
    else
    {
        perror("Error while saving file");
        free(filename);
        return -1;
    }
    
    free(filename);
    
    
    return 0;
}

char * file_get_chunk_filename(segment_t *segment, nzb_file *file)
{
    char *filename;
    
    assert(segment->post->filename != NULL);
    
    asprintf(&filename, "%s%s%s.segment.%03d", file->temporary_path, PATH_SEP,
             segment->post->filename, segment->number);
    
    return filename;
}

char * file_get_complete_filename(post_t *post, nzb_file *file)
{
    char *path;
    char *filename;
    path = file_get_path(file->storage_path);
    if (path == NULL)
    {
        assert(0);
        return NULL;
    }
    asprintf(&filename, "%s%s%s", path, PATH_SEP, post->filename);
    
    return filename;
}

int file_combine(post_t * post, nzb_file *file)
{
    char *path;
    char *filename;
    char buffer[BUFFER_SIZE];
    FILE *fp_chunk, *fp_target;
    int i;
    size_t bytes;
    
    // Open the target file
    filename = file_get_complete_filename(post, file);

#ifdef WIN32
    fp_target = fopen(filename, "wb");
#else
    fp_target = fopen(filename, "w");
#endif

	assert(fp_target != NULL);

    free(filename);
    //free(path);
    
    /* TODO:
        Get array of segment_index sorted on part number
        iterate that array
    */
    path = file_get_path(file->temporary_path);
    for (i = 0; i < post->num_segments; i++)
    {
        if (post->segments_status[i] == SEGMENT_ERROR)
        {
            printf("Segment status = ERROR skipping\n");
            continue;
        }
        
        // Create the filename
        // TODO change ->number in yenc_part
        asprintf(&filename, "%s%s%s.segment.%03d", path, PATH_SEP,
                post->filename, post->segments[i]->number);
        
#ifdef WIN32
        fp_chunk = fopen(filename, "rb");
#else
        fp_chunk = fopen(filename, "r");
#endif

        // The chunk went missing, return -1 so that the process thread can
        // identify the problem.
        if (fp_chunk == NULL)
        {
            printf("::: %s\n", filename);
            perror("Unable to open chunk");
            free(filename);
            fclose(fp_target);
            //free(path);
            return -1;
        }
        free(filename);
        filename = NULL;
        
        while(!feof(fp_chunk))
        {
            bytes = fread(&buffer, (size_t)1, (size_t)BUFFER_SIZE-1, fp_chunk);
            fwrite(&buffer, 1, bytes, fp_target);
        }
        fclose(fp_chunk);
    }
    
    //free(path);
    fclose(fp_target);
    return 0;
}

int file_chunk_exists(segment_t *segment, nzb_file *file)
{
    char *filename;
    int ret;
	
    filename = file_get_chunk_filename(segment, file);
	return 0;

    ret = access(filename, /* F_OK */ 0);
    free(filename);
    
    if(ret == 0)
        return 1;

    return 0;
}

int file_complete_exists(post_t *post, nzb_file *file)
{
    char *filename;
    int ret;

    filename = file_get_complete_filename(post, file);

    ret = access(filename, /* F_OK */ 0);
    free(filename);
    
    if(ret == 0)
        return 1;

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

    segment = segment_create();
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