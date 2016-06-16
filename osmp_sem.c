#include "osmp.h"
#include "osmp_sem.h"


int semid=-1;

int osmp_sem_init(key_t key){
    
    semid = semget(key, 0, 0);
    return OSMP_SUCCESS;
}

int osmp_sem_wait(int semaphore){

    struct sembuf semabuf;
    semabuf.sem_num = (short unsigned int)semaphore;
    semabuf.sem_op = -1;
    semabuf.sem_flg = 0;
    semop(semid,&semabuf,1);
    return OSMP_SUCCESS;
}


int osmp_sem_signal(int semaphore){

    struct sembuf semabuf;
    semabuf.sem_num = (short unsigned int)semaphore;
    semabuf.sem_op = 1;
    semabuf.sem_flg = 0;
    semop(semid,&semabuf,1);
    return OSMP_SUCCESS;
}
