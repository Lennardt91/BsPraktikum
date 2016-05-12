/*
 * author: Lennardt Grommel
 *         Nikolas Sandmann
 * file: shm_test.c
 *
 * 
 */
#include "shm_test.h"

int main ( int argc, char* argv[]){
    
    int rank=-1,size=-1;
    
    OSMP_Init(&argc, &argv);
    OSMP_Size(&size);
    OSMP_Rank(&rank);
   int i;
    
    pid_t pid;
    pid = getpid();
    printf("PROZESS ID:%i\n", pid);
    printf("size: %i\n", size);
    printf("rank: %i\n", rank);
    printf("argc: %i\n", argc);
    for (i = 0; i< argc; i++){
        printf("Argument %i:%s\n", i+1, argv[i]);
    }
    if(rank==0)
        OSMP_Send("test",sizeof(char)*5,1);
    if(rank==1){
	char str[10];
	int src,len;
	sleep(2);
	OSMP_Recv(str,sizeof(char)*10,&src,&len);
	printf("%s\n",str);
    }
    sleep(5);
    OSMP_Finalize();
    return 0;
}
 
