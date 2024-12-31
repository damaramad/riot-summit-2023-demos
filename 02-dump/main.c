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

#include <inttypes.h>
#include <limits.h>
#include <stddef.h>

#include "stdriot.h"

#define FST_COL_END 8
#define SND_COL_END 16

static void
dump(void *addr, size_t size)
{
    volatile unsigned char *ptr, *eaddr;
    unsigned char byte;
    unsigned int i, j;

    ptr = addr;
    eaddr = (void *)(uintptr_t)ptr + size;
    while (ptr < eaddr) {
        printf("%" PRIxPTR "  ", (uintptr_t)ptr);

        i = 0;
        while (i < FST_COL_END && ptr + i < eaddr)
            printf("%02x ", ptr[i++]);

        printf("  ");

        while (i < SND_COL_END && ptr + i < eaddr)
            printf("%02x ", ptr[i++]);

        j = i;

        while (i < SND_COL_END) {
            printf("   ");
            i++;
        }

        printf(" |");

        i = 0;
        while (i < j) {
            byte = ptr[i];
            if (isprint(byte) != 0)
                printf("%c", byte);
            else
                printf(".");
            i++;
        }

        printf("|\n");

        ptr += i;
    }

    printf("%" PRIxPTR "\n", (uintptr_t)ptr);
}

int
main(int argc, char **argv)
{
    void *address;
    char *endptr;
    size_t size;

    if (argc < 3) {
        printf("%s: address size\n", argv[0]);
        return 1;
    }

    address = (void *)strtol(argv[1], &endptr, 16);
    size = (size_t)strtol(argv[2], &endptr, 10);

    dump(address, size);

    return 0;
}
