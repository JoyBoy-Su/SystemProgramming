#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SIZE 512
#define FSIZE 32

#define ERR_CMD_NOT_FOUND 1
#define ERR_REDIRECT_LACK_PARAM 2
#define ERR_PIPE_LACK_PARAM 3

#define CMD_ARGV_INCREMENT 32

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

int instruct2elfname(const char* instruct, char* elfname, int len)
{
    if (strcmp(instruct, "ls") == 0 || strcmp(instruct, "pwd") == 0 ||
        strcmp(instruct, "cat") == 0 || strcmp(instruct, "echo") == 0 ||
        strcmp(instruct, "mkdir") == 0)
    {
        memcpy(elfname, instruct, len);
        return 0;
    }
    else if (strcmp(instruct, "ll") == 0)
    {
        memcpy(elfname, "ls", 2);
        return 0;
    }
    else
    {
        memcpy(elfname, instruct, len);
        return ERR_CMD_NOT_FOUND;
    }
}

void parse(const char* line)
{
    char iterator;
    int left = 0, right = 0;

    /* read string */
    char fragment[FSIZE] = {0};
    char unnamed = 1;

    // child proc
    char elfname[1024] = {0};		    // 指令对应的可执行文件名，处理ls -l和ll这种对应同一elf文件的情况
    char** argv = NULL;					// line中参数二级指针
   	int argc = 0;						// line中参数的个数
    // shell proc
    unsigned char pipe;					// 是否需要创建pipe，最后一个redirect对象为pipe的重定向
    char redirect = 0;
    char redirectname[1024] = {0};
    int overwrite;
    int fd;
    // errno
    unsigned int errno;                 // 是否出错，以及出错码

    while (line[right] != '\0')
    {
        /* generate a fragment */
        for (right = left; (iterator = line[right]) != '\0'; right++)
            if (iterator == ' ') break;

        int len = right - left;
        if (len == 0) continue;

        /* '>' / '<', redirect fragment */
        if ((len == 1 && line[left] == '<') || (len == 1 && line[left] == '>') ||
            (len == 2 && line[left] == '<' && line[left + 1] == '<') || 
            (len == 2 && line[left] == '>' && line[left + 1] == '>'))
        {

            char rchar = line[left];
            overwrite = (len == 1);
            /* TODO: >= 2 digits */
            if (rchar == '>')
            {
                if (strlen(fragment) == 1 && fragment[0] >= '0' && fragment[0] <= '9')
                {
                    fd = (int) (fragment[0] - '0');
                    /* clear error arg */
                    argc--;
                    free(argv[argc]);
                }
                else fd = STDOUT_FILENO;
            }
            else fd = STDIN_FILENO;
            /* read file name */
            left = right + 1;
            for (right = left; (iterator = line[right]) != '\0'; right++)
                if (iterator == ' ') break;
            if (right - left == 0)
            {
                errno = ERR_REDIRECT_LACK_PARAM;
                break;
            }
            memcpy(redirectname, line + left, right - left);
            redirect = 1;
            memcpy(fragment, line + left, right - left);
            left = right + 1;
            continue;
        }
        /* '|', pipe fragment */
        else if (len == 1 && line[left] == '|')
        {
            /* read next instruct */
            left = right + 1;
            for (right = left; (iterator = line[right]) != '\0'; right++)
                if (iterator == ' ') break;
            if (right - left == 0)
            {
                errno = ERR_PIPE_LACK_PARAM;
                break;
            }
            memcpy(fragment, line + left, right - left);
            int result = instruct2elfname(fragment, elfname, right - left);
            if (result == 0) pipe = 1;
            else errno = result;
            left = right + 1;
            continue;
        }

        /* normal fragment */
        memset(fragment, 0, FSIZE);
        memcpy(fragment, line + left, len);
        // printf("fragment: %s, len = %d\n", fragment, len);

        /* fill variables */
        if (unnamed)
        {
            /* name */
            errno = instruct2elfname(fragment, elfname, len);
            unnamed = 0;
        }
        else
        {
            /* alloc arg */
            char* arg = malloc(len + 1);
            memcpy(arg, fragment, len);
            arg[len] = 0;
            /* alloc new argv */
            if (argc % CMD_ARGV_INCREMENT == 0)
            {
                char** new_argv = malloc(sizeof(char*) * (argc + CMD_ARGV_INCREMENT));
                for (int i = 0; i < argc; i++)
                    new_argv[i] = argv[i];
                if (argv) free(argv);
                argv = new_argv;
            }
            /* insert into argv */
            argv[argc++] = arg;
        }
        /* update cyclic variable */
        left = right + 1;
    }

    /* show parse result */
    if (errno != 0)
    {
        switch (errno)
        {
        case ERR_CMD_NOT_FOUND:
            fprintf(stderr, "command %s not found!\n", elfname);
            break;
        case ERR_REDIRECT_LACK_PARAM:
            fprintf(stderr, "redirection operation lack parameters!\n");
            break;
        case ERR_PIPE_LACK_PARAM:
            fprintf(stderr, "pipe operation lack parameters!\n");
            break;
        default:
            break;
        }
    }
    else
    {
        printf("elf-name: %s\n", elfname);
        printf("argc = %d\n", argc);
        for (int i = 0; i < argc; i++)
            printf("argv[%d]: %s\n", i, argv[i]);
        if (redirect)
            printf("redirect fd: %d, filename: %s, overwrite: %c\n", 
                fd, redirectname, overwrite ? 'y' : 'n');
        if (pipe)
            printf("this instruct should make pipe!\n");
    }

    /* free mem */
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);

}

int main(int argc, char const *argv[])
{
    char line[SIZE];
    memset(line, 0, SIZE);

    mygetline(line);
    printf("getline: %s\n", line);
    parse(line);

    return 0;
}
