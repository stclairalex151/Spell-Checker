/*
 * Program name: server.c
 * Programmer:   Alex St.Clair
 * Program Desc: Networked multithreadded spell checker using a specified dictionary
 */
//linux location: cis-linux2.temple.edu
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

typedef struct{
    char **log_buf;
    int *job_buf;
    int job_len, log_len;
    int job_count, log_count;
    int job_front;
    int job_rear;
    int log_front;
    int log_rear;
    pthread_cond_t job_cv_cs, job_cv_pd;
    pthread_cond_t log_cv_cs, log_cv_pd;
}buf;

//****declarations****
void buf_init(buf *sp, int job_len, int log_len);
void buf_deinit(buf *sp);
void buf_insert_log(buf *sp, char* item);
void buf_insert_job(buf *sp, int item);
void buf_remove_log(buf *sp, char** out_buf);
int buf_remove_job(buf *sp);
int open_listedfd(char *port);


int main(int argc, char** argv) {
    const char* DEFAULT_DICT = "/usr/share/dict/words";
    const int DEFAULT_PORT = 8888;
    char* dict;                 //buffer for path to dictionary
    struct sockaddr_in client;  //holds information about the user connection. (needed for accept())
    int clientLen = sizeof(client);     //size of client structure
    int listenport = DEFAULT_PORT;     //port for listening
    int listenSocket, clientSocket, bytesReturned;  //clientSocket is created when connection is made
    char recvBuffer[BUF_LEN];   //buffer that contains word to be checked
    recvBuffer[0] = '\0';       //null terminates the buffer
    
    if(argc == 2){  //sets up dictionary, if specified
        listenport = atoi(argv[1]);
        printf("Port for listening is now %d\n" , listenport);
    }
    else if(argc > 2){
        listenport = atoi(argv[1]);
        printf("Port for listening is now %d\n" , listenport);
        dict = argv[2];
        printf("Using dictionary in %s\n" , dict);
    }
    if(listenport < 1024 || listenport > 65535){
        printf("Port number is either too low(below 1024), or too high(above 65535).\n");
        return -1;
    }
    
    //binds socket to listening port and begins listening for connection
    listenSocket = open_listenfd(listenport);
    if(listenSocket == -1){
        printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
        return -1;
    }
    
    //accept() waits until a user connects to the server, writing information about that server
    //into the sockaddr_in client.
    //If the connection is successful, we obtain A SECOND socket descriptor. 
    //There are two socket descriptors being used now:
    //One by the server to listen for incoming connections.
    //The second that was just created that will be used to communicate with 
    //the connected user.
    if((clientSocket = accept(listenSocket, (struct sockaddr*)&client, &clientLen)) == -1){
        printf("Error connecting to client.\n");
        return -1;
    }
    printf("Connection success!\n");//server-side

    //prompt the client with welcome and instructions
    char* clientMessage = "Connection established.\n";
    char* msgRequest = "Enter a word to be spell checked."
    "\nSend the escape key to close the connection.\n";
    send(clientSocket, clientMessage, strlen(clientMessage), 0);
    send(clientSocket, msgRequest, strlen(msgRequest), 0);
    
    char* msgPrompt = ">>";
    char* msgError = "I didn't get your message. ):\n";
    char* msgClose = "Goodbye!\n";
    //do next part in infinite while loop with client
    
    return (EXIT_SUCCESS);
}

/**
 * initializes buf struct with 2 circular queues
 * @param sp pointer to struct
 * @param job_len length of job queue
 * @param log_len length of log queue
 */
void buf_init(buf *sp, int job_len, int log_len){
    //allocate and set values
    
    sp->job_buf = malloc(sizeof(int) * job_len);
    sp->log_buf = malloc(sizeof(char*) * log_len); 

    sp->job_len = job_len;
    sp->log_len = log_len;
    //set up circular queues
    sp->job_front = 0;
    sp->job_rear = -1;
    sp->job_count = 0;
    sp->log_front = 0;
    sp->log_rear = -1;
    sp->log_count = 0;
    //will most likely need to set condition variables here as well
}

/**
 * frees memory from buf object 
 * @param sp pointer to object of type buf
 */
void buf_deinit(buf *sp){
    free(sp->job_buf);
    free(sp->log_buf);
    free(sp);
}

/**
 * inserts a string into the log queue 
 * (DO CAPACITY CHECK AT THREAD-LEVEL)
 * @param sp pointer to buf object
 * @param item string to be added
 */
void buf_insert_log(buf *sp, char* item){
    sp->log_rear = (sp->log_rear + 1) % sp->log_len;
    sp->log_buf[sp->log_rear] = item;
    sp->log_count ++;
}

/**
 * inserts a job into the job queue
 * (DO CAPACITY CHECK AT THREAD-LEVEL)
 * @param sp pointer to buf object
 * @param item int to be added
 */
void buf_insert_job(buf *sp, int item){
    sp->job_rear = (sp->job_rear + 1) % sp->job_len;    //make space
    sp->job_buf[sp->job_rear] = item;   //set value
    sp->job_count ++;                   //increase count
}

/**
 * removes a log entry from the front of the queue
 * @param sp pointer to buf object
 * @param out_buf buffer for removed object
 * @return 1 on success, 0 on failure
 */
void buf_remove_log(buf *sp, char** out_buf){
    printf("value is |%s|\n" , sp->log_buf[sp->log_front]);
    
    *out_buf = sp->log_buf[sp->log_front];   //gets value from front
    sp->log_front = (sp->log_front + 1) % sp->log_len;  //move front up one
    sp->log_count --;       //decrease count by 1
}

/**
 * removes a job entry from the front of the queue
 * @param sp pointer to buf object
 * @return jobno on success, -1 on failure
 */
int buf_remove_job(buf *sp){
    int returnval = -1;  //failure by default, gets changed on success
    returnval = sp->job_buf[sp->job_front];   //gets value from front
    sp->job_front = (sp->job_front + 1) % sp->job_len;  //move front up one
    sp->job_count --;       //decrease count by 1
    return returnval;
}