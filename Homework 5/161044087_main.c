#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#define MAX_FLORISTS 3
#define MAX_CLIENTS 1000
#define TYPE_FLOWERS 3
typedef struct client {
	int xCoordinate;
	int yCoordinate;
	double distance;
	char name[10];
	char flower[10];
}CLIENT;
typedef struct florist {
	int xCoordinate;
	int yCoordinate;
	int haveWhichQueue;
	int numberOfTypeOfFlowers;
	double speed;
	char name[10];
	char flowers[TYPE_FLOWERS][10];
}FLORIST;
typedef struct QUEUE{
    CLIENT queue[100];
    int numberOfClients;
    int head;
    int tail;
}queue;
CLIENT clients[MAX_CLIENTS];
queue queueOfClient[MAX_FLORISTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
FLORIST florists[MAX_FLORISTS];
int countOfFlorist; //count of florist.
int countOfClients; //count of client.
int order = 0; // order of next one request.
int completedClients = 0; // count of how many requests is completed.
int count;
int a=0; //count of how many threads exit.
double totaltime[3]; // total spend time for each florist.
int totalRequest[3]; // total request for each florist.
pthread_t threads[16];
void initializeQueue();
void *florist(void *arg);
void *server(void *arg);
void readfile(FLORIST florists[MAX_FLORISTS],CLIENT clients[MAX_CLIENTS],
	          int *countOfClient,int *countOfFlorist,const char *filename);
double calculateEuclideanDistance(int *whichFlorist,CLIENT *client,FLORIST *florists);
int main(int argc, char const *argv[])
{
	if (argc != 2) {
		fprintf(stdout, "Usage : %s <input file>\n", argv[0]);
		exit(-1);
	}
	char *message = "server";
	readfile(florists,clients,&countOfClients,&countOfFlorist,argv[1]);
	count = countOfClients;
	//initialize member of queue;
	initializeQueue();
	for (int i = 0; i < 16; ++i) {
		threads[i] = 0;
		if (i < 15) {
			if (pthread_create(&threads[i],NULL,florist, (void *)(&florists[i%MAX_FLORISTS])) != 0)
				exit(-1);
		}
		else if (i == 15) {
			if (pthread_create(&threads[i],NULL,server,(void *)server) != 0)
				exit(-1);
		}

	}

	for (int i = 0; i < 16; ++i) {
		pthread_join(threads[i],NULL);
	}

	fprintf(stdout, "All proceses is done.\n");
	for (int i = 0; i < MAX_FLORISTS; ++i)
		fprintf(stdout, "%s closing shop.\n",florists[i].name);
	fprintf(stdout, "Sale statistic for today:\n");
	fprintf(stdout, "-------------------------------------------------\n");
	fprintf(stdout, "Florist                # of sales           Total time \n");
	fprintf(stdout, "-------------------------------------------------\n");
	for (int i = 0; i < MAX_FLORISTS; ++i)
		fprintf(stdout, "%5s %18d %23d\n", florists[i].name,totalRequest[i],(int)totaltime[i]);
	return 0;
}
void initializeQueue() {
	for (int i = 0; i < MAX_FLORISTS; ++i) {
        queueOfClient[i].head = 0;
        queueOfClient[i].tail = 0;
        queueOfClient[i].numberOfClients = 0;
        totaltime[i]  = 0.0;
        totalRequest[i] = 0;
    }
}
void *florist(void *arg) {

	while(1) {
		pthread_mutex_lock(&mutex);
		FLORIST florist = *(FLORIST *)arg;
		if (completedClients == count) {
			++a;
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
		}
		if(queueOfClient[florist.haveWhichQueue].numberOfClients == 0) pthread_cond_wait(&cond,&mutex);
		else {

			CLIENT client = queueOfClient[florist.haveWhichQueue].queue[queueOfClient[florist.haveWhichQueue].head];
			++queueOfClient[florist.haveWhichQueue].head;
			--queueOfClient[florist.haveWhichQueue].numberOfClients;
			++completedClients;
			srand(time(0));
			int preparation = rand() % 41 + 10;
			
			useconds_t usec = preparation;
			usleep(usec);
			double arrive = (client.distance)/(florist.speed);
			totaltime[florist.haveWhichQueue] += (preparation + arrive);
			fprintf(stdout, "Florist %8s has delivered a %s to %s in %.2f ms\n",florist.name,client.name,
				                            client.flower, (preparation + (client.distance)/(florist.speed)));

		}
		pthread_mutex_unlock(&mutex);
	}

}
void *server(void *arg) {
	int whichFlorist = 0;
	while(1) {
		pthread_mutex_lock(&mutex);
		if (countOfClients != 0) {
			CLIENT client = clients[order];
			++order;
			--countOfClients;
			client.distance = calculateEuclideanDistance(&whichFlorist,&client,florists);
			queueOfClient[whichFlorist].queue[queueOfClient[whichFlorist].tail] = client;
			++queueOfClient[whichFlorist].tail;
			++queueOfClient[whichFlorist].numberOfClients;
			++totalRequest[whichFlorist];
			
		}
		pthread_cond_signal(&cond);
		//all requests is done and all threads is exited.
		if (countOfClients == 0 && a == 15) {
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
		} 
		pthread_mutex_unlock(&mutex);
	}
}
void readfile(FLORIST florists[MAX_FLORISTS],CLIENT clients[MAX_CLIENTS],
	          int *countOfClient,int *countOfFlorist,const char *filename)
{
	FILE *ptr = NULL;
	char arr[256];
	char *line = NULL;
	ptr = fopen(filename,"r");
	int f=0,c=0;
	while(!feof(ptr) && fgets(arr,256,ptr) != NULL && strcmp(arr,"\n") != 0) {
		  line = strtok(arr," ,:();");
		  strncpy(florists[f].name,line,strlen(line));
		  int i = 0;
		  florists[f].numberOfTypeOfFlowers = 0;
		  florists[f].haveWhichQueue = f;
		  while(line != NULL) {
		  	    line = strtok(NULL," ,:();");
		  	    if (line != NULL) {
		  	    	switch(i) {
		  	    		case 0: florists[f].xCoordinate = atoi(line); break;
		  	    		case 1: florists[f].yCoordinate = atoi(line); break;
		  	    		case 2: florists[f].speed = atof(line); break;
		  	    		default: if (strchr(line,'\n') == NULL) {
			  	    				 strncpy(florists[f].flowers[florists[f].numberOfTypeOfFlowers],line,strlen(line));
			  	    		         ++(florists[f].numberOfTypeOfFlowers);
		  	    			     }
 								 else {
 								 	strncpy(florists[f].flowers[florists[f].numberOfTypeOfFlowers],line,strlen(line) - 1);
			  	    		         ++(florists[f].numberOfTypeOfFlowers);
 								 }
 								 
		  	    	}
		  	    	++i;
		  	    }
		  }
		  ++f;
	}

	while(!feof(ptr) && fgets(arr,256,ptr) != NULL) {
		  line = strtok(arr," ,:();");
		  strncpy(clients[c].name,line,strlen(line));
		  clients[c].distance = 0.0;
		  int i = 0;
		  while(line != NULL) {
		  		//fprintf(stdout, "%s",line);
		  	    line = strtok(NULL," ,:();");
		  	    if (line != NULL) {
		  	    	switch(i) {
		  	    		case 0: clients[c].xCoordinate = atoi(line); break;
		  	    		case 1: clients[c].yCoordinate = atoi(line); break;
		  	    		default: strncpy(clients[c].flower,line,strlen(line) - 1); break; 
		  	    	}
		  	    	++i;
		  	    }
		  }
		  ++c;
	}
	*countOfFlorist = f;
	*countOfClient = c;
	fclose(ptr);
}
double calculateEuclideanDistance(int *whichFlorist,CLIENT *client,FLORIST *florists) {
	
	double min = 100000;
	for (int i = 0; i < MAX_FLORISTS; ++i) {

		if (strcmp(client->flower,florists[i].flowers[0]) == 0 || strcmp(client->flower,florists[i].flowers[1]) == 0
			                                                   || strcmp(client->flower,florists[i].flowers[2]) == 0) {
			if (sqrt(pow(client->xCoordinate - (florists[i].xCoordinate) , 2) + pow(client->yCoordinate - (florists[i].yCoordinate) , 2)) < min) {
				//closest florist.
				*whichFlorist = i;
				//minimum distance to closest florist.
				min = sqrt(pow(client->xCoordinate - (florists[i].xCoordinate) , 2) + pow(client->yCoordinate - (florists[i].yCoordinate) , 2));
			}
		}

	}
	return min;
}
