/* $Id: sigs.h,v 1.1.1.1 2003/08/18 05:40:13 kaohj Exp $ */

/*
 * sigs.h
 *
 *	common header definitions for signature wrappers.
 *
 * Gene Kim
 * Purdue University
 * October 12, 1992
 */

#include <stdio.h>
#include <fcntl.h>
#if !defined(SYSV) || (SYSV > 3)
# include <sys/file.h>
#else
# include <unistd.h>
#endif 	/* SYSV */
#if (defined(SYSV) && (SYSV < 3))
# include <limits.h>
# include <unistd.h>
#endif	/* SVR2 */

#ifndef SEEK_SET
# define SEEK_SET L_SET
#endif

char *pltob64();
extern int printhex;

/* prototypes */
#ifdef __STDC__
int sig_null_get(int, char *, int);
#else
int sig_null_get();
#endif
