#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

pid_t execute(char* filename, char* params[], int size, char* background);
void waitfix();

int main()
{
	const char *white = " \n\r\f\t\v";
	char buffer[81];

	char *token;
	char *directory;
	char *username;
	char hostname[81];
	char *buf;
	long size;
	char *command[10] = {NULL};

	int errorcheck;
	int operand;
	int openfile;
	int ioacctflag = 0;
	int stout;
	int stin;
	int inflag = 0;

	pid_t procid;
	char procfile[50];
	//Saves stout/stin state for later use
	stout = dup(1);
	stin = dup(0);

	while(1) { 
		// infinite loop to return to shell after running executable
		//Collect user information and store into variables for prompt

		size = pathconf(".", _PC_PATH_MAX);
		username = getlogin();
		gethostname(hostname, sizeof(hostname));

		if ((buf = (char *)malloc((size_t)size)) != NULL)
			directory = getcwd(buf, (size_t)size);


		//print prompt
		printf("%s@%s:%s $ ",username,hostname,directory);
		fgets(buffer, 81, stdin);
		
		
		//For testing  **DELETE
		printf("buffer: %s",buffer);
		if(buffer[0] != '\n') { // don't do anything if nothing is entered into prompt!
			// fixes a segfault issue	

			char** argarray; // argv
			int args = 0; // argc
			operand = 0;
			char* bgprocess = NULL;
			argarray = malloc(2*sizeof(*argarray));
			argarray[args++] = NULL;

			token = strtok(buffer, white);	
			while(token!=NULL) {

				//Checks for input output redirectiong and responds accordingly			
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
					{	inflag = 1;	//Input
						token = strtok(NULL, white);
						command[operand] = token;
						openfile = open(command[operand],O_RDONLY, 0);}
						dup2(openfile,0);
						close(openfile);
						token = strtok(NULL,white);
				}
				else {
					command[operand] = token;
					operand++;
					printf("ARGS: %i\n",args);
					if(strcmp(token,"ioacct") == 0) { 
						// don't add ioacct to array
					} else {
						argarray[args++] = token;
						argarray = realloc(argarray,(args+2)*sizeof(*argarray));
					}

					printf("ArgArray %i: %s\n",operand, argarray[operand]);
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
			}//End while loop


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

				FILE *fp;
				char ch;
				char pid[10];
				// removed for now
				/*operand=0;
				  while(command[operand+1] != NULL)
				  {
				  argarray[operand+1] = command[operand+1];
				  printf("Argarray: %s", argarray[operand]);
				  operand++;
				  }*/
				ioacctflag = 1;
				goto execute;  // set the flag before here, the process won't reach past this

print:
				strcpy(procfile, "/proc/");
				snprintf(pid,10,"%d",procid);
				strcat(procfile,pid); 
				strcat(procfile,"/io");
				fp = fopen(procfile,"r");			
				if(fp == NULL){
				printf("There was an error opening the proc file: %s\n",procfile);
				continue;}
			while((ch = fgetc(fp)) != EOF)
			printf("%c",ch);
			fclose(fp);
				

			}
			//Otherwise run executable
			else
			{
execute:		
				printf("Running Executable: %i\n", args);
				waitfix();
				procid = execute(argarray[1], argarray, args, bgprocess);
				if(ioacctflag)
				{goto print;}
			}
			free(argarray); //deallocate dynamic array


		} // end newline skip

			if(inflag == 1)
			{dup2(stin,0);
			close(stin);
			inflag = 0;}
			else{//redirect output back to screen
			dup2(stout,1);
			close(stout);}

	}//end infinite loop
	return 0;
}

// filename, argv, argc
int execute(char* filename, char* params[], int size, char* background) {
	//initializations
	char slash = '/';
	char* path; 
	char* token;
	char buffer[256];
	char** patharray;
	int pv = 0;
		pid_t finished; 

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
	
		if(background == NULL) {
			finished = waitpid(-1, (int *)NULL, 0);
			printf("### process %d completed\n",finished);
		}
	}
return finished;
}

void waitfix() {
	while(!waitpid(-1,(int*)NULL,WNOHANG) > 0) {
		// just chill	
	}
}

