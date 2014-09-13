//searches for a file and runs it, takes file name as input
//"./a.out FILENAME"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main ( int argc, char *argv[] ) {
	//initializations
	char slash = '/';
	char* path; 
	char* token;
	char* buffer;
	char** patharray;
	int pv = 0;
	//pid_t child;

	path = getenv("PATH");   
	printf( "\n%s\n", path );
	patharray = malloc((pv+1)*sizeof(*patharray));
   	token = strtok(path, ":");  

	//read all of the paths from environment variable PATH into an array
	while( token != NULL )  {
		patharray[pv++] = token;
		printf( " %s\n", token );
		token = strtok(NULL, ":");
		patharray = realloc(patharray,(pv+1)*sizeof(*patharray));
  	}

	//try each path to see if executable exists
	int i = 0;
	for(i = 0; i < pv; i++) {
		asprintf(&buffer, "%s%c%s", patharray[i], slash, argv[1]);

		if(access(buffer,X_OK) == 0) {
			char* const params[] = {buffer, argv[2]};
			printf("@@@ File found at %s, executing '%s'\n", buffer,argv[1]);
			execv(buffer,params);
		} else {
			printf("### File not found at %s\n", buffer);		
		}		
	}

	free(patharray);
	return 0;
}

