#include <osmp.h>


int osmp_sem_wait(sem_t semaphore){

  if ( sem_wait(semaphore) == -1 ){
    errorp("Fehler bei sem_wait");
    exit();
  }
  return OSMP_SUCCESS;
}


int osmp_sem_signal(sem_t semaphore){

  if( sem_post(semaphore) == -1 ){
    errorp("Fehler bei sem_post");
    exit();
  }
  return OSMP_SUCCESS;
}
