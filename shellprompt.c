#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
char buffer[81];

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

return 0;
}
