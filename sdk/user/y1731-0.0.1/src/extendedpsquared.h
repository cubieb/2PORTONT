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
#ifndef OAM_EXTENDEDPSQUARED_H
#define OAM_EXTENDEDPSQUARED_H

#include <stdint.h>

/**
 * Holds the information needed fo the extended p^2
 */
struct oam_ep2 {
    double       *q; /**< pointer to the heights array (2*m+3) */
    double       *n; /**< pointer to the actual positions array (2*m+3) */
    double       *f; /**< pointer to the increments of desired positions array (2*m+3) */
    double       *d; /**< pointer to the desired positions array (2*m+3) */
    unsigned int  m; /**< Number of quantiles, */
    unsigned int  num_of_markers; /**< Number of markers, i.e. 2 * m + 3 */
    uint64_t      num_of_observations; /**< Number of observations */
} __attribute__ ((packed));

int oam_ep2_allocate(const int m, int size, struct oam_ep2 **ptr);
void oam_ep2_free(struct oam_ep2 *ptr);

void oam_ep2_create_quantiles(struct oam_ep2 *ptr, const unsigned int m);
void oam_ep2_set_quantiles(struct oam_ep2 *ptr, const unsigned int m, ...);
void oam_ep2_add_value(double value, struct oam_ep2 *ptr);

void oam_ep2_pretty_print(const struct oam_ep2 *const ptr);

void oam_ep2_print_serial(const int rotations, const struct oam_ep2 *const ptr);

#endif /* OAM_EXTENDEDPSQUARED_H */
