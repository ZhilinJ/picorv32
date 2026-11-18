OUTPUT_ARCH(riscv)
ENTRY(_start)
MEMORY {
    RAM (rwx) : ORIGIN = 0x00010000, LENGTH = 16K
}
PHDRS {
    code PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}
SECTIONS {
    .text : {
        KEEP(*(.text.start))
        *(.text .text.* .rodata .rodata.* .srodata .srodata.*)
        . = ALIGN(4);
    } > RAM :code
    .data : {
        __data_start = .;
        *(.data .data.* .sdata .sdata.*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM :data
    __data_load = LOADADDR(.data);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.* .sbss .sbss.* COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM :data
    __stack_top = ORIGIN(RAM) + LENGTH(RAM) - (DEFINED(__reserved_top) ? __reserved_top : 0);
    ASSERT(__bss_end <= __stack_top - 512, "Application leaves less than 512 bytes for stack")
}
