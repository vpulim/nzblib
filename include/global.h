#ifndef _GLOBALS_H
#define _GLOBALS_H

#define BUFFER_SIZE 8000

extern int num_locked;

#ifdef DEBUG
#define MTX_LOCK(s) \
    num_locked++;\
    printf("MTX_LOCK(%d)\n", num_locked); \
    pthread_mutex_lock(s);

#define MTX_UNLOCK(s) \
    num_locked--;\
    printf("MTX_UNLOCK(%d)\n", s, num_locked); \
    pthread_mutex_unlock(s);

#define KEEP_RAW_MESSAGES
#else

#define MTX_LOCK(s) pthread_mutex_lock(s);
#define MTX_UNLOCK(s) pthread_mutex_unlock(s);

#endif // #IFDEF DEBUG

#endif
