#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void execute(char* filename, char* params[], int size);

int main()
{
	char buffer[81];

	//char *path;
	char *token;

	char *directory;
	char *username;
	char hostname[81];
	char *buf;

	long size;


	while(1) { // infinite loop to return to shell after running executable
	//Collect user information and store into variables for prompt
	size = pathconf(".", _PC_PATH_MAX);
	username = getlogin();
	gethostname(hostname, sizeof(hostname));

	if ((buf = (char *)malloc((size_t)size)) != NULL)
		directory = getcwd(buf, (size_t)size);

	printf("%s@%s:%s $ ",username,hostname,directory);
	fgets(buffer, 81, stdin);
	//printf("buffer: %s",buffer);
	
	// main execution hand-off
	// it's kinda messy but what can ya do
	char** argarray; // argv
	char* saveptr;
	int args = 0; // argc
	
	argarray = malloc(2*sizeof(*argarray));
	argarray[args++] = NULL;

	for(token = strtok_r(buffer, " \n",  &saveptr);
	    token != NULL;
	    token = strtok_r(NULL, " \n",  &saveptr)) {

		if(strcmp(token,"exit") == 0) {
			token = strtok_r(NULL, " \n",  &saveptr);
			if(token == NULL) {
				//printf("### quitting with value 0\n"); 
				return 0;
			} else {
				//printf("### quitting with value %i\n",atoi(token));				
				return atoi(token);			
			}
			 
		} // 'exit' to break out of loop

		//printf(" token: %s\n",token);
		argarray[args++] = token;
		argarray = realloc(argarray,(args+2)*sizeof(*argarray));
	}
	argarray[args] = NULL;
	
	execute(argarray[1], argarray, args);

	free(argarray);

	/*
	if(strcmp(token,"cd") == 0)
	{printf("cd!");}
	else if (strcmp(token,"exit") == 0)
	{printf("exit!\n");}
	else if (strcmp(token,"ioacct") == 0)
	{printf("ioacct!\n");}
	else
	{printf("Success!\n");}
	*/
	}
	
	return 0;
}

// filename, argv, argc
void execute(char* filename, char* params[], int size) {
	//initializations
	char slash = '/';
	char* path; 
	char* token;
	char buffer[256];
	char** patharray;
	int pv = 0;
	
	pid_t pid;
	pid = fork();	

	if(pid == 0) {
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
				sprintf(buffer, "%s%c%s", patharray[i], slash, filename);
			} else {
				sprintf(buffer, "%s", filename);		
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

				//printf("@@@ File found at %s, executing '%s'\n", buffer,filename);
				execv(buffer,tp);
				printf("### execv failed!");
				exit(1);
			} else {
				//printf("### File not found at %s\n", buffer);		
			}		
		}

		free(patharray);
		exit(1);
	}

	if(pid > 0) {
		//parent process
		pid_t finished;
		finished = waitpid(-1, (int *)NULL, 0);

		printf("### process %d completed\n",finished);
	}
}
