
// includes used in triangle.c must be outside of the namespace
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef NO_TIMER
#include <sys/time.h>
#endif /* not NO_TIMER */
#ifdef CPU86
#include <float.h>
#endif /* CPU86 */
#ifdef LINUX
#include <fpu_control.h>
#endif /* LINUX */

#define REAL double

namespace jrs {  // wrap Triangle in a C++ namespace to avoid conflicts

static int (*triunsuitable_callback) (REAL* v1, REAL* v2, REAL* v3, REAL area);  // callback function for external use

void set_triunsuitable_callback(int (*callback) (REAL* v1, REAL* v2, REAL* v3, REAL area))
{
    triunsuitable_callback = callback;
}

int triunsuitable(REAL* triorg, REAL* tridest, REAL* triapex, REAL area)  // triunsuitable used in triangle.c is implemented here
{
    if (triunsuitable_callback)
    {
        return triunsuitable_callback(triorg, tridest, triapex, area);
    }
    else
    {
        // default behaviour copied from triangle.c
        REAL dxoa, dxda, dxod;
        REAL dyoa, dyda, dyod;
        REAL oalen, dalen, odlen;
        REAL maxlen;

        dxoa = triorg[0] - triapex[0];
        dyoa = triorg[1] - triapex[1];
        dxda = tridest[0] - triapex[0];
        dyda = tridest[1] - triapex[1];
        dxod = triorg[0] - tridest[0];
        dyod = triorg[1] - tridest[1];
        /* Find the squares of the lengths of the triangle's three edges. */
        oalen = dxoa * dxoa + dyoa * dyoa;
        dalen = dxda * dxda + dyda * dyda;
        odlen = dxod * dxod + dyod * dyod;
        /* Find the square of the length of the longest edge. */
        maxlen = (dalen > oalen) ? dalen : oalen;
        maxlen = (odlen > maxlen) ? odlen : maxlen;

        if (maxlen > 0.05 * (triorg[0] * triorg[0] + triorg[1] * triorg[1]) + 0.02) {
            return 1;
        } else {
            return 0;
        }
    }
}

#include "triangle.c"  // compile this with C++ compiler to make use of the namespace

void trifree(void* memptr) {  // VOID* in triangle.c is defined as int*, but has to be void* in C++
    trifree((int*) memptr);
}

}
