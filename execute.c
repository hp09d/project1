//searches for a file and runs it, takes file name as input
//"./a.out FILENAME"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Filename (argv[1]), Argv[], Argc
void execute(char* filename, char* params[], int size);

int main ( int argc, char* argv[] ) {
	pid_t process;
	process = fork();
	
	switch(process) {
		case 0: printf("@@@ child process\n");
			execute(argv[1],argv,argc);
			break;

		default: //parent process
			 break;
	}
	
	pid_t finished;
	finished = waitpid(-1, (int *)NULL, 0);
	printf("@@@ process %d completed\n", finished);
	return 0;
}

void execute(char* filename, char* params[], int size) {
	//initializations
	char slash = '/';
	char* path; 
	char* token;
	char* buffer;
	char** patharray;
	int pv = 0;
	pid_t child;

	path = getenv("PATH");   
	//printf( "\n%s\n", path );
	patharray = malloc((pv+1)*sizeof(*patharray));
	patharray[pv++] = "";
   	token = strtok(path, ":");  

	//read all of the paths from environment variable PATH into an array
	while( token != NULL )  {
		patharray[pv++] = token;
		//printf( " %s\n", token );
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
			char* tp[size+1];

			// preparing execv
			// tp acts as argv with 'size' plus one elements
			// array is { filename, parameter 1, parameter 2,...., NULL }
			tp[0] = filename;			
			int j = 1;
			for(j = 1; j<size; j++) {
				tp[j] = params[j+1];			
			}
			tp[size] = NULL;

			printf("@@@ File found at %s, executing '%s'\n", buffer,filename);
			execv(buffer,tp);
		} else {
			//printf("### File not found at %s\n", buffer);		
		}		
	}

	free(patharray);
}

