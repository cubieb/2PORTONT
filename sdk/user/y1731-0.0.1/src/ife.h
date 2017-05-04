/*
 * Copyright (c) 2010-2012 Helsinki Institute for Information Technology
 * and University of Helsinki.
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
 *
 * @note Functionality is based on the HIPL's (www.infrahip.net) ife.h
 */ 

#ifndef OAM_IFE_H
#define OAM_IFE_H

#include "debug.h"

/** 
 * Use this macro to free memory of variables.
 *
 * @param variable to be freed.
 *
 * @note Only memory that free() can free, i.e. do not use for ezxml etc.
 */
#define OAM_FREE(variable) \
    { \
        if (variable) { \
	     free(variable);			\
        } \
    }

/**
 * Use this macro to detect failures and exit function in case
 * of such. Variable 'err' must be defined, usually type int.
 * Label 'out_err' must be defined, on errors this label is used
 * as destination after proper actions.
 *
 * @param func Nonzero, if failure.
 * @param eval Set variable called 'err' to this value.
 */
#define OAM_IFE(func, eval) \
    { \
        if (func) { \
            err = eval; \
            goto out_err; \
        } \
    }

/**
 * Use this macro to detect failures and exit function in case
 * of such. Variable 'err' must be defined, usually type int.
 * Label 'out_err' must be defined, on errors this label is used
 * as destination after proper actions.
 *
 * @param func Nonzero, if failure.
 * @param eval Set variable called 'err' to this value.
 */
#define OAM_IFEL(func, eval, ...) \
    { \
        if (func) { \
            OAM_ERROR(__VA_ARGS__); \
            err = eval; \
            goto out_err; \
        } \
    }


#endif /* OAM_IFE_H */
