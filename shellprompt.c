#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
	char buffer[81];

	char *path;
	char *token;

	char *directory;
	char *username;
	char hostname[81];
	char *buf;

	long size;

	//Collect user information and store into variables for prompt
	size = pathconf(".", _PC_PATH_MAX);
	username = getlogin();
	gethostname(hostname, sizeof(hostname));

	if ((buf = (char *)malloc((size_t)size)) != NULL)
		directory = getcwd(buf, (size_t)size);

	printf("%s@%s:%s $ ",username,hostname,directory);
	fgets(buffer, 81, stdin);
	printf("buffer: %s",buffer);

	token = strtok(buffer, " ");
	if(strcmp(token,"cd") == 0)
	{printf("cd!");}
	else if (strcmp(token,"exit") == 0)
	{return 55;}
	else if (strcmp(token,"ioacct") == 0)
	{printf("ioacct!\n");}
	else
	{
	path = getenv("PATH");
	printf("\n%s\n",path);
	}
	return 0;
}
