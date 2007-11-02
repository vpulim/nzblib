#ifndef _FILE_H
#define _FILE_H

char *file_get_path(char *path);
int file_write_data(char *filename, char *data, int size);
int file_write_chunk(segment_t *segment, nzb_file *file);

#endif
