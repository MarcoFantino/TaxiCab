#include "PHead.h"
#include <stdio.h>
void getData(){
	FILE* f = fopen("Requisiti.txt" , "r");
	fscanf(f , "SO_TAXI=%d \n" , &SO_TAXI);
	fscanf(f , "SO_SOURCES=%d \n" , &SO_SOURCES);
	fscanf(f , "SO_HOLES=%d \n" , &SO_HOLES);
	fscanf(f , "SO_TOP_CELLS=%d \n" , &SO_TOP_CELLS);
	fscanf(f , "SO_CAP_MIN=%d \n" , &SO_CAP_MIN);
	fscanf(f , "SO_CAP_MAX=%d \n" , &SO_CAP_MAX);
	fscanf(f , "SO_TIME_SEC_MIN=%d \n" , &SO_TIME_SEC_MIN);
	fscanf(f , "SO_TIME_SEC_MAX=%d \n" , &SO_TIME_SEC_MAX);
	fscanf(f , "SO_TIMEOUT=%d \n" , &SO_TIMEOUT);
	fscanf(f , "SO_DURATION=%d \n" , &SO_DURATION);
	fclose(f);
}

