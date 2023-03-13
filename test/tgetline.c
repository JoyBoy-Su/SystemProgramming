#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    /* code */
    char line[1024] = {0};
    memset(line, 0, 1024);

    int character, index = 0;
    while ((character = getc(stdin)) != '\n')
        line[index++] = (char) character;
    
    printf("getline: %s\n", line);

    return 0;
}
