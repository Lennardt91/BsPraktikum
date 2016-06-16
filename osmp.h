 /*
 * FILE: osmp.h
 * DESCRIPTION:
 * 	Init of (Interface-) Functions and RETURN_VALUES
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
/*
 * Define Return Values
 */
#ifndef RETURN_VALUES
#define RETURN_VALUES

#define OSMP_SUCCESS 0
#define OSMP_ERROR -1
#define OSMP_SHM_PROJ_ID 5
#define OSMP_SHM_SIZE (200*1024)
#define OSMP_MAX_MESSAGES_PROC 16
#define OSMP_MAX_SLOTS 256
#define OSMP_MAX_PAYLOAD_LENGTH 128

#endif
typedef struct{
    int source;
    int size;
    int next;
    char msg[OSMP_MAX_PAYLOAD_LENGTH];
} shm_msg;

typedef struct{
    pid_t pid;
    int rcv_queue_start;
    int rcv_queue_end;
} shm_child_block;

typedef struct{
    int size;
    int free_queue_start;
    int free_queue_end;
    int msg_offset;
    int cb_offset;
} shm_header;

/*
 * Initialization of Interface Funktions
 */

int OSMP_Init (int *argc, char ***argv);
int OSMP_Size (int *size);
int OSMP_Rank (int *rank);
int OSMP_Send (const void *buf, int count, int dest);
int OSMP_Recv (void *buf, int count, int *source, int *len);
int OSMP_Finalize (void);


