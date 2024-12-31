/*******************************************************************************/
/*  © Université de Lille, The Pip Development Team (2015-2022)                */
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

#ifndef __CRT0_H__
#define __CRT0_H__

#include "interface.h"

typedef int (*entryPoint_t)(interface_t *interface, void *relGotSecAddr, void *oldGotAddr, void **syscalls);

typedef struct symbolTable_s
{
	/*!
	 * \brief The entry point address within the
	 *        partition.
	 */
	uint32_t entryPoint;
	/*!
	 * \brief The '.rom' section size, in bytes,
	 *        of the partition.
	 */
	uint32_t romSecSize;
	/*!
	 * \brief The '.rom.ram' section size, in
	 *        bytes, of the partition.
	 */
	uint32_t romRamSecSize;
	/*!
	 * \brief The '.ram' section size, in bytes,
	 *        of the partition.
	 */
	uint32_t ramSecSize;
	/*!
	 * \brief The '.got' section size, in bytes,
	 *        of the partition.
	 */
	uint32_t gotSecSize;
	/*!
	 * \brief The '.romRam' section end address
	 *        of the partition.
	 */
	uint32_t romRamEnd;
} symbolTable_t;

typedef struct patchinfoEntry_s
{
	/*!
	 * \brief The pointer offest to patch.
	 */
	uint32_t ptrOff;
} patchinfoEntry_t;

typedef struct patchinfoTable_s
{
	/*!
	 * \brief The number of patchinfo entry.
	 */
	uint32_t entryNumber;
	/*!
	 * \brief The patchinfo entries.
	 */
	patchinfoEntry_t entries[];
} patchinfoTable_t;

typedef struct metadata_s
{
	/*!
	 * \brief The symbol table.
	 */
	symbolTable_t symbolTable;
	/*!
	 * \brief The patchinfo table.
	 */
	patchinfoTable_t patchinfoTable;
} metadata_t;

/*!
 * \brief the offset of the metadata structure.
 */
extern uint32_t *__metadataOff;

#endif /* __CRT0_H__ */
