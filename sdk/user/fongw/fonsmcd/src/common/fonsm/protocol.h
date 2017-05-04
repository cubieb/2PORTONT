/*
 * FONSM protocol library
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * Created: 20070306 Pablo Martin Medrano <pablo@fon.com>
 *
 * $Id: protocol.h,v 1.1 2012/09/20 03:52:23 paula Exp $
 */
/* Protocol version 0 */
#ifndef _QUERY_H
#define _QUERY_H

#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif/* __cplusplus__ */

#define MAC_ADDRESS_SIZE 18 	/* 17 bytes */
#define MAX_SERIALIZE_BUFFER 4096
#define REGULAR_QUERY_SIZE 39
	/* 39 bytes, that will be converted 
	   into 63 + 1 + 2 + 1 + 3 + 1 + 3 (74)
	 */
#define REGULAR_ANSWER_SIZE 256
#define REGULAR_NOP_SIZE 16
/* 00-00-00-00-00-00 */

typedef enum {
	QR_ARCH_LINKSYS = 0,
	QR_ARCH_FONERA = 1,
	QR_ARCH_NETGEAR = 2,
	QR_ARCH_FONGW = 3 // PANTERA
} QR_ENUM_ARCH;

typedef enum {
	QR_TYPE_REGULAR_START = 0,
	QR_TYPE_REGULAR_RETRY = 1,
	QR_TYPE_REGULAR_ONLINE = 2,
	QR_TYPE_DATA = 3
} QR_ENUM_TYPE;

typedef enum {
	QR_MODE_ENCODE = 0,
	QR_MODE_DECODE
} QR_ENCODING_MODE;

typedef struct {
	time_t timestamp;
	char mac[MAC_ADDRESS_SIZE];
	unsigned int protocol_version; 
	QR_ENUM_TYPE type;
	QR_ENUM_ARCH arch;
	unsigned int major_version;
	unsigned int minor_version;
	unsigned int micro_version;
	unsigned int revision;
	unsigned int status;
	unsigned int trid; 	/* random trid */
	unsigned int retries; 	/* Times retried this query */
} QR_regular_query;

typedef enum {
	QR_EXEC_NOTHING = 0,
	QR_EXEC_CMC = 1,
	QR_EXEC_UMC = 2,
} QR_ENUM_EXEC;

typedef struct {
	time_t ts; 	/* Server timestamp upon building the answer */
	unsigned int trid; 	/* the trid of the associated query */
	QR_ENUM_EXEC exec;
	size_t size_data;
	char data[REGULAR_ANSWER_SIZE];
	char mac[MAC_ADDRESS_SIZE];
} QR_regular_answer;

typedef struct {
	time_t timestamp;
	unsigned int trid;
} QR_fhtagn_answer;
/* Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn */

typedef enum {
	QR_OK = 0,
	QR_PARSE = 1,
	QR_UNKNOWN = 2,
	QR_LIMITS = 3,
	QR_NOMEM = 4,
	QR_CRC = 5,
	QR_DECODE = 6,
	QR_ENCODE = 7,
	QR_CRYPT = 8,
	QR_DECRYPT = 9,
	QR_OVERFLOW = 10, 	/* The data does not fit in the buffer */
	QR_ASSERT = 11,
	QR_KEYCRYPT = 12,
	QR_SIGN = 13
} QR_ERROR;

typedef enum {
	QR_TYPE_ERROR = 0,
	QR_REGULAR_QUERY = 1,
	QR_REGULAR_ANSWER = 2,
	QR_REGULAR_QUERY_FIRST = 3,
	QR_REGULAR_ANSWER_NOP = 4
} QR_STRUCT_TYPE;


typedef int (QR_key_retriever_function_t)(void *data, char *wlmac, unsigned char *key, size_t size_buffer, size_t *size);
typedef int (QR_verify_function_t)(void *data, void *text, size_t size_text, void *signature, size_t size_signature);
typedef int (QR_sign_function_t)(void *data, unsigned char *text, int size_text, unsigned char *crypted_text, unsigned int crypted_text_buffer_size, int *crypted_size);

typedef int (QR_crypt_function_t)(void *, unsigned char *, int , unsigned char *, unsigned int, int *);
typedef int (QR_decrypt_function_t)(void *, unsigned char *, int, unsigned char *, int, int *);

typedef struct {
	QR_key_retriever_function_t *key_retriever;
	void *key_retriever_data;
	QR_crypt_function_t *crypt;
	void *crypt_data;
	QR_decrypt_function_t *decrypt;
	void *decrypt_data;
} QR_Init;

void *QR_start(QR_Init *init);
void QR_end(void *handle);
QR_ERROR QR_print_struct(char *buffer, size_t size_buffer, void *structure, QR_STRUCT_TYPE type);
QR_STRUCT_TYPE QR_get_query_type(char *buffer, size_t size_buffer); 
QR_ERROR QR_encode_struct(void *handle, QR_STRUCT_TYPE type, char *buffer, size_t size_buffer, size_t *size_structure, void *structure);
QR_ERROR QR_decode_struct(void *handle, QR_STRUCT_TYPE type, void *structure, char *buffer, size_t size_buffer);
char *QR_get_exec_string(QR_ENUM_EXEC exec);
QR_ERROR QR_convert_mac_string_to_six_bytes(unsigned char *array, char *mac_address);
QR_ERROR QR_convert_six_bytes_to_mac_string(char *mac_address, unsigned char *array);

#ifdef __cplusplus
}
#endif/* __cplusplus__ */

#endif

