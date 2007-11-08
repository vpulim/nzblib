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

#include <stdio.h>
#include <expat.h>
#include <string.h>
#include <assert.h>

#include "config.h"

#if !HAVE_REALLOCF
#   include "reallocf.h"
#endif

#include "post.h"
#include "parse_nzb.h"


static inline int max(int a, int b) {
  return a > b ? a : b;
}

static inline int min(int a, int b) {
  return a < b ? a : b;
}

/*!
 * Parse an nzb file and return a post_t structure or NULL when an error
 * occured.
 */
post_t * parse_nzb(char *filename)
{
    FILE *fp;
    XML_Parser parser;
    char buffer[8192];
    int done;
    size_t len;    
    archive_t archive;

    // Init the archive structure    
    archive.last = NULL;
    archive.first = NULL;
    archive.in_file = 0;
    archive.in_group = 0;
    archive.in_segment = 0;


    // Open the file
    fp = fopen(filename, "r");
    if(fp == NULL)
    {
        fprintf(stderr, "Unable to open %s: ", filename);
        perror("");
        return NULL;
    }

    // The encoding type should be read from the first line of the nzb file
    parser = XML_ParserCreate((XML_Char *)"ISO-8859-1");
    XML_SetUserData(parser, &archive);
    XML_SetElementHandler(parser, element_start, element_end);
    XML_SetCharacterDataHandler(parser, element_data);

    
    do
    {
        len = fread(buffer, 1, sizeof(buffer), fp);
        done = len < sizeof(buffer);
        if (!XML_Parse(parser, buffer, len, done)) {
            fprintf(stderr, "%s at line %d\n",
                    XML_ErrorString(XML_GetErrorCode(parser)),
                    (int)XML_GetCurrentLineNumber(parser));
            
            done = -1;
            break;
        }
    } while (!done);

    XML_ParserFree(parser);
    fclose(fp);
    
    // Error occured, return null
    if (done < 0)
        return NULL;
    
    return archive.first;
}


/*!
 * Called when a new element is encountered
 */
void element_start(void *data, const char *name, const char **atts)
{
    archive_t *archive = data;

    if(strcmp(name, "file") == 0)
        parse_file_element(archive, atts);

    else if(strcmp(name, "segment") == 0)
        parse_segment_element(archive, atts);
    
    else if(strcmp(name, "group") == 0)
        parse_group_element(archive, atts);
}

/*!
 * Called when the end of an element is encountered
 */
void element_end(void *data, const char *name)
{
    archive_t * archive = data;
    if(strcmp(name, "file") == 0)
    {
        archive->in_file = 0;
        post_segments_sort(archive->last, 0, archive->last->num_segments);
    }

    else if(strcmp(name, "segment") == 0)
        archive->in_segment = 0;

    else if(strcmp(name, "group") == 0)
        archive->in_group = 0;
        
}





/*!
 * Handle the data of an element. Note that his might be called with only a
 * chunk of data!
 */
void element_data(void *data, const XML_Char *s, int len)
{
    archive_t *archive = data;
    segment_t *segment;
    post_t *file;
    int i;
    int pos = 0;
    char *group;

    // Remove leading whitespaces/newlines
    while(*s == '\n' || *s == '\r' || *s == ' ')
    {
        *s++;
        len--;
        if(len == 0)
            break;
    } 

    if(len == 0)
        return;

    if(archive->in_segment)
    {
        segment = archive->last_segment;

        if (segment->messageid != NULL)
            pos = strlen(segment->messageid);

        segment->messageid = reallocf(segment->messageid,
                                      sizeof(char) * (pos + len + 1));

        memcpy(segment->messageid + pos, s, len);
        segment->messageid[pos + len] = '\0';
    }

    if(archive->in_group)
    {
        file = archive->last;
        group = file->groups[file->num_groups - 1];
        
        if(group != NULL)
            pos = strlen(group);
            len += pos;

        group = reallocf(group, len);

        for(i = pos; i < len; i++)
            group[i] = *s++;
        group[i] = '\0';

        file->groups[file->num_groups - 1] = group;
    }
}

/*!
 * Parse the file element including its attributes and store it in
 * archive->last which is of type post_t
 */
void parse_file_element(archive_t *archive, const char **atts)
{
    post_t *post;
    char *key, *value;
    int i = 0;
    
    archive->in_file = 1;

    post = post_create();

    // Parse attributes
    while(atts[i])
    {
        key   = (char *)atts[i++];
        value = (char *)atts[i++];

        if(strcmp(key, "subject") == 0)
        {
            post->subject = strdup(value);
            post->filename = find_filename(value);
        }
    }

    if(archive->first == NULL)
        archive->first = post;

    if(archive->last)
        (archive->last)->next = post;
    archive->last = post;
    
}

/*!
 * Parse the group element including its attributes and store it in
 * archive->last which is of type post_t
 */
void parse_group_element(archive_t *archive, const char **atts)
{
    post_t *post = archive->last;
    
    archive->in_group = 1;
    post->num_groups++;
    post->groups = reallocf(post->groups, sizeof(char *) * post->num_groups);
    post->groups[post->num_groups - 1] = NULL;
}

/*!
 * Parse the segment element including its attributes and the store it in
 * archive->last which is of type post_t. Also set the created segment as
 * archive->last_segment
 */
void parse_segment_element(archive_t *archive, const char **atts)
{
    post_t *post = archive->last;
    segment_t *segment;
    char *key, *value;
    int i = 0;
    
    archive->in_segment = 1;

    // Create segment block
    segment = segment_create();
    assert(segment->messageid == NULL);
    assert(post != NULL);
    
    // Get attributes
    while(atts[i])
    {
        key   = (char *)atts[i++];
        value = (char *)atts[i++];
    
        if(strcmp(key, "bytes") == 0)
            segment->nzb_bytes = strtol(value, (char **)NULL, 10);
        
        if(strcmp(key, "number") == 0)
            segment->number = strtol(value, (char **)NULL, 10);
    }
    
    post->segments = reallocf(post->segments,
                              sizeof(segment_t *) * (post->num_segments + 1));
    
    
    post->segments_status = reallocf((char *)post->segments_status,
                                     sizeof(int) * (post->num_segments + 1));
    
    segment->index = post->num_segments;
    
    post->segments[segment->index] = segment;
    post->segments_status[segment->index] = SEGMENT_NEW;
    
    segment->post = post;
    post->num_segments++;
    archive->last_segment = segment;    
}



/*!
 * Try to find the filename in the subject. This method tries three different
 * subject 'layouts'.
 *  1. filename between ""
 *  2. filename between ''
 *  3. filename between ' - '
 */
char * find_filename(char *value)
{
    char *filename_start;
    char *filename_end;
    
    /* Get filename from subject */
    filename_end = rindex(value, '"');
    if(filename_end)
    {
        filename_start = filename_end;
        while(*--filename_start != '"') {} // rindex() ?
        
        *filename_start++;
        *filename_end = '\0';
        return strdup(filename_start);
    }

    filename_end = rindex(value, '\'');
    if(filename_end)
    {
        filename_start = filename_end;
        filename_start = rindex(filename_end, '\'');
        while(*--filename_start != '\'') {} // rindex() ?
        
        *filename_start++;
        *filename_end = '\0';
        
        return strdup(filename_start);
    }
    
    // Third subject layout method
    // [id] - message - title - filename - [02/54] (114/114) 
    filename_end = rindex(value, '-');
    if(filename_end)
    {
        filename_start = filename_end;
        *filename_start--;

        while(strncmp(filename_start, "- ", 2) != 0)
            *filename_start--;

        while(*filename_end != ' ')
            *filename_end++;
            
        filename_start+=2; // Skip '- ' part
        *(filename_end) = '\0'; // Terminate ' -' part
        return strdup(filename_start);
    }
    
    return NULL;
}

#ifdef DEBUG
int main(int argc, char **argv)
{
    post_t * file, *tmp_file;
    segment_t * segment;
    int i = 0;

    file = parse_nzb(argv[1]);
    if(file == NULL)
    {
        printf("Error\n");
        return -1;
    }

    
    for(file; file; file = file->next)
    {
        printf("Subject: %s\n", file->subject);
        printf("Filename: %s\n", file->filename);
        printf("Number of segments: %d\n", file->num_segments);
        for(i = 0; i < file->num_groups; i++)
        {
            printf("Group: %s\n", file->groups[i]);
        }
        for(i = 0; i < file->num_segments; i++)
        {
            assert(file->segments[i]->messageid != NULL);
            printf("%d ", file->segments[i]->number);
            printf("\t%s\n", file->segments[i]->messageid);
        }
        printf("\n");
    }
    
    return 0;
}
#endif
