/*
 * Base64 encoding/decoding routines
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070412 Pablo Martín Medrano <pablo@fon.com>
 *
 * $Id: base64.h,v 1.1 2012/09/20 03:51:52 paula Exp $
 */
#ifndef _BASE64_H_
#define _BASE64_H_
#ifdef __cplusplus
extern "C" {
#endif
int B64_encode(char *source, char *destination, int size_source, int size_destination);
int B64_decode(char *source, char *destination, int size_source, int size_destination);
#ifdef __cplusplus
}
#endif
#endif
