#include "PHead.h"

int MovimentoGiu(struct strada mappa[],int i, int j, int semaforo);
int MovimentoSu(struct strada mappa[],int i, int j, int semaforo);
int MovimentoDx(struct strada mappa[],int i, int j, int semaforo);
int MovimentoSx(struct strada mappa[],int i, int j, int semaforo);
void MovimentoTaxi(struct strada mappa[],int *i, int *j, int x, int y, int semaforo);
int goToS(struct request* sources, int i, int j);
int end=1;
int trueend=1;
int usura=0;
int abortiti=0;
int fail = 0;
struct timespec my_time;

void handlerTaxi(int sig){
	end=0;
	trueend=0;
}

void taxiTimer(int sig) {
	
}

void creaTaxi(struct strada mappa[], int taxi) {
	time_t f, a, b;
	int i , j , k , msgId, semId, TsemId, SsemId, shmId2,shmId3, taxiTot, n;
	struct mymsg mss;
	struct request* start;
	struct sigaction action1, action2;
	int temp;
	/*array contente le statistiche generali*/
	int* stat;
	/*dati in cui ogni taxi raccoglie le sue statistiche*/
	int successi=0;
	long tempo=0;
	taxiTot = 0;
	srand((unsigned) time(&f)+getpid());
	/*mi collego al semaforo*/
	if((semId=semget(SEMKEY, 0, 0)) == -1)
		EXIT_ON_ERROR
	/*mi collego al semaforo per il traffico */
	if((TsemId=semget(TSEMKEY, 0, 0)) == -1)
		EXIT_ON_ERROR
	/*mi collego al semaforo per le statistiche*/
	if((SsemId=semget(SSEMKEY, 0, 0)) == -1)
		EXIT_ON_ERROR
	/*mi collego alla coda di messaggi*/
	if((msgId = msgget(MSGKEY, 0))==-1)
		EXIT_ON_ERROR
	/*ricava l'id della shm2*/
	if((shmId2=shmget(SHM2, 0, 0)) == -1)
		EXIT_ON_ERROR
	/*ricava l'id della shm3*/
	if((shmId3=shmget(SHM3, 0, 0)) == -1)
		EXIT_ON_ERROR
	/*attacca la memoria condivisa 2*/
	if((start = (struct request*)shmat(shmId2, NULL , 0)) == (void *)-1)
		EXIT_ON_ERROR
	/*attacca la memoria condivisa 3*/
	if((stat = (int*)shmat(shmId3, NULL , 0)) == (void *)-1)
		EXIT_ON_ERROR
	for(n = 0; n < taxi; ++n) {
		/*provo a generare un taxi nella cella [i][j]*/
		i = rand()%SO_HEIGHT;
		j = rand()%SO_WIDTH;
		if(mappa[(i*SO_WIDTH)+j].flag_a != 1 && mappa[(i*SO_WIDTH)+j].taxi < mappa[(i*SO_WIDTH)+j].C_attr && reserveSem2(TsemId, (i*SO_WIDTH)+j)!=-1) {
			
			mappa[(i*SO_WIDTH)+j].taxi++;
			taxiTot++;
			switch(fork()) {
				case -1:
					EXIT_ON_ERROR
				case 0:
					action1.sa_handler = handlerTaxi;
					sigemptyset (&action1.sa_mask);
					action1.sa_flags = 0;
					/*gestisce il segnale*/
					sigaction(SIGUSR1, &action1, NULL);
					if(taxiTot == SO_TAXI)
						kill(getppid(), SIGUSR1);
					reserveSem(semId,0);
					releaseSem(semId, 0);
					while(end){ /*esegue richieste*/
						temp=goToS(start,i, j);
						MovimentoTaxi(mappa,&i, &j, start[temp].x, start[temp].y, TsemId);
						if(abortiti==1) {
							abortiti = 0;
							break;
						}
						action2.sa_handler = taxiTimer;
						sigemptyset (&action2.sa_mask);
						action2.sa_flags = 0;
						sigaction(SIGALRM, &action2, NULL);
						alarm(SO_TIMEOUT);
						if(msgrcv(msgId,&mss, sizeof(struct mymsg)-sizeof(long), mappa[(i*SO_WIDTH)+j].sorgente, 0)==-1) {
							fail++;
							mappa[(i*SO_WIDTH)+j].taxi--;
							releaseSem(TsemId, (i*SO_WIDTH)+j);
							break;
						}
						alarm(0);
						a = time(&a);
						MovimentoTaxi(mappa,&i, &j, mss.array[0], mss.array[1], TsemId);
						b = time(&b);
						if(difftime(b, a) >= tempo)
							tempo = difftime(b, a);
						if(abortiti==1)
							break;
						else if(i==mss.array[0] && j==mss.array[1])
							successi++;
					}
					/*riservo il semaforo per scrivere le statistiche*/
					reserveSem(SsemId, 0);
					stat[0]+=successi;
					stat[2]+=abortiti;
					stat[9]+=fail;
					if(stat[4]<=usura){
						stat[4]=usura;
						stat[3]=getpid();
					}
					if(stat[6]<=(int) tempo){
						stat[6]=(int) tempo;
						stat[5]=getpid();
					}
					if(stat[8]<=successi){
						stat[8]=successi;
						stat[7]=getpid();
					}
					/*rilascio semaforo delle statistiche*/
					releaseSem(SsemId, 0);
					if(trueend) {
						abortiti=0;
						fail=0;
						usura=0;
						if(end==1) {
							creaTaxi(mappa, 1);
						}
					}
					exit(EXIT_SUCCESS);
				default:
					break;
			}
		}
		else{
			n--;
		}
	}
	/*stacca la memoria 2*/
	if(shmdt(start) == -1)
		EXIT_ON_ERROR
}

void MovimentoTaxi(struct strada mappa[], int *i, int *j, int x, int y , int semaforo) {
	int temp = 0;
	if(mappa[(x*SO_WIDTH)+y].flag_a==1) {
		printf("ma è un buco!\n");
		return;
	}
	if(x>*i) {
		for(*i;*i<x;*i) {
			if(mappa[(*i*SO_WIDTH)+*j].bordo!=5 && mappa[(*i*SO_WIDTH)+*j].bordo!=6 && mappa[(*i*SO_WIDTH)+*j].bordo!=7 && mappa[((*i+1)*SO_WIDTH)+*j].flag_a==0) {
				temp = MovimentoGiu(mappa,*i, *j, semaforo);
				if(temp==-1)
					return;
				else {
					*i = temp;
				}
			}
			else if(mappa[((*i+1)*SO_WIDTH)+*j].flag_a==1){
				if(y>*j) {
					temp = MovimentoDx(mappa,*i, *j, semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
				}
				else if(y<*j) {
					temp = MovimentoSx(mappa,*i, *j, semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
				}
				else if(y==*j) {
					if(mappa[(*i*SO_WIDTH)+*j].bordo==1 || mappa[(*i*SO_WIDTH)+*j].bordo==7 || mappa[(*i*SO_WIDTH)+*j].bordo==8) {
						temp = MovimentoDx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
						temp = MovimentoGiu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoGiu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoSx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
					}
					else {
						temp = MovimentoSx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
						temp = MovimentoGiu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoGiu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoDx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
					}
				}
			}
		}
	}
	else if(x<*i) {
		for(*i;*i>x;*i) {
			if(mappa[(*i*SO_WIDTH)+*j].bordo!=1 && mappa[(*i*SO_WIDTH)+*j].bordo!=2 && mappa[(*i*SO_WIDTH)+*j].bordo!=3 && mappa[((*i-1)*SO_WIDTH)+*j].flag_a==0) {
				temp = MovimentoSu(mappa,*i, *j, semaforo);
				if(temp==-1)
					return;
				else {
					*i = temp;
				}
			}
			else if(mappa[((*i-1)*SO_WIDTH)+*j].flag_a==1){
				if(y>*j) {
					temp = MovimentoDx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
				}
				else if(y<*j) {
					temp = MovimentoSx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
				}
				else if(y==*j) {
					if(mappa[(*i*SO_WIDTH)+*j].bordo!=3 && mappa[(*i*SO_WIDTH)+*j].bordo!=4 && mappa[(*i*SO_WIDTH)+*j].bordo!=5) {
						temp = MovimentoDx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
						temp = MovimentoSu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoSu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoSx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
					}
					else {
						temp = MovimentoSx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
						temp = MovimentoSu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoSu(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*i = temp;
						}
						temp = MovimentoDx(mappa,*i, *j,semaforo);
						if(temp==-1)
							return;
						else {
							*j = temp;
						}
					}
				}
			}
		}
	}
	if(y>*j) {
		for(*j;*j<y;*j) {
			if(mappa[(*i*SO_WIDTH)+*j].bordo!=3 && mappa[(*i*SO_WIDTH)+*j].bordo!=4 && mappa[(*i*SO_WIDTH)+*j].bordo!=5 && mappa[(*i*SO_WIDTH)+*j+1].flag_a==0) {
				temp = MovimentoDx(mappa,*i, *j,semaforo);
				if(temp==-1)
					return;
				else {
					*j = temp;
				}
			}
			else if(mappa[(*i*SO_WIDTH)+*j+1].flag_a==1){
				if(mappa[(*i*SO_WIDTH)+*j].bordo==5 || mappa[(*i*SO_WIDTH)+*j].bordo==6 || mappa[(*i*SO_WIDTH)+*j].bordo==7) {
					temp = MovimentoSu(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*i = temp;
					}
					temp = MovimentoDx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
					temp = MovimentoDx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
					temp = MovimentoGiu(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*i = temp;
					}
				}
				else {
					temp = MovimentoGiu(mappa,*i, *j,semaforo);
					if(temp==-1)
							return;
						else {
							*i = temp;
						}
					temp = MovimentoDx(mappa,*i, *j,semaforo);
					if(temp==-1)
							return;
						else {
							*j = temp;
						}
					temp = MovimentoDx(mappa,*i, *j,semaforo);
					if(temp==-1)
							return;
						else {
							*j = temp;
						}
					temp = MovimentoSu(mappa,*i, *j,semaforo);
					if(temp==-1)
							return;
						else {
							*i = temp;
						}
				}
			}
		}
	}
	else if(y<*j) {
		for(*j;*j>y;*j) {
			if(mappa[(*i*SO_WIDTH)+*j].bordo!=1 && mappa[(*i*SO_WIDTH)+*j].bordo!=7 && mappa[(*i*SO_WIDTH)+*j].bordo!=8 && mappa[(*i*SO_WIDTH)+*j-1].flag_a==0) {
				temp = MovimentoSx(mappa,*i, *j,semaforo);
				if(temp==-1)
					return;
				else {
					*j = temp;
				}
			}
			else if(mappa[(*i*SO_WIDTH)+*j-1].flag_a==1){
				if(mappa[(*i*SO_WIDTH)+*j].bordo==1 || mappa[(*i*SO_WIDTH)+*j].bordo==2 || mappa[(*i*SO_WIDTH)+*j].bordo==3) {
					temp = MovimentoGiu(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*i = temp;
					}
					temp = MovimentoSx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
					temp = MovimentoSx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
					temp = MovimentoSu(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*i = temp;
					}
				}
				else {
					temp = MovimentoSu(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*i = temp;
					}
					temp = MovimentoSx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
					temp = MovimentoSx(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*j = temp;
					}
					temp = MovimentoGiu(mappa,*i, *j,semaforo);
					if(temp==-1)
						return;
					else {
						*i = temp;
					}
				}
			}
		}
	}
}

int MovimentoSx(struct strada mappa[],int i, int j, int semaforo) {
	if(mappa[(i*SO_WIDTH)+j].bordo!=1 || mappa[(i*SO_WIDTH)+j].bordo!=2 || mappa[(i*SO_WIDTH)+j].bordo!=3 ){
		if(reserveSem2(semaforo, (i*SO_WIDTH)+j-1)==-1) {
			abortiti=1;
			releaseSem(semaforo, (i*SO_WIDTH)+j);
			mappa[(i*SO_WIDTH)+j].taxi--;
			return -1;
		}
		mappa[(i*SO_WIDTH)+j].taxi--;
		j--;
		mappa[(i*SO_WIDTH)+j].passaggi++;
		mappa[(i*SO_WIDTH)+j].taxi++;
		releaseSem(semaforo, (i*SO_WIDTH)+j+1);
		usura++;
		my_time.tv_sec = 0;
		my_time.tv_nsec = mappa[(i*SO_WIDTH)+j].T_attr;
		nanosleep(&my_time, NULL);
	}
	return j;
}

int MovimentoDx(struct strada mappa[],int i, int j, int semaforo) {
	if(mappa[(i*SO_WIDTH)+j].bordo!=5 || mappa[(i*SO_WIDTH)+j].bordo!=6 || mappa[(i*SO_WIDTH)+j].bordo!=7){
		if(reserveSem2(semaforo, (i*SO_WIDTH)+j+1)==-1) {
			abortiti=1;
			releaseSem(semaforo, (i*SO_WIDTH)+j);
			mappa[(i*SO_WIDTH)+j].taxi--;
			return -1;
		}
		mappa[(i*SO_WIDTH)+j].taxi--;
		j++;
		mappa[(i*SO_WIDTH)+j].passaggi++;
		mappa[(i*SO_WIDTH)+j].taxi++;
		releaseSem(semaforo, (i*SO_WIDTH)+j-1);
		usura++;
		my_time.tv_sec = 0;
		my_time.tv_nsec = mappa[(i*SO_WIDTH)+j].T_attr;
		nanosleep(&my_time, NULL);
	}
	return j;
}

int MovimentoGiu(struct strada mappa[],int i, int j, int semaforo) {
	if(mappa[(i*SO_WIDTH)+j].bordo!=3 || mappa[(i*SO_WIDTH)+j].bordo!=4 || mappa[(i*SO_WIDTH)+j].bordo!=5){
		if(reserveSem2(semaforo, ((i+1)*SO_WIDTH)+j)==-1) {
			abortiti=1;
			releaseSem(semaforo, (i*SO_WIDTH)+j);
			mappa[(i*SO_WIDTH)+j].taxi--;
			return -1;
		}
		mappa[(i*SO_WIDTH)+j].taxi--;
		i++;
		mappa[(i*SO_WIDTH)+j].passaggi++;
		mappa[(i*SO_WIDTH)+j].taxi++;
		releaseSem(semaforo, ((i-1)*SO_WIDTH)+j);
		usura++;
		my_time.tv_sec = 0;
		my_time.tv_nsec = mappa[(i*SO_WIDTH)+j].T_attr;
		nanosleep(&my_time, NULL);
	}
	return i;
}

int MovimentoSu(struct strada mappa[],int i, int j, int semaforo) {
	if(mappa[(i*SO_WIDTH)+j].bordo!=3 || mappa[(i*SO_WIDTH)+j].bordo!=4 || mappa[(i*SO_WIDTH)+j].bordo!=5){
		if(reserveSem2(semaforo, ((i-1)*SO_WIDTH)+j)==-1) {
			abortiti=1;
			releaseSem(semaforo, (i*SO_WIDTH)+j);
			mappa[(i*SO_WIDTH)+j].taxi--;
			return -1;
		}
		mappa[(i*SO_WIDTH)+j].taxi--;
		i--;
		mappa[(i*SO_WIDTH)+j].passaggi++;
		mappa[(i*SO_WIDTH)+j].taxi++;
		releaseSem(semaforo, ((i+1)*SO_WIDTH)+j);
		usura++;
		my_time.tv_sec = 0;
		my_time.tv_nsec = mappa[(i*SO_WIDTH)+j].T_attr;
		nanosleep(&my_time, NULL);
	}
	return i;
}

int goToS(struct request* sources, int i, int j){
	int a, sour;
	sour=0;
	for(a=0; a<SO_SOURCES-1; a++){
		if(abs(i-sources[sour].x)+abs(j-sources[sour].y)>= abs(i-sources[a+1].x)+abs(j-sources[a+1].y)){
			sour=a+1;
		}
	}
	return sour;
}
