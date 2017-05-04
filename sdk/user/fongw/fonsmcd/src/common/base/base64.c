/*
 * Base64 encoding/decoding routines
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070412 Pablo Martín Medrano <pablo@fon.com>
 *
 * $Id: base64.c,v 1.1 2012/09/20 03:51:52 paula Exp $
 */

#include "base64.h"
#include <stdio.h>

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/* Note that '=' (padding) is 0 */
static const unsigned char fb64[256] = {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
	255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
	255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

static int encodeblock( unsigned char *in, char *out, int len )
{
	char s[3];
	int i;

	for (i = 0; i < len; i++)
		s[i] = in[i];
	for (i = len; i < 3; i++)
		s[i] = 0;
	out[0] = (unsigned char)(cb64[(s[0] & 0xfc) >> 2 ]);
	out[1] = (unsigned char)(cb64[((s[0] & 0x03) << 4) | ((s[1] & 0xf0) >> 4) ]);
	out[2] = (unsigned char)(cb64[((s[1] & 0x0f) << 2) | ((s[2] & 0xc0) >> 6) ]);
	out[3] = (unsigned char)(cb64[s[2] & 0x3f ]);
	switch (len) {
		case 1:
			out[3] = '=';
		case 2:
			out[2] = '=';
			break;
		default:
			break;
	}

	return 4;
}

static int decodeblock(char *ins, unsigned char *out, int len)
{
	int i, s[4];
	unsigned char in[4];
	int skip = 0;

	if (len != 4)
		return -1;
	for (i = 0; i < len; i++) {
		if (ins[i] == '=') {
			in[i] = 0;
			skip++;  
		} else
			in[i] = fb64[(int)(ins[i])];
		if (in[i] == 255) {
			return -1;
		}
	}
	out[0] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	if (skip == 2) {
		return 1;
	}
	out[1] = (unsigned char )((in[1] & 0x0f) << 4 | in[2] >> 2);
	if (skip == 1) {
		return 2;
	}
	out[2] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);

	return 3;
}

int B64_encode(char *source, char *destination, int size_source, int size_destination)
{
	int chunks, reminder, size, d, i, size_expected;
	char *s;
	unsigned char *t;

	chunks = size_source / 3;
	reminder = size_source % 3;
	size = 0;
	size_expected = (chunks * 4) + (reminder?(reminder + 1):0);
	if (size_destination < ((chunks * 4) + (reminder?4:0))) {
		return 1;
	}
	for (i = 0; i < chunks; i++) {
		s = source + (i * 3); 
		t = destination + (i * 4);
		d = encodeblock(s, t, 3);
		if (d == -1) {
			return 1;
		}
		size += d;
	}
	if (reminder) {
		d = encodeblock(source + (chunks * 3), destination + (chunks * 4), reminder);
		if (d == -1) {
			return 1;
		}
		size += d;
	}
	return size;
}

int B64_decode(char *source, char *destination, int size_source, int size_destination)
{
	int chunks, reminder, size, d, i;

	chunks = size_source / 4;
	reminder = size_source % 4;
	size = 0;
	if (reminder) {
		return 1;
	}
	if (size_destination < ((chunks * 3))) {
		size_destination, ((chunks * 3) + reminder);
		return -1;
	}
	for (i = 0; i < chunks; i++) {
		d = decodeblock(source + (i * 4), destination + (i * 3), 4);
		if (d == -1) {
			return -1;
		}
		size += d;
	}
	return size;
}

