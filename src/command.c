#include "command.h"

#include <unistd.h>
#include <fcntl.h>

// 初始化pipe状态机
void initPipe(enum pipe_state* state)
{
    *state = P_INIT;
}

// 更新pipe状态机
void updatePipe(enum pipe_state* state)
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

// 由用户输入的line，填充commands与signal
int getCommands(const char* line, Command* commands)
{
    
}

// 释放count个command管理的空间，主要空间在argv和redirects
void freeCommands(Command* cmd, int count)
{

}
