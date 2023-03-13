#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char buf[64] = "hello, here is append test";
    int fd = open("test.txt", O_APPEND | O_WRONLY);
    if (fd == -1) 
    {
        perror("file open failed!\n");
        return 0;
    }
    int cnt = write(fd, buf, strlen(buf));
    if (cnt != strlen(buf))
    {
        perror("write error!\n");
        return 0;
    }
    return 0;
}
