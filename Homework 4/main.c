/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: cengo
 *
 * Created on May 9, 2018, 10:49 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/sem.h>
#define PERMS (S_IRUSR | S_IWUSR)
#define EMPTY -1
#define SUGAR  1
#define FLOUR  2
#define EGGS   3
#define BUTTER 4
/*informations of chef*/
typedef struct chef {
    char name[10];
    /*name of lock ingredients.*/
    char nameOfLockIngredients[2][10];
    /*available ingredients*/
    int infiniteIngredients[2];
     /*lock ingredients*/
    int lockIngredients[2];
}CHEF;

int initelement(int semid, int semnum, int semvalue);
void setsembuf(struct sembuf *s, int num, int op, int flg);
int removesem(int semid);
int r_semop(int semid, struct sembuf *sops, int nsops);
void createIngredient(char *name);
void createdesert(CHEF *chef);
struct sembuf semsignal;
struct sembuf semwait;
int semid;
int *randomIngredients;
const char *share = "shared";
pid_t  pidArray[7];
void signal_handler(int signo) {
        fprintf(stdout, "\nAll processes killed.\n");
        for(int i = 0; i < 7; ++i)
            kill(pidArray[i],SIGTERM);
        shm_unlink(share);
        removesem(semid);
        exit(0);
}
int main(int argc, char** argv) {
    const int SIZE = 2;
    int shm_fd;
    void* ptr;
    pid_t pid = 0;
    struct sigaction act;
    act.sa_handler = signal_handler;
    act.sa_flags = 0;
    int ingredient[6][2];
    int lockingredients[6][2];
    ingredient[0][0] = SUGAR;
    ingredient[0][1] = EGGS;
    lockingredients[0][0] = BUTTER; 
    lockingredients[0][1] = FLOUR;
    ingredient[1][0] = SUGAR;
    ingredient[1][1] = BUTTER;
    lockingredients[1][0] = EGGS; 
    lockingredients[1][1] = FLOUR;
    ingredient[2][0] = SUGAR;    
    ingredient[2][1] = FLOUR;
    lockingredients[2][0] = BUTTER; 
    lockingredients[2][1] = EGGS;
    ingredient[3][0] = EGGS;
    ingredient[3][1] = FLOUR;
    lockingredients[3][0] = BUTTER;
    lockingredients[3][1] = SUGAR;
    ingredient[4][0] = EGGS;
    ingredient[4][1] = BUTTER;
    lockingredients[4][0] = SUGAR;
    lockingredients[4][1] = FLOUR;
    ingredient[5][0] = BUTTER;
    ingredient[5][1] = FLOUR;
    lockingredients[5][0] = EGGS;
    lockingredients[5][1] = SUGAR;
    if ((sigemptyset(&act.sa_mask) == -1) || 
        (sigaction(SIGINT, &act, NULL) == -1)) {
        perror("Failed to set Signal handler.");
        exit(1);
    }
    if ((semid = semget(IPC_PRIVATE, 1, PERMS)) == -1) 
        return 1;
    setsembuf(&semwait, 0, -1, 0);
    setsembuf(&semsignal, 0, 1, 0);
    if (initelement(semid, 0, 0) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        removesem(semid);
        return 1;
    }
    shm_fd = shm_open(share, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SIZE);
    ptr = mmap(0, sizeof(SIZE)*2, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    randomIngredients = (int *)ptr;
    int ingredients[2];
    while(1) {
        ingredients[0] = rand() % 3 + 1;
        ingredients[1] = rand() % 3 + 1;
        if (ingredients[0] !=  ingredients[1]) {
            randomIngredients[0] = ingredients[0];
            randomIngredients[1] = ingredients[1];
            char *name = "wholesaler";
            if ((ingredients[0] == SUGAR && ingredients[1] == EGGS) || (ingredients[1] == SUGAR && ingredients[0] == EGGS))
                fprintf(stdout,"%s create sugar and eggs\n",name);
            else if ((ingredients[0] == SUGAR && ingredients[1] == FLOUR) || (ingredients[1] == SUGAR && ingredients[0] == FLOUR))
                fprintf(stdout,"%s create sugar and flour\n",name);
            else if ((ingredients[0] == SUGAR && ingredients[1] == BUTTER) || (ingredients[1] == SUGAR && ingredients[0] == BUTTER))
                fprintf(stdout,"%s create sugar and butter\n",name);
            else if ((ingredients[0] == BUTTER && ingredients[1] == EGGS) || (ingredients[1] == BUTTER && ingredients[0] == EGGS))
                fprintf(stdout,"%s create butter and eggs\n",name);
            else if ((ingredients[0] == FLOUR && ingredients[1] == EGGS) || (ingredients[1] == FLOUR && ingredients[0] == EGGS))
                fprintf(stdout,"%s create flour and eggs\n",name);
            else if ((ingredients[0] == BUTTER && ingredients[1] == FLOUR) || (ingredients[1] == BUTTER && ingredients[0] == FLOUR))
                fprintf(stdout,"%s create butter and flour\n",name);
            break;

        }
    }
     r_semop(semid, &semsignal, 1);
    int i;

    for (i = 0; i < 7; ++i) {

        pid = fork();
        pidArray[i] = pid;
        if (pid == -1)
            return 1;

        if (pid == 0) { 
            break;
        }
    }
        if (pid == 0) {

            sigset_t newsigset;
            if ((sigemptyset(&newsigset) == -1) ||
                (sigaddset(&newsigset, SIGINT) == -1))
                    perror("Failed to initialize the signal set");
            else if (sigprocmask(SIG_BLOCK, &newsigset, NULL) == -1)
                    perror("Failed to block SIGINT");
            char chefNo[2];
            CHEF chef;
            switch(i) {
                case 0:
                        srand(time(0));
                        while(1) createIngredient("wholesaler ");
                        break;
                case 1:
                        sprintf(chefNo,"%d",1);
                        chef.infiniteIngredients[0] = ingredient[0][0];
                        chef.infiniteIngredients[1] = ingredient[0][1];
                        chef.lockIngredients[0] = lockingredients[0][0];
                        chef.lockIngredients[1] = lockingredients[0][1];
                        strcpy(chef.name,"chef ");
                        strcat(chef.name,chefNo);
                        strcpy(chef.nameOfLockIngredients[0],"butter"); 
                        strcpy(chef.nameOfLockIngredients[1],"flour"); 
                        while(1) createdesert(&chef);
                        break;
                case 2:
                        sprintf(chefNo,"%d",2);
                        chef.infiniteIngredients[0] = ingredient[1][0];
                        chef.infiniteIngredients[1] = ingredient[1][1];
                        chef.lockIngredients[0] = lockingredients[1][0];
                        chef.lockIngredients[1] = lockingredients[1][1];
                        strcpy(chef.name,"chef ");
                        strcat(chef.name,chefNo);
                        strcpy(chef.nameOfLockIngredients[0],"eggs");
                        strcpy(chef.nameOfLockIngredients[1],"flour"); 
                        while(1) createdesert(&chef);
                        break;
                case 3:
                        sprintf(chefNo,"%d",3);
                        chef.infiniteIngredients[0] = ingredient[2][0];
                        chef.infiniteIngredients[1] = ingredient[2][1];
                        chef.lockIngredients[0] = lockingredients[2][0];
                        chef.lockIngredients[1] = lockingredients[2][1];
                        strcpy(chef.name,"chef ");
                        strcat(chef.name,chefNo);
                        strcpy(chef.nameOfLockIngredients[0],"eggs"); 
                        strcpy(chef.nameOfLockIngredients[1],"butter"); 
                        while(1) createdesert(&chef);
                        break;
                case 4:
                        sprintf(chefNo,"%d",4);
                        chef.infiniteIngredients[0] = ingredient[3][0];
                        chef.infiniteIngredients[1] = ingredient[3][1];
                        chef.lockIngredients[0] = lockingredients[3][0];
                        chef.lockIngredients[1] = lockingredients[3][1];
                        strcpy(chef.name,"chef ");
                        strcat(chef.name,chefNo);
                        strcpy(chef.nameOfLockIngredients[0],"sugar");
                        strcpy(chef.nameOfLockIngredients[1],"butter");
                        while(1) createdesert(&chef);
                        break;
                case 5:
                        sprintf(chefNo,"%d",5);
                        chef.infiniteIngredients[0] = ingredient[4][0];
                        chef.infiniteIngredients[1] = ingredient[4][1];
                        chef.lockIngredients[0] = lockingredients[4][0];
                        chef.lockIngredients[1] = lockingredients[4][1];
                        strcpy(chef.name,"chef ");
                        strcat(chef.name,chefNo);
                        strcpy(chef.nameOfLockIngredients[0],"sugar");
                        strcpy(chef.nameOfLockIngredients[1],"flour");
                        while(1) createdesert(&chef);
                        break;
                case 6:
                        sprintf(chefNo,"%d",6);
                        chef.infiniteIngredients[0] = ingredient[5][0];
                        chef.infiniteIngredients[1] = ingredient[5][1];
                        chef.lockIngredients[0] = lockingredients[5][0];
                        chef.lockIngredients[1] = lockingredients[5][1];
                        strcpy(chef.name,"chef ");
                        strcat(chef.name,chefNo);
                        strcpy(chef.nameOfLockIngredients[0],"sugar");
                        strcpy(chef.nameOfLockIngredients[1],"eggs");
                        while(1) createdesert(&chef);
                        break;
            }  
            
        }
        else {
            wait(NULL);
            shm_unlink(share);
            removesem(semid);
            return (EXIT_SUCCESS);
        }

}
void createIngredient(char *name) {
    int ingredient[2];
    int flag = 0; 
    r_semop(semid, &semwait, 1);
        if (randomIngredients[0] == EMPTY && randomIngredients[1] == EMPTY) {
            while(1) {
                ingredient[0] = rand() % 4 + 1;
                ingredient[1] = rand() % 4 + 1;
                if (ingredient[0] !=  ingredient[1]) {
                    randomIngredients[0] = ingredient[0];
                    randomIngredients[1] = ingredient[1];
                    if ((ingredient[0] == SUGAR && ingredient[1] == EGGS) || (ingredient[1] == SUGAR && ingredient[0] == EGGS))
                        fprintf(stdout,"%s create sugar and eggs\n",name);
                    else if ((ingredient[0] == SUGAR && ingredient[1] == FLOUR) || (ingredient[1] == SUGAR && ingredient[0] == FLOUR))
                        fprintf(stdout,"%s create sugar and flour\n",name);
                    else if ((ingredient[0] == SUGAR && ingredient[1] == BUTTER) || (ingredient[1] == SUGAR && ingredient[0] == BUTTER))
                        fprintf(stdout,"%s create sugar and butter\n",name);
                    else if ((ingredient[0] == BUTTER && ingredient[1] == EGGS) || (ingredient[1] == BUTTER && ingredient[0] == EGGS))
                        fprintf(stdout,"%s create butter and eggs\n",name);
                    else if ((ingredient[0] == FLOUR && ingredient[1] == EGGS) || (ingredient[1] == FLOUR && ingredient[0] == EGGS))
                        fprintf(stdout,"%s create flour and eggs\n",name);
                    else if ((ingredient[0] == BUTTER && ingredient[1] == FLOUR) || (ingredient[1] == BUTTER && ingredient[0] == FLOUR))
                        fprintf(stdout,"%s create butter and flour\n",name);
                     r_semop(semid, &semsignal, 1);                      
                    break;

                }
            }
        }
        else {
            fprintf(stdout,"%s wait for dessert.\n",name);
            r_semop(semid, &semsignal, 1);
        } 
}
void createdesert(CHEF *chef) {
     r_semop(semid, &semwait, 1);
        if((chef->lockIngredients[0] == randomIngredients[0] || chef->lockIngredients[0] == randomIngredients[1])&&
           (chef->lockIngredients[1] == randomIngredients[0] || chef->lockIngredients[1] == randomIngredients[1])) {
            if ((randomIngredients[0] == SUGAR && randomIngredients[1] == EGGS) || 
                  (randomIngredients[1] == SUGAR && randomIngredients[0] == EGGS))
                            fprintf(stdout,"%s takes sugar,eggs and make desert\n",chef->name);
            else if ((randomIngredients[0] == SUGAR && randomIngredients[1] == FLOUR) || 
                       (randomIngredients[1] == SUGAR && randomIngredients[0] == FLOUR))
                            fprintf(stdout,"%s takes sugar,flour  and make desert\n",chef->name);   
            
            else if ((randomIngredients[0] == SUGAR && randomIngredients[1] == BUTTER) || 
                       (randomIngredients[1] == SUGAR && randomIngredients[0] == BUTTER))
                            fprintf(stdout,"%s takes sugar,butter and make desert\n",chef->name);
            
            else if ((randomIngredients[0] == BUTTER && randomIngredients[1] == EGGS) || 
                       (randomIngredients[1] == BUTTER && randomIngredients[0] == EGGS))
                            fprintf(stdout,"%s takes butter,eggs and make desert\n",chef->name);
            
            else if ((randomIngredients[0] == FLOUR && randomIngredients[1] == EGGS) || 
                       (randomIngredients[1] == FLOUR && randomIngredients[0] == EGGS)) 
                            fprintf(stdout,"%s takes flour,eggs and make desert\n",chef->name);   
            
            else if ((randomIngredients[0] == BUTTER && randomIngredients[1] == FLOUR) || 
                       (randomIngredients[1] == BUTTER && randomIngredients[0] == FLOUR))
                            fprintf(stdout,"%s takes  butter,flour and make desert\n",chef->name);
            fprintf(stdout, "wholesaler obtained dessert from  %s\n",chef->name);
            randomIngredients[0] = EMPTY;
            randomIngredients[1] = EMPTY;
        }
        else fprintf(stdout,"%s wait for %s and %s.\n",chef->name,chef->nameOfLockIngredients[0],chef->nameOfLockIngredients[1]);
    r_semop(semid, &semsignal, 1);
}

/*Bellow functions takes from this books:*/
/*Unix Systems Programming , Communications, Concurrency and Threads”, Prentice Hall, Robins and
    Robins, 2003 (Supplementary)*/
int r_semop(int semid, struct sembuf *sops, int nsops) {
   while (semop(semid, sops, nsops) == -1) 
      if (errno != EINTR) 
         return -1;
   return 0; 
}

int initelement(int semid, int semnum, int semvalue) {
   union semun {
      int val;
      struct semid_ds *buf;
      unsigned short *array;
   } arg;
   arg.val = semvalue;
   return semctl(semid, semnum, SETVAL, arg);
}


void setsembuf(struct sembuf *s, int num, int op, int flg) {
   s->sem_num = (short)num;
   s->sem_op = (short)op;
   s->sem_flg = (short)flg;
   return;
}

int removesem(int semid) {
   return semctl(semid, 0, IPC_RMID);
}