/*
 * Convert a string to and from it's base32 representation
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20060403 Pablo Martín Medrano <pablo@fon.com>
 *
 * $Id: base32.h,v 1.1 2012/09/20 03:51:52 paula Exp $
 */
#ifndef _BASE32_H_
#define _BASE32_H_
#ifdef __cplusplus
extern "C" {
#endif
int B32_encode(char *source, char *destination, int size_source, int size_destination);
int B32_decode(char *source, char *destination, int size_source, int size_destination);
#ifdef __cplusplus
}
#endif
#endif
