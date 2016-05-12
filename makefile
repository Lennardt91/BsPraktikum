FLAGS = -std=c99 -Wall -Wconversion -Wextra -g -m64
ECHODATEIEN = echoall.h echoall.c
SHMTESTDATEIEN = shm_test.h shm_test.c
OSMPDATEIEN = osmp.h osmp_run.c

all:
	gcc -o echoall $(FLAGS) $(ECHODATEIEN) 
	gcc -o shmtest $(FLAGS) osmp.h osmp.c $(SHMTESTDATEIEN) 
	gcc -o osmprun $(FLAGS) $(OSMPDATEIEN)
