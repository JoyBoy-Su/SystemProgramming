#include "command.h"

#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* Pipe State 部分 */

// 初始化pipe状态机
void initPipe(PipeState* state)
{
    *state = P_INIT;
}

// 更新pipe状态机
void updatePipe(PipeState* state)
{
    switch (*state)
    {
    case P_INIT:
        *state = P_WRITECLOSED;
        break;
    case P_WRITECLOSED:
        *state = P_CLOSED;
        break;
    case P_CLOSED:
    case P_DESTORY:
        *state = P_DESTORY;
        break;
    default:
        break;
    }
}

/* Redirect 部分 */

// 根据参数对进程进行文件描述符重定向
int redirect(const Redirect* redirect)
{
    // 1、判断重定向的类型：文件 / 管道
    if (redirect->type == RT_PIPE)
    {
        // 若是管道重定向，pipe已由parent进程创建完毕，完成重定向和子进程的关闭即可
        // 2、区分管道的读进程和写进程
        int pipe_fd = (redirect->info.pipe.write) ? 
            redirect->info.pipe.pipe_fd[PIPE_WPORT] : redirect->info.pipe.pipe_fd[PIPE_RPORT];
        dup2(pipe_fd, redirect->info.pipe.fd);
        // 3、在child进程中关闭pipe
        close(redirect->info.pipe.pipe_fd[PIPE_RPORT]);
        close(redirect->info.pipe.pipe_fd[PIPE_WPORT]);
    } 
    else if (redirect->type == RT_FILE)
    {
        // 若是文件重定向，需要由child进程打开文件，并在完成重定向后关闭
        // 2、打开文件，得到描述符old_fd
        int mode = O_WRONLY;
        if (redirect->info.file.mode == RMT_APPEND) mode |= O_APPEND;
        int file_fd = open(redirect->info.file.filename, mode);
        if (file_fd == -1) return -1;
        // 3、调用dup2进行重定向
        dup2(file_fd, redirect->info.file.fd);
        // 4、关闭文件描述符old_fd
        close(file_fd);
    }
    return 0;
}

/* line -> command 部分 */

// 将instrct对应的elf文件名复制到elfname，返回0代表成功（指令没有出错）
int instruct2elfname(const char* instruct, char* elfname, int len)
{
    if (strcmp(instruct, "ls") == 0 || strcmp(instruct, "pwd") == 0 ||
        strcmp(instruct, "cat") == 0 || strcmp(instruct, "echo") == 0 ||
        strcmp(instruct, "mkdir") == 0)
    {
        memcpy(elfname, instruct, len);
        return 0;
    }
    // else if (strcmp(instruct, "ll") == 0)
    // {
    //     memcpy(elfname, "ls", 2);
    //     return 0;
    // }
    else
    {
        memcpy(elfname, instruct, len);
        return ERR_CMD_NOT_FOUND;
    }
}

// 为oldv所指向的空间扩充increment * size个字节，并返回扩充后的首地址
char* increaseSize(char* oldv, int cnt, size_t size, int increment)
{
    char* newv = (char*) oldv;
    if (cnt % increment == 0)
    {
        newv = (char*) malloc(size * (cnt + increment));
        for (int i = 0; i < cnt * size; i++)
            newv[i] = oldv[i];
        free(oldv);
    }
    return newv;
}

// 为redirects增加一个重定向对象（insert一个指针）
Redirect** addRedirect(Redirect** redirects, int redirectc, Redirect* rp)
{
    redirects = (Redirect**) increaseSize(
        (char*) redirects, redirectc, sizeof(Redirect*), CMD_REDIRECT_INCREMENT);
    memcpy(redirects[redirectc], rp, sizeof(Redirect));
    return redirects;
}

void initCommand(Command* command)
{
    command->argc = 0;
    command->argv = NULL;
    command->errno = 0;
    command->pipe = 0;
    command->redirectc = 0;
    command->redirects = NULL;
    memset(command->name, 0, COMMAND_NAME_LENGTH);
}

// 由用户输入的line，填充commands与signal
int getCommands(const char* line, Command* command)
{
    char iterator;
    int left = 0, right = 0;
    char unnamed = 1;
    int cmd_cnt = 0;

    /* read strings */
    char fragment[FRG_SIZE] = {0};
    while (line[right] != '\0')
    {
        /* generate a fragment */
        for (right = left; (iterator = line[right]) != '\0'; right++)
            if (iterator == ' ') break;

        int len = right - left;
        if (len == 0) continue;

        /* '>' or '<' redirect */
        if ((len == 1 && line[left] == '<') || (len == 1 && line[left] == '>') ||
            (len == 2 && line[left] == '<' && line[left + 1] == '<') || 
            (len == 2 && line[left] == '>' && line[left + 1] == '>'))
        {
            Redirect* rp = (Redirect*) malloc(sizeof(Redirect));
            rp->type = RT_FILE;
            /* file.mode */
            rp->info.file.mode = (len == 1) ? RMT_OVERWRITE : RMT_APPEND;
            /* file.fd, TODO: >= 2 digits */
            if (line[left] == '>')
            {
                if (strlen(fragment) == 1 && fragment[0] >= '0' && fragment[0] <= '9')
                {
                    rp->info.file.fd = (int) (fragment[0] - '0');
                    /* clear error arg */
                    command->argc--;
                    free(command->argv[command->argc]);
                }
                else rp->info.file.fd = STDOUT_FILENO;
            }
            else rp->info.file.fd = STDIN_FILENO;
            /* file.filename */
            left = right + 1;
            for (right = left; (iterator = line[right]) != '\0'; right++)
                if (iterator == ' ') break;
            if (right - left == 0) command->errno = ERR_REDIRECT_LACK_PARAM;
            else
            {
                memcpy(rp->info.file.filename, line + left, right - left);
                command->redirectc++;
                memcpy(fragment, line + left, right - left);
                left = right + 1;
            }
            command->redirects = addRedirect(command->redirects, command->redirectc, rp);
            command->redirectc++;
            if (rp) free(rp);
            continue;
        }
        /* '|' pipe */
        else if (len == 1 && line[left] == '|')
        {
            /* add a new redirect */
            Redirect* rp = (Redirect*) malloc(sizeof(Redirect));
            rp->type = RT_PIPE;
            rp->info.pipe.fd = STDOUT_FILENO;
            rp->info.pipe.write = 1;
            command->redirects = (command->redirects, command->redirectc, rp);
            command->redirectc++;
            free(rp);
            /* read next instruct */
            left = right + 1;
            for (right = left; (iterator = line[right]) != '\0'; right++)
                if (iterator == ' ') break;
            if (right - left == 0) command->errno = ERR_PIPE_LACK_PARAM;
            else
            {
                memcpy(fragment, line + left, right - left);
                /* move to next command */
                command++;
                cmd_cnt++;
                int result = instruct2elfname(fragment, command->name, right - left);
                if (result == 0) 
                {
                    rp = (Redirect*) malloc(sizeof(Redirect));
                    rp->type = RT_PIPE;
                    rp->info.pipe.fd = STDIN_FILENO;
                    rp->info.pipe.write = 0;
                    command->redirects = addRedirect(command->redirects, command->redirectc, rp);
                    command->redirectc++;
                }
                else command->errno = ERR_CMD_NOT_FOUND;
                left = right + 1;
            }
            continue;
        }

        /* normal fragment (name or argv) */
        memset(fragment, 0, FRG_SIZE);
        memcpy(fragment, line + left, len);
        // printf("fragment = %s, len = %d\n", fragment, len);

        if (unnamed)
        {
            /* set command name */
            command->errno = instruct2elfname(fragment, command->name, len);
            unnamed = 0;
        }
        else 
        {
            /* alloc and fill a arg */
            char* arg = malloc(len + 1);
            memcpy(arg, fragment, len);
            arg[len] = 0;
            /* alloc a bigger argv */
            command->argv = (char**) increaseSize(
                (char*)command->argv, command->argc, sizeof(char*), CMD_ARGV_INCREMENT);
            /* insert into argv */
            command->argv[command->argc++] = arg;
        }
        /* update cyclic variable */
        left = right + 1;
    }
    return cmd_cnt;
}

// 释放count个command管理的空间，主要空间在argv和redirects
void freeCommands(Command* commands, int count)
{
    for (int i = 0; i < count; i++)
    {
        Command* cp = commands + i;
        /* argv */
        for (int j = 0; j < cp->argc; j++) free(cp->argv[j]);      /* free arg */
        free(cp->argv);         /* free argv */
        cp->argv = NULL;
        cp->argc = 0;
        /* redirect */
        for (int j = 0; j < cp->redirectc; j++) free(cp->redirects[j]);
        free(cp->redirects);
        cp->redirects = NULL;
        cp->argc = 0;
        /* other params */
        cp->errno = 0;
        cp->pipe = 0;
        memset(cp->name, 0, COMMAND_NAME_LENGTH);
    }
}

void mygetline(char* line)
{
    int character, last = 0, index = 0;
    while ((character = getc(stdin)) != '\n')
    {
        if (character == ' ' && last == ' ') continue;  /* clear blank */
        if (character == '>' || character == '<')
        {
            if (last != character) 
            {
                if (last != ' ') line[index++] = ' ';   /* insert a blank */
                line[index++] = (char) character;
                last = character;
            }
            else
            {
                line[index++] = (char) character;
                line[index++] = ' ';                    /* insert a blank */
                last = ' ';
            }
        }
        else if (character == '|')
        {
            if (last != ' ') line[index++] = ' ';       /* insert a blank */
            line[index++] = '|';
            last = '|';
        }
        else
        {
            if ((last == '>' || last == '<' || last == '|') &&
                character != ' ') line[index++] = ' ';
            line[index++] = (char) character;
            last = character;
        }
    }
    if (last == '|' || last == '>' || last == '<') line[index++] = ' ';
}
