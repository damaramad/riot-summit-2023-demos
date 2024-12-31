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

#include "stdriot.h"

#define LED_COLOR_GREEN (0)
#define LED_COLOR_RED   (1)
#define LED_COLOR_BLUE  (3)

#define LED_STATUS_OFF  (0)
#define LED_STATUS_ON   (1)

#define LED_TOGGLE(value, color) \
    do { \
        if (strcmp(argv[1], color) == 0) { \
            if (strcmp(argv[2], "on") == 0) { \
                led_on(value); \
                return 0; \
            } \
            if (strcmp(argv[2], "off") == 0) { \
                led_off(value); \
                return 0; \
            } \
            printf("%s: %s: unknown led status\n", \
                argv[0], argv[2]); \
            return 1; \
        } \
    } while (0)

static int
strcmp(const char *l, const char *r)
{
    for (; *l==*r && *l; l++, r++);
    return *(unsigned char *)l - *(unsigned char *)r;
}

static void
led_on(int led)
{
    int status;

    status = get_led(led);

    if (status == LED_STATUS_ON) {
        printf("the led is already on\n");
        return;
    }

    if (status != LED_STATUS_OFF) {
        printf("failed to get led status\n");
        return;
    }

    if (set_led(led, LED_STATUS_ON) != 0) {
        printf("failed to set led status\n");
    }
}

static void
led_off(int led)
{
    int status;

    status = get_led(led);

    if (status == LED_STATUS_OFF) {
        printf("the led is already off\n");
        return;
    }

    if (status != LED_STATUS_ON) {
        printf("failed to get the led status\n");
        return;
    }

    if (set_led(led, LED_STATUS_OFF) != 0) {
        printf("failed to set led status\n");
    }
}

extern int
main(int argc, char **argv)
{
    if (argc < 3) {
        printf("%s: <red | green | blue> <on | off>\n",
            argv[0]);
        return 1;
    }

    LED_TOGGLE(LED_COLOR_RED, "red");
    LED_TOGGLE(LED_COLOR_GREEN, "green");
    LED_TOGGLE(LED_COLOR_BLUE, "blue");

    printf("%s: %s: unknown led color\n", argv[0],
        argv[1]);

    return 1;
}
