/*
 * FONSM protocol library
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070306 Pablo Martin Medrano <pablo@fon.com>
 *
 * $Id: protocol.c,v 1.1 2012/09/20 03:52:23 paula Exp $
 */
/*
 * Protocol 1
 *
 * REGULAR QUERY: 39 bytes
 *
 * | [MAC][CRC/TYPE] | [PrVER][TYPE][ARCH][VER][REV][ RANDOM GARB ] |
 * |   6      1      |  1       1     1     3    1    19 bytes      |
 * |     CLEAR       |                   CRYPTED                    |
 * |        7 bytes  |                   32 bytes                   |
 * | Converted to base32                                            |.sm.fon.com
 *
 * REGULAR ANSWER (do nothing)
 *
 * http://www.ece.unb.ca/tervo/ee4253/crc.htm
 *
 * REGULAR QUERY (39 data bytes), 44 bytes after encoding
 *
 * | [TYPE]  [CRC] [MAC ADDRESS] | [PrVER][TYPE][ARCH][VER][REV][STATUS][TRID][RETRIES][ RANDOM GARB] | 
 * |  5 b      3b       6B       |  1B      1B    1B   3B   1B    4B     4B       17B                 |
 * |        CLEAR                |                       CRYPTED  with AES256                         |.sm.fon .com 
 * |        7 BYTES              |                      32 BYTES                                      |  11 BYTES 
 * |       --------------------- CONVERTED TO BASE32 ---------------------------------------          |
 * |                                   63 bytes                                                       |  11 BYTES
 *  
 * NOTE: If the type goes above 26, the first character will be a digit, 
 * thus not a standard DNS domain name
 *
 * Further extensions have a limit of (63 chars).(63 chars).(63 chars).(53 chars).sm.fon.com
 * 
 * Thats 39+39+39+33 bytes = 150 bytes
 */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h> /* htonl, ntohl */
#include "fonsm/urandom.h"
#include "fonsm/protocol.h"
#include "fonsm/log.h"
#include "fonsm/aes.h"
#include "base/base16.h"
#include "base/base32.h"
#include "base/base64.h"
#include "base/base85.h"

#define MAX_SERIALIZE_BUFFER 4096

typedef enum {
	QR_MODE_ENCRYPT,
	QR_MODE_DECRYPT
} QR_ENUM_MODE;

typedef struct {
	UR_HANDLE ur;
	QR_key_retriever_function_t *key_retriever;
	void *key_retriever_data;

	QR_crypt_function_t *crypt;
	void *crypt_data;
	QR_decrypt_function_t *decrypt;
	void *decrypt_data;
} State;

typedef QR_ERROR (print_struct_function)(char *buffer, size_t size_buffer, void *structure);
typedef QR_ERROR (encode_struct_function)(State *s, char *buffer, size_t size_buffer, size_t *size_ret, void *structure);
typedef QR_ERROR (decode_struct_function)(State *s, void *structure, char *buffer, size_t size_buffer);

static print_struct_function *QR_get_print_struct_function(QR_STRUCT_TYPE type);
static encode_struct_function *QR_get_encode_struct_function(QR_STRUCT_TYPE type);
static decode_struct_function *QR_get_decode_struct_function(QR_STRUCT_TYPE type);

static QR_ERROR QR_print_regular_query(char *buffer, size_t size_buffer, void *query);
static QR_ERROR QR_print_regular_answer(char *buffer, size_t size_buffer, void *answer);
static QR_ERROR QR_print_nop_answer(char *buffer, size_t size_buffer, void *a);
static QR_ERROR QR_encode_regular_query(State *s, char *buffer, size_t size_buffer, size_t *size_ret, void *structure);
static QR_ERROR QR_encode_regular_answer(State *s, char *buffer, size_t size_buffer, size_t *size_ret, void *structure);
static QR_ERROR QR_encode_nop_anwser(State *s, char *buffer, size_t size_buffer, size_t *size_ret, void *structure);
static QR_ERROR QR_decode_regular_query(State *s, void *structure, char *buffer, size_t size_buffer);
static QR_ERROR QR_decode_regular_answer(State *s, void *structure, char *buffer, size_t size_buffer);
typedef int (QR_encoding_function_t)(char *, char *, int , int);
static QR_encoding_function_t *QR_get_encoding_function(QR_ENCODING_MODE mode, QR_STRUCT_TYPE type);

static char *QR_get_type_string(QR_ENUM_TYPE type);
static char *QR_get_arch_string(QR_ENUM_ARCH arch);
static QR_ERROR QR_crypt(State *s, QR_ENUM_MODE mode, char *buffer, size_t size, char *wlmac);

static QR_ERROR QR_encode_nop_answer(State *s, char *buffer, size_t size_buffer, size_t *size_ret, void *structure);
static QR_ERROR QR_decode_nop_answer(State *s, void *structure, char *buffer, size_t size_buffer);

void QR_crc3_init(void);
static inline unsigned char QR_crc3(unsigned char *buf, int size);

static unsigned char global_crc_table3[256];
static int global_crc_table3_init = 0;

#define DECLARE_FUNCTIONS_ARRAY(prefix) \
static prefix ## _struct_function *QR_get_ ## prefix ## _struct_function(QR_STRUCT_TYPE type) \
{ \
	struct _print_functions_struct { \
		QR_STRUCT_TYPE type; \
		prefix ## _struct_function *function; \
	} *p, functions[] = { \
		{ QR_REGULAR_QUERY, QR_ ## prefix ## _regular_query}, \
		{ QR_REGULAR_ANSWER, QR_ ## prefix ## _regular_answer}, \
		{ QR_REGULAR_ANSWER_NOP, QR_ ## prefix ## _nop_answer}, \
		{ 0, NULL} \
	}; \
	for (p = functions; p->function != NULL; p++) { \
		if (p->type == type) \
			return p->function; \
	} \
	return NULL; \
}

DECLARE_FUNCTIONS_ARRAY(print)
DECLARE_FUNCTIONS_ARRAY(encode)
DECLARE_FUNCTIONS_ARRAY(decode)

void *QR_start(QR_Init *init)
{
	State *s;

	if (aes_self_test()) {
		fon_critical("%s: aes_self_test() returns != 0!!!!", __FUNCTION__);
		return NULL;
	}

	s = (State *)malloc(sizeof(State));
	memset((void *)s, 0, sizeof(State));
	if((s->ur = UR_start()) == NULL) {
		fon_critical("%s: Error calling UR_start()", __FUNCTION__);
		free(s);
		return NULL;
	}
	s->key_retriever = init->key_retriever;
	s->key_retriever_data = init->key_retriever_data;
	s->crypt = init->crypt;
	s->crypt_data = init->crypt_data;
	s->decrypt = init->decrypt;
	s->decrypt_data = init->decrypt_data;
	return (void *)s;
}

void QR_end(void *handle)
{
	State *s = (State *)handle;
	UR_stop(s->ur);
	free(s);
}

/* QR_print_struct */
QR_ERROR QR_print_struct(char *buffer, size_t size_buffer, void *structure, QR_STRUCT_TYPE type)
{
	print_struct_function *printfunction;

	if ((printfunction = QR_get_print_struct_function(type)) == NULL)
		return QR_ASSERT;
	return printfunction(buffer, size_buffer, structure);
}

static QR_encoding_function_t *QR_get_encoding_function(QR_ENCODING_MODE mode, QR_STRUCT_TYPE type)
{
	struct {
		QR_encoding_function_t *efunction;
		QR_encoding_function_t *dfunction;
		QR_STRUCT_TYPE type;
	} *p, a[] = {
		{ B32_encode, B32_decode, QR_REGULAR_QUERY },
		{ B64_encode, B64_decode, QR_REGULAR_ANSWER },
		{ B64_encode, B64_decode, QR_REGULAR_ANSWER_NOP },
		{ NULL, 0 }
	};
	for (p = a; p->efunction != NULL; p++)
		if (p->type == type)
			return (mode == QR_MODE_DECRYPT)? p->dfunction: p->efunction;

	fon_critical("%s: Unknown encoding function for type %d", __FUNCTION__, type);
	return NULL;
}

/* QR_encode_struct 
 * Fills the buffer with the contents of structure, storing
 * size_structure bytes. If it doesn't fit, returns QR_OVERFLOW
 */
QR_ERROR QR_encode_struct(void *handle, QR_STRUCT_TYPE type, char *buffer, size_t size_buffer, size_t *size_structure, void *structure)
{
	char encoded_buffer[512]; 	/* max size of a DNS packet anyway */
	size_t encoded_structure_size;
	QR_ERROR qr_error;
	QR_encoding_function_t *encodebuffer;
	encode_struct_function *encodefunction;
	State *s = (State *)handle;

	if ((encodefunction = QR_get_encode_struct_function(type)) == NULL)
		return QR_ASSERT;
	if ((qr_error = encodefunction(s, encoded_buffer, 512, &encoded_structure_size, structure)) != QR_OK) {
		fon_critical("%s: encodefunction() returns %d", __FUNCTION__, qr_error);
		return qr_error;
	}
	if((encodebuffer = QR_get_encoding_function(QR_MODE_ENCODE, type)) == NULL) {
		fon_warning("%s: Error retrieving encoding function for type %d", __FUNCTION__, type);
		return QR_ASSERT;
	}
	/*
	QR_REGULAR_QUERY = 1,
	QR_REGULAR_ANSWER = 2,
	QR_REGULAR_QUERY_FIRST = 3,
	QR_REGULAR_ANSWER_NOP = 4
	*/
	/* FIXME: The following is a chapu */
	if ((type == QR_REGULAR_ANSWER) || (type == QR_REGULAR_ANSWER_NOP)) {
		buffer[0] = (type == QR_REGULAR_ANSWER_NOP)?'a':'b';
		buffer[1] = '=';
		if (((*size_structure) = encodebuffer(encoded_buffer, buffer+2, encoded_structure_size, size_buffer)) < 0)
			return QR_OVERFLOW;
		(*size_structure) += 2;
	} else {
		if (((*size_structure) = encodebuffer(encoded_buffer, buffer, encoded_structure_size, size_buffer)) < 0)
			return QR_OVERFLOW;
	}
	return QR_OK;
}

/* QR_get_query_type */
QR_STRUCT_TYPE QR_get_query_type(char *buffer, size_t size_buffer)
{
	return QR_TYPE_ERROR;
}

/* QR_decode_struct */
QR_ERROR QR_decode_struct(void *handle, QR_STRUCT_TYPE type, void *structure, char *buffer, size_t size_buffer)
{
	char decoded_buffer[512];
	int decoded_buffer_size;
	QR_ERROR qr_error;
	decode_struct_function *decodefunction;
	QR_encoding_function_t *decodebuffer;
	State *s = (State *)handle;

	if ((decodefunction = QR_get_decode_struct_function(type)) == NULL)
		return QR_ASSERT;
	if((decodebuffer = QR_get_encoding_function(QR_MODE_DECODE, type)) == NULL) {
		fon_warning("%s: Error retrieving encoding function for type %d", __FUNCTION__, type);
		return QR_ASSERT;
	}
	if ((type == QR_REGULAR_ANSWER) || (type == QR_REGULAR_ANSWER_NOP)) {
		if ((decoded_buffer_size = decodebuffer(buffer + 2, decoded_buffer, size_buffer - 2, 512)) < 0) {
			fon_warning("%s: Error calling decodebuffer()", __FUNCTION__);
			return QR_OVERFLOW;
		}
	} else {
		if ((decoded_buffer_size = decodebuffer(buffer, decoded_buffer, size_buffer, 512)) < 0) {
			fon_warning("%s: Error calling decodebuffer()", __FUNCTION__);
			return QR_OVERFLOW;
		}
	}
	if ((qr_error = decodefunction(s, structure, decoded_buffer, decoded_buffer_size)) != QR_OK) {
		fon_warning("%s: Error calling decodefunction()", __FUNCTION__);
		return qr_error;
	}

	return QR_OK;
}

static QR_ERROR QR_print_regular_query(char *buffer, size_t size_buffer, void *q)
{
	QR_regular_query *query = (QR_regular_query *)q;

	snprintf(buffer, size_buffer, "%s,%s,%s,%d.%d.%d.%d,st=%x,trid=%x,retries=%u",
		query->mac, QR_get_type_string(query->type),
		QR_get_arch_string(query->arch), query->major_version,
		query->minor_version, query->micro_version,
		query->revision, query->status, query->trid, query->retries);
	return QR_OK;
}

static QR_ERROR QR_print_regular_answer(char *buffer, size_t size_buffer, void *a)
{
	QR_regular_answer *answer = (QR_regular_answer *)a;

	sprintf(buffer, "exec=%s, trid=[%8.8x], data=[%s]", QR_get_exec_string(answer->exec), answer->trid, answer->data);
	return QR_OK;
}

static QR_ERROR QR_print_nop_answer(char *buffer, size_t size_buffer, void *a)
{
	QR_regular_answer *answer = (QR_regular_answer *)a;

	sprintf(buffer, "exec=%s, trid=[%8.8x] ts=[%lu]", QR_get_exec_string(answer->exec), answer->trid, answer->ts);
	return QR_OK;
}

/* Initialize a table given a 256 bytes table and the polynom to use */
void QR_crc3_init(void) {
	unsigned char crc;
	int i,j;
 
	for (i=0; i<256; i++) {
		crc = i;
		for (j=0; j<8; j++) {
			if (crc & 1)
				crc = (crc>>1) ^ (unsigned char)3;
			else
				crc = (crc>>1);
		}
		global_crc_table3[i] = crc;
	}
}

static inline unsigned char QR_crc3(unsigned char *buf, int size) {
	int i;
	unsigned char crc = 0x7;

	if (!global_crc_table3_init) {
		QR_crc3_init();
	}
	for (i=0; i<size; i++) {
		crc = global_crc_table3[buf[i]^(crc&7)];
	}
	return crc | ((unsigned char)0x07);
}

static QR_ERROR QR_encode_regular_query(State *s, char *buffer, size_t size_buffer, size_t *size_ret, void *structure)
{
	QR_ERROR qr_error;
	QR_regular_query *query = (QR_regular_query *)structure;

	if (size_buffer < REGULAR_QUERY_SIZE)
		return QR_OVERFLOW;
	*size_ret = REGULAR_QUERY_SIZE;
	buffer[0] = 0xf8 & (((unsigned char)query->type)<<3);
	if (QR_convert_mac_string_to_six_bytes((unsigned char *)buffer+1, query->mac))
		return QR_PARSE;
	buffer[0] |= QR_crc3((unsigned char *)buffer + 1, 6);
	buffer[7] = (unsigned char)query->protocol_version;
	buffer[8] = (unsigned char)query->type;
	buffer[9] = (unsigned char)query->arch;
	buffer[10] = (unsigned char)query->major_version;
	buffer[11] = (unsigned char)query->minor_version;
	buffer[12] = (unsigned char)query->micro_version;
	buffer[13] = (unsigned char)query->revision;
	*((uint32_t *)(buffer + 14)) = htonl(query->status);
	query->trid = rand();
	*((uint32_t *)(buffer + 18)) = htonl(query->trid);
	*((uint32_t *)(buffer + 22)) = htonl(query->retries);
	if (UR_get(s->ur, buffer + 26, 13) != UR_OK) {
		fon_critical("%s: Error calling UR_get()", __FUNCTION__);
		return QR_UNKNOWN;
	}
	qr_error = QR_crypt(s, QR_MODE_ENCRYPT, buffer+7, 32, query->mac);
	if (qr_error != QR_OK)
		return qr_error;
	return QR_OK;
}

static QR_ERROR QR_crypt(State *s, QR_ENUM_MODE mode, char *buffer, size_t size, char *wlmac)
{
	unsigned char key[256];
	size_t size_key;
	unsigned char text[16];
	int i;
	aes_context ctx;

	if (s->key_retriever == NULL) {
		return QR_ASSERT;
	}
	if (s->key_retriever(s->key_retriever_data, wlmac, key, 256, &size_key)) {
		return QR_KEYCRYPT;
	}
	if (size % 16)
		return QR_OVERFLOW;
#ifdef FONTEST
	{
		char hex[65];
		int r;
		r = B16_encode(key, hex, 32, 64);
		hex[64] = '\0';
		fon_debug("key [%s]", hex);
	}
#endif
	aes_set_key(&ctx, key, size_key*8);
	/* CBC encryption leaks information about the plaintext... blah blah */
	for (i = 0; i < size; i+=16) {
		memcpy((void *)text, (void *)buffer + i, (size_t)16);
		if (mode == QR_MODE_ENCRYPT) {
			aes_encrypt(&ctx, text, (unsigned char *)(buffer + i));
		} else if (mode == QR_MODE_DECRYPT) {
			aes_decrypt(&ctx, text, (unsigned char *)(buffer + i));
		} else
			return QR_ASSERT;
	}
	return QR_OK;
}

static QR_ERROR QR_decode_regular_query(State *s, void *structure, char *buffer, size_t size_buffer)
{
	QR_ERROR qr_error;
	QR_ENUM_TYPE type;
	QR_regular_query *query = (QR_regular_query *)structure;

	if (size_buffer != REGULAR_QUERY_SIZE) {
		fon_debug("%s: size_buffer (%lu) != REGULAR_QUERY_SIZE, aborting", __FUNCTION__, size_buffer);
		return QR_PARSE;
	}
	type = (buffer[0] & 0x0f8)>>3;
	if ((qr_error = QR_convert_six_bytes_to_mac_string(query->mac, (unsigned char *)(buffer+1))) != QR_OK) {
		fon_debug("%s: Error converting bytest to mac string", __FUNCTION__);
		return qr_error;
	}
	qr_error = QR_crypt(s, QR_MODE_DECRYPT, buffer+7, 32, query->mac);
	if (qr_error != QR_OK) {
		fon_critical("%s: Error decrypting query", __FUNCTION__);
		return qr_error;
	}
	query->protocol_version = (unsigned int)buffer[7];
	if (query->protocol_version != 0) {
		fon_critical("%s: Unknown protocol version %d", __FUNCTION__, query->protocol_version);
		return QR_PARSE;
	}
	query->type = (QR_ENUM_TYPE)buffer[8];
	if (query->type != type) {
		fon_critical("%s: Corrupt query", __FUNCTION__);
		return QR_PARSE;
	}
	query->arch = (QR_ENUM_ARCH)buffer[9];
	query->major_version = (unsigned int)buffer[10];
	query->minor_version = (unsigned int)buffer[11];
	query->micro_version = (unsigned int)buffer[12];
	query->revision = (unsigned int)buffer[13];
	query->status = ntohl((uint32_t)*((uint32_t *)(buffer + 14)));
	query->trid = ntohl((uint32_t)*((uint32_t *)(buffer + 18)));
	query->retries = ntohl((uint32_t)*((uint32_t *)(buffer + 22)));
	return QR_OK;
}

static QR_ERROR QR_encode_regular_answer(State *s, char *base_buffer, size_t size_buffer,
	size_t *size_ret, void *structure)
{
	QR_ERROR qr_error;
	unsigned char buffer[REGULAR_ANSWER_SIZE];
	int crypted_size;
	QR_regular_answer *answer = (QR_regular_answer *)structure;

	if (size_buffer < REGULAR_ANSWER_SIZE)
		return QR_OVERFLOW;
	*size_ret = REGULAR_ANSWER_SIZE;
	buffer[0] = QR_REGULAR_ANSWER;
	*((uint32_t *)(buffer + 1)) = htonl(answer->trid);
	*((uint32_t *)(buffer + 5)) = htonl(answer->size_data);
	buffer[9] = (unsigned char)answer->exec;
	if (answer->size_data > (REGULAR_ANSWER_SIZE - 10)) {
		fon_warning("%s: Data string too big", __FUNCTION__);
		return QR_PARSE;
	}
	memcpy((void *)buffer+10, answer->data, answer->size_data);
	if (UR_get(s->ur, buffer + answer->size_data + 10, (*size_ret) - answer->size_data - 10) != UR_OK) {
		fon_critical("%s: Error calling UR_get()", __FUNCTION__);
		return QR_UNKNOWN;
	}
	qr_error = QR_crypt(s, QR_MODE_ENCRYPT, buffer, (unsigned int)REGULAR_ANSWER_SIZE, answer->mac);
	if (qr_error != QR_OK) {
		fon_critical("%s: Error calling QR_crypt", __FUNCTION__);
		return qr_error;
	}
	fon_debug("About to call s->crypt buffer[0] = 0x%2.2x, buffer[1] = 0x%2.2x", buffer[0], buffer[1]);
	if (s->crypt(s->crypt_data, buffer, REGULAR_ANSWER_SIZE, base_buffer, size_buffer, &crypted_size)) {
		fon_critical("%s: Error calling s->crypt", __FUNCTION__);
		fon_critical("%s: Returning caller %d", __FUNCTION__, QR_SIGN);
		return QR_SIGN;
	}
	if (crypted_size != REGULAR_ANSWER_SIZE)
		return QR_OVERFLOW;
	return QR_OK;
}

static QR_ERROR QR_decode_regular_answer(State *s, void *structure, char *base_buffer,
	size_t size_buffer)
{
	QR_regular_answer *answer = (QR_regular_answer *)structure;
	QR_ERROR qr_error;
	char *buffer;
	char decrypted_buffer[REGULAR_ANSWER_SIZE];
	int decrypted_buffer_size;

	if (size_buffer != REGULAR_ANSWER_SIZE) {
		fon_warning("%s: size_buffer = %lu != %lu", __FUNCTION__, size_buffer, REGULAR_ANSWER_SIZE); 
		return QR_PARSE;
	}
	if (s->decrypt(s->decrypt_data, base_buffer, size_buffer, decrypted_buffer, REGULAR_ANSWER_SIZE, &decrypted_buffer_size)) {
		fon_warning("%s: Error decrypting buffer", __FUNCTION__);
		return QR_PARSE;
	}
	if (decrypted_buffer_size != size_buffer)
		return QR_PARSE;
	buffer = decrypted_buffer;
	qr_error = QR_crypt(s, QR_MODE_DECRYPT, buffer, REGULAR_ANSWER_SIZE, NULL);
	if (buffer[0] != QR_REGULAR_ANSWER) {
		fon_warning("%s: Invalid type", __FUNCTION__);
		return QR_CRC;
	}
	if (qr_error != QR_OK) {
		fon_critical("%s: Error calling QR_decrypt", __FUNCTION__);
		return qr_error;
	}
	answer->trid = ntohl((uint32_t)*((uint32_t *)(buffer + 1)));
	answer->size_data = ntohl((uint32_t)*((uint32_t *)(buffer + 5)));
	answer->exec = (QR_ENUM_EXEC)buffer[9];
	if (answer->size_data > (REGULAR_ANSWER_SIZE - 10)) {
		fon_warning("%s: Data size too big %lu > %lu", __FUNCTION__, answer->size_data, (REGULAR_ANSWER_SIZE - 10));
		return QR_LIMITS;
	}
	memcpy(answer->data, buffer + 10, answer->size_data);
	return QR_OK;
}

char *QR_get_exec_string(QR_ENUM_EXEC exec)
{
	int i;
	struct {
		QR_ENUM_EXEC exec;
		char *string;
	} type_to_string[] = {
		{ QR_EXEC_NOTHING, "nothing" },
		{ QR_EXEC_CMC, "CM client" },
		{ QR_EXEC_UMC, "UM client" },
		{ 0, NULL }
	};
	for (i = 0; type_to_string[i].string != NULL; i++) {
		if (type_to_string[i].exec == exec)
			return type_to_string[i].string;
	}
	return "error";
}
/* QR_get_type_string */
static char *QR_get_type_string(QR_ENUM_TYPE type)
{
	int i;
	struct {
		QR_ENUM_TYPE type;
		char *string;
	} type_to_string[] = {
		{ QR_TYPE_REGULAR_START, "regular_start" },
		{ QR_TYPE_REGULAR_RETRY, "regular_retry" },
		{ QR_TYPE_REGULAR_ONLINE, "regular_online" },
		{ QR_TYPE_DATA, "data" },
		{ 0, NULL }
	};

	for (i = 0; type_to_string[i].string != NULL; i++) {
		if (type_to_string[i].type == type)
			return type_to_string[i].string; 
	}
	return "error";
}

/* QR_get_arch_string */
static char *QR_get_arch_string(QR_ENUM_ARCH arch)
{
	int i;
	struct {
		QR_ENUM_ARCH arch;
		char *string;
	} arch_to_string[] = {
		{ QR_ARCH_FONERA, "fonera" },
		{ QR_ARCH_LINKSYS, "linksys" },
		{ 0, NULL }
	};
	for (i = 0; arch_to_string[i].string != NULL; i++) {
		if (arch_to_string[i].arch == arch)
			return arch_to_string[i].string;
	}
	return "error";
}

/*
 * QR_fill_regular_query:
 *
 * Generates a regular query parsing tokens as strings. The query is
 * allocated before calling this function
 */
QR_ERROR QR_fill_regular_query(QR_regular_query *query, char *mac_address,
	char *arch, char *version, char revision, unsigned char status)
{
	char *p;

	p = mac_address;
	if (strlen(mac_address) != 17) {
		fon_warning("%s: mac_address too long", __FUNCTION__);
		return QR_PARSE;
	}

	return QR_OK;
}

/* QR_convert_mac_string_to_six_bytes */
QR_ERROR QR_convert_mac_string_to_six_bytes(unsigned char *array, char *mac_address)
{
        int nib0, nib1, byte;
        char *p = mac_address;
        int i = 0;

        while(1) {
                nib0 = B16_ascii_to_nibble(*p);
                nib1 = B16_ascii_to_nibble(*(p + 1));
                if ((nib0 == -1) || (nib1 == -1)) {
			fon_warning("%s: Error decoding mac address. Invalid caracters", __FUNCTION__);
                        return QR_PARSE;
		}
                byte = (nib0*16) + nib1;
                if ((byte < 0) || (byte > 255)) {
			fon_warning("%s: Error decoding mac address. byte", __FUNCTION__);
                        return QR_PARSE;
		}
                array[i] = (unsigned char)byte;

		if (*(p+2) == '\0')
			break;
		p+=3;
                if (i == 5) {
			fon_warning("%s: Error decoding mac address", __FUNCTION__);
                        return QR_PARSE;
		}
                i++;
        }
        return QR_OK;
}

/* QR_convert_mac_string_to_six_bytes */
QR_ERROR QR_convert_six_bytes_to_mac_string(char *mac_address, unsigned char *array)
{
	char buffer[2];
	int i;

	mac_address[0] = '\0';
	for (i = 0; i < 6; i++) {
		B16_byte_to_ascii(&(buffer[0]), &(buffer[1]), array[i]);
		strncat(mac_address, buffer, 2);
		if (i != 5)
			strncat(mac_address, "-", 1);	
	}
        return QR_OK;
}
/*
 * QR_encode_nop_answer
 */
static QR_ERROR QR_encode_nop_answer(State *s, char *buffer, size_t size_buffer, size_t *size_ret, void *structure)
{
	QR_ERROR qr_error;
	int i;
	QR_regular_answer *answer = (QR_regular_answer *)structure;

	if (size_buffer < REGULAR_NOP_SIZE)
		return QR_OVERFLOW;
	*size_ret = REGULAR_NOP_SIZE;
	buffer[0] = QR_REGULAR_ANSWER_NOP;
	*((uint32_t *)(buffer + 1)) = htonl(answer->trid);
	*((uint32_t *)(buffer + 5)) = htonl(answer->ts);
	memset(buffer + 9, 0, 7); 
	qr_error = QR_crypt(s, QR_MODE_ENCRYPT, buffer, (unsigned int)REGULAR_NOP_SIZE, answer->mac);
	if (qr_error != QR_OK) {
		fon_critical("%s: Error calling QR_crypt", __FUNCTION__);
		return qr_error;
	}
	return QR_OK;
}
/*
 * QR_decode_nop_answer
 */
static QR_ERROR QR_decode_nop_answer(State *s, void *structure, char *buffer, size_t size_buffer)
{
	QR_regular_answer *answer = (QR_regular_answer *)structure;
	QR_ERROR qr_error;
	char b[REGULAR_NOP_SIZE];

	if (size_buffer != REGULAR_NOP_SIZE) {
		fon_warning("%s: size_buffer = %lu != %lu", __FUNCTION__,
			size_buffer, REGULAR_NOP_SIZE);
		return QR_PARSE;
	}
	memcpy(b, buffer, REGULAR_NOP_SIZE);
	qr_error = QR_crypt(s, QR_MODE_DECRYPT, buffer, REGULAR_NOP_SIZE, answer->mac);
	if (buffer[0] != QR_REGULAR_ANSWER_NOP) {
		fon_warning("%s: Invalid type", __FUNCTION__);
		return QR_CRC;
	}
	answer->trid = ntohl((uint32_t)*((uint32_t *)(buffer + 1)));
	answer->ts = ntohl((uint32_t)*((uint32_t *)(buffer + 5)));
	answer->exec = QR_EXEC_NOTHING;
	answer->size_data = 0;
	answer->data[0] = '\0';
	return QR_OK;
}

