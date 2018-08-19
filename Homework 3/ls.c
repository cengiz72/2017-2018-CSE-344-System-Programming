#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
int ls_function( const char * pathName);
int main(int argc, const char  *argv[])
{
	if (argc != 1)
		return -1;

	if (ls_function(argv[0]) == -1)
		return -1;
	return 0;
} 

int ls_function(const char * pathName) {

	DIR *dirp = NULL;
	struct dirent *direntp = NULL;
	if ((dirp = opendir(pathName)) == NULL) {
		fprintf(stderr, "Faield to open %s: %s\n", pathName, strerror(errno));
		return -1;
	}
	while ((direntp = readdir(dirp)) != NULL) {

		if (strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..")) {
			fprintf(stdout, "File name         : %5s	", direntp->d_name);
			fprintf(stdout, "	File type         : %10u	", direntp->d_type);
			fprintf(stdout, "	File inode number : %10d 	\n", direntp->d_ino);
		}
	}

	closedir(dirp);

	return 0;
}
