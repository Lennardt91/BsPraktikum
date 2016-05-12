/*
 * author: Lennardt Grommel
 *         Nikolas Sandmann
 * file: echoall.c
 *
 * Gibt seine Prozess ID und alle Argumente aus, die es bekommen hat.
 */
#include "echoall.h"

int main ( int argc, char* argv[]){
    int i;
    pid_t pid;
    pid = getpid();
    printf("PROZESS ID:%i\n", pid);
    printf("argc: %i\n", argc);
    for (i = 0; i< argc; i++){
        printf("Argument %i:%s\n", i+1, argv[i]);
    }
    sleep(5);
    return 0;
}
