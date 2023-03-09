#include <stdio.h>
#include <stdlib.h>

#define ARGV_CNT 1
#define ARGV_n 0

static char params[ARGV_CNT] = {0};

// 将argv的字符串输出到标准输出上
int main(int argc, char const *argv[])
{
    // params部分
    char index = 0;
    for (int i = 1; i < argc; i++)
    {
        // TODO: null pointer
        if (argv[i][0] != '-')
        {
            index = i;
            break;
        }
        else 
        {
            char invalid = 1;
            switch (argv[i][1])
            {
            case 'n':
                params[ARGV_n] = 1;
                invalid = 0;
                break;
            default:
                break;
            }
            if (invalid) 
            {
                index = i;
                break;
            }
        }
    }
    // 处理data部分
    for (int i = index; i > 0 && i < argc; i++)
    {
        printf("%s", argv[i]);
        if (i != argc - 1) printf(" ");
    }
    // 换行部分
    if (params[ARGV_n] == 0) printf("\n");

    exit(0);
}
