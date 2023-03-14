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
    char line[LINE_LENGTH] = {0};
    int pipe_ids[2] = {0};
    PipeState pstate;

    Command commands[COMMAND_CNT_PER_LINE];
    for (int i = 0; i < COMMAND_CNT_PER_LINE; i++) initCommand(commands + i);

    // loop
    while (1)
    {
        // TODO: get line
        memset(line, 0, LINE_LENGTH);
        mygetline(line);
        /* parse line to get commands */
        int cnt = getCommands(line, commands);
        if (cnt < 0)
        {
            // TODO: error handler
        }
        
        /* execute each command */
        for (int i = 0; i < cnt; i++)
        {
            Command* command = commands + i;
            // TODO: valid errno
            
            /* create pipe or not */
            if (command->pipe)
            {
                // TODO: delete this if
                if (i == cnt - 1)
                {
                    fprintf(stderr, "pipe write proc is the last command!\n");
                    return 0;
                }
                if (pipe(pipe_ids) < 0)
                {
                    // TODO: error handler
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
                // TODO: error handler
            }
            /* child proc */
            else if (pid == 0)
            {
                /* preprocess: redirect */
                Redirect* redirects = command->redirects;
                for (int j = 0; j < command->redirectc; j++) redirect(redirects + j);
                /* exec */
                execv(command->name, command->argv);
            }
            else 
            {
                /* wait for child proc */
                if (wait(NULL) < 0)
                {
                    // TODO: error handler or signal interruption
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
