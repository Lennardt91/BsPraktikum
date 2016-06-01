FLAGS = -std=c99 -Wall -Wconversion -Wextra -g -m64 -D_XOPEN_SOURCE=600
ECHODATEIEN = echoall.h echoall.c
SHMTESTDATEIEN = shm_test.h shm_test.c
OSMPDATEIEN = osmp.h osmp_run.c

all:
	colorgcc -o echoall $(FLAGS) $(ECHODATEIEN) 
	colorgcc -o shmtest $(FLAGS) osmp.h osmp.c $(SHMTESTDATEIEN) 
	colorgcc -o osmprun $(FLAGS) $(OSMPDATEIEN)
