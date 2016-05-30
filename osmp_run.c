/*
 * file: osmp_run.c 
 * author: Lennardt Grommel
 *         Nikolas Sandmann
 *
 *
 * Startet einen Kindprozess, der durch ./echoall ersetzt wird und gibt danach seine eingene PID aus
 */

#include "osmp.h"

pid_t createChild(char *argv[]);

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
    int shm_id =shmget(shm_key,OSMP_SHM_SIZE, 0660 | IPC_CREAT);
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
    shm_ptr->free_msg = 0;
    shm_ptr->closed_msg = OSMP_MAX_SLOTS;
    
    
    for(i=0;i<OSMP_MAX_SLOTS;i++)
	shm_ptr->msg_num[i]=i;
    
    
    // starten der Kindprozesse
    for(i = 0; i<child_process_count; i++){
        shm_ptr->child_block[i].send_msg=0;
        shm_ptr->child_block[i].rcv_msg=0;
        child_pid[i]=createChild(&argv[2]);
        shm_ptr->child_block[i].pid=child_pid[i];
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
    
    // shm löschen
    if(shmctl(shm_id,IPC_RMID,NULL)==-1){
        perror("shmget error");
        exit(OSMP_ERROR);
    }
    
    printf("ende\n");
    return OSMP_SUCCESS;
}

pid_t createChild(char *childArgv[]){
    pid_t pid =fork();
    if ( pid == -1 ){
        perror("fork()");
        exit(OSMP_ERROR);
    }
    if ( pid == 0 ){
        execvp(childArgv[0], childArgv);
        perror("exec error");
        exit(OSMP_ERROR);
    }
    return pid;
    
}

/* int status; 
 * pid_t pid;
 *  pid_t child_pid;
    //char *env_init[] = { "USER=unknown", "PATH=/tmp", NULL };
    char *env[] = { "unknown", "tmp", NULL };
    pid = getpid();
    for(i = 0; i<5; i++){
        child_pid = fork();
        if ( child_pid == -1 ){
            perror("fork()");
            exit(OSMP_ERROR);
        }
        if ( child_pid == 0 ){
            printf("\n_____________________________\n");
            printf("KINDPROZESS\n");
            //execlp("./echoall", "echoall", "Hallo", "EXECLP", NULL); 
            //execle("./echoall", "echoall", "Hallo", "EXECLE", NULL, env_init);
            execvp("./echoall", env);
            
            perror("execlp() error");
            exit(OSMP_ERROR);
        }*/
