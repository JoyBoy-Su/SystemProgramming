#include "command.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define LINE_LENGTH 512

int main(int argc, char const *argv[])
{
    // define some variables ...
    char line[LINE_LENGTH] = {0};               /* input string */
    int pipe_ids[2] = {0};                      /* pipe id */
    PipeState pstate;                           /* pipe state machine */
    Command commands[COMMAND_CNT_PER_LINE];     /* line -> command */
    for (int i = 0; i < COMMAND_CNT_PER_LINE; i++) initCommand(commands + i);

    // loop
    while (1)
    {
        write(STDOUT_FILENO, "$ ", 2);
        // get line
        memset(line, 0, LINE_LENGTH);
        mygetline(line);
        // printf("mygetline: %s\n", line);
        /* parse line to get commands */
        int cnt = getCommands(line, commands);
        if (cnt <= 0) continue;
        // printf("get cnt: %d\n", cnt);
        /* execute each command */
        for (int i = 0; i < cnt; i++)
        {
            Command* command = commands + i;
            /* command error */
            if (command->errno)
            {
                showError(command->errno);
                continue;
            }
            // printf("command: %s\targc=%d\n", command->argv[0], command->argc);
            // for (int i = 0; i < command->argc; i++)
            //     printf("argv[%d] = %s ", i, command->argv[i]);
            // printf("\n");
            /* create pipe or not */
            if (command->pipe)
            {
                if (i == cnt - 1)
                {
                    fprintf(stderr, "pipe write proc is the last command!\n");
                    continue;
                }
                if (pipe(pipe_ids) < 0)
                {
                    perror("create pipe failed");
                    continue;
                }
                /* write proc (last redirect object) */
                command->redirects[command->redirectc - 1]->info.pipe.pipe_fd[PIPE_RPORT] = pipe_ids[PIPE_RPORT];
                command->redirects[command->redirectc - 1]->info.pipe.pipe_fd[PIPE_WPORT] = pipe_ids[PIPE_WPORT];
                /* read proc (first redirect object) */
                Command* next = command + 1;
                next->redirects[0]->info.pipe.pipe_fd[PIPE_RPORT] = pipe_ids[PIPE_RPORT];
                next->redirects[0]->info.pipe.pipe_fd[PIPE_WPORT] = pipe_ids[PIPE_WPORT];
                /* init pipe state */
                initPipe(&pstate);
            }
            /* fork */
            pid_t pid = fork();
            if (pid < 0)
            {
                perror("create child proc failed");
                return 0;
            }
            /* child proc */
            else if (pid == 0)
            {
                /* preprocess: redirect */
                Redirect** redirects = command->redirects;
                for (int j = 0; j < command->redirectc; j++)
                    if (redirect(redirects[j]) == -1) perror("redirect error");
                /* exec */
                execv(command->argv[0], command->argv);
            }
            else 
            {
                /* wait for child proc */
                if (wait(NULL) < 0)
                {
                    // TODO: error handler or signal interruption
                    perror("parent wait error");
                    return 0;
                }
                /* update pipe state and close pipe in parent proc */
                updatePipe(&pstate);
                if (pstate == P_CLOSED)
                {
                    close(pipe_ids[PIPE_RPORT]);
                    close(pipe_ids[PIPE_WPORT]);
                }
            }
        }
        
        /* free commands */
        freeCommands(commands, cnt);
    }
    return 0;
}
