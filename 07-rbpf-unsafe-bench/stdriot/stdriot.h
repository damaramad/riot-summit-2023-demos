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

#ifndef STDRIOT_H
#define STDRIOT_H

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \def VIDT_INTERRUPT_NUMBER
 *
 * \brief The number of interrupt for the nRF52832.
 */
#define VIDT_INTERRUPT_NUMBER 54

/*!
 * \brief blockAttr structure
 */
typedef struct blockAttr
{
    uint32_t* blockentryaddr    ;   //!< Block's local id
    uint32_t* blockstartaddr    ;   //!< Block's start address
    uint32_t* blockendaddr      ;   //!< Block's end address
    uint32_t read               ;   //!< Read permission bit
    uint32_t write              ;   //!< Write permission bit
    uint32_t exec               ;   //!< Exec permission bit
    uint32_t accessible         ;   //!< Block accessible bit
}__attribute__((packed)) blockAttr_t;

/*!
 * \brief When the block is empty, the error flag is set to -1,
 *        otherwise contains a block's public attributes.
 */
typedef union blockOrError_t
{
    int32_t error               ;   //!< Error -1 for an empty block
    blockAttr_t blockAttr       ;   //!< A block's publicly exposed attributes
}__attribute__((packed)) blockOrError;

/*!
 * \brief Structure representing the VIDT.
 */
typedef struct vidt_s
{
	uint32_t currentInterrupt;             /*<! The last interrupt number. */
	void *contexts[VIDT_INTERRUPT_NUMBER]; /*<! Pointers to contexts to restore. */
} vidt_t;

static inline uint32_t
Pip_createPartition(
	uint32_t *blockLocalId
) {
	register uint32_t r0 __asm__("r0");

	r0 = (uint32_t) blockLocalId;

	__asm__ volatile
	(
		"svc #0"
		: "+r" (r0)
		:
		: "memory"
	);

	return r0;
}

static inline uint32_t*
Pip_cutMemoryBlock(
	uint32_t *blockToCutLocalId,
	uint32_t *cutAddr,
	int32_t  mpuRegionNb
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");
	register uint32_t r2 __asm__("r2");

	r0 = (uint32_t) blockToCutLocalId;
	r1 = (uint32_t) cutAddr;
	r2 = (uint32_t) mpuRegionNb;

	__asm__ volatile
	(
		"svc #1"
		: "+r" (r0)
		: "r"  (r1),
		  "r"  (r2)
		: "memory"
	);

	return (uint32_t *) r0;
}

static inline uint32_t*
Pip_mergeMemoryBlocks(
	uint32_t *blockToMerge1LocalId,
	uint32_t *blockToMerge2LocalId,
	int32_t  mpuRegionNb
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");
	register uint32_t r2 __asm__("r2");

	r0 = (uint32_t) blockToMerge1LocalId;
	r1 = (uint32_t) blockToMerge2LocalId;
	r2 = (uint32_t) mpuRegionNb;

	__asm__ volatile
	(
		"svc #2"
		: "+r" (r0)
		: "r"  (r1),
		  "r"  (r2)
		: "memory"
	);

	return (uint32_t *) r0;
}

static inline uint32_t
Pip_prepare(
	uint32_t *partDescBlockId,
	int32_t  projectedSlotsNb,
	uint32_t *requisitionedBlockLocalId
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");
	register uint32_t r2 __asm__("r2");

	r0 = (uint32_t) partDescBlockId;
	r1 = (uint32_t) projectedSlotsNb;
	r2 = (uint32_t) requisitionedBlockLocalId;

	__asm__ volatile
	(
		"svc #3"
		: "+r" (r0)
		: "r"  (r1),
		  "r"  (r2)
		: "memory"
	);

	return r0;
}

static inline uint32_t*
Pip_addMemoryBlock(
	uint32_t *childPartDescBlockLocalId,
	uint32_t *blockToShareLocalId,
	uint32_t r,
	uint32_t w,
	uint32_t e
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");
	register uint32_t r2 __asm__("r2");

	r0 = (uint32_t) childPartDescBlockLocalId;
	r1 = (uint32_t) blockToShareLocalId;
	r2 = ((r & 1) << 2) | ((w & 1) << 1) | (e & 1);

	__asm__ volatile
	(
		"svc #4"
		: "+r" (r0)
		: "r"  (r1),
		  "r"  (r2)
		: "memory"
	);

	return (uint32_t *) r0;
}

static inline uint32_t
Pip_removeMemoryBlock(
        uint32_t *blockToRemoveLocalId
) {
	register uint32_t r0 __asm__("r0");

	r0 = (uint32_t) blockToRemoveLocalId;

	__asm__ volatile
	(
		"svc #5"
		: "+r" (r0)
		:
		: "memory"
	);

	return r0;
}

static inline uint32_t
Pip_deletePartition(
	uint32_t *childPartDescBlockLocalId
) {
	register uint32_t r0 __asm__("r0");

	r0 = (uint32_t) childPartDescBlockLocalId;

	__asm__ volatile
	(
		"svc #6"
		: "+r" (r0)
		:
		: "memory"
	);

	return r0;
}

static inline uint32_t*
Pip_collect(
	uint32_t *partDescBlockId
) {
	register uint32_t r0 __asm__("r0");

	r0 = (uint32_t) partDescBlockId;

	__asm__ volatile
	(
		"svc #7"
		: "+r" (r0)
		:
		: "memory"
	);

	return (uint32_t *) r0;
}

static inline uint32_t
Pip_mapMPU(
	uint32_t *partDescBlockId,
	uint32_t *blockToMapLocalId,
	int32_t  mpuRegionNb
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");
	register uint32_t r2 __asm__("r2");

	r0 = (uint32_t) partDescBlockId;
	r1 = (uint32_t) blockToMapLocalId;
	r2 = (uint32_t) mpuRegionNb;

	__asm__ volatile
	(
		"svc #8"
		: "+r" (r0)
		: "r"  (r1),
		  "r"  (r2)
		: "memory"
	);

	return r0;
}

static inline uint32_t*
Pip_readMPU(
	uint32_t *partDescBlockId,
	int32_t   mpuRegionNb
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");

	r0 = (uint32_t) partDescBlockId;
	r1 = (uint32_t) mpuRegionNb;

	__asm__ volatile
	(
		"svc #9"
		: "+r" (r0)
		: "r"  (r1)
		: "memory"
	);

	return (uint32_t *) r0;
}

static inline int32_t
Pip_findBlock(
	uint32_t     *partDescBlockId,
	uint32_t     *addrInBlock,
	blockOrError *blockAddr
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");
	register uint32_t r2 __asm__("r2");

	r0 = (uint32_t) partDescBlockId;
	r1 = (uint32_t) addrInBlock;
	r2 = (uint32_t) blockAddr;

	__asm__ volatile
	(
		"svc #10"
		: "+r" (r0)
		: "r" (r1),
		  "r" (r2)
		: "memory"
	);

	return (int32_t) r0;
}

static inline uint32_t
Pip_setVIDT(
	uint32_t *partDescBlockId,
	uint32_t *vidtBlockLocalId
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");

	r0 = (uint32_t) partDescBlockId;
	r1 = (uint32_t) vidtBlockLocalId;

	__asm__ volatile
	(
		"svc #11"
		: "+r" (r0)
		: "r"  (r1)
		: "memory"
	);

	return r0;
}

static inline uint32_t
Pip_yield(
	uint32_t *calleePartDescBlockId,
	uint32_t userTargetInterrupt,
	uint32_t userCallerContextSaveIndex,
	uint32_t flagsOnYield,
	uint32_t flagsOnWake
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");
	register uint32_t r2 __asm__("r2");
	register uint32_t r3 __asm__("r3");
	register uint32_t r4 __asm__("r4");

	r0 = (uint32_t) calleePartDescBlockId;
	r1 = userTargetInterrupt;
	r2 = userCallerContextSaveIndex;
	r3 = flagsOnYield;
	r4 = flagsOnWake;

	__asm__ volatile
	(
		"svc #12"
		: "+r" (r0)
		: "r"  (r1),
		  "r"  (r2),
		  "r"  (r3),
		  "r"  (r4)
		: "memory"
	);

	return r0;
}

static inline uint32_t
Pip_getIntState(
	uint32_t *childPartDescBlockLocalId
) {
	register uint32_t r0 __asm__("r0");

	r0 = (uint32_t) childPartDescBlockLocalId;

	__asm__ volatile
	(
		"svc #13"
		: "+r" (r0)
		:
		: "memory"
	);

	return r0;
}

static inline uint32_t
Pip_getSelfIntState(void)
{
	register uint32_t r0 __asm__("r0");

	__asm__ volatile
	(
		"svc #14"
		: "=r" (r0)
		:
		: "memory"
	);

	return r0;
}

static inline void
Pip_setIntState(
	uint32_t interruptState
) {
	register uint32_t r0 __asm__("r0");

	r0 = interruptState;

	__asm__ volatile
	(
		"svc #15"
		:
		: "r" (r0)
		: "memory"
	);
}

static inline uint32_t
Pip_in(
	uint32_t registerId
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");

	r0 = registerId;

	__asm__ volatile
	(
		"svc #16"
		: "=r" (r1)
		: "r" (r0)
		: "memory"
	);

	return r1;
}

static inline void
Pip_out(
	uint32_t registerId,
	uint32_t value
) {
	register uint32_t r0 __asm__("r0");
	register uint32_t r1 __asm__("r1");

	r0 = registerId;
	r1 = value;

	__asm__ volatile
	(
		"svc #17"
		:
		: "r" (r0),
		  "r" (r1)
		: "memory"
	);
}

/*!
 * \brief This structure defines the interface that
 *        PIP provides to partitions.
 */
typedef struct interface_s
{
	/*!
	 * \brief The ID of the block containing the
	 *        partition descriptor of the root
	 *        partition.
	 */
	void *partDescBlockId;

	/*!
	 * \brief The limit address of the stack of
	 *        the root partition.
	 */
	void *stackLimit;

	/*!
	 * \brief The stack top address of the root
	 *        partition.
	 */
	void *stackTop;

	/*!
	 * \brief The VIDT start address of the root
	 *        partition.
	 */
	void *vidtStart;

	/*!
	 * \brief The VIDT end address of the root
	 *        partition.
	 */
	void *vidtEnd;

	/*!
	 * \brief The start address of the root
	 *        partition binary.
	 */
	void *root;

	/*!
	 * \brief The start address of the unused
	 * ROM.
	 */
	void *unusedRomStart;

	/*!
	 * \brief The end address of the ROM.
	 */
	void *romEnd;

	/*!
	 * \brief The start address of the unused
	 * RAM.
	 */
	void *unusedRamStart;

	/*!
	 * \brief The end address of the RAM.
	 */
	void *ramEnd;

} interface_t;

extern void syscall_init(void *prev_got, void *curr_got, void **table);

extern void exit(int status);

extern int printf(const char * format, ...);

extern int get_temp(void);

extern int isprint(int character);

extern long strtol(const char *str, char **endptr, int base);

extern int get_led(int pos);

extern int set_led(int pos, int val);

extern ssize_t copy_file(const char *name, void *buf, size_t nbyte);

extern int get_file_size(const char *name, size_t *size);

#endif /* STDRIOT_H */
