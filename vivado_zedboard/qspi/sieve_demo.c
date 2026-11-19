#include "console.h"
#include "../../firmware/sieve.c"
#include "../../firmware/stats.c"

void print_chr(char ch)
{
    char text[] = {ch, '\0'};
    console_puts(text);
}

void print_str(const char *text)
{
    console_puts(text);
}

void print_dec(unsigned int value)
{
    char buffer[11];
    char *text = buffer + sizeof(buffer) - 1;
    *text = '\0';

    do {
        *--text = '0' + value % 10;
        value /= 10;
    } while (value != 0);

    console_puts(text);
}

void print_hex(unsigned int value, int digits)
{
    static const char hex[] = "0123456789ABCDEF";
    char buffer[9];
    char *text = buffer + sizeof(buffer) - 1;
    *text = '\0';

    if (digits > 8)
        digits = 8;
    for (; digits > 0; --digits) {
        *--text = hex[value & 0xf];
        value >>= 4;
    }

    console_puts(text);
}

int main(void)
{
    sieve();
    stats();
    return 0;
}
