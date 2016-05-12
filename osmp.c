
/*
 * file: osmp.c 
 * author: Lennardt Grommel
 *         Nikolas Sandmann
 *
 *
 *
 */
#include "osmp.h"

int child_msg_inc(int num){
    return num<OSMP_MAX_MESSAGES_PROC ? num++ : 0;
}

int msg_inc(int num){
    return num<OSMP_MAX_SLOTS ? num++ : 0;
}
int Rank=-1;
void *shm_ptr=NULL;

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
    // shm einrichten
    int shm_id =shmget(shm_key,OSMP_SHM_SIZE,IPC_PRIVATE);//?
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
    pid_t pid = getpid();
    int i;
    for(i=0;i<shmhead->size;i++)
        if(pid==shmhead->child_block[i].pid){
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
    if(count>OSMP_MAX_PAYLOAD_LENGTH || count <= 0)
	return OSMP_ERROR;
    shm_header *shmhead = shm_ptr;
    if(shmhead->free_msg==shmhead->closed_msg){
        return OSMP_ERROR;
    }
    if(child_msg_inc(shmhead->child_block[dest].send_msg)==shmhead->child_block[dest].rcv_msg){
        return OSMP_ERROR;
    }
    
    int i=shmhead->msg_num[shmhead->free_msg];
    
    shmhead->free_msg=msg_inc(shmhead->free_msg);
    
    shmhead->child_block[dest].rcv_msg_num[shmhead->child_block[dest].send_msg]=i;
    shmhead->child_block[dest].send_msg=child_msg_inc(shmhead->child_block[dest].send_msg);
    shmhead->msg[i].source=Rank;
    shmhead->msg[i].size=count;
    int d;
    for(d=0;d<count;d++)
	shmhead->msg[i].msg[d]=((char*)buf)[d];
    
    return OSMP_SUCCESS;
}

int OSMP_Recv (void *buf, int count, int *source, int *len)
{
    shm_header *shmhead = shm_ptr;
    if(shmhead->child_block[Rank].send_msg==shmhead->child_block[Rank].rcv_msg){
        return OSMP_ERROR;
    }
    int i=shmhead->child_block[Rank].rcv_msg_num[shmhead->child_block[Rank].rcv_msg];
    
    shmhead->child_block[Rank].rcv_msg=child_msg_inc(shmhead->child_block[Rank].rcv_msg);
    
    *source=shmhead->msg[i].source;
    *len=shmhead->msg[i].size;
    int d;
    for(d=0;d<count;d++)
	((char*)buf)[d] = shmhead->msg[i].msg[d];
    shmhead->msg_num[shmhead->closed_msg]=i;
    shmhead->closed_msg=msg_inc(shmhead->closed_msg);
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

