#ifndef _LIBNZB_FETCH_H
#define _LIBNZB_FETCH_H

#include "types.h"
#include "queue.h"

typedef struct nzb_file_s
{
    char *target;
    char *filename;
    char *storage_path;
    char *temporary_path;
    post_t *posts;
    
} nzb_file;

typedef struct nzb_fetch_s
{
    struct server_s *servers;
    struct queue_list_s *queue;
    struct queue_list_s *data_queue;
    struct queue_list_s **priority_queues;
    struct nzb_file_s *file;
    pthread_t process_thread_id;
} nzb_fetch;



nzb_fetch * nzb_fetch_init();
int nzb_fetch_add_server(nzb_fetch *fetcher, char *address, int port,
                         char *username, char *password, int threads,
                         int priority);

nzb_file *nzb_fetch_parse(char *filename);
int nzb_fetch_connect(nzb_fetch *fetcher);
int nzb_fetch_storage_path(nzb_file *file, char *path);
int nzb_fetch_temporary_path(nzb_file *file, char *path);
int nzb_fetch_download(nzb_fetch *fetcher, nzb_file *file);
#endif