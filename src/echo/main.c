#include <stdio.h>
#include <stdlib.h>

#define ARGV_CNT 1
#define ARGV_n 0

static char args[ARGV_CNT];

int main(int argc, char const *argv[])
{
    // arguments部分
    char index = 0;
    for (int i = 1; i < argc; i++)
    {
        // TODO: null pointer
        char out = 0;
        if (argv[i][0] != '-') out = 1;
        else 
        {
            char invalid = 1;
            switch (argv[i][1])
            {
            case 'n':
                args[ARGV_n] = 1;
                invalid = 0;
                break;
            default:
                break;
            }
            if (invalid) out = 1;
        }
        if (out == 1)
        {
            index = i;
            break;
        }
    }
    // 处理data部分
    for (int i = index; i > 0 && i < argc; i++)
    {
        fprintf(stdout, "%s", argv[i]);
        if (i != argc - 1) fprintf(stdout, " ");
    }
    // 换行部分
    if (args[ARGV_n] == 0) fprintf(stdout, "\n");

    exit(0);
}
