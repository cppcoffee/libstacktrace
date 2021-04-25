#include <stdio.h>
#include "stacktrace.h"


static void bar()
{
    char *p = 0;
    *p = 'a';
}


static void foo()
{
    bar();
}


int main()
{
    init_stacktrace();

    foo();

    return 0;
}

