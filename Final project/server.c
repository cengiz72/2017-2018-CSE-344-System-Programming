// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#define PI 3.14
#define MAX 1000
#define MAX_PROVIDER 500
typedef struct client {
    char name[20];
    char priority;
    int degree;
}CLIENT;

typedef struct provider {
    char name[20];
    int performance;
    int price;
    int duration;
    int id;
    int countOfServe;
    int Islogout;
}PROVIDER;

typedef struct queue {
    char name[2][20]; //client names.
    int head ; 
    int tail ;
    int size ;
    int fd[2] ;
    int degree[2]; //client degrees
}QUEUE;
int wakeup = -1;
typedef struct server {
    int sock; // file descriptor.
}SERVER;
PROVIDER provider[MAX_PROVIDER];
QUEUE    queue[MAX_PROVIDER];
pthread_mutex_t mutex[MAX_PROVIDER];
pthread_cond_t cond[MAX_PROVIDER];
pthread_mutex_t mutexsrv = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condsrv = PTHREAD_COND_INITIALIZER;
int countOfProvider;
int flag = 0;
int signalr = -1;
int server_fd = 0;
FILE *log_file = NULL;
int readfile(const char *filename);
void *thread_function(void *arg);
void *server_thread(void *arg);
int factorial(int number);
double cosinus(int angle_part1,int  numberOfTerm);
int whichProviderServe(char priority);
pthread_t threads[MAX];
int file_descriptors[MAX];
int countOfFileDescriptor = 0;
struct timeval start,end;
void signal_handler(int signo) {

    signalr = 1; 
    /*Wake up  all threads.*/
    for(int i = 0; i < countOfProvider; ++i) {
    	pthread_cond_broadcast(&cond[i]);
    }
    /*Wait all threads to exit.*/
    for (int i = 0; i < countOfProvider; ++i)
    {	
    	pthread_join(threads[i],NULL);
    }
    /*destroy all conditon variable and mutexes.*/
    for (int i = 0; i < countOfProvider; ++i)
    {
    	pthread_mutex_destroy(&mutex[i]);
    	pthread_cond_destroy(&cond[i]);
    }
    pthread_mutex_destroy(&mutexsrv);
    pthread_cond_destroy(&condsrv);
    /*close all file descriptors which socket is opened if they are open.*/
    for (int i = 0; i < countOfFileDescriptor; ++i) {
    	if (fcntl(file_descriptors[i], F_GETFD) != -1 || errno != EBADF) {
    		char message[1024] = "SERVER SHUTDOWN";
    		send(file_descriptors[i],message,sizeof(message),0);
    		close(file_descriptors[i]);
    	}
    }

    fprintf(stderr, "Termination signal is received\n");
    fprintf(stderr, "Terminating all clients\n");
    fprintf(stderr, "Terminating all providers\n");
    fprintf(log_file, "Termination signal is received\n");
    fprintf(log_file, "Terminating all clients\n");
    fprintf(log_file, "Terminating all providers\n");
    fprintf(stderr, "Statistics\n");
    fprintf(stderr, "Name              Number of client served\n");
    fprintf(log_file, "Statistics\n");
    fprintf(log_file, "Name              Number of client served\n");
    for (int i = 0; i < countOfProvider; ++i)
    {
    	fprintf(stderr, "%s %15d\n", provider[i].name,provider[i].countOfServe);
    	fprintf(log_file, "%s %15d\n", provider[i].name,provider[i].countOfServe);
    }
        /*close socket*/
    close(server_fd);
    /*close log file*/
    fclose(log_file);
    exit(0);
}
int main(int argc, char const *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage : %s <port address> <provider file : data.dat> <log file : log.data>\n",argv[0]);
        exit(-1);
    }
    int new_socket = 0, valread = 0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    countOfProvider =  readfile(argv[2]);
    log_file = fopen(argv[3],"w");
    if (log_file == NULL) {
    	fprintf(stderr, "File is not found\n");
    	exit(-1);
    }
    fprintf(stderr, "Logs kept at %s\n", argv[3]);
    fprintf(log_file, "Logs kept at %s\n", argv[3]);
    fprintf(stderr, "%d provider threads created\n", countOfProvider);
    fprintf(log_file, "%d provider threads created\n", countOfProvider);
    struct sigaction act;
    act.sa_handler = signal_handler;
    act.sa_flags = 0;

    if ((sigemptyset(&act.sa_mask) == -1) || 
        (sigaction(SIGINT, &act, NULL) == -1) || 
        (sigaction(SIGTERM, &act, NULL) == -1) ||
        (sigaction(SIGQUIT, &act, NULL) == -1)) {
        perror("Failed to set Signal handler.");
        exit(1);
    }
    fprintf(stderr, "Name       Performance     Price     Duration\n");
    fprintf(log_file, "Name       Performance     Price     Duration\n");
    for (int i = 0; i < countOfProvider; ++i) {
    	fprintf(stderr, "%s %10d %15d %10d\n",provider[i].name,provider[i].performance,provider[i].price,provider[i].duration);
    	fprintf(log_file, "%s %10d %15d %10d\n",provider[i].name,provider[i].performance,provider[i].price,provider[i].duration);
    }
    gettimeofday(&start,NULL);
    for (int i = 0; i < countOfProvider; ++i) {
        if (pthread_create(&threads[i],NULL,thread_function, (void *)(&provider[i])) != 0)
                exit(-1);
        pthread_mutex_init(&mutex[i],NULL);
        pthread_cond_init(&cond[i],NULL);
    }


        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
          
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                      &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(atoi(argv[1]));
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 5) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Server is waiting for client connections at port %d\n",atoi(argv[1]));
        fprintf(log_file, "Server is waiting for client connections at port %d\n",atoi(argv[1]));
        while ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                           (socklen_t*)&addrlen)) >= 0)
        {
			int valread = 0;
			char buffer[1024] = {0};
			file_descriptors[countOfFileDescriptor++] = new_socket;
			valread = read(new_socket , buffer, 1024);
			CLIENT client = *(CLIENT *)buffer;
			pthread_mutex_lock(&mutexsrv);
			for (int i = 0; i < countOfProvider; ++i)
			{
				if (provider[i].Islogout == 1) {
					gettimeofday(&end,NULL);
					provider[i].duration = provider[i].duration - (end.tv_sec-start.tv_sec);
					if (provider[i].duration < 0) {
						pthread_cond_broadcast(&cond[i]);
					}
				}
			}
			int serve = whichProviderServe(client.priority);
			if (serve == -1) {
				char message[1024] = "NO PROVIDER IS AVAILABLE";
				send(new_socket,message,sizeof(message),0);
			}
			else {
			pthread_mutex_lock(&mutex[serve]);
			queue[serve].fd[queue[serve].tail] = new_socket;
			queue[serve].degree[queue[serve].tail] = client.degree;
			strcpy(queue[serve].name[queue[serve].tail],client.name);
			++queue[serve].size;
			++queue[serve].tail;
			fprintf(stderr, "Client %s (%c %d) connected,forward to provider %s\n", client.name
				                         ,client.priority,client.degree,provider[serve].name);
			fprintf(log_file, "Client %s (%c %d) connected,forward to provider %s\n", client.name
				                         ,client.priority,client.degree,provider[serve].name);
			if (queue[serve].tail == 2) queue[serve].tail = 0;
			if (flag == 0 || queue[serve].size == 1) pthread_cond_broadcast(&cond[serve]);
			pthread_mutex_unlock(&mutex[serve]);
			}
			pthread_mutex_unlock(&mutexsrv);
        }
      	for (int i = 0; i < countOfProvider; i++) 
  			pthread_detach(threads[i]);
    return 0;
}

int readfile(const char *filename)
{
    FILE *ptr = NULL;
    char arr[256] = {0};
    char *line = NULL;
    if ((ptr = fopen(filename,"r")) == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);    
    }

    int countofprovider = 0;
    int j = 0;
    fgets(arr,256,ptr);
    while(!feof(ptr) && fgets(arr,256,ptr) != NULL)
        ++countofprovider;
    fseek(ptr,0,SEEK_SET);
    fgets(arr,256,ptr);
    while(!feof(ptr) && fgets(arr,256,ptr) != NULL) {
          line = strtok(arr," ");
          strncpy(provider[j].name,line,strlen(line));
          int i = 0;
          provider[j].id = j;
          provider[j].countOfServe = 0;
          provider[j].Islogout = 1;
          queue[j].head = 0;
          queue[j].tail = 0;
          queue[j].size = 0;
          while(line != NULL) {
                line = strtok(NULL," ");
                if (line != NULL) {
                    switch(i) {
                        case 0: provider[j].performance = atoi(line); break;
                        case 1: provider[j].price = atoi(line); break;
                        case 2: provider[j].duration = atoi(line); break;
                    }
                    ++i;
                }
          }
          ++j;
    }
	fclose(ptr);
    return countofprovider;
}

int whichProviderServe(char priority) {
    
    int which_provider_serve = -1; 
    if (priority == 'C') {
        int min = 1000000;
        for (int i = 0; i < countOfProvider; ++i) {
                if (queue[i].size == 0 && provider[i].price < min && provider[i].duration >= 0) {
                    min = provider[i].price;
                    which_provider_serve = i;
                }
                else if (queue[i].size == 1 && provider[i].price < min && provider[i].duration >= 15) {
                    min = provider[i].price;
                    which_provider_serve = i;
                }

        }
    }
    else if (priority == 'Q') {
        int min = -10000000;
        for (int i = 0; i < countOfProvider; ++i) {
                    if (queue[i].size == 1 && provider[i].performance > min && provider[i].duration >= 15) {
                        min = provider[i].performance;
                        which_provider_serve = i;
                    }
                    else if (queue[i].size == 0 && provider[i].performance > min && provider[i].duration >= 0) {
                        min = provider[i].performance;
                        which_provider_serve = i;
                    } 

        }
    }
    /*find least busy*/
    else if (priority == 'T') {

        for (int i = 0; i < countOfProvider; ++i) {
        		if (queue[i].size == 0 && provider[i].duration >=0) {
                    return i;
                }
                else if (queue[i].size == 1 && provider[i].duration >= 15) {
                    which_provider_serve = i;
                }                

        }
    }

    return which_provider_serve;
}

void *thread_function(void *arg) {
    while(1) {
    	PROVIDER prov =  *(PROVIDER *)arg;
        int index = prov.id;
        int num = 0;
        pthread_mutex_lock(&mutexsrv);
        pthread_mutex_lock(&mutex[index]);
        if (provider[index].duration < 0 && queue[index].size == 0) {
        	pthread_mutex_unlock(&mutexsrv);
        	pthread_mutex_unlock(&mutex[index]);
        	fprintf(stderr, "Provider %s is logged out.\n", prov.name);
        	fprintf(log_file, "Provider %s is logged out.\n", prov.name);
        	pthread_exit(0);
        }
        if (signalr == 1) {
        	pthread_mutex_unlock(&mutexsrv);
        	pthread_mutex_unlock(&mutex[index]);
        	pthread_exit(0);
        }
        if (queue[index].size == 0) {
        	fprintf(stderr, "Provider %s is waiting for tasks.\n",prov.name);
        	fprintf(log_file, "Provider %s is waiting for tasks.\n",prov.name);
        	pthread_mutex_unlock(&mutexsrv);
            pthread_cond_wait(&cond[index],&mutex[index]);
        }
        else {
        	srand(time(0));
        	num = rand() % 11 + 5;
            int degree = queue[index].degree[queue[index].head];
            int fd = queue[index].fd[queue[index].head];
            char buffer[1024] = {0};
            provider[index].duration -= num;
            fprintf(stderr, "Provider %s is processing task number %d:%d\n", prov.name,queue[index].head + 1,degree);
            fprintf(log_file, "Provider %s is processing task number %d:%d\n", prov.name,queue[index].head + 1,degree);
            double calculate = cosinus(degree,10);
           	fprintf(stderr, "Provider %s completed task number %d : cos(%d) = %.3f in %d seconds\n", 
           								prov.name,queue[index].head + 1,degree,calculate,num);
           	fprintf(log_file, "Provider %s completed task number %d : cos(%d) = %.3f in %d seconds\n", 
           								prov.name,queue[index].head + 1,degree,calculate,num);
            sprintf(buffer,"%s's task completed by %s in %d seconds,cos(%d) = %.3f,cost is %dTL.",
            	                              queue[index].name[queue[index].head],prov.name,
            	                              num,degree,calculate,prov.price);
            send(fd,buffer,sizeof(buffer),0);
            ++queue[index].head;
            --queue[index].size;
            ++provider[index].countOfServe;
            if (queue[index].head == 2) queue[index].head = 0;
            flag=1; 
        }
        pthread_mutex_unlock(&mutex[index]);
        pthread_mutex_unlock(&mutexsrv);
        if (flag == 1) { 
          sleep(num);
          flag = 0;
       }
    }

}

int factorial(int number)
{
  int factorial = 1 ,i = 0;
  for (i = 0; i < number ; ++i){
    factorial = factorial*(i+1);
  }
  return factorial;
}
/*calculate cosinus using taylor series*/
double cosinus(int angle_part1,int  numberOfTerm)
{
  int j = 0 , sign = -1;
  double radian=0;
  double result  = 0;
  angle_part1=(angle_part1)%360;
  radian = ((angle_part1)*PI)/180.0;
  for (j = 0; j < numberOfTerm*2; j+=2)
  {   
    sign = sign*-1;
    result+= (pow(radian,j)/factorial(j))*sign;
  }
  return result;
}