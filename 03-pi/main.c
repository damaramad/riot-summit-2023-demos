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

/* https://crypto.stanford.edu/pbc/notes/pi/code.html */

#include <inttypes.h>
#include <stddef.h>

#include "stdriot.h"

#define MAX 500

static int r[MAX + 1];

extern int
main(int argc, char **argv)
{
    int i, k, l;
    int b, d;
    int c;

    for (l = 50; l >= 0; l--)
    {
        c = 0;
        for (i = 0; i < MAX; i++) {
            r[i] = 2000;
        }
        r[i] = 0;

        for (k = MAX; k > 0; k -= 14) {
            d = 0;

            i = k;
            for (;;) {
                d += r[i] * 10000;
                b = 2 * i - 1;

                r[i] = d % b;
                d /= b;
                i--;
                if (i == 0) break;
                d *= i;
            }
            if (l == 0) {
                printf("%.4d", c + d / 10000);
            }

            c = d % 10000;
        }
        if (l == 0) {
            printf("\n");
        }
    }

    return c;
}
