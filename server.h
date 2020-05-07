#ifndef _SIMPLE_SERVER_H
#define _SIMPLE_SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#define NUM_THREADS 2

typedef struct{
    char **log_buf;
    int *job_buf;
    int job_len, log_len;
    int job_count, log_count;
    int job_front;
    int job_rear;
    int log_front;
    int log_rear;
    pthread_mutex_t jobmutex, logmutex;
    pthread_cond_t job_cv_cs, job_cv_pd;
    pthread_cond_t log_cv_cs, log_cv_pd;
}buf;

int open_listenfd(int);
//****declarations****
void buf_init(buf *sp, int job_len, int log_len);
void buf_deinit(buf *sp);
void buf_insert_log(buf *sp, char* item);
void buf_insert_job(buf *sp, int item);
void buf_remove_log(buf *sp, char** out_buf);
int buf_remove_job(buf *sp);
int word_exists(FILE *fp, char* word);
void spawn_worker_threads();
void* worker(void* vargp);
void* logger(void* args);

#endif