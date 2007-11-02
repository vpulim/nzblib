#ifndef _PARSE_NZB_H
#define _PARSE_NZB_H

#include <expat.h>

typedef struct archive_s
{
    post_t *first;
    post_t *last;

    segment_t *last_segment;
    int in_file;
    int in_group;
    int in_segment;
} archive_t;

post_t * parse_nzb(char *filename);
char * find_filename(char *value);
void element_start(void *data, const char *name, const char **atts);
void element_end(void *data, const char *name);
void element_data(void *data, const XML_Char *s, int len);

void parse_file_element(archive_t *archive, const char **atts);
void parse_group_element(archive_t *archive, const char **atts);
void parse_segment_element(archive_t *archive, const char **atts);

#endif
