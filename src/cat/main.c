#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGV_CNT 4
#define ARGV_n 0
#define ARGV_b 1
#define ARGV_s 2
#define ARGV_e 3

#define FILE_NAME_SIZE
#define BUFF_SIZE 4096

static char args[ARGV_CNT];

char parseArgs(int argc, char const* argv[], char* filename);

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        fprintf(stderr, "too few arguments\n");
        exit(-1);
    }

    char filename[FILE_NAME_SIZE] = {0};
    char buf[BUFF_SIZE] = {0};
    if (parseArgs(argc, argv, filename) != 0)
    {
        FILE* fp = fopen(filename, "rb");
        if (fp == NULL)
        {
            fprintf(stderr, "file oper failed!\n");
            exit(-1);
        }
        int cnt;
        while ((cnt = fread(buf, 1, BUFF_SIZE, fp)) > 0)
        {
            fwrite(buf, 1, cnt, stdout);
            memset(buf, 0, BUFF_SIZE);
        }
        fclose(fp);
    }
    
    exit(0);
}

char parseArgs(int argc, char const* argv[], char* filename)
{
    char result = 0;
    for (int i = 1; i < argc; i++)
    {
        // arguments
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'n':
                args[ARGV_n] = 1;
                break;
            case 'b':
                args[ARGV_b] = 1;
                break;
            case 's':
                args[ARGV_s] = 1;
                break;
            case 'e':
                args[ARGV_e] = 1;
                break;
            default:
                break;
            }    
        }
        else
        {
            memcpy(filename, argv[i], strlen(argv[i]) + 1);
            result = 1;
        }
    }
    return result;
}