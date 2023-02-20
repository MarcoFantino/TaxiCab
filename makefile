# flags per la compilazione
FLAGS = -c -std=c89 -pedantic -D_POSIX_C_SOURCE=199309L

all: PMain

PMain: PMain.o Taxi.o Semafori.o CreazioneMappa.o GetData.o 
	gcc -o PMain PMain.o Taxi.o Semafori.o CreazioneMappa.o GetData.o 

PMain.o: PMain.c PHead.h
	gcc $(FLAGS) PMain.c 

Taxi.o: Taxi.c PHead.h
	gcc $(FLAGS) Taxi.c

Semafori.o: Semafori.c PHead.h
	gcc $(FLAGS) Semafori.c

CreazioneMappa.o: CreazioneMappa.c PHead.h 
	gcc $(FLAGS) CreazioneMappa.c 

GetData.o: GetData.c PHead.h
	gcc $(FLAGS) GetData.c

