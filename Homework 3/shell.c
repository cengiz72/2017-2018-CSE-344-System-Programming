#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#define PIPE 				"|"
#define REDIRECTIONOFSTDOUT ">"
#define REDIRECTIONOFSTDIN 	"<"
#define CAT 				"cat"
#define LS 					"ls"
#define WC 					"wc"
#define CD 					"cd"
#define PWD 				"pwd"
#define HELP 				"help"
#define EXIT 				"exit"
int runFiveArguments(char args[][256],char *currentDirectory);
int IsPipeOrRedirection(char * arg);
int wrongFormat(char args[][256],int size);
int run(char args[][256],int size,char *currentDirectory);
int type_parameters(char args[][256]) ;
void freeMemory(char args[][256]);
void allocate(char args[][256]);
int cd(char *arg, char *currentDirectory);
int isDirectory(char *pathName);
int IsCommand(char *command);
int runOneArguments(char args[][256],char *currentDirectory);
int runThreeArguments(char args[][256],char *currentDirectory);
int runTwoArguments(char args[][256],char *currentDirectory);          
void help();
int exit_();
int pwd(char *currentDirectory);
void signal_handler(int signo) {

	if (signo == SIGINT)
		fprintf(stdout, "\nCtrl + c handled by.\n");
	else if (signo == SIGTERM)
		fprintf(stdout, "\nSIGTERM hendled by.\n");

	exit(0);
}
int main(int argc, char const *argv[])
{ 
	/*
		en fazla 5 adet string girilebilir.
		wc ve cat te biraz sikinti  var dosya
		alirken 
	*/
	char args[50][256];
	char currentDirectory[512];
	getcwd(currentDirectory, 512);
	int size = 0;
  	struct sigaction act;
  	act.sa_handler = signal_handler;
  	act.sa_flags = 0;
  	if ((sigemptyset(&act.sa_mask) == -1) || 
  	  	(sigaction(SIGINT, &act, NULL) == -1) ||
  	  	(sigaction(SIGTERM, &act, NULL) == -1)) {
  		perror("Failed to set Signal handler.");
  		return 1;
  	}
	for ( ; ; ) {
		
		fprintf(stdout, "gtu@shell : ~%s$ ", currentDirectory);
		size = type_parameters(args);
		if (size > 0) run(args,size,currentDirectory);
	}
	return 0;
}

int cd(char *arg, char *currentDirectory) {

	char tempPath[512];
	DIR *dirp = NULL;
	struct dirent *direntp = NULL;
	if (strcmp(arg, "..") == 0) {
		getcwd(tempPath, 512);
		chdir(currentDirectory);
		chdir("..");
		getcwd(currentDirectory, 512);
		chdir(tempPath);
		return 1;
	}
	else {

		if ((dirp = opendir(currentDirectory)) == NULL) {
			fprintf(stderr, "Faield to open %s: %s\n", currentDirectory, strerror(errno));
			return -1;
		}

		while ((direntp = readdir(dirp)) != NULL) {


			if (strcmp(direntp->d_name,arg) == 0 && strcmp(direntp->d_name,".")) {

				
				strcpy(tempPath,currentDirectory);
				strcat(tempPath,"/");
				strcat(tempPath,direntp->d_name);
				if (isDirectory(tempPath)) {
					strcpy(currentDirectory,tempPath);
					return 1;
				}
				else  {
					fprintf(stderr, "\n%s is not a directory\n", arg);
					return -1;
				}
			}
			else {
				fprintf(stderr, "No such file or directory\n");
				return -1;
			}
		}
	}
}
int type_parameters(char args[][256]) {

	char arr[256];
	char *ptr;
	int i = 0;
	int size = 256;
	fgets(arr,size,stdin);

	ptr = strtok(arr," \n\t");
	while (ptr != NULL) {
		strcpy(args[i++],ptr);
		ptr = strtok(NULL," \n\t");
	}
	
	if (wrongFormat(args,i) == 0) return 0;
	return i;
}

int isDirectory(char *pathName) {
	struct stat statbuf;
	if (stat(pathName, &statbuf) == -1)
		return 0;
	else
		return S_ISDIR(statbuf.st_mode);
}
int runOneArguments(char args[][256],char *currentDirectory) {
	if (strcmp(args[0],LS) == 0) {
		
		pid_t childPid = fork();

		if (childPid == -1) {
			fprintf(stderr, "Failed to fork : %s\n", strerror(errno));
			return -1;
		}

		if (childPid == 0) {
			if (execlp("./ls", currentDirectory,NULL) == -1) {
				fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
				return -1;
			}

		}

		else {
			
			waitpid(childPid,0,0);
			return 1;
		}					
	}
	else if (strcmp(args[0],CAT) == 0) {
		pid_t childPid = fork();

		if (childPid == -1) {
			fprintf(stderr, "Failed to fork : %s\n", strerror(errno));
			return -1;
		}

		if (childPid == 0) {
			if (execlp("./CAT",NULL) == -1) {
				fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
				return -1;
			}

		}

		else {
			
			waitpid(childPid,0,0);
			return 1;
		}
	}
	else if (strcmp(args[0],WC) == 0) {
		pid_t childPid = fork();

		if (childPid == -1) {
			fprintf(stderr, "Failed to fork : %s\n", strerror(errno));
			return -1;
		}

		if (childPid == 0) {
			if (execlp("./WC",NULL) == -1) {
				fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
				return -1;
			}

		}

		else {

			waitpid(childPid,0,0);
			return 1;
		}
	}
	else if (strcmp(args[0],CD) == 0) return cd(args[0],currentDirectory);
	else if (strcmp(args[0],PWD) == 0) {
		return pwd(currentDirectory);
	}
	else if (strcmp(args[0],HELP) == 0) {
		help();
		return 1;
	}
	else if (strcmp(args[0],EXIT) == 0) return exit_();
}
int runTwoArguments(char args[][256],char *currentDirectory) {
	
	if (strcmp(CAT,args[0]) == 0) {
		int id  = 0 ;
		pid_t childPid = fork();

		if (childPid == -1) {
			fprintf(stderr, "Failed to fork : %s\n", strerror(errno));
			return -1;
		}

		if (childPid == 0) {
			char currentDirectory[255];
			id = open(args[1],O_RDONLY);
			if (id == -1) {
				fprintf(stderr, "Failed to open file : %s\n", strerror(errno));
				return -1;
			}
			dup2(id,STDIN_FILENO);
			if (execlp("./CAT","1",NULL) == -1) {
				fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
				close(id);
				return -1;
			}
		}

		else {
			waitpid(childPid,0,0);
			close(id);
			return 1;
		}
	}
	else if (strcmp(WC,args[0]) == 0) {
		int id  = 0;
		pid_t childPid = fork();

		if (childPid == -1) {
			fprintf(stderr, "Failed to fork : %s\n", strerror(errno));
			return -1;
		}

		if (childPid == 0) {
			id = open(args[1],O_RDONLY);
			if (id == -1) {
				fprintf(stderr, "Failed to open file : %s\n", strerror(errno));
				return -1;
			}
			if(dup2(id,STDIN_FILENO) == -1) {
				fprintf(stdout, "%s\n", strerror(errno));
				return -1;
			}
			if (execlp("./WC","1",NULL) == -1) {
				fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
				close(id);
				return -1;
			}

		}

		else {
			waitpid(childPid,0,0);
			close(id);
			return 1;
		}
	}
	else if (strcmp(args[0],CD) == 0) {
		return cd(args[1],currentDirectory);
	}
	else {
		fprintf(stderr, "\nWrong usage \n");
		return -1;
	}
}
int runThreeArguments(char args[][256],char *currentDirectory) {

	if ((strcmp(args[0],LS) == 0 || strcmp(args[0],WC) || strcmp(args[0],CAT))  
		                         && strcmp(args[1] ,REDIRECTIONOFSTDOUT) == 0)
	{
		pid_t child  = 0;
		if ((child = fork()) == -1) {
			fprintf(stdout, "Failed to fork : %s\n",strerror(errno));
			return -1;
		}
		else if (child == 0){
			int id = open(args[2],O_CREAT | O_WRONLY,0777);
			if (id == -1) {
				fprintf(stdout, "Failed to open file: %s\n",strerror(errno));
				return -1;
			}
			if (dup2(id,STDOUT_FILENO) == -1) {
				fprintf(stdout, "Failed to dup : %s\n",strerror(errno));
				return -1;
			}
			if (strcmp(args[0],LS) == 0) {
				if (execlp("./ls", currentDirectory,NULL) == -1) {
					fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
					return -1;
				}
			}
			else if (strcmp(args[0],CAT) == 0) {
				if (execlp("./CAT",NULL) == -1) {
					fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
					return -1;
				}
			}
			else if (strcmp(args[0],WC) == 0) {
				if (execlp("./WC",NULL) == -1) {
					fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
					return -1;
				}
			}
		}
		else {
			waitpid(child,0,0);
			return 1;
		}
	}
	else if ((strcmp(args[0],CAT) == 0 || strcmp(args[0],WC) == 0)  && strcmp(args[1],REDIRECTIONOFSTDIN) == 0) {
		pid_t child = 0;
		if ((child = fork()) == -1) {
			fprintf(stdout, "Failed to fork : %s\n",strerror(errno));
			return -1;
		}
		else if (child == 0){
			int id = open(args[2],O_CREAT | O_RDONLY,0777);
			if (id == -1) {
				fprintf(stdout, "Failed to open file: %s\n",strerror(errno));
				return -1;
			}
			if (dup2(id,STDIN_FILENO) == -1) {
				fprintf(stdout, "Failed to dup : %s\n",strerror(errno));
				return -1;
			}
			if (strcmp(args[0],CAT) == 0) {
				if (execlp("./CAT","1",NULL) == -1) {
					fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
					return -1;
				}
			}
			else if (strcmp(args[0],WC) == 0) {
				if (execlp("./WC","1",NULL) == -1) {
					fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
					return -1;
				}
			}

		}
		else {
			waitpid(child,0,0);
			return 1;
		}
	}
	else if ((strcmp(args[0],LS) == 0)  && strcmp(args[1],PIPE) == 0 && 
		     (strcmp(args[2],WC) == 0 || strcmp(args[2],CAT) == 0)) 
	{
		pid_t childPid1 = 0;
		pid_t childPid2 = 0;
		int _pipe[2];
		if ((childPid1 = fork())== -1) {
			fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
			return -1;
		} 
		else if (childPid1 == 0) {
			if (pipe(_pipe) < 0) {
				fprintf(stdout, "Failed to pipe : %s\n", strerror(errno));
				return -1;
			}
			if ((childPid2  = fork())== -1) {
				fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
				return -1;
			} 
			else if (childPid2 == 0) {
				dup2(_pipe[0],STDIN_FILENO);
				close(_pipe[1]);
				if (strcmp(args[2],WC) == 0) {
					if (execlp("./WC","1",NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1;
					}
				}
				else if (strcmp(args[2],CAT) == 0) {
					if (execlp("./CAT","1",NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1; 
					}
				}
				
			}
			else {
				char current[255];
				dup2(_pipe[1],STDOUT_FILENO);
				close(_pipe[0]);
				if (execlp("./ls",currentDirectory,NULL) == -1) {
					fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
					return -1;
				}
			}
		}
		else {

			waitpid(childPid1,0,0);
			waitpid(childPid2,0,0);
			return -1;
		}
	} 
	else if ((strcmp(args[0],WC)  == 0 || strcmp(args[0],CAT)  == 0)  && strcmp(args[1],PIPE) == 0
	        && (strcmp(args[2],CAT) == 0 || strcmp(args[2],WC)  == 0))
	{
		pid_t childPid1 = 0;
		pid_t childPid2 = 0;
		int _pipe[2];
		if ((childPid1 = fork())== -1) {
			fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
			return -1;
		} 
		else if (childPid1 == 0) {
			if (pipe(_pipe) < 0) {
				fprintf(stdout, "Failed to pipe : %s\n", strerror(errno));
				return -1;
			}
			if ((childPid2  = fork())== -1) {
				fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
				return -1;
			} 
			else if (childPid2 == 0) {
				dup2(_pipe[1],STDOUT_FILENO);
				close(_pipe[0]);
				if (strcmp(args[2],WC) == 0) {
					if (execlp("./WC",NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1;
					}
				}
				else if (strcmp(args[2],CAT) == 0) {
					if (execlp("./CAT",NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1; 
					}
				}
				
			}
			else {
				char current[255];
				dup2(_pipe[0],STDIN_FILENO);
				close(_pipe[1]);
				if (strcmp(args[2],CAT) == 0) {
					if (execlp("./CAT","1",NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1;
					}
				}
				else if (strcmp(args[2],WC) == 0) {
					if (execlp("./WC","1",NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1;
					}
				}
			}
		}
		else {

			waitpid(childPid1,0,0);
			waitpid(childPid2,0,0);
			return 1;
		}
	}
	return -1;
}
int runFiveArguments(char args[][256],char *currentDirectory) {
	if (strcmp(args[0],LS) == 0 && strcmp(args[1],PIPE) == 0 && 
       (strcmp(args[2],WC) == 0 || strcmp(args[2],CAT) == 0) &&
       ((strcmp(args[3],PIPE) == 0 && (strcmp(args[4],CAT) == 0) || strcmp(args[4],WC) == 0)
       || (strcmp(args[3],REDIRECTIONOFSTDOUT) == 0))) {

       	if (strcmp(args[3],REDIRECTIONOFSTDOUT) == 0) {
			pid_t childPid1 = 0;
			pid_t childPid2 = 0;
			int _pipe[2];
			if ((childPid1 = fork())== -1) {
				fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
				return -1;
			} 
			else if (childPid1 == 0) {
				if (pipe(_pipe) < 0) {
					fprintf(stdout, "Failed to pipe : %s\n", strerror(errno));
					return -1;
				}
				if ((childPid2  = fork())== -1) {
					fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
					return -1;
				} 
				else if (childPid2 == 0) {
					int id = open(args[4],O_CREAT | O_WRONLY,0777);
					if (id == -1) {
						fprintf(stderr, " File : %s\n", strerror(errno));
						return -1;
					}
					if (dup2(_pipe[0],STDIN_FILENO) == -1) {
					    fprintf(stderr, "%s\n", strerror(errno));
					    return -1;
					    close(id);
					}
					close(_pipe[1]);
					if (strcmp(args[2],WC) == 0) {
						if (dup2(id,STDOUT_FILENO) == -1) {
						    fprintf(stderr, "%s\n", strerror(errno));
						    return -1;
						    close(id);
						}
						if (execlp("./WC","1",NULL) == -1) {
							fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
							return -1;
							 close(id);
						}
					}
					else if (strcmp(args[2],CAT) == 0) {
						if (dup2(id,STDOUT_FILENO) == -1) {
						    fprintf(stderr, "%s\n", strerror(errno));
						    return -1;
						     close(id);
						}
						if (execlp("./CAT","1",NULL) == -1) {
							fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
							return -1; 
							 close(id);
						}
					}
					
				}
				else {
					char current[255];
					dup2(_pipe[1],STDOUT_FILENO);
					close(_pipe[0]);
					if (execlp("./ls",currentDirectory,NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1;
					}
				}
       	    }
	       	else {
	       		waitpid(childPid2,0,0);
	       		waitpid(childPid1,0,0);
	       		return 0;
	       	} 
		}
		else if (strcmp(args[3],PIPE) == 0) {
			pid_t childPid1 = 0;
			pid_t childPid2 = 0;
			pid_t childPid3 = 0;
			int _pipe[2];
			if ((childPid1 = fork())== -1) {
				fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
				return -1;
			} 
			else if (childPid1 == 0) {
				if (pipe(_pipe) < 0) {
					fprintf(stdout, "Failed to pipe : %s\n", strerror(errno));
					return -1;
				}
				if ((childPid2  = fork())== -1) {
					fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
					return -1;
				} 
				else if (childPid2 == 0) {

					if (dup2(_pipe[0],STDIN_FILENO) == -1) {
					    fprintf(stderr, "%s\n", strerror(errno));
					    return -1;
					 
					}
					close(_pipe[1]);
					if (strcmp(args[2],WC) == 0) {
						int pipe_[2];
						if (pipe(pipe_) < 0) {
							fprintf(stderr, "%s\n",strerror);
							return -1;
						}
						if ((childPid3 = fork()) == -1) {
							fprintf(stderr, "%s\n",strerror(errno));
							return -1;
						}
						else if (childPid3==0) {
							if (dup2(pipe_[0],STDIN_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[1]);
							if (strcmp(args[4],WC) == 0) {
								if (execlp("./WC","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
									 
								}
							}
							else if (strcmp(args[4],CAT) == 0) {
								if (execlp("./CAT","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
								 
							   }
							}
						}
						else {
							if (dup2(pipe_[1],STDOUT_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[0]);
							if (execlp("./WC","1",NULL) == -1) {
								fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
								return -1;
								 
							}
							return 1;
						}
					}
					else if (strcmp(args[2],CAT) == 0) {
						
						int pipe_[2];
						if (pipe(pipe_) < 0) {
							fprintf(stderr, "%s\n",strerror);
							return -1;
						}
						if ((childPid3 = fork()) == -1) {
							fprintf(stderr, "%s\n",strerror(errno));
							return -1;
						}
						else if (childPid3==0) {
							if (dup2(pipe_[0],STDIN_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[1]);
							if (strcmp(args[4],WC) == 0) {
								if (execlp("./WC","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
									 
								}
							}
							else if (strcmp(args[4],CAT) == 0) {
								if (execlp("./CAT","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
								 
							   }
							}
						}
						else {
							if (dup2(pipe_[1],STDOUT_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[0]);
							if (execlp("./CAT","1",NULL) == -1) {
								fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
								return -1;
								 
							}
							return 1;
						}
					}
					
				}
				else {
					char current[255];
					dup2(_pipe[1],STDOUT_FILENO);
					close(_pipe[0]);
					if (execlp("./ls",currentDirectory,NULL) == -1) {
						fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
						return -1;
					}
				}
       	    }
	       	else {
	       		waitpid(childPid3,0,0);
	       		waitpid(childPid2,0,0);
	       		waitpid(childPid1,0,0);
	       		return 0;
	       	}
		}
	}
	else if ((strcmp(args[0],WC) == 0 || strcmp(args[0],CAT) == 0) && strcmp(args[1],PIPE) == 0 && 
       (strcmp(args[2],WC) == 0 || strcmp(args[2],CAT) == 0) &&
        strcmp(args[3],PIPE) == 0 && (strcmp(args[4],CAT) == 0 || strcmp(args[4],WC) == 0)) {

			pid_t childPid1 = 0;
			pid_t childPid2 = 0;
			pid_t childPid3 = 0;
			int _pipe[2];
			if ((childPid1 = fork())== -1) {
				fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
				return -1;
			} 
			else if (childPid1 == 0) {
				if (pipe(_pipe) < 0) {
					fprintf(stdout, "Failed to pipe : %s\n", strerror(errno));
					return -1;
				}
				if ((childPid2  = fork())== -1) {
					fprintf(stdout, "Failed to fork : %s\n", strerror(errno));
					return -1;
				} 
				else if (childPid2 == 0) {

					if (dup2(_pipe[0],STDIN_FILENO) == -1) {
					    fprintf(stderr, "%s\n", strerror(errno));
					    return -1;
					 
					}
					close(_pipe[1]);
					if (strcmp(args[2],WC) == 0) {
						int pipe_[2];
						if (pipe(pipe_) < 0) {
							fprintf(stderr, "%s\n",strerror);
							return -1;
						}
						if ((childPid3 = fork()) == -1) {
							fprintf(stderr, "%s\n",strerror(errno));
							return -1;
						}
						else if (childPid3==0) {
							if (dup2(pipe_[0],STDIN_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[1]);
							if (strcmp(args[4],WC) == 0) {
								if (execlp("./WC","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
									 
								}
							}
							else if (strcmp(args[4],CAT) == 0) {
								if (execlp("./CAT","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
								 
							   }
							}
						}
						else {
							if (dup2(pipe_[1],STDOUT_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[0]);
							if (execlp("./WC","1",NULL) == -1) {
								fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
								return -1;
								 
							}
							return 1;
						}
					}
					else if (strcmp(args[2],CAT) == 0) {
						
						int pipe_[2];
						if (pipe(pipe_) < 0) {
							fprintf(stderr, "%s\n",strerror);
							return -1;
						}
						if ((childPid3 = fork()) == -1) {
							fprintf(stderr, "%s\n",strerror(errno));
							return -1;
						}
						else if (childPid3==0) {
							if (dup2(pipe_[0],STDIN_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[1]);
							if (strcmp(args[4],WC) == 0) {
								if (execlp("./WC","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
									 
								}
							}
							else if (strcmp(args[4],CAT) == 0) {
								if (execlp("./CAT","1",NULL) == -1) {
									fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
									return -1;
								 
							   }
							}
						}
						else {
							if (dup2(pipe_[1],STDOUT_FILENO) == -1) {
								fprintf(stderr, "%s\n",strerror(errno));
								return -1;
							}
							close(pipe_[0]);
							if (execlp("./CAT","1",NULL) == -1) {
								fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
								return -1;
								 
							}
							return 1;
						}
					}
					
				}
				else {
					dup2(_pipe[1],STDOUT_FILENO);
					close(_pipe[0]);
					if (strcmp(args[0],WC) == 0) {
						if (execlp("./WC",NULL) == -1) {
							fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
							return -1;
						}
					}
					else if (strcmp(args[0],CAT) == 0) {
						if (execlp("./CAT",NULL) == -1) {
							fprintf(stderr, "Failed to exec: %s\n", strerror(errno));
							return -1;
						}
					}
					return 1;
				}
       	    }
	       	else {
	       		waitpid(childPid3,0,0);
	       		waitpid(childPid2,0,0);
	       		waitpid(childPid1,0,0);
	       		return 0;
	       	}
	}
}
int run(char args[][256],int size,char *currentDirectory) {
	switch(size) {

		case 1:  return runOneArguments(args,currentDirectory);
		case 2:  return runTwoArguments(args,currentDirectory);
		case 3:  return runThreeArguments(args,currentDirectory);
		case 5:  return runFiveArguments(args,currentDirectory);
				break;
	}
	return -1;
}
int wrongFormat(char args[][256],int size) {
	switch(size) {

		case 1: 
				if (IsCommand(args[0]) == 1 && !(strcmp(args[0],CD) == 0)) return 1;
				else {
					fprintf(stdout, "\nWrong usage\n");
					return 0;
				}
				break;
		case 2: 
				if (strcmp(args[0],CAT) == 0  || strcmp(args[0],WC) == 0) {
					struct stat st;
					if (stat(args[1],&st) == -1) {
						fprintf(stdout, "\n%s\n",strerror(errno));
						return 0;
					}

					else {
						int id = S_ISREG(st.st_mode);
						if (id == 0) {
							fprintf(stdout, "\n %s is not reguler file .\n",args[1]);
							return 0;
						} 
						else return 1;
					}
				}
				else if (strcmp(args[0],CD) == 0 ) return 1;
				else {  
					fprintf(stdout, "\n Wrong Usage .\n");
					return 0;
				}
				break;
		case 3: 
				if (strcmp(args[0],LS) == 0  && strcmp(args[1],PIPE) == 0 && 
				   (strcmp(args[2],CAT) == 0 || strcmp(args[2],WC) == 0) )
					return 1;
				if ((strcmp(args[0],CAT) == 0 || strcmp(args[0],WC) == 0 ) && strcmp(args[1],PIPE) == 0 
											&& (strcmp(args[2],CAT) == 0 || strcmp(args[2],WC) == 0 ))
					return 1;
				if ((strcmp(args[0],CAT) == 0 || strcmp(args[0],WC) == 0 || strcmp(args[0],LS) == 0)
				                && strcmp(args[1],REDIRECTIONOFSTDOUT) == 0)
					return 1;
				if ((strcmp(args[0],CAT) == 0 || strcmp(args[0],WC) == 0)
				                && strcmp(args[1],REDIRECTIONOFSTDIN) == 0) {
					struct stat st;
					if (stat(args[2],&st) == -1) {
						fprintf(stdout, "\n%s\n",strerror(errno));
						return 0;
					}

					else {
						int id = S_ISREG(st.st_mode);
						if (id == 0) {
							fprintf(stdout, "\n %s is not reguler file .\n",args[2]);
							return 0;
						} 
						else return 1;
					}

				}
				else {

					fprintf(stdout, "\n Wrong Usage.\n");
					return 0;
				}
				break;
		case 4: 
				fprintf(stderr, "Usage error\n");
				return 0;
				break;
		case 5: 
				if (strcmp(args[0],LS) == 0 && strcmp(args[1],PIPE) == 0 && 
			       (strcmp(args[2],WC) == 0 || strcmp(args[2],CAT) == 0) ||
			       (strcmp(args[3],PIPE) == 0 && (strcmp(args[4],CAT) == 0) || strcmp(args[4],WC) == 0)
			       || (strcmp(args[3],REDIRECTIONOFSTDOUT) == 0))
				return 1;
				if ((strcmp(args[0],WC) == 0 || strcmp(args[0],CAT) == 0) && strcmp(args[1],PIPE) == 0 && 
			       (strcmp(args[2],WC) == 0 || strcmp(args[2],CAT) == 0) ||
			       (strcmp(args[3],PIPE) == 0 && (strcmp(args[4],CAT) == 0) || strcmp(args[4],WC) == 0)
			       || (strcmp(args[3],REDIRECTIONOFSTDOUT) == 0))
				return 1;
				else {
					fprintf(stdout, "\nUsage error\n");
					return 0;
				}
				break;
		case 6:
				fprintf(stdout, "\nUsage error\n");
				return 0;
		case 7:
				if (strcmp(args[0],LS) == 0 && strcmp(args[1],PIPE) == 0 && 
			       (strcmp(args[2],WC) == 0 || strcmp(args[2],CAT) == 0) && strcmp(args[3],PIPE) == 0
			       && (strcmp(args[4],WC) == 0 || strcmp(args[4],CAT) == 0) && strcmp(args[5],REDIRECTIONOFSTDOUT) == 0)
				return 1;
				if (((strcmp(args[0],WC) == 0 || strcmp(args[0],CAT) == 0)) && strcmp(args[1],PIPE) == 0 && 
			       (strcmp(args[2],WC) == 0 || strcmp(args[2],CAT) == 0) && strcmp(args[3],PIPE) == 0
			       && (strcmp(args[4],WC) == 0 || strcmp(args[4],CAT) == 0) && strcmp(args[5],REDIRECTIONOFSTDOUT) == 0)
				return 1;
				else {
					fprintf(stdout, "\nUsage error\n");
					return 0;
				}
				break;
		default : return 0;
	}
}

int IsCommand(char *command) {
	if (strcmp(command,PWD)  == 0) return 1;
	if (strcmp(command,HELP) == 0) return 1;
	if (strcmp(command,EXIT) == 0) return 1;
	if (strcmp(command,CAT)  == 0) return 1;
	if (strcmp(command,LS)   == 0) return 1;
	if (strcmp(command,CD)   == 0) return 1;
	if (strcmp(command,WC)   == 0) return 1;
	return 0;
}

int IsPipeOrRedirection(char *arg) {

	if (strcmp(arg,REDIRECTIONOFSTDOUT)) return 1;
	if (strcmp(arg,REDIRECTIONOFSTDIN))  return 1;
	if (strcmp(arg,PIPE)) 				 return 1;
	return 0;
}

void help() {

	fprintf(stdout, ">>>>> ls command         : ls  lists file type, access rights, file size and\n");
	fprintf(stderr, "                      file name of all files in the present working directory.\n");
	fprintf(stdout, ">>>>> pwd command :                pwd  prints the present working directory.\n");
	fprintf(stdout, ">>>>> cd command         :  cd  changes the present working directory to the\n");
	fprintf(stdout,							           			  " location provided as argument.\n");
	fprintf(stdout,  ">>>> help command  :             help prints the list of supported commands.\n");
	fprintf(stdout,  ">>>> cat command   :     		cat prints on standard output the contents of\n");
	fprintf(stderr, 								   		 "the file provided to it as argument.\n");
	fprintf(stdout,  ">>>> wc command         :  wc prints on standard output the number of lines\n"); 
	fprintf(stdout,                            		      "in the file provided to it as argument.\n");
	fprintf(stdout,  ">>>> exit command       :                              exit exits the shell.\n");

}

int exit_() {
	
	return 1;	
}

int pwd(char *currentDirectory) {
	fprintf(stdout, "%s\n",currentDirectory);
 return 1;
}