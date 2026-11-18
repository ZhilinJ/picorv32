#include <stdint.h>
#include "console.h"

#define LOG_CAPACITY 1008u

static volatile uint32_t *const mailbox = (volatile uint32_t *)0x00013c00;
static volatile char *const log_buffer = (volatile char *)0x00013c10;

void console_puts(const char *message)
{
    uint32_t length = mailbox[1];
    while (*message && length < LOG_CAPACITY)
        log_buffer[length++] = *message++;
    mailbox[1] = length;
    if (*message)
        mailbox[3] = 1;
}

int __real_main(void);

int __wrap_main(void)
{
    int result = __real_main();
    mailbox[2] = (uint32_t)result;
    mailbox[0] = 0x5049434f;
    return result;
}
