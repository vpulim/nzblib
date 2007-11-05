#ifndef _FILE_H
#define _FILE_H

char *file_get_path(char *path);
int file_write_chunk(segment_t *segment, nzb_file *file);
char * file_get_chunk_filename(segment_t *segment, nzb_file *file);
int file_combine(post_t * post, nzb_file *file);
int file_write_raw(segment_t *segment, nzb_file *file);
#endif
