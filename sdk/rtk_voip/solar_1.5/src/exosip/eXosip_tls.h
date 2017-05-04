#ifndef _EXOSIP_TLS_H_
#define _EXOSIP_TLS_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define CERT1_PATH	"/var/web/cacert_01.pem"
#define CERT2_PATH	"/var/web/cacert_02.pem"
#define CERT3_PATH	"/var/web/cacert_03.pem"
#define CERT4_PATH	"/var/web/cacert_04.pem"
#define CERT_SIZE		16384
#define CERT_DEV_PATH	"/dev/mtdblock2"


typedef struct tlsObject_s
{
	char *readBuffer;
	int readBufferLen;
	SSL *ssl_conn;
	SSL_CTX *ssl_ctx;
	BIO *sbio;
}tlsObject_t;

SSL_CTX *initialize_client_ctx (int fxs, int proxy);
SSL *initialize_client_ssl(SSL_CTX *ctx, int socket);
int SSL_Handshake(SSL *ssl);
int tls_send_message(SSL *ssl, char *msg, int len);
int tls_read_message(SSL *ssl, char *msg, int len);
SSL_SESSION *tls_get_session(SSL *ssl);
int tls_resuming_session(SSL *ssl, SSL_SESSION *sess);
void tls_free(tlsObject_t tlsObj);
#endif /*_EXOSIP_TLS_H_*/
