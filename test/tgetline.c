#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    /* code */
    char line[1024] = {0};
    memset(line, 0, 1024);

    int character, last = 0, index = 0;
    while ((character = getc(stdin)) != '\n')
    {
        if (character == ' ' && last == ' ') continue;
        line[index++] = (char) character;
        last = character;
    }
    
    printf("getline: %s\n", line);

    return 0;
}
