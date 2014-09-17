#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
//testing
int main()
{
	const char *white = " \n\r\f\t\v";

	char buffer[81];

	char *path;
	char *token;
	char home[50] = "/home/class/";
	char *directory;
	char *username;
	char hostname[81];
	char *buf;

	long size;
	
	int errorcheck;
	char *command[10] = {NULL};
	int operand;
while(1)
{
	//Collect user information and store into variables for prompt
	size = pathconf(".", _PC_PATH_MAX);
	username = getlogin();
	gethostname(hostname, sizeof(hostname));

	if ((buf = (char *)malloc((size_t)size)) != NULL)
		directory = getcwd(buf, (size_t)size);


	//prints prompt
	printf("%s@%s:%s $ ",username,hostname,directory);
	fgets(buffer, 81, stdin);
	printf("buffer: %s",buffer);

	//Separates the command into different key words
	operand = 0; 
	token = strtok(buffer, white);
	while(token!=NULL)
	{			
	command[operand] = token;
	printf("Command %i: %s\n",operand, command[operand]);
	operand++;
	token = strtok(NULL,white);
	}
 
	
	//Checks for Change Directory
	if(strcmp(command[0],"cd") == 0)
	{
		//Attempts to change directory, gives an error message if failed

		if((command[1] == NULL)|| (strcmp(command[1],"~")==0))
		{	
		strcat(home,username);
		chdir(home);}
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
	//Set a flag, when the process is done have the process either jump back into this loop or 
	//output information in that process?
	}
	//Otherwise run executable
	else
	{
		path = getenv("PATH");
		printf("Running Executable\n");
	}
}
	return 0;
}
