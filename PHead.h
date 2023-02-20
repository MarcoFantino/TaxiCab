#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <errno.h>
#include <sys/types.h> 
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/msg.h>

#define EXIT_ON_ERROR if (errno) {fprintf(stderr,  \
              "%d: pid %ld; errno: %d (%s)\n",     \
              __LINE__,                            \
              (long) getpid(),                     \
              errno,                               \
              strerror(errno)); exit(EXIT_FAILURE);}
 
#define SO_HEIGHT 10 /*	Altezza-righe*/
#define SO_WIDTH 20  /*	Larghezza-colonne*/
#define SHM1 67234
#define SHM2 54672
#define SHM3 59243
#define SEMKEY 83563
#define TSEMKEY 733589
#define SSEMKEY 55423
#define MSGKEY 12345

/*variabili globali*/
int SO_TAXI , SO_SOURCES , SO_HOLES , SO_TOP_CELLS , SO_CAP_MIN , SO_CAP_MAX , SO_TIME_SEC_MIN , SO_TIME_SEC_MAX ,
SO_TIMEOUT , SO_DURATION;

int end;


struct mymsg{
	long mytype ; /*tipo del messaggio*/
	int array[2]; /* corpo del messaggio */
};

/*struttura che tiene traccia di quali celle fanno partire una richiesta di taxi*/
struct request{
	int x;
	int y;
};

struct pack{
	int xi;
	int yi;
	int xf;
	int yf;
};

struct strada {
	int T_attr; /*tempo attraversamento*/
	int C_attr;	/*capacità max di taxi*/
	int flag_a; /* indica se la strada e' bloccata*/
	int flag_v; /* indica se e' nelle vicinanze di una strada bloccata*/
	int flag_req; /* indica se e' stata effettuata una richiesta*/
	int sorgente; /* indica il pid del processo associato alla cella*/
	int taxi; /* indica quanti taxi ci sono attualmente nella cella*/
	int bordo;
	int passaggi; /*n di attraversamenti in questa cella*/
};

union semun {
	int val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO                                         (Linux-specific) */
};


/*acquisimento dati da input testuale*/

void getData();

void creazioneM(struct strada mappa[], int rows , int cols);

int holesGen(struct strada mappa[], int holes , int rows , int cols);

void trafficSemIn(struct strada mappa[], int rows , int cols);

void stationsGen(struct strada mappa[], int n_req);

void creaTaxi(struct strada mappa[], int taxi);

int getInevasi(struct mymsg mss, int msgId);

void reqGen(struct strada mappa[]);

void sources();

void Stamp(struct strada mappa[], int tipo);

/* Initialize semaphore to 1 (i.e., "available")*/
int initSemAvailable(int semId, int semNum, int nVal);

/* Initialize semaphore to 0 (i.e., "in use")*/
int initSemInUse(int semId, int semNum);

/* Reserve semaphore - decrement it by 1*/
int reserveSem(int semId, int semNum);

/* Reserve semaphore - decrement it by 1 with timer*/
int reserveSem2(int semId, int semNum);

/*Release semaphore - increment it by 1*/
int releaseSem(int semId , int semNum);











