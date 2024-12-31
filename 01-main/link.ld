/*
 * Copyright (C) 2024 Université de Lille
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

OUTPUT_FORMAT(
    "elf32-littlearm",
    "elf32-littlearm",
    "elf32-littlearm"
)
OUTPUT_ARCH(arm)
ENTRY(start)

SECTIONS
{
    /*
     * The .rom output section gathers all input sections that
     * must be retained in ROM
     */
    .rom :
    {
        . = ALIGN( 4 ) ;
        __rom_start = . ;

        *(.text*)
        . = ALIGN( 4 ) ;
        *(.rodata*)

        . = ALIGN( 4 ) ;
        __rom_end = . ;
    }
    __rom_size = __rom_end - __rom_start;

    /*
     * The .got output section gathers all .got input sections
     * that need to be relocated to RAM and patched by the CRT0.
     * To optimize ROM space, this section should be placed
     * immediately after the .rom section
     */
    .got :
    {
        . = ALIGN( 4 ) ;
        __got_start = . ;

        *(.got*)

        . = ALIGN( 4 ) ;
        __got_end = . ;
    }
    __got_size = __got_end - __got_start;

    /*
     * The .rom.ram output section gathers all input sections
     * with initialized global variables that the CRT0 must copy
     * from ROM to RAM
     */
    .rom.ram :
    {
        . = ALIGN( 4 ) ;
        __rom_ram_start = . ;

        *(.data*)

        . = ALIGN( 4 ) ;
        __rom_ram_end = . ;
    }
    __rom_ram_size = __rom_ram_end - __rom_ram_start;

    /*
     * The .ram output section gathers all input sections with
     * uninitialized global variables that must be initialized
     * to zero in RAM
     */
    .ram (NOLOAD) :
    {
        . = ALIGN( 4 ) ;
        __ram_start = . ;

        *(.bss* COMMON)

        . = ALIGN( 4 ) ;
        __ram_end = . ;
    }
    __ram_size = __ram_end - __ram_start;
}
