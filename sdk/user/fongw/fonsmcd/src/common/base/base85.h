/*
 * Ascii85 encode/decode functions
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070411 Pablo Martín Medrano <pablo@fon.com>
 *
 * $Id: base85.h,v 1.1 2012/09/20 03:51:52 paula Exp $
 */
#ifndef _BASE85_H_
#define _BASE85_H_
#ifdef __cplusplus
extern "C" {
#endif
int B85_encode(char *source, char *destination, int size_source, int size_destination);
int B85_decode(char *source, char *destination, int size_source, int size_destination);
#ifdef __cplusplus
}
#endif
#endif
