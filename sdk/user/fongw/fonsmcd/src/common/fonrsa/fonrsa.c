/*
 * FONSM RSA handling library
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Created: 20070306 Pablo Martin Medrano <pablo@fon.com>
 *
 * Based on axTLS
 *
 * $Id: fonrsa.c,v 1.1 2012/09/20 03:52:09 paula Exp $
 */
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "rmd160.h"
#include "bigint.h"
#include "fonrsa.h"
#include <base/base64.h>

typedef struct {
	uint8_t *buffer;
	size_t size;
} DER_key;

typedef struct {
	bigint *m;              /* modulus */
	bigint *e;              /* public exponent */
	int num_octets;
	BI_CTX *bi_ctx;         /* big integer handle */
} RSA_parameters;

typedef struct {
	DER_key *derkey;
	RSA_parameters *rsaparms;
} RSA;

static uint8_t *CH_load_raw_file(char *filename, size_t *size);
static DER_key *CH_load_der_key(char *filename);
static void CH_free_der_key(DER_key *key);
static int asn1_get_public_key(const uint8_t *buf, int len, RSA_parameters **rsa_parameters);
void CH_pub_key_new(RSA_parameters **rsa_parameters, const uint8_t *modulus, int mod_len, const uint8_t *pub_exp, int pub_len);
int CH_decrypt(RSA_parameters *rsa, uint8_t *buffer_in, uint8_t *buffer_out);
byte *RMDbinary(char *fname);
int CH_get_rmd160_hash_from_signature(byte *hash, char *signature_file, char *public_key_file);
static unsigned char *load_file_in_buffer(char *path, int *size);
static int save_file_from_buffer(char *path, unsigned char *buffer, int size);

#define RMDsize 160		/* A RIPEMD-160 hash has 160 bits */

/*
 * returns RMD(message in file fname) fname is read as binary data.
 * non-reentrant
 */
byte *RMDbinary(char *fname)
{
	FILE *mf;		/* pointer to file <fname>      */
	byte data[1024];	/* contains current mess. block */
	dword nbytes;		/* length of this block         */
	dword MDbuf[RMDsize / 32];	/* contains (A, B, C, D(, E))   */
	static byte hashcode[RMDsize / 8];	/* for final hash-value         */
	dword X[16];		/* current 16-word chunk        */
	unsigned int i, j;	/* counters                     */
	dword length[2];	/* length in bytes of message   */
	dword offset;		/* # of unprocessed bytes at    */
	/*          call of MDfinish    */

	/* initialize */
	if ((mf = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "\nRMDbinary: cannot open file \"%s\".\n",
			fname);
		exit(1);
	}
	MDinit(MDbuf);
	length[0] = 0;
	length[1] = 0;
	while ((nbytes = fread(data, 1, 1024, mf)) != 0) {
		/* process all complete blocks */
		for (i = 0; i < (nbytes >> 6); i++) {
			for (j = 0; j < 16; j++)
				X[j] = BYTES_TO_DWORD(data + 64 * i + 4 * j);
			compress(MDbuf, X);
		}
		/* update length[] */
		if (length[0] + nbytes < length[0])
			length[1]++;	/* overflow to msb of length */
		length[0] += nbytes;
	}
	/* finish: */
	offset = length[0] & 0x3C0;	/* extract bytes 6 to 10 inclusive */
	MDfinish(MDbuf, data + offset, length[0], length[1]);

	for (i = 0; i < RMDsize / 8; i += 4) {
		hashcode[i] = MDbuf[i >> 2];
		hashcode[i + 1] = (MDbuf[i >> 2] >> 8);
		hashcode[i + 2] = (MDbuf[i >> 2] >> 16);
		hashcode[i + 3] = (MDbuf[i >> 2] >> 24);
	}
	fclose(mf);

	return (byte *) hashcode;
}
byte *RMDbinary_buffer(char *buffer, int size_buffer)
{
	return NULL;
}

/*
 * Extracts the RMD 160 hash from the signature file
 */
int CH_get_rmd160_hash_from_signature(byte *hash, char *signature_file, char *public_key_file)
{
	RSA_parameters *rsa_parameters;
	DER_key *derkey;
	uint8_t *signature;
	size_t signature_size;
	uint8_t *decrypted;

	signature = CH_load_raw_file(signature_file, &signature_size);
	if ((signature == NULL)||(signature_size != 512)) {
		fprintf(stderr, "Error: Loading signature key '%s'\n", signature_file);
		exit(-1);
	}
	derkey = CH_load_der_key(public_key_file);
	if (derkey == NULL) {
		fprintf(stderr, "Error: opening DER key file '%s'\n", public_key_file);
		exit(-1);
	}
	if ((asn1_get_public_key(derkey->buffer, derkey->size, &rsa_parameters)) != 0) {
		fprintf(stderr, "Error: Extracting public key from DER file\n");
		exit(-1);
	}
	CH_free_der_key(derkey);
	if (rsa_parameters->num_octets != 512)
		fprintf(stderr, "Error: The RSA public key size is not 4096 bits %d\n", rsa_parameters->num_octets);
	decrypted = (uint8_t *)malloc(rsa_parameters->num_octets);
	if (CH_decrypt(rsa_parameters, signature, decrypted)) {
		fprintf(stderr, "Error: Decrypting signature\n");
		exit(-1);
	}
	memcpy(hash, decrypted + 492, 20);		
	free(decrypted);
	free(signature);
	return 0;
}

/*
 * Decrypts the signature buffer using the rsa public key loaded
 */
int CH_decrypt(RSA_parameters *rsa, uint8_t *buffer_in, uint8_t *buffer_out)
{
	bigint *dat_bi;
	bigint *decrypted_bi;
	int byte_size;

	byte_size = rsa->num_octets; 
	dat_bi = bi_import(rsa->bi_ctx, buffer_in, byte_size);
	rsa->bi_ctx->mod_offset = BIGINT_M_OFFSET;
	bi_copy(rsa->m);
	decrypted_bi = bi_mod_power(rsa->bi_ctx, dat_bi, rsa->e);
	bi_export(rsa->bi_ctx, decrypted_bi, buffer_out, byte_size);
	return 0;
}
/*
 * Loads a file in a uint8_t buffer
 */
static uint8_t *CH_load_raw_file(char *filename, size_t *size)
{
	struct stat st;
	int fd;
	ssize_t br;
	uint8_t *ret;

	if ((stat(filename, &st)) == -1)
		return NULL;
	if ((ret = (uint8_t *)malloc(st.st_size)) == NULL)
		return NULL;
	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		free(ret);
		return NULL;
	}
	br = read(fd, ret, st.st_size);
	close(fd);
	if (br != st.st_size) {
		free(ret);
		return NULL;
	}
	*size = st.st_size;
	return ret;
}
/*
 * Loads a .der file in a buffer
 */
static DER_key *CH_load_der_key(char *filename)
{
	DER_key *ret;

	if ((ret = (DER_key *)malloc(sizeof(DER_key))) == NULL)
		return NULL;
	if ((ret->buffer = CH_load_raw_file(filename, &(ret->size))) == NULL) {
		free(ret);
		return NULL;
	}
	return ret;
}
/*
 * CH_load_pem_key
 */
static DER_key *CH_load_pem_key(char *filename)
{
	DER_key *ret;
	uint8_t *buffer;
	char *b64,*p,*t;
	char key[1024];
	size_t filesize;
	int size;

	if ((ret = (DER_key *)malloc(sizeof(DER_key))) == NULL)
		return NULL;
	if ((buffer = CH_load_raw_file(filename, &filesize)) == NULL) {
		free(ret);
		return NULL;
	}
	p = (char *)buffer;
	while ((*p != '\n') && (*p != '\0'))
		p++;
	if (*p == '\0') {
		free(ret);
		return NULL;
	}
	p++;
	b64 = t = p;
	while((p - b64) <= filesize) {
		if ((*p == '-')) {
			break;
		} else if ((*p != '\n') && (*p != ' ') && (*p != '\t')) {
			*t = *p;
			t++;
		}
		p++;
	}
	*t = '\0';
	size = B64_decode(b64, key, strlen(b64), 1024);
	if (size < 0) {
		free(buffer);
		free(ret);
		return NULL;
	}
	free(buffer);
	ret->buffer = (char *)malloc(size);
	ret->size = size;
	memcpy((void *)ret->buffer, (void *)key, size);
	return ret;
}

/*
 * CH_free_der_key
 */
static void CH_free_der_key(DER_key *key)
{
	free(key->buffer);
	free(key);
}

/*
 * Get the public key specifics from an ASN.1 encoded file
 * A function lacking in the exported axTLS API
 *
 * This is a really weird hack that only works with RSA public key
 * files
 */
static int asn1_get_public_key(const uint8_t *buf, int len, RSA_parameters **rsa_parameters)
{
	uint8_t *modulus, *pub_exp;
	int mod_len, pub_len;

	pub_len = 3;
	mod_len = len - 37;
	if (buf[0] != 0x30) {
		return -1;
	}

	pub_exp = (uint8_t *)malloc(3);
	modulus = (uint8_t *)malloc(mod_len);
	memcpy(modulus, buf + 32 , mod_len);
	memcpy(pub_exp, buf + 34 + mod_len, 3); 
	if (mod_len <= 0 || pub_len <= 0 )
		return -1;
	CH_pub_key_new(rsa_parameters, modulus, mod_len, pub_exp, pub_len);

	free(modulus);
	free(pub_exp);
	return 0;
}

/*
 * Similar to RSA_pub_key_new, rewritten to make this program depend only on bi module
 */
void CH_pub_key_new(RSA_parameters **rsa, const uint8_t *modulus, int mod_len, const uint8_t *pub_exp, int pub_len)
{
	RSA_parameters *rsa_parameters;

	BI_CTX *bi_ctx = bi_initialize();
	*rsa = (RSA_parameters *)calloc(1, sizeof(RSA_parameters));
	rsa_parameters = *rsa;
	rsa_parameters->bi_ctx = bi_ctx;
	rsa_parameters->num_octets = (mod_len & 0xFFF0);
	rsa_parameters->m = bi_import(bi_ctx, modulus, mod_len);
	bi_set_mod(bi_ctx, rsa_parameters->m, BIGINT_M_OFFSET);
	rsa_parameters->e = bi_import(bi_ctx, pub_exp, pub_len);
	bi_permanent(rsa_parameters->e);
}

static unsigned char *load_file_in_buffer(char *path, int *size)
{
	unsigned char *buffer;
	struct stat st;
	int fd;

	if (stat(path, &st))
		return NULL;
	buffer = (unsigned char *)malloc(st.st_size);
	if (buffer == NULL)
		return NULL;
 	if ((fd = open(path, O_RDONLY)) == -1) {
		free(buffer);
		return NULL;
	}
	if (read(fd, (void *)buffer,st.st_size) != (ssize_t)st.st_size) {
		free(buffer);
		close(fd);
		return NULL;
	}
	*size = (int)st.st_size;
	close(fd);
	return buffer;
}

static int save_file_from_buffer(char *path, unsigned char *buffer, int size)
{
	int fd;

	if ((fd = open(path, O_WRONLY | O_CREAT, 0644)) == -1)
		return -1;
	if (write(fd, buffer, (size_t)size) != ((ssize_t)size)) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

/* FR_init */
void *FR_init(char *public_key_path)
{
	DER_key *derkey;
	RSA_parameters *rsa_parameters;
	char *ending;

	ending = public_key_path + strlen(public_key_path) - 3;
	if (!strcmp(ending, "der"))
		derkey = CH_load_der_key(public_key_path);	
	else if (!strcmp(ending, "pem"))
		derkey = CH_load_pem_key(public_key_path);
	else {
		fprintf(stderr, "Error: unknown key format\n");
		exit(-1);
	}
	if (derkey == NULL) {
		fprintf(stderr, "Error: opening key file '%s'\n", public_key_path);
		exit(-1);
	}
	if ((asn1_get_public_key(derkey->buffer, derkey->size, &rsa_parameters)) != 0) {
		fprintf(stderr, "Error: Extracting public key from file\n");
		exit(-1);
	}
	CH_free_der_key(derkey);
	return (void *)rsa_parameters;
}

/* FR_end */
FONRSA_ERROR FR_end(void *handle)
{
	RSA_parameters *rsa_parameters = (RSA_parameters *)handle;

	if (rsa_parameters->bi_ctx != NULL) {
		if (rsa_parameters->e != NULL)
			bi_free(rsa_parameters->bi_ctx, rsa_parameters->e);
		if (rsa_parameters->m != NULL)
			bi_free(rsa_parameters->bi_ctx, rsa_parameters->m);
		bi_terminate(rsa_parameters->bi_ctx);
	}
	free(rsa_parameters);
	return FONRSA_OK;
}

/* FR_verify_buffer */
FONRSA_ERROR FR_verify_buffer(void *handler, unsigned char *text, unsigned int size_text,
	unsigned char *signature, unsigned int size_signature)
{
	return FONRSA_NOSYS;
}

/* FR_decrypt_buffer */
FONRSA_ERROR FR_decrypt_buffer(void *handler, unsigned char *cryptext, int cryptext_size,
	unsigned char *plaintext, int plaintext_buffer_size, int *plaintext_size)
{
	RSA_parameters *rsaparms = (RSA_parameters *)handler;

	if (cryptext_size != rsaparms->num_octets) {
		return FONRSA_SIZE;
	}
	if (plaintext_buffer_size < cryptext_size) {
		return FONRSA_SIZE;
	}
	if (CH_decrypt(rsaparms, (uint8_t *)cryptext, (uint8_t *)plaintext)) {
		return FONRSA_DECRYPT;
	}
	*plaintext_size = cryptext_size;
	return FONRSA_OK;
}

/* Standalone FR_verify_file */
FONRSA_ERROR FR_verify_file(void *handler, char *file_path, char *signature_file_path)
{
	int j;
	byte *hashcode;
	byte hash[20];
	uint8_t *decrypted;
	RSA_parameters *rsa_parameters = (RSA_parameters *)handler;
	char *signature_buffer;
	int signature_size;

	/* Calculates the RIPEMD-160 hash of the file */
	hashcode = RMDbinary (file_path);
	/* Decrypts the signature file using the RSA public key */
	signature_buffer = load_file_in_buffer(signature_file_path, &signature_size);
	if (signature_buffer == NULL)
		return FONRSA_OPENKEY;

	if (rsa_parameters->num_octets != signature_size)
		return FONRSA_SIZE;
	decrypted = (uint8_t *)malloc(rsa_parameters->num_octets);
	if (CH_decrypt(rsa_parameters, signature_buffer, decrypted)) {
		fprintf(stderr, "Error: Decrypting signature\n");
		exit(-1);
	}
	memcpy(hash, decrypted + 492, 20);
	free(decrypted);
	free(signature_buffer);
	for (j = 0; j < RMDsize/8; j++) {
		if (hash[j] != hashcode[j])
			return FONRSA_VERIFICATION_FAILURE;
	}
	return FONRSA_OK;
}

/* FR_decrypt_file */
FONRSA_ERROR FR_decrypt_file(void *handle, char *crypted_file_path, char *plaintext_file_path)
{
	int size;
	FONRSA_ERROR ret;
	char *filebuffer;
	char crypted[1024];
	int crypted_size;

	if ((filebuffer = load_file_in_buffer(crypted_file_path, &size)) == NULL) {
		return FONRSA_LOADFILE;
	}

	ret = FR_decrypt_buffer(handle, filebuffer, size, crypted, 1024, &crypted_size);
	if (ret != FONRSA_OK) {
		free(filebuffer);
		return ret;
	}
	free(filebuffer);

	if (save_file_from_buffer(plaintext_file_path, crypted, crypted_size)) {
		printf("Error writing %lu bytes into %s", crypted_size, plaintext_file_path);
		return FONRSA_SAVEFILE;
	}
	return FONRSA_OK;
}

#ifdef __MAINTEST__
int main(int argc, char **argv)
{
	void *handle = NULL;
	FONRSA_ERROR ret;
	char *filebuffer = NULL;
	char crypted[1024];
	int size, crypted_size;

	if (argc != 4) {
		printf("Usage: %s <key_file> <crypted_file> <output_file>\n", argv[0]);
		return 1;
	}

	handle = FR_init(argv[1]);
	if (handle == NULL) {
		printf("Error loading keys\n");
		return 1;
	}
	ret = FR_decrypt_file(handle, argv[2], argv[3]);
	if (ret != FONRSA_OK) {
		printf("FR_decrypt_file returns %d\n", ret);
	}
	FR_end(handle);
	return (int)ret;
}
#endif

