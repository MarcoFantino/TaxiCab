#include "PHead.h"

int woke = 0;

void my_handler(int sig){
	kill(-getpid(), SIGUSR1);
}

void wake(int sig){
	woke++;
}

void richiesta(int sig){
	struct strada* p;
	int shm,i,j;
	/*ricava l'id della shm1*/
	if((shm=shmget(SHM1, 0, 0)) == -1)
		EXIT_ON_ERROR
	/*si attacca alla matrice in mem condivisa*/
	if((p = (struct strada*)shmat(shm, NULL , 0)) == (void *)-1)
		EXIT_ON_ERROR
	for(i=0; i<SO_HEIGHT; i++){
		for(j=0; j<SO_WIDTH; j++){
			if(p[(i*SO_WIDTH)+j].sorgente > 0){
				kill(p[(i*SO_WIDTH)+j].sorgente, SIGUSR2);
			}
		}
	}
	/*stacca il puntatore alla memoria */
	if(shmdt(p) == -1)
		EXIT_ON_ERROR
	printf("SEGNALE FINITO \n");
}

int main(int argc , char* argv[]){
	int i,j,shmId1,shmId2,shmId3,TsemId,semId,SsemId,msgId,pid,proc;
	struct request* start;
	struct strada* mappa;
	int* statistiche;
	struct mymsg mss;
	struct sigaction action1, action2, action3;
	
	action1.sa_handler = wake;
	sigemptyset (&action1.sa_mask);
	action1.sa_flags = 0;
	
	action2.sa_handler = my_handler;
	sigemptyset (&action2.sa_mask);
	action2.sa_flags = 0;
	
	action3.sa_handler = richiesta;
	sigemptyset (&action3.sa_mask);
	action3.sa_flags = 0;
	/*gestisce il segnale*/
	sigaction(SIGALRM, &action2, NULL);
	
	/*prende i dati*/
	getData();
	if(SO_SOURCES > (SO_WIDTH*SO_HEIGHT)-SO_HOLES) {
		printf("troppe sorgenti per questa mappa\n");
		return 0;
	}
	if(SO_TAXI > ((SO_WIDTH*SO_HEIGHT)-SO_HOLES)*SO_CAP_MIN) {
		printf("troppi taxi per questa mappa\n");
		return 0;
	}
	if(SO_HOLES > (SO_WIDTH*SO_HEIGHT)-SO_SOURCES) {
		printf("troppi buchi per questa mappa\n");
		return 0;
	}
	/*creo la matrice in shm1*/
	if((shmId1=shmget(SHM1 , sizeof(struct strada)*(SO_HEIGHT*SO_WIDTH), IPC_CREAT | 0644)) == -1)
		EXIT_ON_ERROR
	/*attacca la memoria condivisa 1*/
	if((mappa = (struct strada*)shmat(shmId1, NULL , 0)) == (void *)-1)
		EXIT_ON_ERROR
	/* creo un array di struct request in shm2*/
	if((shmId2=shmget(SHM2, sizeof(struct request)*SO_SOURCES, IPC_CREAT | 0644)) == -1)
		EXIT_ON_ERROR
	/*inizializza la mappa*/
	creazioneM(mappa , SO_HEIGHT , SO_WIDTH);
	/*stampa per controllo bordi, silenziata*/
	/*Stamp(mappa, 1);*/
	/*crea i buchi */
	if(holesGen(mappa , SO_HOLES , SO_HEIGHT , SO_WIDTH)==1) {/*se fallisce la bucatura mappa elimino le memorie create*/
		
		shmctl(shmId1 , IPC_RMID ,NULL);
		
		shmctl(shmId2 , IPC_RMID ,NULL);
		return 0;
	}
	printf("Mappa bucata con successo \n");
	/* creo un array di int statistiche in shm3*/
	if((shmId3=shmget(SHM3, sizeof(int)*10, IPC_CREAT | 0644)) == -1)
		EXIT_ON_ERROR
	/*attacca la memoria condivisa 3*/
	if((statistiche = (int*)shmat(shmId3, NULL , 0)) == (void *)-1)
		EXIT_ON_ERROR
	/*inizializza i valori dell'array a 0*/
	for(i=0; i<9; i++){
		statistiche[i]=0;
	}
	
	/*crea semaforo per scrivere nell'array delle statistiche*/
	if((SsemId=semget(SSEMKEY, 1, IPC_CREAT | IPC_EXCL | 0644)) == -1)
		EXIT_ON_ERROR
		
	/*inizializza semaforo delle statistiche a 1*/
	initSemAvailable(SsemId, 0,1);
	
	/*crea semaforo per il traffico*/
	if((TsemId=semget(TSEMKEY, (SO_HEIGHT*SO_WIDTH), IPC_CREAT | IPC_EXCL | 0644)) == -1)
		EXIT_ON_ERROR
		
	/*inizializzo i valori dei vari semfari relativi ad ogni cella della mappa*/
	trafficSemIn(mappa, SO_HEIGHT , SO_WIDTH);
	printf("Capacita' delle varie celle \n");
	Stamp(mappa, 2);
	printf("Posizione dei buchi \n");
	Stamp(mappa, 0);
	
	/*crea semaforo per far partire il gioco */
	if((semId=semget(SEMKEY, 1, IPC_CREAT | IPC_EXCL | 0644)) == -1)
		EXIT_ON_ERROR
		
	/*inizializza semaforo a */
	initSemInUse(semId, 0);
	/*creo coda di messaggi*/
	if((msgId = msgget(MSGKEY, IPC_CREAT | IPC_EXCL | 0666))==-1)
		EXIT_ON_ERROR
 
	sigaction(SIGUSR1, &action1, NULL);
	/*genera le SO_SOURCES CELLE DI PARTENZA*/
	stationsGen(mappa , SO_SOURCES);
	if(woke!=1)
		pause();
	creaTaxi(mappa, SO_TAXI);
	printf("aspetto tutti i taxi prima dello start...\n");
	if(woke!=2)
		pause();
	printf("START\n");
	alarm(SO_DURATION);
	
	releaseSem(semId, 0);
	/*inizializzo processo master per stampare la mappa e fermarsi con l'alarm*/
	switch(pid = fork()) {
		case -1:
			EXIT_ON_ERROR
		case 0: /*processo master*/
			sigaction(SIGUSR2, &action3, NULL);
			for(i=0; ; i++){
				printf("Master %d \n" , getpid());
				Stamp(mappa, 0);
				sleep(1);
			}
		default:
			break;
	}
	pause();
	printf("endgame\n");
	kill(pid, SIGKILL);
	printf("Mappa con buchi e sorgenti evidenziati:\n");
	Stamp(mappa, 3);
	printf("Mappa con evidenziate le %d celle piu' attraversate", SO_TOP_CELLS);
	Stamp(mappa, 4);
	statistiche[1]=getInevasi(mss, msgId);
	printf("Numero viaggi eseguiti con successo: %d \n", statistiche[0]);
	printf("Numero viaggi inevasi: %d \n", statistiche[1]);
	printf("Numero viaggi abortiti: %d \n", statistiche[2]);
	printf("Numero di volte in cui un taxi è esploso per troppa attesa ad una sorgente: %d \n", statistiche[9]);
	printf("Il taxi %d ha fatto più strada di tutti con %d strade attraversate \n", statistiche[3], statistiche[4]);
	printf("Il taxi %d ci ha messo più tempo di tutti a eseguire una richiesta con %d secondi impiegati\n", statistiche[5], statistiche[6]);
	printf("Il taxi %d ha raccolto più richieste di tutti, ben %d! \n", statistiche[7], statistiche[8]);
	/*elimina memoria1*/
	shmctl(shmId1 , IPC_RMID ,NULL);
	/*elimina memoria2*/
	shmctl(shmId2 , IPC_RMID ,NULL);
	/*elimina memoria3*/
	shmctl(shmId3 , IPC_RMID ,NULL);
	/*elimina semaforo*/
	semctl(semId,0, IPC_RMID, NULL);
	/*elimina la coda di messaggi*/
	msgctl(msgId, IPC_RMID, NULL);
	/*elimina semafori per traffico*/
	semctl(TsemId,0, IPC_RMID, NULL);
	/*elimina semafori per statistiche*/
	semctl(SsemId,0, IPC_RMID, NULL);
	printf("memorie staccate\n");
	proc = ((SO_TAXI+SO_SOURCES)/500)+1;
	sleep(proc);
	while(wait(NULL)>0);
	return 0;
}
