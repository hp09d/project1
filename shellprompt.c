#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
const char *dir = "HOME";
char buffer[81];

char *directory;
char *username;
char hostname[81];

username = getlogin();
gethostname(hostname, sizeof(hostname));
directory = getenv(dir);

printf("%s@%s#",username,hostname);
fgets(buffer, 81, stdin);

return 0;
}
