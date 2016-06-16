 /*
 * FILE: osmp_sem.h
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
#include <semaphore.h>

int osmp_sem_init(key_t key);
int osmp_sem_signal(int semaphore);
int osmp_sem_wait(int semaphore);
