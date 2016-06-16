
/*
 * file: osmp.c 
 * author: Lennardt Grommel
 *         Nikolas Sandmann
 *
 *
 *
 */
#include "osmp.h"
#include "osmp_sem.h"
int Rank=-1;
void *shm_ptr=NULL;
#define MS 0
#define S0 1
int sem_recv(int rank){
    return rank*2+2;
}

int sem_send(int rank){
    return rank*2+3;
}

int OSMP_Init (int *argc, char ***argv)
{
    // shm_key generieren nach dateinamen des aufzurufenden programms
    if(*argc<1){
	printf("argc<1");
	return OSMP_ERROR;    
    }
    key_t shm_key=ftok(*argv[0],OSMP_SHM_PROJ_ID);
    if(shm_key==-1){
        perror("ftok error");
        return OSMP_ERROR;
    }
    
    
    // einrichten der semaphore
    osmp_sem_init(shm_key);
    
    // shm einrichten
    int shm_id =shmget(shm_key,0,IPC_PRIVATE);//?
    if(shm_id==-1){
        perror("shmget error");
        return OSMP_ERROR;
    }
    shm_ptr = shmat(shm_id,NULL,0);
    if(shm_ptr==(void*)-1){
        perror("shmat error");
        return OSMP_ERROR;
    }
    shm_header *shmhead = shm_ptr;
    shm_child_block *shm_cb = shm_ptr + shmhead->cb_offset;
    pid_t pid = getpid();
    int i;
    for(i=0;i<shmhead->size;i++)
        if(pid==shm_cb[i].pid){
            Rank=i;
            break;
        }
    return OSMP_SUCCESS;
}

int OSMP_Size (int *size)
{
    shm_header *shmhead = shm_ptr;
    *size=shmhead->size;
    return OSMP_SUCCESS;
}

int OSMP_Rank (int *rank)
{
    if(Rank==-1)
        return OSMP_ERROR;
    *rank=Rank;
    return OSMP_SUCCESS;
}


int OSMP_Send (const void *buf, int count, int dest)
{
    shm_header *shmhead = shm_ptr;
    if(buf==NULL || count < 1 || dest >= shmhead->size)
        return OSMP_ERROR;
    osmp_sem_wait(sem_send(dest));
    osmp_sem_wait(S0);
    osmp_sem_wait(MS);
    //Slot holen
    int offset = shmhead->free_queue_start;
    if(shmhead->free_queue_end == offset)
        shmhead->free_queue_end = -1;
    shm_msg *shm_ms = shm_ptr + offset;
    shmhead->free_queue_start = shm_ms->next;
    
    osmp_sem_signal(MS);
    //Narchicht kopieren
    shm_ms->source=Rank;
    shm_ms->size=count;
    shm_ms->next=-1;
    char *msg = (char*)&(shm_ms->msg);
    int i;
    for(i=0;i<count;i++)
        msg[i]=((char*)buf)[i];
    
    osmp_sem_wait(MS);
    //Slot in Recv
    shm_child_block *shm_cb = shm_ptr + shmhead->cb_offset;
    if(shm_cb[dest].rcv_queue_end != -1){
        shm_msg *shm_ms_last = shm_ptr + shm_cb[dest].rcv_queue_end;
        shm_ms_last->next = offset;
    }
    shm_cb[dest].rcv_queue_end = offset;
    if(shm_cb[dest].rcv_queue_start == -1)
        shm_cb[dest].rcv_queue_start = offset;
    
    osmp_sem_signal(MS);
    osmp_sem_signal(sem_recv(dest));
    return OSMP_SUCCESS;
}

int OSMP_Recv (void *buf, int count, int *source, int *len)
{
    osmp_sem_wait(sem_recv(Rank));
    osmp_sem_wait(MS);
    //Slot holen
    shm_header *shmhead = shm_ptr;
    shm_child_block *shm_cb = shm_ptr + shmhead->cb_offset;
    int offset = shm_cb[Rank].rcv_queue_start;
    shm_msg *shm_ms = shm_ptr + offset;
    if(count < shm_ms->size){
        osmp_sem_signal(sem_recv(Rank));
        osmp_sem_signal(MS);
        *len = shm_ms->size;
        return OSMP_ERROR;
    }
    if(shm_cb[Rank].rcv_queue_end == offset)
        shm_cb[Rank].rcv_queue_end=-1;
    shm_cb[Rank].rcv_queue_start = shm_ms->next;
    
    osmp_sem_signal(MS);
    //kopieren
    shm_ms->next=-1;
    *source = shm_ms->source;
    *len = shm_ms->size;
    char *msg = (char*)&(shm_ms->msg);
    int i;
    for(i=0;i<*len;i++)
        ((char*)buf)[i]=msg[i];
    
    
    osmp_sem_wait(MS);
    //Slot zurückgeben
    if(shmhead->free_queue_end != -1){
        shm_msg *shm_ms_last = shm_ptr + shmhead->free_queue_end;
        shm_ms_last->next=offset;
    }
    else{
        shmhead->free_queue_start = offset;
    }
    shmhead->free_queue_end = offset;

    
    osmp_sem_signal(MS);
    osmp_sem_signal(sem_send(Rank));
    osmp_sem_signal(S0);
    return OSMP_SUCCESS;
}

int OSMP_Finalize (void)
{
    if(shmdt(shm_ptr)==-1){
        perror("shmdt error");
        return OSMP_ERROR;
    }
    return OSMP_SUCCESS;
}

