#ifndef _TYPES_H
#define _TYPES_H

#include <pthread.h>
#include <netinet/in.h>

#include "queue.h"
#define FILE_COMPLETE   0x01


/*!
 * Fileinfo type contains the filename and other unused variables
 */
typedef struct fileinfo_s
{
    char *filename;         //!< Filename
    int num_segments;
    
    unsigned int flags;
    
} fileinfo_t;

/*!
 * A segment represents one article on the NTTP server.
 *
 * TODO: decoded_size and bytes should be of type size_t;
 */
typedef struct segment_s
{
    int already_exists;     //!< Flag TODO BITMASK
    int number;             //!< Segment number as in the nzb
    int nzb_bytes;          //!< Bytes according to the nzb file.    
    char *messageid;        //!< The message id 
    
    int complete;           //!< Set to 1 is complete 0 otherwise

    int bytes;              //!< Size of the raw data 
    char *data;             //!< Raw data
    size_t decoded_size;    //!< Size of the decoded data
    char *decoded_data;     //!< Decoded data

    struct post_s *post;    //!< Reference to post which segment belongs to
} segment_t;


/*!
 * A post can exists out of multiple segments and generaly represents one file.
 */
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


struct connection_thread
{
    int thread_num;
    int ready;

    int sock;                         //!< Socket

    struct queue_list_s **queues;     //!< Array of queues with empy segments
    struct queue_list_s *data_queue;  //!< Pointer to queue with full segments
    pthread_t thread_id;
    struct server_s *server;
    //postlist_t *postlist;
};


typedef struct server_s
{
    char *username;             //!< Username to authenticate with
    char *password;             //!< Password to authenticate with
    
    int priority;               //!< Priority of the server, the lower the better
    int user_priority;          //!< Priority as specified by client
    int num_threads;            //!< Number of connection threads
    
    int port;                   //!< NTTP server port
    char *address;              //!< NTTP server address
    
    queue_list_t *queue;        //!< Reference to the queue used for this server
    struct server_s *next;      //!< Reference to the next server
    struct server_s *prev;      //!< Referecen to the previous server
    
    struct sockaddr_in server_addr;     //!< sockaddr_in structure
    struct connection_thread *threads;  //!< Array to the connection_threads used by the connections
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
