/*
 * Copyright (c) 2010-2012 Helsinki Institute for Information Technology.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file
 *
 * @author Samu Varjonen
 */ 

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "debug.h"
#include "extendedpsquared.h"

#if 0
/**
 * Sort function for integers. 
 *
 * @param x First number for the comparison
 * @param y Second number for the comparison
 *
 * @return Negative if y is larger, posivite if x is larger and 0 if x == y
 */
static int oam_ep2_sort_int(const void *const x, const void *const y) 
{
    return (*(int*)x - *(int*)y);
}
#endif 

/**
 * Sort function for doubles. 
 *
 * @param x First number for the comparison
 * @param y Second number for the comparison
 *
 * @return Negative if y is larger, posivite if x is larger and 0 if x == y
 */
static int oam_ep2_sort_double(const void *const x, const void *const y) 
{
    if (*(double*)x > *(double*)y) {
        return 1;
    } else if (*(double*)x < *(double*)y) {
        return -1;
    } else {
        /* equal */
        return 0;
    }
}


/**
 * Allocates the required memory for the extended P^2, i.e. ep2 structure 
 *
 * @param m How many quantiles, i.e. cells
 * @param size What is the size of the cell
 * @param ptr Pointer to the created memory is stored here.
 *
 * @return Zero on success, non-zero else.
 *
 * @note Caller is responsible of freeing this structure with oam_ep2_free()
 * 
 * @see oam_ep2_free
 */
int oam_ep2_allocate(const int m, int size, struct oam_ep2 **ptr)
{    
    struct oam_ep2 *ep2;

    ep2 = calloc(1, sizeof(struct oam_ep2));    
    if (!ep2) {
        oam_ep2_free(ep2);
        return -1;
    }

    ep2->m = m;
    ep2->num_of_markers = 2 * m + 3;
    ep2->num_of_observations = 0;

    ep2->q = calloc(ep2->num_of_markers, size);
    if (!ep2->q) {
        oam_ep2_free(ep2);
        return -1;
    }
    ep2->n = calloc(ep2->num_of_markers, size);
    if (!ep2->n) {
        oam_ep2_free(ep2);
        return -1;
    }
    ep2->f = calloc(ep2->num_of_markers, size);
    if (!ep2->f) {
        oam_ep2_free(ep2);
        return -1;
    }
    ep2->d = calloc(ep2->num_of_markers, size);
    if (!ep2->d) {
        oam_ep2_free(ep2);
        return -1;
    } 

    *ptr = ep2;

    return 0;
}

/**
 * Free ep2 structure
 *
 * @return Zero on success, non-zero else.
 */
void oam_ep2_free(struct oam_ep2 *ptr) 
{
    free(ptr->q);
    free(ptr->n);
    free(ptr->f);
    free(ptr->d);
    free(ptr);
}

/**
 * Sets the given number <i>m</i> quantiles based on the 
 * formula p_i = i/(m + i), i = 1,...,m
 *
 * @param ptr Pointer to the quantiles array
 * @param m Number of quantiles 
 */
void oam_ep2_create_quantiles(struct oam_ep2 *ptr, const unsigned int m)
{
    unsigned int  i;
    int           index;
    double       *f = ptr->f;
    double       *n = ptr->n;
    double       *d = ptr->d;

    for (i = 0; i < ptr->num_of_markers; i++) {
        n[i] = i + 1;
    }

    f[0] = 0.0;
    f[ptr->num_of_markers -1] = 1.0;
    for (i = 1; i <= m; i++) {
        index = 2 * i;
        f[index] = (i / (m + 1.0));
    }

    for (i = 0; i <= m; i++) {
        f[2 * i +  1] = (f[2 * i] + f[2 * (i + 1)]) / 2.0;
    }
    for (i = 0; i < ptr->num_of_markers; i++) {
        d[i] = 1.0 + 2.0 * (m + 1.0) * f[i];
    }
}

/**
 * Sets the given quantiles into the ep2->q
 *
 * @param ptr Pointer to the quantiles array
 * @param m Number of quantiles
 * @param ... quantiles separated by comma 
 *
 * @note Caller is responsible for the inclusion of corresponding 
 *       number of quantiles, i.e. exactly what the m tells.
 *
 * @note Give the quantiles in *order*, i.e. 0.2, 0.5, 0.7, etc.
 */
void oam_ep2_set_quantiles(struct oam_ep2 *ptr, const unsigned int m, ...)
{
    unsigned int  i;
    double        j;
    double       *f = ptr->f;
    double       *n = ptr->n;
    double       *d = ptr->d;
    va_list       ap; 

    for (i = 0; i < ptr->num_of_markers; i++) {
        n[i] = i + 1;
    }

    va_start(ap, m);
    f[0] = 0.0;
    f[ptr->num_of_markers -1] = 1.0;
    for(i = 0; i < m; i++) {
        j = va_arg(ap, double);
        f[2 * (i + 1)] = j;
    }
    va_end(ap);

    for (i = 0; i <= m; i++) {
        f[2 * i +  1] = (f[2 * i] + f[2 * (i + 1)]) / 2.0;
    }
    for (i = 0; i < ptr->num_of_markers; i++) {
        d[i] = 1.0 + 2.0 * (m + 1.0) * f[i];
    }

}

#if 0
/**
 * Check if the given q array, in ep2 struct, contains distinct values or not.
 *
 * @param ptr Pointer to the ep2 struct where the given q is to be checked
 *
 * @return Zero if all the cells contain distinct values, negative else.
 */
static int oam_ep2_check_q(struct oam_ep2 *ptr) 
{
    int i;

    qsort(ptr->q, 
          ptr->num_of_markers, 
          sizeof(double), 
          oam_ep2_sort_double);

    for (i = 1; i < ptr->num_of_markers; i++) {
        if (ptr->q[i -1] == ptr->q[i]) {
            return -1;
        }
    } 
    return 0;
}
#endif

/**
 * Add a value 
 *
 * @param value Value to be added
 * @param ptr Pointer to the struct where the value is added
 */
void oam_ep2_add_value(double value, struct oam_ep2 *ptr) 
{
    unsigned int i;
    unsigned int cell_index = 0;
    double       d;
    double       dm;
    double       dp;
    double       qm;
    double       qp;
    double       qt;
    
    if (ptr->num_of_observations < (uint64_t)ptr->num_of_markers) {
        /*
         * Initialization phase: wait for 2*m+3 observations.
         * They do need to be different. This does not check that yet!
         *
         * After the required observations is gathered the array is sorted.
         */
        ptr->q[ptr->num_of_observations] = value;
        if (ptr->num_of_observations == (uint64_t)(ptr->num_of_markers -1)) {
            qsort(ptr->q, 
                  ptr->num_of_markers, 
                  sizeof(double), 
                  oam_ep2_sort_double);
        }
#if 0
    } else if (ptr->num_of_observations >= (uint64_t)ptr->num_of_markers && 
               oam_ep2_check_q(ptr) == -1) {
        /* Required number of observations for initialization 
           but not distinct IS THIS CORRECT */
        OAM_DEBUG("WENT TO TEST FEATURE NOT IN ALGO\n");
        for (i = 1; i < ptr->num_of_markers; i++) {
            if (ptr->q[i -1] == ptr->q[i]) {
                ptr->q[i] = value;
            }
        } 
#endif
    } else {
        /*
         * Addition phase: Add value to the array and follow the algorithm
         */
        /* a: find cell where q[i-1] <= value < q[i] */
        if (value < ptr->q[0]) {
            cell_index = 1; 
            ptr->q[0] = value;
        } else if (value >= ptr->q[ptr->num_of_markers - 1]) {
            cell_index = ptr->num_of_markers - 1;
            ptr->q[ptr->num_of_markers - 1] = value;
        } else {
            for (i = 1; i < ptr->num_of_markers; i++) {
                if ((value >= ptr->q[i -1]) && (value < ptr->q[i])) {
                    cell_index = i;
                    break;
                }                    
            }
        }
        /* b: increase actual positions of markers 
              cell_index, ..., ptr->num_of_markers -1*/
        for (i = cell_index; i < ptr->num_of_markers; i++) {
            ptr->n[i]++;
        }
        /* c: increase desired positions of all markers */
        for (i = 0; i < ptr->num_of_markers; i++) {
            ptr->d[i] += ptr->f[i];
        }
        /* d: adjust heigths and actual positions of markers if needed */
        for (i = 1; i < (ptr->num_of_markers - 1); i++) {
            /* desired position offset */
            d = ptr->d[i] - ptr->n[i];
            /* next position offset */
            dp = ptr->n[i + 1] - ptr->n[i];
            /* previous position offset */
            dm = ptr->n[i - 1] - ptr->n[i];
            
            qp = (ptr->q[i +1] - ptr->q[i]) / dp;
            qm = (ptr->q[i -1] - ptr->q[i]) / dm;
            // #if 0            
            if ((d >= 1) && (dp > 1)) {
                qt = ptr->q[i] + 
                    ((1.0 - dm) * qp + (dp - 1.0) * qm) / 
                     (dp -dm);
                if ((qt > ptr->q[i -1]) && (qt < ptr->q[i + 1])) {
                    ptr->q[i] = qt;
                } else {
                    ptr->q[i] += qp;
                }
                ptr->n[i]++;
            } else if ((d <= -1) && (dm < -1)) {
                qt = ptr->q[i] -
                    ((1.0 + dp) * qm - (dm + 1.0) * qp) /
                     (dp -dm);
                if ((qt > ptr->q[i - 1]) && (qt < ptr->q[i + 1])) {
                    ptr->q[i] = qt;
                } else {
                    ptr->q[i] -= qm;
                }
                ptr->n[i]--;
            }
        }
    }
    ptr->num_of_observations++;
}

#if 0 
/**
 * Debug print the contents of the ep2
 *
 * @param ptr Pointer to the ep2 structure 
 *
 * @note Convert printfs to debug print macros.
 */
static void oam_ep2_debug_print(const struct oam_ep2 *const ptr)
{
    int     i;
    double *cell;

    OAM_DEBUG("\nep2->q: ");
    cell = ptr->q;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        printf("%f ", cell[i]);
    } 
    OAM_DEBUG("\nep2->n: ");
    cell = ptr->n;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        OAM_DEBUG("%f ", cell[i]);
    } 
    OAM_DEBUG("\nep2->f: ");
    cell = ptr->f;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        OAM_DEBUG("%f ", cell[i]);
    } 
    OAM_DEBUG("\nep2->d: ");
    cell = ptr->d;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        OAM_DEBUG("%f ", cell[i]);
    } 
    OAM_DEBUG("\nep2->m: %d\n", ptr->m);
    OAM_DEBUG("ep2->num_of_markers: %d\n", ptr->num_of_markers);
    OAM_DEBUG("ep2->num_of_observations: %lu\n", ptr->num_of_observations);
}
#endif 

/**
 * Print the contents of the ep2
 *
 * @param ptr Pointer to the ep2 structure 
 *
 * @note Convert printfs to debug print macros.
 */
void oam_ep2_pretty_print(const struct oam_ep2 *const ptr)
{
    unsigned int i;

    if (ptr->num_of_observations < ptr->num_of_markers) {
        OAM_DEBUG("Too few observations to complete ep^2\n");
    }

    OAM_DEBUG("Quantile:\n");
    //OAM_DEBUG("%.0f%% = %f\n", 
    OAM_DEBUG("MIN = %f\n", 
              //ptr->f[0] * 100, 
              ptr->q[0]);
    for (i = 0; i < ptr->m; i++) {
        OAM_DEBUG("%.0f%% = %f\n", 
                  ptr->f[2 * (i + 1)] * 100, 
                  ptr->q[2 * (i + 1)]);
    }
    //OAM_DEBUG("%.0f%% = %f\n", 
    OAM_DEBUG("MAX = %f\n", 
              //ptr->f[ptr->num_of_markers - 1] * 100, 
              ptr->q[ptr->num_of_markers - 1]);
    OAM_DEBUG("Num of observations %lu\n", ptr->num_of_observations);
}

/**
 * Print the contents of the ep2 in semicomma separated format
 * in order:
 * m
 * num_of_observartions
 * rotations
 * q[]
 * n[]
 * f[]
 * d[]
 * num_of_markers
 *
 * @param rotations how many times over 1000 samples
 * @param ptr Pointer to the ep2 structure 
 *
 * @note uses printf so DO NOT use in the daemon
 */
void oam_ep2_print_serial(const int rotations, const struct oam_ep2 *const ptr)
{
    unsigned int  i;
    double       *cell;

    fprintf(stderr, "%d;", ptr->m);
    fprintf(stderr, "%" PRIu64 ";", ptr->num_of_observations);
    fprintf(stderr, "%d;", rotations);
    cell = ptr->q;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        fprintf(stderr, "%f;", cell[i]);
    } 
    cell = ptr->n;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        fprintf(stderr, "%f;", cell[i]);
    } 
    cell = ptr->f;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        fprintf(stderr, "%f;", cell[i]);
    } 
    cell = ptr->d;
    for (i = 0 ; i < ptr->num_of_markers ; i ++) {
        fprintf(stderr, "%f;", cell[i]);
    } 
    fprintf(stderr, "%d\n", ptr->num_of_markers);
}
