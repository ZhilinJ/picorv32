#include "console.h"
#include "../../firmware/hello.c"

void print_str(const char *text)
{
    console_puts(text);
}

int main(void)
{
    hello();
    return 0;
}
