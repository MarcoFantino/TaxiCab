#include "PHead.h"

int endmap=1;
int inevasi=0;

void stopSource(int sig){
	endmap=0;
}

void request_handler(int sig){
	printf("Genero una richiesta tramite segnale \n");
}

/*crea la mappa*/
void creazioneM(struct strada mappa[], int rows , int cols){
	int i , j;
	time_t t;
	srand((unsigned) time(&t));
	/*ciclo per assegnare dei valori ad ogni struct all'interno della matrice*/
	for(i=0;i<rows ;i++){
		for(j=0;j<cols;j++){
			mappa[(i*cols)+j].T_attr = rand() % ((SO_TIME_SEC_MAX+1)- SO_TIME_SEC_MIN)+(SO_TIME_SEC_MIN);
			mappa[(i*cols)+j].C_attr = rand() % ((SO_CAP_MAX+1)- SO_CAP_MIN)+(SO_CAP_MIN);
			mappa[(i*cols)+j].flag_a = 0;
			mappa[(i*cols)+j].flag_v = 0;
			mappa[(i*cols)+j].taxi = 0;
			mappa[(i*cols)+j].sorgente = 0;
			mappa[(i*cols)+j].flag_req = 0;
			mappa[(i*cols)+j].passaggi = 0;
			if(i==0 && j==0) { 					/*primo angolo*/
				mappa[(i*cols)+j].bordo=1;
			}
			else if(i==0 && j==cols-1) {		/*secondo angolo*/
				mappa[(i*cols)+j].bordo=3;
			}
			else if(i==rows-1 && j==cols-1) {	/*terzo angolo*/
				mappa[(i*cols)+j].bordo=5;
			}
			else if(i==rows-1 && j==0) {		/*quarto angolo*/
				mappa[(i*cols)+j].bordo=7;
			}
			else if(i==0) {						/*prima riga*/
				mappa[(i*cols)+j].bordo=2;
			}
			else if(j==cols-1) {				/*ultima colonna*/
				mappa[(i*cols)+j].bordo=4;
			}
			else if(i==rows-1) {				/*ultima riga*/
				mappa[(i*cols)+j].bordo=6;
			}		
			else if(j==0) {						/*prima colonna*/
				mappa[(i*cols)+j].bordo=8;
			}
			else
				mappa[(i*cols)+j].bordo = 0;
		}
	}
}
/*  1 2 2 2 2 2 3
	8 0 0 0 0 0 4
	8 0 0 0 0 0 4
	8 0 0 0 0 0 4
	7 6 6 6 6 6 5 */

/*genera i buchi */
int holesGen(struct strada mappa[], int holes , int rows , int cols){
	time_t t;
	int i , j , k , maxCap, n;
	srand((unsigned) time(&t));
	maxCap = rows * cols;
	for(n = 0; n<31; n++) {
		if(n==30) {
			printf("Non e' stato possibile inserire %d in questa mappa\n", holes);
			return 1;
		}
		for(k=0 ; k<holes ; k++){
			/*controllo che il numero di celle disponibili non sia inferiore al numero di buchi*/
			if(maxCap < k){
				printf("Tentativo %d fallito\n", n+1);
				maxCap = rows * cols;
				for(i=0;i<rows ;i++){
					for(j=0;j<cols;j++){
						mappa[(i*SO_WIDTH)+j].flag_a=0;
						mappa[(i*SO_WIDTH)+j].flag_v=0;
					}
				}
				break;
			}
			/*scelgo una cordinata a caso*/
			i = rand()%(rows);
			j = rand()%(cols);
			/*controllo che non sia ne un hole ne un contorno*/
			if(mappa[(i*SO_WIDTH)+j].flag_a!=1 && mappa[(i*SO_WIDTH)+j].flag_v!=1){
				printf("strada bloccata [%d][%d] \n" , i , j);
				mappa[(i*SO_WIDTH)+j].flag_a=1;
				/*cella in mezzo*/
				if((i>0 && i<rows-1) && (j>0 && j<cols-1)){
					mappa[((i+1)*SO_WIDTH)+j].flag_v=1;
					mappa[((i-1)*SO_WIDTH)+j].flag_v=1;
					mappa[(i*SO_WIDTH)+j+1].flag_v=1;
					mappa[(i*SO_WIDTH)+j-1].flag_v=1;
					mappa[((i+1)*SO_WIDTH)+j-1].flag_v=1;
					mappa[((i-1)*SO_WIDTH)+j-1].flag_v=1;
					mappa[((i+1)*SO_WIDTH)+j+1].flag_v=1;
					mappa[((i-1)*SO_WIDTH)+j+1].flag_v=1;
					maxCap -= 9;
				/*cella in un angolo*/
				}else if((i==0 || i ==rows-1) && (j==0 || j ==cols-1)){
					maxCap -= 4;
					switch(i){
						case 0 :
							if(j==0){
								mappa[(i*SO_WIDTH)+j+1].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j+1].flag_v=1;
							}else{	
								mappa[(i*SO_WIDTH)+j-1].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j-1].flag_v=1;
							}
							break;
							
						default :
							if(j==0){	
								mappa[(i*SO_WIDTH)+j+1].flag_v=1;
								mappa[((i-1)*SO_WIDTH)+j].flag_v=1;
								mappa[((i-1)*SO_WIDTH)+j+1].flag_v=1;
							}else{	
								mappa[(i*SO_WIDTH)+j-1].flag_v=1;
								mappa[((i-1)*SO_WIDTH)+j].flag_v=1;
								mappa[((i-1)*SO_WIDTH)+j-1].flag_v=1;
							}
							break;		
					}	
				/*cella in un bordo*/
				}else{
					maxCap -= 6;
					if(i==0 || i==rows-1){
						mappa[(i*SO_WIDTH)+j-1].flag_v=1;
						mappa[(i*SO_WIDTH)+j+1].flag_v=1;	
						switch(i){
							case 0 :
								mappa[((i+1)*SO_WIDTH)+j-1].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j+1].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j].flag_v=1;
								break;
							default :
								mappa[((i-1)*SO_WIDTH)+j-1].flag_v=1;
								mappa[((i-1)*SO_WIDTH)+j+1].flag_v=1;
								mappa[((i-1)*SO_WIDTH)+j].flag_v=1;
								break;
						}
					}else{
						mappa[((i-1)*SO_WIDTH)+j].flag_v=1;
						mappa[((i+1)*SO_WIDTH)+j].flag_v=1;
						switch(j){
							case 0 :
								mappa[((i-1)*SO_WIDTH)+j+1].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j+1].flag_v=1;
								mappa[(i*SO_WIDTH)+j+1].flag_v=1;
								break;
							default :
								mappa[((i-1)*SO_WIDTH)+j-1].flag_v=1;
								mappa[((i+1)*SO_WIDTH)+j-1].flag_v=1;
								mappa[(i*SO_WIDTH)+j-1].flag_v=1;
								break;
						}
					}
				}
			}else{
				printf("Strada [%d][%d] non eleggibile \n" , i , j);
				k--;
			}
		if((k+1)==holes)
			return 0;
		}
	}
}

void trafficSemIn(struct strada mappa[], int rows , int cols){
	int TsemId, i, j;
	/*mi collego al set di semafori*/
	if((TsemId=semget(TSEMKEY, 0 , 0)) == -1)
					EXIT_ON_ERROR
	for(i=0; i<rows; i++){
		for(j=0; j<cols; j++){
			if(mappa[(i*SO_WIDTH)+j].flag_a!=1){
				/*inizializzo il semaforo al valore C_attr segnato nella cella */
				if((initSemAvailable(TsemId, (i*SO_WIDTH)+j, mappa[(i*SO_WIDTH)+j].C_attr))== -1 )
					EXIT_ON_ERROR
			}
		}
	}
}
/*determina le SO_SOURCES celle di partenza e fa partire i processi generatori di richieste */
void stationsGen(struct strada mappa[], int n_req){
	time_t t;
	int i , j , k , msgId, semId,shmId2, pid, n, xfin, yfin;
	struct mymsg mss;
	struct request* start;
	struct timespec my_time;
	struct sigaction action1, action2;
	srand((unsigned) time(&t));
	/*mi collego al semaforo*/
	if((semId=semget(SEMKEY, 0, 0)) == -1)
						EXIT_ON_ERROR
	/*mi collego alla coda di messaggi*/
	if((msgId = msgget(MSGKEY, 0))==-1)
		EXIT_ON_ERROR
	/*ricava l'id della shm2*/
	if((shmId2=shmget(SHM2, 0, 0)) == -1)
		EXIT_ON_ERROR
	/*attacca la memoria condivisa 2*/
	if((start = (struct request*)shmat(shmId2, NULL , 0)) == (void *)-1)
		EXIT_ON_ERROR
	for(n = 0; n < SO_SOURCES; n++) {
		i = rand()%SO_HEIGHT;
		j = rand()%SO_WIDTH;
		if(mappa[(i*SO_WIDTH)+j].flag_a != 1 && mappa[(i*SO_WIDTH)+j].sorgente == 0) {
			printf("ho creato una sorgente in [%d][%d], sor n: %d\n", i, j, n);
			start[n].x=i;
			start[n].y=j;
			pid = fork();
			switch(pid) {
				case -1:
					EXIT_ON_ERROR
				case 0:
					srand((unsigned) time(&t)+getpid());
					action1.sa_handler = stopSource;
					sigemptyset (&action1.sa_mask);
					action1.sa_flags = 0;
					
					action2.sa_handler = request_handler;
					sigemptyset (&action2.sa_mask);
					action2.sa_flags = 0;
					
					/*gestisce il segnale*/
					sigaction(SIGUSR1, &action1, NULL);
					sigaction(SIGUSR2, &action2, NULL);
					
					if(n == SO_SOURCES-1)
						kill(getppid(), SIGUSR1);
					/*si blocca aspettando che vengano generati i taxi*/
					reserveSem(semId,0);
					releaseSem(semId, 0);
					while(endmap){
						/*attesa tra una richiesta e l'altra*/
						my_time.tv_sec = 2;
						my_time.tv_nsec = 0;
						nanosleep(&my_time, NULL);
						/*genero una cella finale*/
						xfin=rand()%SO_HEIGHT;
						yfin=rand()%SO_WIDTH;
						while(mappa[(xfin*SO_WIDTH)+yfin].flag_a==1 || ((i==xfin)&&(j==yfin))){
							xfin=rand()%SO_HEIGHT;
							yfin=rand()%SO_WIDTH;
						}
						/*mando il messaggio sulla coda*/
						mss.mytype = getpid();
						mss.array[0]=xfin;
						mss.array[1]=yfin;
						msgsnd(msgId,&mss, sizeof(struct mymsg)-sizeof(long), 0);
					}
					exit(EXIT_SUCCESS);
				default:
					mappa[(i*SO_WIDTH)+j].sorgente=pid;
					printf("io sono il padre: %ld e ho %d figli \n", (long)getpid(), n+1);
					break;
			}
		}
		else {
			--n;
		}
	}
	if(shmdt(start) == -1)
		EXIT_ON_ERROR
}

void Stamp(struct strada mappa[], int tipo) {
	int i, j, s, temp, n;
	temp = 0;
	printf("\n");
	if(tipo == 4) {
		for(s = 0; s < SO_TOP_CELLS; s++) {
			for(n = 0; n < SO_WIDTH*SO_HEIGHT; n++) {
				if(mappa[n].passaggi >= mappa[temp].passaggi) {
					temp = n;
				}
			}
			mappa[temp].passaggi = -1;
		}
		for(i=0;i<SO_HEIGHT ;i++){
			for(j=0;j<SO_WIDTH;j++){
				if(mappa[(i*SO_WIDTH)+j].flag_a==1)
					printf(" O ");
				else if(mappa[(i*SO_WIDTH)+j].passaggi == -1)
					printf(" T ");
				else
					printf(" + ");
			}
			printf("\n\n");
		}
	}
	else if(tipo==2) {
		for(i=0;i<SO_HEIGHT ;i++){
			for(j=0;j<SO_WIDTH;j++){
				printf(" %d ", mappa[(i*SO_WIDTH)+j].C_attr);
			}
			printf("\n\n");
		}
	}
	else if(tipo == 3) {
		for(i=0;i<SO_HEIGHT ;i++){
			for(j=0;j<SO_WIDTH;j++){
				if(mappa[(i*SO_WIDTH)+j].flag_a==1)
					printf(" O ");
				else if(mappa[(i*SO_WIDTH)+j].sorgente > 0) {
					printf(" S ");
				}
				else
					printf(" + ");
			}
			printf("\n\n");
		}
	}
	else {
		for(i=0;i<SO_HEIGHT ;i++){
			for(j=0;j<SO_WIDTH;j++){
				if(mappa[(i*SO_WIDTH)+j].flag_a==1)
					printf(" O ");
				else if(mappa[(i*SO_WIDTH)+j].taxi > 0) 
					printf(" %d ", mappa[(i*SO_WIDTH)+j].taxi);
				else if(mappa[(i*SO_WIDTH)+j].sorgente > 0)
					printf(" S ");
				else if(mappa[(i*SO_WIDTH)+j].bordo > 0 && tipo==1) 
					printf(" %d ", mappa[(i*SO_WIDTH)+j].bordo);
				else printf(" + ");
			}
			printf("\n\n");
		}
	}
}

int getInevasi(struct mymsg mss, int msgId) {
	int msgid, msglength;
	struct msqid_ds buf;
	msgid = msgget(MSGKEY, 0);
	msglength = sizeof(struct mymsg)-sizeof(long);
	msgctl(msgid, IPC_STAT, &buf);
	return buf.msg_qnum;
}
