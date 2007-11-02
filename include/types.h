#ifndef _TYPES_H
#define _TYPES_H

#include <pthread.h>
#include <netinet/in.h>

#include "queue.h"
#define FILE_COMPLETE   0x01





typedef struct fileinfo_s
{
    char *filename;
    int num_segments;
    
    int *part_status;
    unsigned int flags;
    
} fileinfo_t;


typedef struct segment_s
{
    int already_exists;     // Flag TODO BITMASK
    int number;             // Segment number as in the nzb
    int nzb_bytes;          // Bytes according to the nzb file.    
    char *messageid;

    int bytes;              // Size of the raw data 
    char *data;             // Raw data
    int decoded_size;       // Size of the decoded data
    char *decoded_data;     // Decoded data

    fileinfo_t *fileinfo;   //

    struct post_s *post;    // Reference to post which segment belongs to
    struct segment_s *next; // Remove me ?
} segment_t;


typedef struct post_s
{
    int id;
    char *subject;
    fileinfo_t *fileinfo;

    int num_groups;         // Number of groups
    char **groups;          // Array with groups

    int num_segments;       // Number of segments
    segment_t **segments;   // Array with segments
    
    struct post_s *next, *prev;
} post_t;



//typedef struct postlist_s
//{
//    pthread_mutex_t mtx_post;
//    pthread_mutex_t mtx_data;
//    pthread_mutex_t mtx_cond;
//
//    pthread_cond_t cond_data;       // Protected by mtx_cond
//
//    post_t *post;                   // Protected by mtx_post
//    int num_total_posts;
//    int num_posts_left;
//    
//    segment_t *datasegments_first;  // Protected by mtx_data
//    segment_t *datasegments_last;   // Protected by mtx_data
//
//} postlist_t;

struct connection_thread
{
    int thread_num;
    int ready;
    
    int sock;
    
    struct queue_list_s **queues;       // Array of queues with empy segments
    struct queue_list_s *data_queue;    // Pointer to queue with full segments
    pthread_t thread_id;
    struct server_s *server;
    //postlist_t *postlist;
};


typedef struct server_s
{
    char *username;
    char *password;
    
    int priority;
    int num_threads;
    
    int port;
    char *address;
    
    queue_list_t *queue;
    struct server_s *next;
    struct server_s *prev;
    
    struct sockaddr_in server_addr;
    
    struct connection_thread *threads;
} server_t;




//void types_postlist_init(postlist_t *postlist);
segment_t * types_create_segment();

void types_free_segment(segment_t *segment);
void types_free_post(post_t *post);
post_t * types_create_post();

fileinfo_t * types_create_fileinfo();
void types_free_fileinfo(fileinfo_t *fileinfo);

void types_post_remove(post_t *target, post_t *post);
void types_post_insert(post_t *target, post_t *post);

#endif
