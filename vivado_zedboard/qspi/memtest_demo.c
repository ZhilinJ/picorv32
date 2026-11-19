/*
 * Adapted from cmd_memtest() and xorshift32() in picosoc/firmware.c.
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

#include <stdint.h>
#include "console.h"

#define TEST_WORDS 1024u
#define WORD_STRIDE 256u
#define TEST_ROUNDS 5u
#define BYTE_COUNT 128u

/* Dedicated 4 KiB region allocated by the linker in BRAM. */
static volatile uint32_t test_memory[TEST_WORDS];

static uint32_t xorshift32(uint32_t *state)
{
    uint32_t value = *state;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    *state = value;
    return value;
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

static void report_failure(const char *kind, uint32_t offset,
                           uint32_t expected, uint32_t actual)
{
    console_puts("\nMEMTEST FAIL: ");
    console_puts(kind);
    console_puts(" at byte offset 0x");
    print_hex(offset);
    console_puts(" expected 0x");
    print_hex(expected);
    console_puts(" actual 0x");
    print_hex(actual);
    console_puts("\n");
}

static int cmd_memtest(void)
{
    volatile uint32_t *base_word = test_memory;
    volatile uint8_t *base_byte = (volatile uint8_t *)test_memory;

    console_puts("Running memtest: 4 KiB array, 5 strided word rounds\n");
    console_puts("Word checks ");

    for (uint32_t round = 1; round <= TEST_ROUNDS; ++round) {
        uint32_t state = round;
        for (uint32_t word = 0; word < TEST_WORDS; word += WORD_STRIDE)
            base_word[word] = xorshift32(&state);

        state = round;
        for (uint32_t word = 0; word < TEST_WORDS; word += WORD_STRIDE) {
            uint32_t expected = xorshift32(&state);
            uint32_t actual = base_word[word];
            if (actual != expected) {
                report_failure("WORD", word * sizeof(uint32_t), expected, actual);
                return 1;
            }
        }
        console_puts(".");
    }

    console_puts(" passed\nByte checks: first 128 bytes ");
    for (uint32_t byte = 0; byte < BYTE_COUNT; ++byte)
        base_byte[byte] = (uint8_t)byte;

    for (uint32_t byte = 0; byte < BYTE_COUNT; ++byte) {
        uint8_t actual = base_byte[byte];
        if (actual != (uint8_t)byte) {
            report_failure("BYTE", byte, (uint8_t)byte, actual);
            return 1;
        }
    }

    console_puts("passed\nMEMTEST PASS\n");
    return 0;
}

int main(void)
{
    return cmd_memtest();
}
