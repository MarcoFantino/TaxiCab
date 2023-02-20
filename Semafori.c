#include "PHead.h"


/* Initialize semaphore to nVal (i.e., "available")*/
int initSemAvailable(int semId, int semNum, int nVal){
	union semun arg;
	arg.val = nVal;
	return semctl(semId, semNum, SETVAL, arg);
}

/* Initialize semaphore to 0 (i.e., "in use")*/
int initSemInUse(int semId, int semNum){
	union semun arg;
	arg.val = 0;
	return semctl(semId, semNum, SETVAL, arg);
}

/* Reserve semaphore - decrement it by 1*/
int reserveSem(int semId, int semNum){
	struct sembuf sops;
	sops.sem_num = semNum;
	sops.sem_op = -1 ;
	sops.sem_flg = 0;
	return semop(semId , &sops, 1);	
}

/*Release semaphore - increment it by 1*/
int releaseSem(int semId , int semNum){
	struct sembuf sops;
	sops.sem_num = semNum;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	return semop(semId , &sops , 1);
}

int reserveSem2(int semId, int semNum){
	struct sembuf sops;
	struct timespec my_time;
	my_time.tv_sec = SO_TIMEOUT;
	my_time.tv_nsec = 0;
	sops.sem_num = semNum;
	sops.sem_op = -1 ;
	sops.sem_flg = 0;
	return semtimedop(semId , &sops, 1, &my_time);
}
