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

#include <stdarg.h>
#include <stddef.h>

#include "stdriot.h"

enum syscall_index_e {
    PIP,
    EXIT,
    PRINTF,
    GET_TEMP,
    ISPRINT,
    STRTOL,
    GET_LED,
    SET_LED,
};

typedef int (*exit_t)(int status);
typedef int (*vprintf_t)(const char *format, va_list ap);
typedef int (*get_temp_t)(void);
typedef int (*isprint_t)(int character);
typedef long (*strtol_t)(const char *str, char **endptr, int base);
typedef int (*get_led_t)(int pos);
typedef int (*set_led_t)(int pos, int val);

extern int main(int argc, char **argv);

static void **syscall_prev_got = NULL;
static void **syscall_curr_got = NULL;
static void **syscall_table = NULL;
static int syscall_is_init = 0;

static inline void
_set_sl(volatile void *val)
{
    __asm__ volatile ("mov sl, %0" : : "r" (val) :);
}

extern void
syscall_init(void *prev_got, void *curr_got, void **table)
{
    syscall_prev_got = prev_got;
    syscall_curr_got = curr_got;
    syscall_table = table;
    syscall_is_init = 1;
}

extern void
exit(int status)
{
    volatile void *prev_got;
    volatile exit_t func;

    if (syscall_table[PIP] == (void *)0) {
        func = syscall_table[EXIT];
        prev_got = syscall_prev_got;

        _set_sl(prev_got);
        (*func)(status);
    } else {
        __asm__ volatile
        (
            "mov r0, #0\n"
            "mov r1, %0\n"
            "push {r0, r1}\n"
            "mov r0, #0\n"
            "mov r1, %1\n"
            "mov r2, #0\n"
            "mov r3, #1\n"
            "mov r4, #1\n"
            "svc #12\n"
            /* UNREACHABLE */
            :
            : "r" (status),
              "r" (syscall_table[EXIT])
            :
        );
    }
}

extern int
printf(const char * format, ...)
{
    volatile vprintf_t func;
    volatile void *prev_got;
    volatile void *curr_got;
    int res = 0;
    va_list ap;

    if (syscall_table[PIP] == (void *)0) {
        func = syscall_table[PRINTF];
        prev_got = syscall_prev_got;
        curr_got = syscall_curr_got;

        va_start (ap, format);
        _set_sl(prev_got);
        res = (*func)(format, ap);
        _set_sl(curr_got);
        va_end (ap);
    } else {
        va_start (ap, format);
        __asm__ volatile
        (
            "mov r0, #1\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            "push {r0, r1, r2}\n"
            "mov r0, #0\n"
            "mov r1, %3\n"
            "mov r2, #0\n"
            "mov r3, #1\n"
            "mov r4, #1\n"
            "svc #12\n"
            "pop {%0}\n"
            "add sp, sp, #8\n"
            : "=r" (res)
            : "r" (format),
              "r" (ap),
              "r" (syscall_table[PRINTF])
            : "r0", "r1", "r2", "r3", "r4"
        );
        va_end (ap);
    }

    return res;
}

extern int
get_temp(void)
{
    volatile get_temp_t func;
    volatile void *prev_got;
    volatile void *curr_got;
    int res = 0;

    if (syscall_table[PIP] == (void *)0) {
        func = syscall_table[GET_TEMP];
        prev_got = syscall_prev_got;
        curr_got = syscall_curr_got;

        _set_sl(prev_got);
        res = (*func)();
        _set_sl(curr_got);
    } else {
        __asm__ volatile
        (
            "mov r0, #2\n"
            "push {r0}\n"
            "mov r0, #0\n"
            "mov r1, %1\n"
            "mov r2, #0\n"
            "mov r3, #1\n"
            "mov r4, #1\n"
            "svc #12\n"
            "pop {%0}\n"
            : "=r" (res)
            : "r" (syscall_table[GET_TEMP])
            : "r0", "r1", "r2", "r3", "r4"
        );
    }

    return res;
}

extern int
isprint(int character)
{
    volatile isprint_t func;
    volatile void *prev_got;
    volatile void *curr_got;
    int res = 0;

    if (syscall_table[PIP] == (void *)0) {
        func = syscall_table[ISPRINT];
        prev_got = syscall_prev_got;
        curr_got = syscall_curr_got;

        _set_sl(prev_got);
        res = (*func)(character);
        _set_sl(curr_got);
    } else {
        __asm__ volatile
        (
            "mov r0, #3\n"
            "mov r1, %1\n"
            "push {r0, r1}\n"
            "mov r0, #0\n"
            "mov r1, %2\n"
            "mov r2, #0\n"
            "mov r3, #1\n"
            "mov r4, #1\n"
            "svc #12\n"
            "pop {%0}\n"
            "add sp, sp, #4\n"
            : "=r" (res)
            : "r" (character),
              "r" (syscall_table[ISPRINT])
            : "r0", "r1", "r2", "r3", "r4"
        );
    }

    return res;
}

extern long
strtol(const char *str, char **endptr, int base)
{
    volatile strtol_t func;
    volatile void *prev_got;
    volatile void *curr_got;
    int res = 0;

    if (syscall_table[PIP] == (void *)0) {
        func = syscall_table[STRTOL];
        prev_got = syscall_prev_got;
        curr_got = syscall_curr_got;

        _set_sl(prev_got);
        res = (*func)(str, endptr, base);
        _set_sl(curr_got);
    } else {
        __asm__ volatile
        (
            "mov r0, #4\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            "mov r3, %3\n"
            "push {r0-r3}\n"
            "mov r0, #0\n"
            "mov r1, %4\n"
            "mov r2, #0\n"
            "mov r3, #1\n"
            "mov r4, #1\n"
            "svc #12\n"
            "pop {%0}\n"
            "add sp, sp, #12\n"
            : "=r" (res)
            : "r" (str),
              "r" (endptr),
              "r" (base),
              "r" (syscall_table[STRTOL])
            : "r0", "r1", "r2", "r3", "r4"
        );
    }

    return res;
}

extern int
get_led(int pos)
{
    volatile get_led_t func;
    volatile void *prev_got;
    volatile void *curr_got;
    int res = 0;

    if (syscall_table[PIP] == (void *)0) {
        func = syscall_table[GET_LED];
        prev_got = syscall_prev_got;
        curr_got = syscall_curr_got;

        _set_sl(prev_got);
        res = (*func)(pos);
        _set_sl(curr_got);
    } else {
        __asm__ volatile
        (
            "mov r0, #5\n"
            "mov r1, %1\n"
            "push {r0, r1}\n"
            "mov r0, #0\n"
            "mov r1, %2\n"
            "mov r2, #0\n"
            "mov r3, #1\n"
            "mov r4, #1\n"
            "svc #12\n"
            "pop {%0}\n"
            "add sp, sp, #4\n"
            : "=r" (res)
            : "r" (pos),
              "r" (syscall_table[GET_LED])
            : "r0", "r1", "r2", "r3", "r4"
        );
    }

    return res;
}

extern int
set_led(int pos, int val)
{
    volatile set_led_t func;
    volatile void *prev_got;
    volatile void *curr_got;
    int res = 0;

    if (syscall_table[PIP] == (void *)0) {
        func = syscall_table[SET_LED];
        prev_got = syscall_prev_got;
        curr_got = syscall_curr_got;

        _set_sl(prev_got);
        res = (*func)(pos, val);
        _set_sl(curr_got);
    } else {
        __asm__ volatile
        (
            "mov r0, #6\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            "push {r0-r2}\n"
            "mov r0, #0\n"
            "mov r1, %3\n"
            "mov r2, #0\n"
            "mov r3, #1\n"
            "mov r4, #1\n"
            "svc #12\n"
            "pop {%0}\n"
            "add sp, sp, #8\n"
            : "=r" (res)
            : "r" (pos),
              "r" (val),
              "r" (syscall_table[SET_LED])
            : "r0", "r1", "r2", "r3", "r4"
        );
    }

    return res;
}

extern int
start(interface_t *interface, void *gotAddr,
    void *oldGotAddr, void **syscalls)
{
    char **argv;
    int argc;

    syscall_init(oldGotAddr, gotAddr, syscalls);

    argc = (int)(((uint32_t *)interface->stackTop)[0]);
    argv = (char **)&(((uint32_t *) interface->stackTop)[1]);

    return main(argc, argv);
}
