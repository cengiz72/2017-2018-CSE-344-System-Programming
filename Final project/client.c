#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
typedef struct client {
    char name[20];
    char priority;
    int degree;
}CLIENT;
  
int main(int argc, char const *argv[])
{
    if (argc < 6) {
        fprintf(stderr, "Usage : %s <name> <priority : C,Q or T> <int degree> <server adress: 127.0.0.1> <server port adress : 5555>\n",argv[0]);
        exit(-1);
    }
    else if (!strcmp(argv[2],"Q") == 0 && !strcmp(argv[2],"T") == 0 && !strcmp(argv[2],"C") == 0) {
        fprintf(stderr, "Usage : %s <name> <priority : C,Q or T> <int degree> <server adress: 127.0.0.1> <server port adress : 5555>\n",argv[0]);
        exit(-1);
    }
    else if (!strcmp(argv[4],"127.0.0.1") == 0) {
        fprintf(stderr, "Usage : %s <name> <priority : C,Q or T> <int degree> <server adress: 127.0.0.1> <server port adress : 5555>\n",argv[0]);
        exit(-1);
    }
    else if (!strcmp(argv[5],"5555") == 0) {
        fprintf(stderr, "Usage : %s <name> <priority : C,Q or T> <int degree> <server adress: 127.0.0.1> <server port adress : 5555>\n",argv[0]);
        exit(-1);
    }
    struct sockaddr_in address;
    int sock = 0, valread = 0;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    CLIENT client;
    strcpy(client.name,argv[1]);
    client.priority = argv[2][0];
    client.degree = atoi(argv[3]);
    int PORT_ADDRESS = atoi(argv[5]);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_ADDRESS);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[4], &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    fprintf(stderr, "Client %s is requesting %c %d from server 127.0.0.1:%d\n", client.name,client.priority,client.degree,PORT_ADDRESS); 
    send(sock , (void *)&client ,sizeof(client), 0);
    valread = read(sock ,buffer, sizeof(buffer));
    if (strstr(buffer,"SERVER SHUTDOWN") != NULL) {
        fprintf(stderr, "Client %s received SERVER SHUTDOWN signal from server\n", client.name);
    }
    else if (strstr(buffer,"NO PROVIDER IS AVAILABLE") != NULL) {
        fprintf(stderr, "Client %s received  signal NO PROVIDER IS AVAILABLE from server\n", client.name);
    } 
    else  fprintf(stderr, "\n%s\n", buffer);

    return 0;
}
