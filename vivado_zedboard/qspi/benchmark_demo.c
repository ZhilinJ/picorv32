/*
 * Adapted from picosoc/firmware.c in the PicoRV32 repository.
 * Copyright (C) 2017 Claire Xenia Wolf <claire@yosyshq.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>
#include "console.h"

/* Independent host reference for 20 iterations with seed 314159265. */
#define EXPECTED_CHECKSUM UINT32_C(0x5B8EB866)

static void print_dec(uint32_t value)
{
    char text[11];
    char *position = text + sizeof(text) - 1;
    *position = '\0';

    do {
        *--position = '0' + value % 10;
        value /= 10;
    } while (value != 0);

    console_puts(position);
}

static void print_hex(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    char text[9];
    text[8] = '\0';

    for (int index = 7; index >= 0; --index) {
        text[index] = digits[value & 0xf];
        value >>= 4;
    }

    console_puts(text);
}

static uint32_t cmd_benchmark(bool verbose, uint32_t *instns_p,
                              uint32_t *checksum_p)
{
    /* Byte access through data shares aligned storage with the word view. */
    uint32_t words[64];
    uint8_t *data = (uint8_t *)words;
    uint32_t x32 = 314159265;

    uint32_t cycles_begin, cycles_end;
    uint32_t instns_begin, instns_end;
    __asm__ volatile ("rdcycle %0" : "=r"(cycles_begin) : : "memory");
    __asm__ volatile ("rdinstret %0" : "=r"(instns_begin) : : "memory");

    for (int iteration = 0; iteration < 20; ++iteration) {
        for (int index = 0; index < 256; ++index) {
            x32 ^= x32 << 13;
            x32 ^= x32 >> 17;
            x32 ^= x32 << 5;
            data[index] = x32;
        }

        for (int index = 0, position = 0; index < 256; ++index) {
            if (data[index])
                data[position++] = index;
        }

        for (int index = 0; index < 64; ++index)
            x32 ^= words[index];
    }

    __asm__ volatile ("rdcycle %0" : "=r"(cycles_end) : : "memory");
    __asm__ volatile ("rdinstret %0" : "=r"(instns_end) : : "memory");

    if (verbose) {
        console_puts("Cycles: 0x");
        print_hex(cycles_end - cycles_begin);
        console_puts("\nInstns: 0x");
        print_hex(instns_end - instns_begin);
        console_puts("\nChksum: 0x");
        print_hex(x32);
        console_puts("\n");
    }

    if (instns_p)
        *instns_p = instns_end - instns_begin;
    *checksum_p = x32;

    return cycles_end - cycles_begin;
}

int main(void)
{
    uint32_t instructions, checksum;
    console_puts("PicoSoC benchmark: 20 iterations\n");
    uint32_t cycles = cmd_benchmark(true, &instructions, &checksum);

    console_puts("Cycles (decimal): ");
    print_dec(cycles);
    console_puts("\nInstructions (decimal): ");
    print_dec(instructions);
    console_puts("\n");

    if (instructions == 0) {
        console_puts("BENCHMARK FAIL: zero instruction count\n");
        return 1;
    }

    uint32_t fraction = ((uint64_t)cycles * 100 / instructions) % 100;
    console_puts("CPI: ");
    print_dec(cycles / instructions);
    console_puts(".");
    if (fraction < 10)
        console_puts("0");
    print_dec(fraction);
    console_puts("\n");

    if (checksum != EXPECTED_CHECKSUM) {
        console_puts("BENCHMARK FAIL: expected checksum 0x");
        print_hex(EXPECTED_CHECKSUM);
        console_puts("\n");
        return 1;
    }

    console_puts("BENCHMARK PASS\n");
    return 0;
}
