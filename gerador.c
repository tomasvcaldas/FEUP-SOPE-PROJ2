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
#define BUFFER_SIZE 100
#define MAX_STATUS 20
#define DEST 10
#define PARK_FULL 2 //log = cheio
#define ENTERING_VEHICLE 4 //log = entrada
#define LEAVING_VEHICLE 5 //log = saida

#define GERADOR_LOG "gerador.log"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

float clockUnit;
float genTime;
int id = 0;
int fd_gerador_log;

typedef enum {NORTH, SOUTH, EAST, WEST} Direction;


typedef struct {
  Direction direction;
  int id;
  float parkedTime;
  char fifoName[FIFO_LENGTH] ;
  float currentTick;
  int numberOfTicks;
  int tLife;
} Vehicle;

void writeToFile(Vehicle *vehicle, int state){

	char buffer[BUFFER_SIZE];
	char status[MAX_STATUS];
	char dest[DEST];

	printf("STATE: %d\n",state);

	if (state == 2) strcpy(status, "cheio");
	if (state == 4) strcpy(status, "entrada");
	if (state == 5) strcpy(status, "saida");

	int tFinal = clock()-vehicle->tLife;

	printf("STATUS: %s \n",status);
	if (vehicle->direction == NORTH) strcpy (dest, "N");
	if (vehicle->direction == SOUTH) strcpy (dest, "S");
	if (vehicle->direction == EAST) strcpy (dest, "E");
	if (vehicle->direction == WEST) strcpy (dest, "O");

	//Format: "t(ticks) ; id_viat ; dest ; t_estacion ; t_vida ; observ \n";

	sprintf(buffer, "%-8f ; %7d ;    %s   ; %10f ; %6d ; %s\n", (vehicle->numberOfTicks-vehicle->currentTick), vehicle->id, dest, vehicle->parkedTime/100, tFinal, status);
	write(fd_gerador_log, buffer,strlen(buffer));
}

void* vehicleFunc(void *arg){
	void *ret = NULL;
	Vehicle vehicle = *(Vehicle*) arg;
	char fifoPath[64];
	sprintf(fifoPath, "/tmp/%s", vehicle.fifoName);
	mkfifo(fifoPath,0660);
	int fdWrite;
	int fdRead;
	int state;

	//printf("Entered in thread \n"); dá print dist

	switch(vehicle.direction){
	case NORTH:
		fdWrite = open("fifoN",O_WRONLY | O_NONBLOCK);
		break;
	case SOUTH:
		fdWrite = open("fifoS",O_WRONLY | O_NONBLOCK);
		break;
	case WEST:
		fdWrite = open("fifoW",O_WRONLY | O_NONBLOCK);
		break;
	case EAST:
		fdWrite = open("fifoE",O_WRONLY | O_NONBLOCK);
		break;
	}


	if(fdWrite != -1){
		write(fdWrite,&vehicle,sizeof(Vehicle));
	}close(fdWrite);

	printf("FIFONAME: |%s| \n",vehicle.fifoName);
	fdRead = open(fifoPath,O_RDONLY);
	if(fdRead != -1){
		printf("entrou \n");
		read(fdRead,&state,sizeof(int));
	}
	else
		perror("erro \n");

	writeToFile(&vehicle,state);

	unlink(fifoPath);

	return ret;
}

int genVehicle(float tick, float totalTick){
	Vehicle vehicle;

	id++;
	vehicle.id = id;

	vehicle.tLife = clock();

	pthread_t mainVehicle;
	vehicle.numberOfTicks = totalTick;
	vehicle.currentTick = tick;
	int dir = rand() %  4;
	switch(dir){
	case 0:
		vehicle.direction = NORTH;
		break;
	case 1:
		vehicle.direction = SOUTH;
		break;
	case 2:
		vehicle.direction = EAST;
		break;
	case 3:
		vehicle.direction = WEST;
		break;
	}

	float pTime = ((rand() %10)+1) * clockUnit;
	vehicle.parkedTime = pTime;

	char buffer[BUFFER_SIZE];

	sprintf(buffer,"%s%d","fifo",id);
	strcpy(vehicle.fifoName, buffer);
	printf("Vehicle %d:     ",vehicle.id);
	switch(vehicle.direction){
	case NORTH:
		printf("North entrance   ");
		break;
	case SOUTH:
		printf("South entrance  ");
		break;
	case EAST:
		printf("East entrace   ");
		break;
	case WEST:
		printf("West entrance   ");
		break;
	}
	printf("%f \n",vehicle.currentTick);

	if(pthread_create(&mainVehicle, NULL, vehicleFunc,&vehicle) != 0)
		perror("Can't create Main Vehicle thread! \n");

	int nextCarTime;
	int park = rand() % 10;

	if(park < 2) nextCarTime= 2;
	else if(park < 5) nextCarTime = 1;
	else nextCarTime = 0;

	return nextCarTime; //tempo ate ao proximo veiculo
}


int main(int argc, char* argv[]){

	if(argc != 3){
		perror("Invalid number of arguments! \n");
		exit(1);
	}

	srand(time(NULL));
	genTime = atoi(argv[1]);
	clockUnit = atoi(argv[2]);
	float numberOfTicks;
	float totalTicks;
	int ticksForNextCar = 0;

	FILE*fd=fopen("gerador.log","w");
	fclose(fd);

	fd_gerador_log = open(GERADOR_LOG, O_WRONLY | O_CREAT , 0600);
	if (fd_gerador_log < 0){
		perror("Error creation gerador.log");
	}

	char buffer[] = "t(ticks) ; id_viat ; dest ; t_estacion ; t_vida ; observ \n";
	write(fd_gerador_log, buffer, strlen(buffer));

	numberOfTicks = (genTime / clockUnit) *1000; // Number of events that are going to happen
	totalTicks = numberOfTicks;
	printf("Number of ticks: %f \n",numberOfTicks);

	do{
		if(ticksForNextCar == 0) ticksForNextCar = genVehicle(numberOfTicks,totalTicks);
		else ticksForNextCar--;
		usleep(clockUnit * 1000);
		numberOfTicks--;
	} while( numberOfTicks > 0);



	pthread_exit(NULL);
}
