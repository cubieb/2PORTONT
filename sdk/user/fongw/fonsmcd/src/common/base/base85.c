/*
 * Ascii85 encoding/decoding module
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070412 Pablo Martín Medrano <pablo@fon.com>
 *
 * $Id: base85.c,v 1.1 2012/09/20 03:51:52 paula Exp $
 */
/*
 * Based on code on the public domain
 * ftp://ftp.webcom.com/pub/haahr/src/decode85.c
 * ftp://ftp.webcom.com/pub/haahr/src/encode85.c
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "base85.h"
#include "fonsm/log.h"

typedef struct {
	unsigned long tuple;
	int count;
	char *stream;
	char *instream;
	size_t size_instream;
	int index;
	int chunks;
} Encoder;

static void sputchar(Encoder *encoder, char c);
static void init85(Encoder *encoder);
void cleanup85(Encoder *encoder);
static void encode(Encoder *encoder);
static void put85(Encoder *encoder, unsigned c);
static void copy85(Encoder *encoder, char *buffer, size_t size);
static void wput(Encoder *encoder, unsigned long tuple, int bytes);

static unsigned long pow85[] = {
	85*85*85*85, 85*85*85, 85*85, 85, 1
};

static void wput(Encoder *encoder, unsigned long tuple, int bytes)
{
	switch (bytes) {
	case 4:
		sputchar(encoder, tuple >> 24);
		sputchar(encoder, tuple >> 16);
		sputchar(encoder, tuple >>  8);
		sputchar(encoder, tuple);
		break;
	case 3:
		sputchar(encoder, tuple >> 24);
		sputchar(encoder, tuple >> 16);
		sputchar(encoder, tuple >>  8);
		break;
	case 2:
		sputchar(encoder, tuple >> 24);
		sputchar(encoder, tuple >> 16);
		break;
	case 1:
		sputchar(encoder, tuple >> 24);
		break;
	}
}
int sgetc(Encoder *encoder)
{
	int c;

	if (encoder->index == encoder->size_instream)
		return EOF;
	else {
		c = (int)encoder->instream[encoder->index];
		encoder->index++;
		return c;
	}
}

int decode85(Encoder *encoder, char *string, size_t size)
{
	int c;
	encoder->tuple = 0;
	encoder->count = 0;
	encoder->instream = string;
	encoder->size_instream = size;
	encoder->index = 0;

	for (;;)
		switch (c = sgetc(encoder)) {
		default:
			if (c < '!' || c > 'u') {
				// fon_critical("bad character in ascii85 region: %#o", c);
				return 1;
			}
			encoder->tuple += (c - '!') * pow85[(encoder->count)++];
			if (encoder->count == 5) {
				wput(encoder, encoder->tuple, 4);
				encoder->count = 0;
				encoder->tuple = 0;
			}
			break;
		case 'z':
			if (encoder->count != 0) {
				// fon_critical("z inside ascii85 5-tuple");
				return 1;
			}
			sputchar(encoder, 0);
			sputchar(encoder, 0);
			sputchar(encoder, 0);
			sputchar(encoder, 0);
			break;
		case '~':
			if (sgetc(encoder) == '>') {
				if (encoder->count > 0) {
					(encoder->count)--;
					(encoder->tuple) += pow85[encoder->count];
					wput(encoder, encoder->tuple, encoder->count);
				}
				c = sgetc(encoder);
				return 0;
			}
			fon_critical("~ without > in ascii85 section");
			return 1;
		case '\n': case '\r': case '\t': case ' ':
		case '\0': case '\f': case '\b': case 0177:
			break;
		case EOF:
			fon_critical("EOF inside ascii85 section");
			return 1;
		}
}

static void init85(Encoder *encoder)
{
	sputchar(encoder, '<');
	sputchar(encoder, '~');
}

void cleanup85(Encoder *encoder)
{
	if (encoder->count > 0) {
		fon_debug("%s: Doing last encoding", __FUNCTION__);
		encode(encoder);
	}
	sputchar(encoder, '~');
	sputchar(encoder, '>');
}

static void sputchar(Encoder *encoder, char c)
{
	*(encoder->stream) = c;
	encoder->stream += 1;
}

static void encode(Encoder *encoder)
{
	int i;
	char buf[5], *s = buf;
	i = 5;
	do {
		*s++ = encoder->tuple % 85;
		encoder->tuple /= 85;
	} while (--i > 0);
	i = encoder->count;
	do {
		sputchar(encoder, *--s + '!');
	} while (i-- > 0);
	(encoder->chunks)++;
}

static void put85(Encoder *encoder, unsigned c)
{
	switch ((encoder->count)++) {
	case 0:	encoder->tuple |= (c << 24); break;
	case 1: encoder->tuple |= (c << 16); break;
	case 2:	encoder->tuple |= (c <<  8); break;
	case 3:
		encoder->tuple |= c;
		if (encoder->tuple == 0) {
			fon_debug("z");
			sputchar(encoder, 'z');
			(encoder->chunks)++;
		} else
			encode(encoder);
		encoder->tuple = 0;
		encoder->count = 0;
		break;
	}
}

static void copy85(Encoder *encoder, char *buffer, size_t size)
{
	unsigned char c;
	int i;

	for (i = 0; i < size; i++) {
		c = buffer[i];
		put85(encoder, c);
	}
}

int B85_encode(char *source, char *destination, int size_source, int size_destination)
{
	Encoder encoder;

	// fon_debug("B8_encode()");
	memset((void *)&encoder, 0, sizeof(Encoder));
	encoder.stream = destination;
	init85(&encoder);
	copy85(&encoder, source, size_source);
	cleanup85(&encoder);
	// fon_debug("%s: Chunks: %d", __FUNCTION__, encoder.chunks); 
	return encoder.stream - destination;
}

int B85_decode(char *source, char *destination, int size_source, int size_destination)
{
	Encoder encoder;

	// fon_debug("B8_decode()");
	memset((void *)&encoder, 0, sizeof(Encoder));
	encoder.stream = destination;

	if ((source[0] != '<') || (source[1] != '~')) {
		// fon_critical("%s: no prolog", __FUNCTION__);
		return -1;
	}
	if (decode85(&encoder, source + 2, size_source - 2))
		return -1;
	// fon_debug("B8_decode(): finished, size = %d", encoder.stream - destination);
	return encoder.stream - destination;
}

