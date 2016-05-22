#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_LENGTH 10
#define PARK_OPEN 0
#define PARK_CLOSED 1
#define PARK_FULL 2
#define PARKING_VEHICLE 3
#define ENTERING_VEHICLE 4
#define LAST_VEHICLE_ID -1

int capacity;
int parkOpen;
int state;
int openTime;
int occupiedSpots = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef enum {NORTH, SOUTH, EAST, WEST} Direction;




typedef struct {
  Direction direction;
  int id;
  float parkedTime;
  char fifoName[FIFO_LENGTH] ;
} Vehicle;




void *parkAVehicle(void* arg){
	void* ret=NULL;
	int fdWrite;
	state = ENTERING_VEHICLE;
	Vehicle vehicle = *(Vehicle*) arg;
	fdWrite = open(vehicle.fifoName,O_WRONLY);

	printf("Vehicle number: %d \n",vehicle.id);

	pthread_mutex_lock(&mutex);//Locks all the other threads.

	if(occupiedSpots < capacity && state==PARK_OPEN ){ // space for the vehicle to enter
		occupiedSpots ++;
		pthread_mutex_unlock(&mutex); //Unlocks all the other threads.
		printf("Vehicle %d is parking ...\n", vehicle.id);
		state = PARKING_VEHICLE;
		usleep(vehicle.parkedTime * 1000); // suspends execution of the calling thread, in miliseconds (*1000);
		occupiedSpots--;
	}

	else if(parkOpen == PARK_CLOSED){ // park is closed
		pthread_mutex_unlock(&mutex);
		printf("Park closed! \n");
		state = PARK_CLOSED;
	}

	else{ // park full
		pthread_mutex_unlock(&mutex);
		printf("Park full! \n");
		state = PARK_FULL;
	}

	write(fdWrite,&state,sizeof(int));
	return ret;
}

void *northFunc(void *arg){
	void *ret = NULL;
	char* fifo = "fifoN";
	mkfifo(fifo,0660);
	pthread_t northEntry;
	int fdRead;
	Vehicle vehicle;
	int readRet;

	printf("Opening fifo \n");
	if ((fdRead = open("fifoN",O_RDONLY | O_NONBLOCK)) < 0){
		perror("Error openning file! \n");
	}
	//open on write mode to avoid busy waiting
	open("fifoN", O_WRONLY);

	printf("Fifo openned \n");

	while(1){
		readRet = read(fdRead, &vehicle, sizeof(Vehicle));
		if(vehicle.id == LAST_VEHICLE_ID)
			break;
		else if(readRet > 0 ){ // if = 0 it means there is nothing else to read from the fifo.
			printf("North entrance vehicle id: %d",vehicle.id);
			if(pthread_create(&northEntry,NULL,parkAVehicle,&vehicle) != 0)
				perror("Error on creating thread for North Entrance \n");
		}
	}
	close(fdRead);
	return ret;
}

void *southFunc(void *arg){
		void *ret = NULL;
		char* fifo = "fifoS";
		mkfifo(fifo,0660);
		pthread_t southEntry;
		int fdRead;
		Vehicle vehicle;
		int readRet;

		printf("Opening fifo \n");
		if ((fdRead = open("fifoS",O_RDONLY | O_NONBLOCK)) < 0){
				perror("Error openning file! \n");
			}
		//open on write mode to avoid busy waiting
		open("fifoS", O_WRONLY);

		printf("Fifo openned \n");

		while(1){
			readRet = read(fdRead, &vehicle, sizeof(Vehicle));
			if(vehicle.id == LAST_VEHICLE_ID)
				break;
			else if(readRet > 0 ){ // if = 0 it means there is nothing else to read from the fifo.
				printf("South entrance vehicle id: %d",vehicle.id);
				if(pthread_create(&southEntry,NULL,parkAVehicle,&vehicle) !=0)
					perror("Error on creating thread for South Entrance \n");
			}
		}
		close(fdRead);
		return ret;

}

void *eastFunc(void *arg){
		void *ret = NULL;
		char* fifo = "fifoE";
		mkfifo(fifo,0660);
		pthread_t eastEntry;
		int fdRead;
		Vehicle vehicle;
		int readRet;

		printf("Opening fifo \n");
		if ((fdRead = open("fifoE",O_RDONLY | O_NONBLOCK)) < 0){
				perror("Error openning file! \n");
			}
		//open on write mode to avoid busy waiting
		open("fifoE", O_WRONLY);

		printf("Fifo openned \n");

		while(1){
			readRet = read(fdRead, &vehicle, sizeof(Vehicle));
			if(vehicle.id == LAST_VEHICLE_ID)
				break;
			else if(readRet > 0 ){ // if = 0 it means there is nothing else to read from the fifo.
				printf("East entrance vehicle id: %d",vehicle.id);
				if(pthread_create(&eastEntry,NULL,parkAVehicle,&vehicle) != 0)
					perror("Error on creating thread for East Entrance \n");
			}
		}
		close(fdRead);
		return ret;

}

void *westFunc(void *arg){
		void *ret = NULL;
			char* fifo = "fifoW";
			mkfifo(fifo,0660);
			pthread_t westEntry;
			int fdRead;
			Vehicle vehicle;
			int readRet;

			printf("Opening fifo \n");
			if ((fdRead = open("fifoW",O_RDONLY | O_NONBLOCK)) < 0){
					perror("Error openning file! \n");
				}
			//open on write mode to avoid busy waiting
			open("fifoW", O_WRONLY);

			printf("Fifo openned \n");

			while(1){
				readRet = read(fdRead, &vehicle, sizeof(Vehicle));
				if(vehicle.id == LAST_VEHICLE_ID)
					break;
				else if(readRet > 0 ){ // if = 0 it means there is nothing else to read from the fifo.
					printf("West entrance vehicle id: %d",vehicle.id);
					if(pthread_create(&westEntry,NULL,parkAVehicle,&vehicle) != 0)
						perror("Error on creating thread for West Entrance \n");
				}
			}
			close(fdRead);
			return ret;

}

void close_park(){
	Vehicle lastVehicle;
	lastVehicle.id = LAST_VEHICLE_ID;
	lastVehicle.parkedTime = 0;
	strcpy(lastVehicle.fifoName, "over");

	int fdNorth = open("fifoN", O_WRONLY);
	int fdSouth = open("fifoS", O_WRONLY);
	int fdEast = open("fifoE", O_WRONLY);
	int fdWest = open("fifoW", O_WRONLY);



	write(fdNorth, &lastVehicle, sizeof(Vehicle));
	write(fdSouth, &lastVehicle, sizeof(Vehicle));
	write(fdEast, &lastVehicle, sizeof(Vehicle));
	write(fdWest, &lastVehicle, sizeof(Vehicle));
	close(fdNorth);
	close(fdSouth);
	close(fdEast);
	close(fdWest);


}

int main (int argc, char* argv[]){

	if(argc != 3){
		perror("Invalid number of arguments! \n");
		exit(1);
	}
	printf("Park is open! \n");
	state = PARK_OPEN; //park open
	parkOpen = PARK_OPEN;
	pthread_t northThread, southThread, eastThread, westThread;
	capacity = atoi(argv[1]);
	openTime = atoi(argv[2]);

	if(pthread_create(&northThread, NULL, northFunc,NULL) != 0) perror("Can't create North thread! \n");
	if(pthread_create(&southThread, NULL, southFunc,NULL) != 0) perror("Can't create South thread! \n");
	if(pthread_create(&eastThread, NULL, eastFunc,NULL) != 0) perror("Can't create East thread! \n");
	if(pthread_create(&westThread, NULL, westFunc,NULL) != 0) perror("Can't create West thread! \n");

	sleep(openTime); // wait for the park to close

	printf("Park closed! \n");
	parkOpen = PARK_CLOSED;
	state = PARK_CLOSED;
	close_park();

	if(pthread_join(northThread,NULL) !=0) perror("Can't wait for North thread");
	if(pthread_join(southThread,NULL) !=0) perror("Can't wait for South thread");
	if(pthread_join(eastThread,NULL) !=0) perror("Can't wait for East thread");
	if(pthread_join(westThread,NULL) !=0) perror("Can't wait for West thread");;

	printf("Waiting for all the cars to leave ... \n");
	while(occupiedSpots != 0);
	printf("The park is now empty! \n");

	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);
	return 0;
}