#include <stdio.h>

int main(int argc, char* argv[])
{
    printf("Number of arguments: %i \n", argc);

    for (int i=0; i<argc; i++)
    {
        printf("Arguments: %s \n", argv[i]);
    }

    printf("Hello World! \n");
}
