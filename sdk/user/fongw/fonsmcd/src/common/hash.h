/* $Id: hash.h,v 1.1 2012/09/20 03:51:37 paula Exp $ */
#ifndef _INCLUDED_HASH_H
#define _INCLUDED_HASH_H
#ifdef __cplusplus
extern "C" {
#endif
unsigned int hash(unsigned char *k, unsigned int length, unsigned int initval);
#ifdef __cplusplus
}
#endif
#endif
