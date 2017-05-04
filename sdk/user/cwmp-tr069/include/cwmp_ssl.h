#ifndef _CWMP_SSL_H_
#define _CWMP_SSL_H_
#ifdef CWMP_ENABLE_SSL

#include "soapH.h"

#include <config/autoconf.h>

#ifdef WITH_OPENSSL
int certificate_verify_cb(int ok, X509_STORE_CTX *store);
int CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();
#elif defined(_WITH_MATRIXSSL_)
#ifdef CONFIG_USER_CWMP_WITH_MATRIXSSL_NEW
int certificate_verify_cb(void *ssl, psX509Cert_t *certInfo, int32 alert);
#else
int certificate_verify_cb(sslCertInfo_t *cert, void *arg);
#endif
#endif

int certificate_setup( struct soap *soap, int use_cert );

#endif /*CWMP_ENABLE_SSL*/
#endif /*_CWMP_SSL_H_*/

