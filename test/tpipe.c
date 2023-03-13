#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>

#define PIPE_RPORT 0
#define PIPE_WPORT 1

int main(int argc, char const *argv[])
{
    printf("start proc:\n");
    // create pipe
    int pipe_ids[2] = {0};
    if (pipe(pipe_ids) == -1)
    {
        perror("pipe create failed!\n");
        return 0;
    }
    char buf[64] = "hello proc, this is pipe content!";
    int len = strlen(buf);

    // create child1: write proc
    pid_t w_pid = fork();
    if (w_pid == 0)
    {
        printf("in write child proc, before dup\n");
        // write child proc
        dup2(pipe_ids[PIPE_WPORT], STDOUT_FILENO);
        close(pipe_ids[PIPE_RPORT]);
        close(pipe_ids[PIPE_WPORT]);
        int cnt = write(STDOUT_FILENO, buf, len);
        if (cnt != len)
        {
            char err_buf[32] = "write pipe failed!\n";
            write(STDERR_FILENO, err_buf, strlen(err_buf));
            exit(-1);
        }
        exit(0);
    } 
    else if (w_pid < 0)
    {
        perror("fork failed!\n");
        return 0;
    }
    else 
    {
        int pid = wait(NULL);
        printf("parent proc wait proc %d done!\n", pid);
        if (pid == w_pid) printf("it's write child proc!\n");
    }

    // create child2: read proc
    pid_t r_pid = fork();
    if (r_pid == 0)
    {
        printf("in read child proc, before dup\n");
        // write child proc
        dup2(pipe_ids[PIPE_RPORT], STDIN_FILENO);
        close(pipe_ids[PIPE_RPORT]);
        close(pipe_ids[PIPE_WPORT]);
        char read_buf[32];
        int cnt = read(STDIN_FILENO, read_buf, len);
        if (cnt != len)
        {
            char err_buf[32] = "read pipe failed!\n";
            write(STDERR_FILENO, err_buf, strlen(err_buf));
            exit(-1);
        }
        printf("read child proc read from pipe: %s\n", read_buf);
        exit(0);
    }
    else if (r_pid < 0)
    {
        perror("fork failed!\n");
        return 0;
    }
    else
    {
        int pid = wait(NULL);
        printf("parent proc wait proc %d done!\n", pid);
        if (pid == r_pid) printf("it's read child proc!\n");
    }

    // close pipe
    close(pipe_ids[PIPE_RPORT]);
    close(pipe_ids[PIPE_WPORT]);

    return 0;
}
