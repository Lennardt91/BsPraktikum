/*
 * file: osmp_run.c 
 * author: Lennardt Grommel
 *         Nikolas Sandmann
 *
 *
 * Startet einen Kindprozess, der durch ./echoall ersetzt wird und gibt danach seine eingene PID aus
 */

#include "osmp.h"
#include "osmp_sem.h"
pid_t createChild(char *argv[], int rank);

int main (int argc, char *argv[]){
    if(argc<3){
        printf("%s <Anzahl Kindprozesse> <Auszufuehrendes Programm> (<Parameter fuer das Programm>)\n",argv[0]);
        exit(OSMP_ERROR);
    }
    
    int child_process_count=atoi(argv[1]);
    pid_t child_pid[child_process_count];
    int i;
    
    // shm_key generieren nach dateinamen des aufzurufenden programms
    key_t shm_key=ftok(argv[2], OSMP_SHM_PROJ_ID);
    if(shm_key==-1){
        perror("ftok error");
        exit(OSMP_ERROR);
    }
    // shm einrichten
    int shm_size = (int)sizeof(shm_header)+
                    child_process_count*(int)sizeof(shm_child_block)+
                    OSMP_MAX_SLOTS*(int)sizeof(shm_msg);
    int shm_id =shmget(shm_key,/*shm_size*/OSMP_SHM_SIZE, 0660 | IPC_CREAT);
    if(shm_id==-1){
        perror("shmget error");
        exit(OSMP_ERROR);
    }
    // einrichten des shm_header
    shm_header *shm_ptr = shmat(shm_id,(void *)0,0);
    if(shm_ptr==(void *)-1){
        perror("shmat error");
        return OSMP_ERROR;
    }
    
    shm_ptr->size = child_process_count;
    shm_ptr->cb_offset = sizeof(shm_header);
    shm_ptr->msg_offset = (int)sizeof(shm_header) + child_process_count * (int)sizeof(shm_child_block);
    shm_child_block *shm_cb = (void*)shm_ptr + shm_ptr->cb_offset;
    shm_msg *shm_ms = (void*)shm_ptr + shm_ptr->msg_offset;
    
    shm_ptr->free_queue_start = shm_ptr->msg_offset;
    shm_ptr->free_queue_end = shm_ptr->msg_offset + (OSMP_MAX_SLOTS-1) * (int)sizeof(shm_msg);
    
    for(i=0;i<OSMP_MAX_SLOTS;i++){
        shm_ms[i].next= shm_ptr->msg_offset + (i+1) * (int)sizeof(shm_msg);
    }
    
    shm_ms[OSMP_MAX_SLOTS-1].next = -1;
    
    /*printf("shm_ptr:%x\n",(void*)shm_ptr);
    printf("cb_offset:%i\n",shm_ptr->cb_offset);
    printf("msg_offset:%i\n",shm_ptr->msg_offset);
    printf("shm_cb:%x\n",(void*)shm_cb);
    printf("shm_ms:%x\n",(void*)shm_ms);*/
    
    // einrichten der semaphore
    
    int semid = semget(shm_key, 2*child_process_count+2, IPC_CREAT | 0640);
    semctl(semid, 0, SETVAL, 1);
    semctl(semid, 1, SETVAL, OSMP_MAX_SLOTS);
    for(i = 0; i<child_process_count;i++){
        semctl(semid, 2*i+2, SETVAL,0);
        semctl(semid, 2*i+3, SETVAL,OSMP_MAX_MESSAGES_PROC);
    }
    osmp_sem_init(shm_key);
    
    //starten der Kindprozesse
    for(i = 0; i<child_process_count; i++){
        shm_cb[i].rcv_queue_start=-1;
        shm_cb[i].rcv_queue_end=-1;;
        child_pid[i]=createChild(&argv[2], i);
        shm_cb[i].pid=child_pid[i];
    }
    
    for(i = 0; i<child_process_count; i++){
        osmp_sem_signal(i*2+2);
    }
    
    if(shmdt(shm_ptr)==-1){
        perror("shmdt error");
        return OSMP_ERROR;
    }
    
    // warten auf Kindprozesse
    for(i = 0; i<child_process_count; i++){
        if( (waitpid(child_pid[i],NULL,WCONTINUED) == -1) ){
	perror("waitpid() error");
	exit(OSMP_ERROR);
	}
    }
    // semaphore löschen
    semctl(semid,0,IPC_RMID);
    
    // shm löschen
    if(shmctl(shm_id,IPC_RMID,NULL)==-1){
        perror("shmget error");
        exit(OSMP_ERROR);
    }
    
    //printf("ende\n");
    return OSMP_SUCCESS;
}

pid_t createChild(char *childArgv[], int rank){
    pid_t pid =fork();
    if ( pid == -1 ){
        perror("fork()");
        exit(OSMP_ERROR);
    }
    if ( pid == 0 ){
        osmp_sem_wait(rank*2+2);
        execvp(childArgv[0], childArgv);
        perror("exec error");
        exit(OSMP_ERROR);
    }
    return pid;
    
}
