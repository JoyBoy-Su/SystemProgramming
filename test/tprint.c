#include <stdio.h>

int main(int argc, char const *argv[])
{
    fprintf(stdout, "stdout: first");
    fprintf(stderr, "stderr: output");
    fprintf(stdout, "stdout: second");
    return 0;
}
