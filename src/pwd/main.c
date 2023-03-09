#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFF_SIZE 512

int main(int argc, char const *argv[])
{
    char buf[BUFF_SIZE] = {0};
    
    if (getcwd(buf, BUFF_SIZE - 1) == NULL)
    {
        fprintf(stderr, "get path error!\n");
        exit(-1);
    }
    fprintf(stdout, "%s\n", buf);
    exit(0);
}
