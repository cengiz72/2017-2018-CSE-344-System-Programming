#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
int cat(int fd);
int main(int argc, char const *argv[]) {
	if (argc == 0)
		cat(0);
	else cat(1);
	return 0;
}
int cat(int fd) {
	if (fd == 1) {
		char character;
		while(read(STDIN_FILENO,&character,sizeof(char))!= 0)
			fprintf(stdout, "%c", character);
		return 0;		
	}
	else {
		char arr[256];
		int size = 256;
		while(fgets(arr, size, stdin) != NULL) {		
			fprintf(stdout,"%s", arr);
			fflush(stdout);
		}
		return 0;
	}

}
