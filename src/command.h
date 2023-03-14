#ifndef __COMMAND_H
#define __COMMAND_H

#define COMMAND_CNT_PER_LINE    8
#define COMMAND_NAME_LENGTH     16
#define FILE_NAME_LENGTH        32
#define FRG_SIZE                32

#define PIPE_RPORT              0
#define PIPE_WPORT              1

#define ERR_CMD_NOT_FOUND       1
#define ERR_REDIRECT_LACK_PARAM 2
#define ERR_PIPE_LACK_PARAM     3

#define CMD_ARGV_INCREMENT      32
#define CMD_REDIRECT_INCREMENT  16

enum redirect_type { RT_FILE, RT_PIPE };
enum redirect_mode_t { RMT_OVERWRITE, RMT_APPEND };
enum pipe_state { P_INIT, P_WRITECLOSED, P_CLOSED, P_DESTORY };

typedef enum redirect_type RedirectType;
typedef enum redirect_mode_t RedirectModeType;
typedef enum pipe_state PipeState;

typedef struct redirect
{
    union {
        struct {
            int fd;
            char filename[FILE_NAME_LENGTH];
            RedirectModeType mode;
        } file;
        struct {
          	int fd;
            int pipe_fd[2];					// parent proc alloc pipe fd;
            char write;
        } pipe;
    } info;
    RedirectType type;
} Redirect;

typedef struct command
{
    // child proc
    char name[COMMAND_NAME_LENGTH];     // 指令对应的可执行文件名，处理ls -l和ll这种对应同一elf文件的情况
    char** argv;						// line中参数二级指针
   	int argc;							// line中参数的个数
    // shell proc
    Redirect** redirects;				// 该指令需要进行的一系列重定向操作
    int redirectc;						// 重定向操作的个数，便于回收内存
    unsigned char pipe;					// 是否需要创建pipe，最后一个redirect对象为pipe的重定向
    // errno
    unsigned int errno;                 // 是否出错，以及出错码
} Command;

void mygetline(char* line);
void initCommand(Command* command);
// 由用户输入的line，填充commands与signal
int getCommands(const char* line, Command* command);
// 释放count个command管理的空间，主要空间在argv和redirects
void freeCommands(Command* command, int count);

// 根据参数对进程进行文件描述符重定向
int redirect(const Redirect* redirect);

// 初始化pipe状态机
void initPipe(PipeState* state);
// 更新pipe状态机
void updatePipe(PipeState* state);

#endif