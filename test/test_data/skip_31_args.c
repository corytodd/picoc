#include <stdio.h>

// TODO how did this test get the args passed to it from the makefile?
int main(int argc, char **argv)
{
    int Count;

    printf("hello world %d\n", argc);
    for (Count = 0; Count < argc; Count++)
        printf("arg %d: %s\n", Count, argv[Count]);

    return 0;
}
