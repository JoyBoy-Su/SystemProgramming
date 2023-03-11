#include "command.h"

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
void redirect(const Redirect* redirect)
{
    
}

// 由用户输入的line，填充commands与signal
int getCommands(const char* line, Command* commands)
{

}

// 释放count个command管理的空间，主要空间在argv和redirects
void freeCommands(Command* cmd, int count)
{

}
