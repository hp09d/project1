//searches for a file and runs it, takes file name as input
//"./a.out FILENAME"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void execute(char* filename, char* params);

int main ( int argc, char* argv[] ) {
	execute(argv[1],argv[2]);
	return 0;
}

void execute(char* filename, char* params) {
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
	patharray[pv++] = "";
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

		//doesn't add extra forward slash to absolute path
		if(i > 0) {
			asprintf(&buffer, "%s%c%s", patharray[i], slash, filename);
		} else {
			asprintf(&buffer, "%s", filename);		
		}

		if(access(buffer,X_OK) == 0) {
			char* const tp[] = {buffer, params, NULL};
			printf("@@@ File found at %s, executing '%s'\n", buffer,filename);
			execv(buffer,tp);
		} else {
			printf("### File not found at %s\n", buffer);		
		}		
	}

	free(patharray);
}

