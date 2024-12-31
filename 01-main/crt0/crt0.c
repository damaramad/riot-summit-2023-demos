/*******************************************************************************/
/*  © Université de Lille, The Pip Development Team (2015-2024)                */
/*                                                                             */
/*  This software is a computer program whose purpose is to run a minimal,     */
/*  hypervisor relying on proven properties such as memory isolation.          */
/*                                                                             */
/*  This software is governed by the CeCILL license under French law and       */
/*  abiding by the rules of distribution of free software.  You can  use,      */
/*  modify and/ or redistribute the software under the terms of the CeCILL     */
/*  license as circulated by CEA, CNRS and INRIA at the following URL          */
/*  "http://www.cecill.info".                                                  */
/*                                                                             */
/*  As a counterpart to the access to the source code and  rights to copy,     */
/*  modify and redistribute granted by the license, users are provided only    */
/*  with a limited warranty  and the software's author,  the holder of the     */
/*  economic rights,  and the successive licensors  have only  limited         */
/*  liability.                                                                 */
/*                                                                             */
/*  In this respect, the user's attention is drawn to the risks associated     */
/*  with loading,  using,  modifying and/or developing or reproducing the      */
/*  software by the user in light of its specific status of free software,     */
/*  that may mean  that it is complicated to manipulate,  and  that  also      */
/*  therefore means  that it is reserved for developers  and  experienced      */
/*  professionals having in-depth computer knowledge. Users are therefore      */
/*  encouraged to load and test the software's suitability as regards their    */
/*  requirements in conditions enabling the security of their systems and/or   */
/*  data to be ensured and,  more generally, to use and operate it in the      */
/*  same conditions as regards security.                                       */
/*                                                                             */
/*  The fact that you are presently reading this means that you have had       */
/*  knowledge of the CeCILL license and that you accept its terms.             */
/*******************************************************************************/

#include <stdint.h>
#include <stddef.h>

#include "crt0.h"
#include "interface.h"

/*
 * WARNING: No global variable must be declared in this file!
 */

/*!
 * \def die
 *
 * \brief This macro displays the string passed in parameter
 *        using the semihosting and stops the execution of the
 *        code.
 *
 * \param msg The string to be displayed. It will be placed
 *        right after the assembly code, so that it can be
 *        referred to wherever the code is loaded and without
 *        the need to relocate it.
 */
#define die(msg)\
	__asm__\
	(\
		"mov r0, #4\n"\
		"adr r1, 1f\n"\
		"bkpt 0xab\n"\
		"b .\n"\
		"1: .asciz " #msg "\n"\
		".align 1\n"\
	)

static __attribute__((always_inline)) inline void*
memcpy(void *dest, const void *src, size_t n)
{
	/* According to Cortex-M4 Technical Reference Manual, 3
	 * Programmers Model, 3.3 Instruction set summary, 3.3.1
	 * Cortex-M4 instructions. */
	while (n >= 44) {
		__asm__ volatile
		(
			"ldmia %0!, {r2-r12}\n"
			"stmia %1!, {r2-r12}\n"
			: "+r" (src), "+r" (dest)
			:
			: "r2", "r3", "r4", "r5",
			  "r6", "r7", "r8", "r9",
			  "r10", "r11", "r12",
			  "memory"
		);
		n -= 44;
	}

	/* There is up to 43 bytes to copy. */
	while (n >= 8) {
		__asm__ volatile
		(
			"ldrd r2, r3, [%0], #8\n"
			"strd r2, r3, [%1], #8\n"
			: "+r" (src), "+r" (dest)
			:
			: "r2", "r3", "memory"
		);
		n -= 8;
	}

	/* There is up to 3 bytes to copy. */
	while (n > 0) {
		__asm__ volatile
		(
			"ldrb r2, [%0], #1\n"
			"strb r2, [%1], #1\n"
			: "+r" (src), "+r" (dest)
			:
			: "r2", "memory"
		);
		n--;
	}

	return dest;
}

#if 0
__attribute__((naked)) static void*
memcpy(void *dest, const void *src, size_t n)
{
	(void)dest;
	(void)src;
	(void)n;
	__asm__ volatile
	(
		"push   {r4, lr}\n"
		"cmp    r2, #43\n"
		"bls    2f\n"
		"push   {r5-r11}\n"
		"1:\n"
		"ldmia  r1!, {r3-r12, lr}\n"
		"stmia  r0!, {r3-r12, lr}\n"
		"subs   r2, #44\n"
		"cmp    r2, #43\n"
		"bhi    1b\n"
		"pop    {r5-r11}\n"
		"2:\n"
		"movs   r4, #10\n"
		"lsrs   r3, #3\n"
		"muls   r3, r4\n"
		"ldr    r4, =3f\n"
		"rsb    r3, r4\n"
		"mov    pc, r3\n"

		"ldrd   r3, r4, [r1], #8\n"
		"strd   r3, r4, [r0], #8\n"
		"subs   r2, #8\n"

		"ldrd   r3, r4, [r1], #8\n"
		"strd   r3, r4, [r0], #8\n"
		"subs   r2, #8\n"

		"ldrd   r3, r4, [r1], #8\n"
		"strd   r3, r4, [r0], #8\n"
		"subs   r2, #8\n"

		"ldrd   r3, r4, [r1], #8\n"
		"strd   r3, r4, [r0], #8\n"
		"subs   r2, #8\n"

		"ldrd   r3, r4, [r1], #8\n"
		"strd   r3, r4, [r0], #8\n"
		"subs   r2, #8\n"
		"3:"
		"pop   {r4, pc}\n"
	);
}
#endif

extern void
_start(interface_t *interface, void **syscalls)
{
	void (*exit)(int) = NULL;
	void *oldGotAddr = NULL;
	int status;

	if (syscalls != NULL) {
		exit = syscalls[1];
	}

	/* Retrieve current memory layout. */
	uint32_t binaryAddr =
		(uint32_t) interface->root;
	uint32_t unusedRamAddr =
		(uint32_t) interface->unusedRamStart;
	uint32_t ramEndAddr =
		(uint32_t) interface->ramEnd;

	/* Retrieve metadata informations. */
	metadata_t *metadata = (metadata_t *)
		(binaryAddr + (uint32_t) &__metadataOff);
	uint32_t entryPointOffset =
		metadata->symbolTable.entryPoint;
	uint32_t romSecSize =
		metadata->symbolTable.romSecSize;
	uint32_t gotSecSize =
		metadata->symbolTable.gotSecSize;
	uint32_t romRamSecSize =
		metadata->symbolTable.romRamSecSize;
	uint32_t ramSecSize =
		metadata->symbolTable.ramSecSize;

	/* Calculation of the section start address in ROM. */
	uint32_t romSecAddr =
		(uint32_t) metadata + sizeof(metadata->symbolTable) +
		sizeof(metadata->patchinfoTable.entryNumber) +
		metadata->patchinfoTable.entryNumber *
		sizeof(patchinfoEntry_t);
	uint32_t gotSecAddr = romSecAddr + romSecSize;
	uint32_t romRamSecAddr = gotSecAddr + gotSecSize;
	uint32_t entryPointAddr = romSecAddr + entryPointOffset;

	/* Calculation of the relocated section start address in RAM. */
	uint32_t relGotSecAddr = unusedRamAddr;
	uint32_t relRomRamSecAddr = relGotSecAddr + gotSecSize;
	uint32_t relRamSecAddr = relRomRamSecAddr + romRamSecSize;

	/* Check if there is enough RAM to perform relocation. */
	if (relGotSecAddr + gotSecSize > ramEndAddr ||
	    relRomRamSecAddr + romRamSecSize > ramEndAddr ||
	    relRamSecAddr + ramSecSize > ramEndAddr)
	{
		die("CRT0: Not enough RAM to perform relocation...\n");
	}

	/* Update of the unused RAM value. */
	interface->unusedRamStart =
		(void *)(relRamSecAddr + ramSecSize);
	/* Update of the unused ROM value. */
	/* XXX bad unusedRomStart... */
	interface->unusedRomStart =
		(void *)((uintptr_t)interface->unusedRomStart + ((uint32_t)&__metadataOff) + sizeof(metadata_t) + metadata->symbolTable.romRamEnd);

	/* Relocation of the '.rom.ram' section. */
	(void)memcpy((void *) relRomRamSecAddr,
	             (void *) romRamSecAddr,
	             (size_t) romRamSecSize);

	/* Initialization of the '.ram' section. */
	for (size_t i = 0; (i << 2) < ramSecSize; i++)
	{
		((uint32_t *) relRamSecAddr)[i] = 0;
	}

	/* Relocation of the '.got' section from the ROM to the RAM,
	 * replacing on the fly each global variable offset, expressed
	 * relative to the beginning of the binary file, with the
	 * address of the memory area where they were relocated or
	 * loaded. */
	for (size_t i = 0; (i << 2) < gotSecSize; i++)
	{
		uint32_t off = ((uint32_t *) gotSecAddr)[i];
		uint32_t addr = 0;

		if (off < romSecSize)
		{
			addr = romSecAddr + off;
			goto validGotEntry;
		}
		off -= romSecSize;

		if (off < gotSecSize)
		{
			/*
			 * Note that offset should always be zero.
			 * This should be for the _rom_size symbol.
			 */
			addr = relGotSecAddr + off;
			goto validGotEntry;
		}
		off -= gotSecSize;

		if (off < romRamSecSize)
		{
			addr = relRomRamSecAddr + off;
			goto validGotEntry;
		}
		off -= romRamSecSize;

		if (off < ramSecSize)
		{
			addr = relRamSecAddr + off;
			goto validGotEntry;
		}

		die("CRT0: An offset is not located in any section...\n");

validGotEntry:
		((uint32_t *) relGotSecAddr)[i] = addr;
	}

	/* Patch each global pointer by assigning the relocated address
	 * of the value pointed to by the pointer to the relocated
	 * pointer address. */
	for (size_t i = 0; i < metadata->patchinfoTable.entryNumber; i++)
	{
		uint32_t ptrOff = metadata->patchinfoTable.entries[i].ptrOff;
		uint32_t off = *((uint32_t *)(romSecAddr + ptrOff));
		uint32_t ptrAddr = 0, addr = 0;

		if (ptrOff < romSecSize)
		{
			goto ptrOffInRom;
		}
		ptrOff -= romSecSize;

		if (ptrOff < gotSecSize)
		{
			goto offInGot;
		}
		ptrOff -= gotSecSize;

		if (ptrOff < romRamSecSize)
		{
			ptrAddr = relRomRamSecAddr + ptrOff;
			goto validPtrAddr;
		}
		ptrOff -= romRamSecSize;

		if (ptrOff < ramSecSize)
		{
			ptrAddr = relRamSecAddr + ptrOff;
			goto validPtrAddr;
		}

		goto offOutBounds;

validPtrAddr:
		if (off < romSecSize)
		{
			addr = romSecAddr + off;
			goto validAddr;
		}
		off -= romSecSize;

		if (off < gotSecSize)
		{
			goto offInGot;
		}
		off -= gotSecSize;

		if (off < romRamSecSize)
		{
			addr = relRomRamSecAddr + off;
			goto validAddr;
		}
		off -= romRamSecSize;

		if (off < ramSecSize)
		{
			addr = relRamSecAddr + off;
			goto validAddr;
		}

offOutBounds:
		die("CRT0: An offset is not located in any section...\n");
ptrOffInRom:
		die("CRT0: A pointer offset is located in the ROM section...\n");
offInGot:
		die("CRT0: An offet is located in the GOT section...\n");
validAddr:
		*((uint32_t *) ptrAddr) = addr;
	}

	/* Initialization of the PIC register with the address of the
	 * relocated GOT, so that the partition can access its global
	 * variables. */
	__asm__("mov %0, sl" : "=r" (oldGotAddr) : :);
	__asm__("mov sl, %0" : : "r" (relGotSecAddr) : "sl");

	/* Branch to the entry point of the partition. */
	status = ((entryPoint_t) entryPointAddr)(interface, (void *) relGotSecAddr, oldGotAddr, syscalls);

	__asm__("mov sl, %0" : : "r" (oldGotAddr) : "sl");

	/* System call to exit */
	if (exit != NULL) {
		if ((uint32_t)syscalls[0] == 0) {
			(*exit)(status);
		} else {
			/* TODO return status instead of 0. */
			__asm__ volatile
			(
				"mov r0, #0\n"
				"mov r1, #0\n"
				"push {r0, r1}\n"
				"mov r0, #0\n"
				"mov r1, #54\n"
				"mov r2, #0\n"
				"mov r3, #1\n"
				"mov r4, #1\n"
				"svc #12\n"
				/* UNREACHABLE */
			);
		}
	}
	for (;;);
}
