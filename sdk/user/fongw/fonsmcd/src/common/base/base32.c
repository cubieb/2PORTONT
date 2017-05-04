/*
 * Convert a string to and from it's base32 representation
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20060403 Pablo Martín Medrano <pablo@fon.com>
 *
 * $Id: base32.c,v 1.1 2012/09/20 03:51:52 paula Exp $
 */
/*
 * This implementation differs from the standard in some ways:
 *  + It uses lower case letters
 *  + It uses a different n->letter array (see encode_table)
 *  + It doesn't use padding
 */
#ifdef __MAINTEST__
#include <stdio.h>
#include <string.h>
#endif
#include "base32.h"
#include <sys/types.h>

static const char encode_table[32] = "eaosrnidlctumpbgyvqhfzjxkw765432";

static int encode_chunk(char *from, char *t, int from_size);
static int decode_chunk(char *s, char *t, int from_size);
#ifdef __MAINTEST__
void print_byte(unsigned char byte)
{
	putchar((byte & 0x80)?'1':'0');
	putchar((byte & 0x40)?'1':'0');
	putchar((byte & 0x20)?'1':'0');
	putchar((byte & 0x10)?'1':'0');
	putchar((byte & 0x08)?'1':'0');
	putchar((byte & 0x04)?'1':'0');
	putchar((byte & 0x02)?'1':'0');
	putchar((byte & 0x01)?'1':'0');
	putchar(' ');
}

void print_5bits(unsigned char byte)
{
	printf("%c%c%c%c%c", (byte & 0x10)?'1':'0', (byte & 0x08)?'1':'0',
		(byte & 0x04)?'1':'0', (byte & 0x02)?'1':'0', (byte & 0x01));
}
#endif

/* Encode the five bytes in s to the eight bytes in t */
static int encode_chunk(char *from, char *t, int from_size)
{
	int i;
	char s[5];
	int sizedest;
	int from_to_dest[] = { -1, 2, 4, 5, 7, 8 };
	if ((from_size > 5) || (from_size < 1))
		return -1;
	sizedest = from_to_dest[from_size];	
	for (i = 0; i < from_size; i++)
		s[i] = from[i];
	for (; i < 5; i++)
		s[i] = 0xff;
#ifdef __MAINTEST__
	printf("s: ");
	for (i = 0; i < 5; i++) {
		print_byte(s[i]);
	}
	printf("\n");
#endif
	t[0] = encode_table[((s[0] & 0xf8) >> 3)];
	t[1] = encode_table[((s[0] & 0x07) << 2) + ((s[1] & 0xc0) >> 6)];
	t[2] = encode_table[((s[1] & 0x3e) >> 1)];
	t[3] = encode_table[((s[1] & 0x01) << 4) + ((s[2] & 0xf0) >> 4)];
	t[4] = encode_table[((s[2] & 0x0f) << 1) + ((s[3] & 0x80) >> 7)];
	t[5] = encode_table[((s[3] & 0x7c) >> 2)];
	t[6] = encode_table[((s[3] & 0x03) << 3) + ((s[4] & 0xe0) >> 5)];
	t[7] = encode_table[(s[4] & 0x1f)];
	/*
	printf("t: ");
	for (i = 0; i < 8; i++) {
		print_byte(t[i]);
	}
	printf("\n");
	*/
	return sizedest;
}


/* Decode the eight bytes in s to the five bytes in i t */
static int decode_chunk(char *s, char *t, int from_size)
{
	int i, j;
	char d[8];
	int sizedest;
	int from_to_dest[] = { -1, -1, 1, -1, 2, 3, -1, 4, 5 };
	if ((from_size > 8) || (from_size < 2))
		return -1;
	sizedest = from_to_dest[from_size];
	if (sizedest == -1) 	/* odd reminder */
		return -1;
	for (i = 0; i < 8; i++) {
		if (i >= from_size) {
			d[i] = 'a';
			continue;
		}
		d[i] = -1;
		for (j = 0; j < 32; j++) {
			if (encode_table[j] == s[i])
				d[i] = j;
		}
		if (d[i] == -1)
			return -1;
	}
	/*
	printf("d: ");
	for (i = 0; i < 8; i++) {
		print_byte(d[i]);
	}
	printf("\n");
	*/
	t[0] = (d[0] << 3) + ((d[1] & 0x1c) >> 2);
	t[1] = ((d[1] & 0x03) << 6) + (d[2] << 1) + ((d[3] & 0x10) >> 4);
	t[2] = ((d[3] & 0x0f) << 4) + ((d[4] & 0x1e) >> 1);
	t[3] = ((d[4] & 0x01) << 7) + (d[5] << 2) + ((d[6] & 0x18) >> 3);
	t[4] = ((d[6] & 0x07) << 5) + (d[7]);
#ifdef __MAINTEST__
	printf("t: ");
	for (i = 0; i < 5; i++) {
		print_byte(t[i]);
	}
	printf("\n");
#endif
	return sizedest;
}
/*
 * encode:
 * encode the string pointer by source in the string pointed by destination, returning
 * the size of the encoded array in size_destination. size_source must be at last
 * ((size_source * 8) / 5) + 8;
 */
int B32_encode(char *source, char *destination, int size_source, int size_destination)
{
	int chunks, reminder, size_reminder, i;

	if (size_destination < (((size_source * 8) / 5) + 8))
		return -1;
	chunks = size_source / 5;
	reminder = size_source % 5;
#ifdef __MAINTEST__
	printf("%s: size_source: %d, chunks %d, reminder %d\n", __FUNCTION__, size_source, chunks, reminder);
#endif
	for (i = 0; i < chunks; i++) {
#ifdef __MAINTEST__
		printf("Chunk %d ", i);
#endif
		if (encode_chunk(source + (i * 5), destination + (i * 8), 5) != 8) {
			return -1;
		}
	}
	if (reminder > 0) {
#ifdef __MAINTEST__
		printf("Remin %d ", reminder);
#endif
		size_reminder = encode_chunk(source + (chunks * 5), destination + (chunks * 8), reminder);
		if (size_reminder < 0)
			return -1;
	} else {
		size_reminder = 0;
	}
	return (int)((chunks * 8) + size_reminder); 
}

int B32_decode(char *source, char *destination, int size_source, int size_destination)
{
	int chunks, reminder, size_reminder, i;

	if (size_destination < (((size_source * 5) / 8) + 5)) {
		return -1;
	}
	chunks = size_source / 8;
	reminder = size_source % 8;
#ifdef __MAINTEST__
	printf("%s: size_source %d, chunks %d, reminder %d\n", __FUNCTION__, size_source, chunks, reminder);
#endif
	for (i = 0; i < chunks; i++) {
#ifdef __MAINTEST__
		printf("Chunk %d ", i);
#endif
		if (decode_chunk(source + (i * 8), destination + (i * 5), 8) != 5) {
			return -1;
		}
	}
	if (reminder > 0) {
#ifdef __MAINTEST__
		printf("Remin %d ", reminder);
#endif
		size_reminder = decode_chunk(source + (chunks * 8), destination + (chunks * 5), reminder);
		if (size_reminder < 0)
			return -1;
	} else {
		size_reminder = 0;
	}
	return (int)((chunks * 5) + size_reminder);
}
#ifdef __MAINTEST__
int main(void) {
	char inh[1024], in[1024];
	char buf32[1024], bufdecoded[1024];
	char out[1024], outh[1024];
	size_t r;
 	int size_encoded;
	int size_decoded;
	int i;

	while(fgets(inh, 1023, stdin) != NULL) {
		if (inh[strlen(inh) - 1] == '\n')
			inh[strlen(inh) - 1] = '\0';
		if (HX_convert_from_hex(in, 1024, &r, inh, strlen(inh))) {
			printf("That's not hex, fool!\n");
			return 1;
		}
		printf("r = %lu\n", r);
		for (i = 0; i < r; i++) {
			print_byte(in[i]);
		}
		size_encoded = B3_encode(in, buf32, r, 1024);
		if (size_encoded == -1) {
			printf("Error calling B3_encode!!!!\n");
			return 1;
		}
		buf32[size_encoded] = '\0';
		printf("* %s, %d\n* %s, %d\n", inh, strlen(inh), buf32, size_encoded);
		size_decoded = B32_decode(buf32, out, size_encoded, 1024);
		if (HX_convert_to_hex(outh, 1024, &r, out, size_decoded)) {
			printf("That's not hex, fool!\n");
			return 1;
		}
		outh[r] = '\0';
		printf("* %s, %lu\n", outh, r);
	}
	return 0;
}
#endif

