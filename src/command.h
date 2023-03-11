#ifndef __COMMAND_H
#define __COMMAND_H

#define COMMAND_NAME_LENGTH 16

enum redirect_type {File, Pipe};
enum redirect_mode_t {Overwrite, Append};
enum pipe_state {P_INIT, P_WRITECLOSED, P_CLOSED, P_DESTORY};

typedef struct redirect
{
    union {
        struct {
            int fd;
            const char* filename;
            enum redirect_mode_t mode;
        } file;
        struct {
          	int fd;
            int pipe_fd;					// parent proc alloc pipe fd;
        } pipe;
    } info;
    enum redirect_type type;
} Redirect;

typedef struct command
{
    // child proc
    char name[COMMAND_NAME_LENGTH];		// 指令对应的可执行文件名，处理ls -l和ll这种对应同一elf文件的情况
    char** argv;						// line中参数二级指针
   	int argc;							// line中参数的个数
    // shell proc
    Redirect* redirects;				// 该指令需要进行的一系列重定向操作
    int redirectc;						// 重定向操作的个数，便于回收内存
    unsigned char pipe;					// 是否需要创建pipe，最后一个redirect对象为pipe的重定向
} Command;

// 由用户输入的line，填充commands与signal
int getCommands(const char* line, Command* commands);	
// 释放count个command管理的空间，主要空间在argv和redirects
void freeCommands(Command* cmd, int count);

// 根据参数对进程进行文件描述符重定向
void redirect(const Redirect* redirect);

// 初始化pipe状态机
void initPipe(enum pipe_state* state);
// 更新pipe状态机
void updatePipe(enum pipe_state* state);

#endif