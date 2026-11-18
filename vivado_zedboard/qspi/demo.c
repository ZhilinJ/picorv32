#include <stdint.h>
#include "console.h"

static volatile uint32_t test_word;

int main(void)
{
    test_word = 42;
    ((volatile uint8_t *)&test_word)[1] = 0x5a;
    if (test_word != 0x5a2a) {
        console_puts("PicoRV32 QSPI demo: BRAM FAIL\n");
        return 1;
    }

    *(volatile uint32_t *)0x2000 = 1;
    console_puts("PicoRV32 QSPI demo: BRAM PASS (0x00005A2A)\n");
    return 0;
}
