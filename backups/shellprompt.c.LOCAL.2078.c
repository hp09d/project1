#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

void execute(char* filename, char* params[], int size, char* background);

int main()
{
	const char *white = " \n\r\f\t\v";
	char buffer[81];

	//char *path; unused?
	char *token;

	char *directory;
	char *username;
	char hostname[81];
	char *buf;
	char *permissions;
	long size;
	
	int errorcheck;
	char *command[10] = {NULL};
	int operand;
	int openfile;
	int ioacctflag = 0;
	int stout;
		pid_t finished;
	
	stout = dup(1);

	while(1) { 
	// infinite loop to return to shell after running executable
	//Collect user information and store into variables for prompt

	size = pathconf(".", _PC_PATH_MAX);
	username = getlogin();
	gethostname(hostname, sizeof(hostname));

	if ((buf = (char *)malloc((size_t)size)) != NULL)
		directory = getcwd(buf, (size_t)size);

	printf("%s@%s:%s $ ",username,hostname,directory);
	fgets(buffer, 81, stdin);
	printf("buffer: %s",buffer);
	if(buffer[0] != '\n') { // don't do anything if nothing is entered into prompt!
				// fixes a segfault issue	

		char** argarray; // argv
		int args = 0; // argc
		char* bgprocess = NULL;
		argarray = malloc(2*sizeof(*argarray));
		argarray[args++] = NULL;
	
		operand = 0;
		token = strtok(buffer, white);	
		while(token!=NULL) {			
			 if((strcmp(token,">") == 0) | (strcmp(token,"<")==0))
                                {
					if(strcmp(token, ">") == 0) //output
					{ 
						token = strtok(NULL, white);
						command[operand] = token;
						openfile = open(command[operand],O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
						dup2(openfile,1);
						close(openfile);
					}			
					else
					{ printf("READ ONLY");
						token = strtok(NULL, white);
						command[operand] = token;
						openfile = open(command[operand],O_RDONLY,permissions);}

				}
			else {
			command[operand] = token;
			printf("Command %i: %s\n",operand, command[operand]);
			argarray[args++] = token;
			argarray = realloc(argarray,(args+2)*sizeof(*argarray));
		}
		operand++;	
		token = strtok(NULL,white);
		}
	
		argarray[args] = NULL;
	
		if(operand > 0) { // segfault safeguard
			bgprocess = strstr(command[operand-1],"&"); // check for ampersand
			if(bgprocess != NULL) {
				char* strip = command[operand-1]; // strip ampersand if found
				strip[strlen(strip)-1] = '\0';
				command[operand-1] = strip;
			}
		}

		//Checks for Change Directory
		if(strcmp(command[0],"cd") == 0)
		{
			//Attempts to change directory, gives an error message if failed	

			if(command[1] == NULL)
			{	printf("NULL");}
			else{
				errorcheck = chdir(command[1]);	

				if(errorcheck == -1)
				{ printf("Error: Directory Does Not Exist\n");}
				else
				{directory = getcwd(buf,(size_t)size);}
			}
		}
		//Checks for exit
		else if (strcmp(command[0],"exit") == 0)
		{
		if(command[1] == NULL)
		{ return 0; }
		else
		{return atoi(command[1]);}
		}
		//Checks for ioacct
		else if (strcmp(command[0],"ioacct") == 0)
		{
		ioacctflag = 1;
		operand=0;
		while(command[operand+1] != NULL)
		{
		argarray[operand] = command[operand+1];
		printf("Argarray: %s", argarray[operand]);
		operand++;
		}
		args = args - 1;
		goto execute;
print:
				
		printf("sucess: %i",finished);
		//Set a flag, when the process is done have the process either jump back into this loop or 
		//output information in that process?
		}
		//Otherwise run executable
		else
		{
execute:
			printf("Running Executable\n");
			execute(argarray[1], argarray, args, bgprocess);
		}
		free(argarray); //deallocate dynamic array
	} // end newline skip
	
dup2(stout,1);
close(stout);
} // end infinite loop

	return 0;
}

// filename, argv, argc
void execute(char* filename, char* params[], int size, char* background) {
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
				printf("### execv failed!\n");
				exit(1);
			}	
		}
		printf("Executable not found! Killing child process.\n");
		free(patharray);
		exit(1);
	}

	if(pid > 0) {
		//parent process
		pid_t finished; 
	
		if(background == NULL) {
			finished = waitpid(-1, (int *)NULL, 0);
			printf("### process %d completed\n",finished);
		}
	}
}
