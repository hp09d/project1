#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

pid_t execute(char* filename, char* params[], int size, int flags);
void pipelining(char** command, int operand, int flags);
void waitfix();

int main()
{
	const char *white = " \n\r\f\t\v";
	char buffer[256];

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
	int stout;
	int stin;
	int inflag = 0;
	int flags = 0;

	//pid_t procid;
	//char procfile[50];
	//Saves stout/stin state for later use
	stout = dup(1);
	stin = dup(0);

	while(1) { 
		// infinite loop to return to shell after running executable
		//Collect user information and store into variables for prompt

		size = pathconf(".", _PC_PATH_MAX);
		username = getlogin();
		gethostname(hostname, sizeof(hostname));

		// flags - 0  regular execution
		//         1  background process
		//         2  ioacct
		//         4  one pipe
		//         8  two pipes
		flags = 0;

		if ((buf = (char *)malloc((size_t)size)) != NULL)
			directory = getcwd(buf, (size_t)size);


		//print prompt
		printf("%s@%s:%s $ ",username,hostname,directory);
		fgets(buffer, 256, stdin);
		
		
		//For testing  **DELETE
		//printf("buffer: %s",buffer);
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
				if((strcmp(token,">") == 0) | (strcmp(token,">>") == 0) | (strcmp(token,"<")==0))
				{
					if(strcmp(token, ">") == 0) //output
					{ 
						token = strtok(NULL, white);
						command[operand] = token;
						openfile = open(command[operand],O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
						dup2(openfile,1);
						close(openfile);
					}
					else if(strcmp(token, ">>") == 0) //output (append)
					{ 
						token = strtok(NULL, white);
						command[operand] = token;
						openfile = open(command[operand],O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
						dup2(openfile,1);
						close(openfile);
					}			
					else if(strcmp(token, "<") == 0) 
					{	inflag = 1;	//Input
						token = strtok(NULL, white);
						command[operand] = token;
						openfile = open(command[operand],O_RDONLY, 0);}
						dup2(openfile,0);
						close(openfile);
						token = strtok(NULL,white);
					}
				else if (strcmp(token,"|") == 0) {
					command[operand++] = token;
					token = strtok(NULL, white);					
					if(!(flags & 4) && !(flags & 8)) {
						flags += 4;					
					} else if ((flags & 4) && !(flags & 8)) {
						flags += 8;					
					} else {
						printf("Error: Too Many Pipes\n");
						goto breakout;	
					}		
				}
				else {
					command[operand] = token;
					operand++;
					//printf("ARGS: %i\n",args);

					if(!(strcmp(token,"ioacct") == 0)) { // don't add ioacct to argv
						argarray[args++] = token;
						argarray = realloc(argarray,(args+2)*sizeof(*argarray));
					}

					//printf("ArgArray %i: %s\n",args-1, argarray[args-1]);
					token = strtok(NULL,white);
				}

				argarray[args] = NULL;

				if(operand > 0) { // segfault safeguard
					bgprocess = strstr(command[operand-1],"&"); // check for ampersand
					if(bgprocess != NULL) {
						char* strip = command[operand-1]; // strip ampersand if found
						if(strip[strlen(strip)-1] != '&') {
							printf("Error: Misplaced &\n");						
						} else {
							strip[strlen(strip)-1] = '\0';
							command[operand-1] = strip;
							flags += 1;
						}
					}
				}

			//printf("\n### command[i] - %s\n", command[operand]);
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
				flags += 2; // set ioacct flag
				goto execute;
			}
			//Otherwise run executable
			else
			{
				if((flags & 4) || (flags & 8)) {
					waitfix();
					pipelining(command,operand,flags);
				} else {

execute:		
				//if(flags & 2) { printf("### ioacct\n"); }
				//printf("Running Executable: %i\n", args);
				waitfix();
				execute(argarray[1], argarray, args, flags);
			}
			}
			free(argarray); //deallocate dynamic array


		} // end newline skip
breakout:

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

// filename, argv, argc, flags
int execute(char* filename, char* params[], int size, int flags) {
	//initializations
	char slash = '/';
	char* path; 
	char* token;
	char buffer[256], iobuffer[256], iopath[256];
	char** patharray;
	int pv = 0;
	pid_t finished; 
	pid_t pid;
	FILE* file;
	int rchar=-1,wchar=-1,readbytes=-1,writebytes=-1,input=-1;

	pid = fork();	
	path = getenv("PATH"); 

	//printf("### flags = %i\n",flags);

	if(pid == 0) {  
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
		printf("Error: '%s' Does Not Exist\n",filename);
		free(patharray);
		exit(1);
	}

	if(pid > 0) {
		//parent process

		//check for ioacct flag
		if(flags & 2) {
			// construct path to file
			snprintf(iopath, 256, "/proc/%d/io", pid);
			
			// read file while child is executing
			while(waitpid(pid, (int*)NULL, WNOHANG) == 0){
				if((file = fopen(iopath,"r")) != NULL) {
					while(fscanf(file, "%s %d",iobuffer, &input) != EOF) {
						//printf("### input = %i\n",input);
						//printf("### iopath = %s\n",iopath);

						//read through io file
						if(strcmp(iobuffer, "rchar:") == 0) { 
							rchar = input;
						}

						if(strcmp(iobuffer, "wchar:") == 0) { 
							wchar = input;
						}

						if(strcmp(iobuffer, "read_bytes:") == 0) {
							readbytes = input;
						}
				
						if(strcmp(iobuffer, "write_bytes:") == 0) {
							writebytes = input;
						}						
					}
					fclose(file);
				}
			}

			printf("\nrchar: %d\n", rchar);	
			printf("wchar: %d\n", wchar);
			printf("read_bytes: %d\n", readbytes);
			printf("write_bytes: %d\n", writebytes);
			waitfix();	
		}
	
		if(flags & 1) { // check background flag
			finished = waitpid(pid, (int *)NULL, WNOHANG);
		} else {
			finished = waitpid(pid, (int *)NULL, 0);
			//printf("### process %d completed\n",finished);
		}
	}
return finished;
}

void waitfix() {
	while(!waitpid(-1,(int*)NULL,WNOHANG) > 0) {
		// just chill	
	}
}

void pipelining(char** command, int operand, int flags) {
	char** argv1;
	char** argv2;
	char** argv3;
	int fd[2], argc1, argc2, argc3;
	int iterator = 1, iterator2 = 0;
	pid_t pid;
	pipe(fd);

	//if(!(flags & 8)) { pt = 1; } else { pt = 2; }

	argv1 = malloc(operand*sizeof(*argv1));
	argv2 = malloc(operand*sizeof(*argv2));
	argv3 = malloc(operand*sizeof(*argv3));

	// initializations
	argv1[0] = "";
	argv2[0] = "";
	argv3[0] = "";
	argc1 = 1;
	argc2 = 1;
	argc3 = 1;
	int i;
	for(i = 0; i < operand; i++) {
		if(strcmp(command[i], "|") == 0) {
			iterator = i+1;
			iterator2 = iterator-1;
			break;		
		} else {
			argv1[i+1] = command[i];	
		}
		argc1++;
	}

	for(i = iterator; i < operand; i++) {
		if(strcmp(command[i], "|") == 0) {
			iterator = i;
			iterator2 = iterator-1;
			break;		
		} else {
			argv2[i-iterator2] = command[i];	
		}
		argc2++;
	}

	for(i = iterator; i < operand; i++) {
		if(strcmp(command[i], "|") == 0) {
			iterator = i;
			iterator2 = iterator-1;
			break;		
		} else {
			argv3[i-iterator2] = command[i];	
		}
		argc3++;
	}

	//one pipe

	pid = fork();
	if(pid == 0) {
		dup2(fd[1],1);
		execute(argv1[1], argv1, argc1, 0);
	} else {
		close(fd[1]);
	}
	
	pid = fork();
	if (pid == 0) {
		dup2(fd[0],0);
		execute(argv2[1], argv2, argc2, 0);
	}

	waitpid(pid,(int*)NULL,0);
}
