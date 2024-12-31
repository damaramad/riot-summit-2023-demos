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

#include <assert.h>
#include <inttypes.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>

#include "rbpf.h"
#include "shared.h"
#include "stdriot.h"

#define PROGNAME "rbpf-unsafe-bench.bin"

#define RBPF_STACK_SIZE   (512)
#define BYTECODE_SIZE_MAX (600)
#define BUFFER_SIZE_MAX   (362)

#define BPF_RUN_N(ctx, size) \
    do { \
        status = RBPF_OK; \
        for (i = 0; i < n && status == RBPF_OK; i++) { \
            status = rbpf_application_run_ctx(rbpf, ctx, \
                size, &result); \
        } \
    } while (0)

typedef struct {
    __bpf_shared_ptr(const uint16_t *, data);
    uint32_t words;
} fletcher32_ctx_t;

static int
bpf_print_result(int64_t result, int status)
{
    switch (status) {
    case RBPF_OK:
        printf(PROGNAME": %lx\n", (uint32_t)result);
        return 0;
        break;
    case RBPF_ILLEGAL_MEM:
        printf(PROGNAME": illegal memory access\n");
        return 1;
        break;
    default:
        printf(PROGNAME": error\n");
        return 1;
    }
}

static int
bpf_run_with_file(rbpf_application_t *rbpf, unsigned n, void *buf,
    size_t buf_size)
{
    rbpf_mem_region_t region;
    int64_t result;
    int status;
    unsigned i;

    fletcher32_ctx_t ctx = {
        .data = (const uint16_t*)(uintptr_t)buf,
        .words = buf_size/2,
    };

    rbpf_memory_region_init(&region, buf, buf_size, RBPF_MEM_REGION_READ);
    rbpf_add_region(rbpf, &region);

    BPF_RUN_N(&ctx, sizeof(ctx));

    return bpf_print_result(result, status);
}

static int
bpf_run_with_integer(rbpf_application_t *rbpf, unsigned n, uint64_t integer)
{
    int64_t result;
    int status;
    unsigned i;

    BPF_RUN_N(&integer, sizeof(integer));

    return bpf_print_result(result, status);
}

static int
bpf_run(rbpf_application_t *rbpf, unsigned n)
{
    int64_t result;
    int status;
    unsigned i;

    BPF_RUN_N(NULL, 0);

    return bpf_print_result(result, status);
}

int
main(int argc, char **argv)
{
    static uint8_t rbpf_stack[RBPF_STACK_SIZE];
    static char buf[BUFFER_SIZE_MAX];
    static char bytecode[BYTECODE_SIZE_MAX];
    static size_t bytecode_size;
    rbpf_application_t rbpf;
    rbpf_mem_region_t region;
    uint64_t integer;
    ssize_t result;
    char *endptr;
    unsigned n;

    if (argc < 3) {
        printf(PROGNAME": <n> <rbpf-file> [file | integer]\n");
        return 1;
    }

    n = (unsigned)strtol(argv[1], &endptr, 10);
    if (argv[1] == endptr ||  *endptr != '\0') {
        printf(PROGNAME": %s: failed to parse run number\n", argv[1]);
        return 1;
    }

    if ((result = copy_file(argv[2], bytecode, BYTECODE_SIZE_MAX)) < 0) {
        printf(PROGNAME": %s: failed to copy bytecode\n", argv[2]);
        return 1;
    }
    bytecode_size = (size_t)result;

    printf(PROGNAME": \"%s\" bytecode loaded at address %p\n", argv[2],
        (void *)bytecode);

    rbpf_application_setup(&rbpf, rbpf_stack, (void *)bytecode,
        bytecode_size);
    rbpf_memory_region_init(&region, bytecode, bytecode_size,
        RBPF_MEM_REGION_READ);
    rbpf_add_region(&rbpf, &region);

    if (argc < 4) {
        return bpf_run(&rbpf, n);
    }

    if ((result = copy_file(argv[3], buf, BUFFER_SIZE_MAX)) >= 0) {
        printf(PROGNAME": \"%s\" data loaded at address %p\n", argv[3],
            (void *)buf);
        return bpf_run_with_file(&rbpf, n, buf, (size_t)result);
    }

    integer = (uint64_t)strtol(argv[3], &endptr, 16);
    if (argv[3] != endptr && *endptr == '\0') {
        return bpf_run_with_integer(&rbpf, n, integer);
    }

    /* other argument types are not yet supported */

    return 1;
}
