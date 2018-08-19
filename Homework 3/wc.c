#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
void wc(int fd);
int main(int argc, char const *argv[]) {
	if (argc == 0) {
		wc(0);
	} 
	else if (argc == 1) wc(1);
	return 0;
}
void wc(int fd) {
	if (fd == 1) {
		char character;
		int count = 0;
		while (read(STDIN_FILENO,&character,sizeof(char)) != 0) {
			if (character == '\n') ++count;
		}
		fprintf(stdout, " \nTotal line : %d\n",count);
	}

	else {
		char arr[256];
		int size = 256;
		int count = 0;
		while(fgets(arr, size, stdin) != NULL) {		
			fprintf(stdout,"%s", arr);
			++count;
			fflush(stdout);
		}
		fprintf(stdout, "\nTotal line : %d\n",count);
	}
}
