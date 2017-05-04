#include "eXosip_tls.h"
#include "../osip/osip_message.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


BIO *bio_err=0;
static char *pass;
static void sigpipe_handle(int x){
	printf("+++++%s+++++\n",__FUNCTION__);
}

static int berr_exit(char *string){
	BIO_printf(bio_err,"%s\n",string);
	ERR_print_errors(bio_err);    
	exit(0);
}
#ifdef USER_CERT_REQUIRE_PASSWD
static int password_cb(char *buf, int num, int rwflag, void *userdata)
{
	if(num<strlen(pass)+1)
		return(0);
	
	strcpy(buf,pass);
	return(strlen(pass));
}
#endif

static int flash_cert_read(int certNum)
{	
	int fh;	
	FILE *fp;	
	int ok=1;	
	char buf[CERT_SIZE];	
	char *certFile;		
	
	fh = open(CERT_DEV_PATH, O_RDWR);	
	if ( fh == -1 )
		return 0;	
	switch(certNum)
	{		
		case 1:
			lseek(fh, 0L, SEEK_SET);
			certFile=CERT1_PATH;		
		break;
		case 2:
			lseek(fh, CERT_SIZE, SEEK_SET);
			certFile=CERT2_PATH;
		break;
		case 3:
			lseek(fh, (CERT_SIZE * 2), SEEK_SET);
			certFile=CERT3_PATH;
			break;		
		case 4:
			lseek(fh, (CERT_SIZE * 3), SEEK_SET);
			certFile=CERT4_PATH;
			break;
	}	
	read(fh, buf, CERT_SIZE);	
	close(fh);	
	fp=fopen(certFile, "w");	
	if(!fp)
		return 0;	
	fprintf(fp, buf);	
	fclose(fp);
	return 1;
}

SSL_CTX *initialize_client_ctx (int fxs, int proxy){
	SSL_METHOD *meth;
	SSL_CTX *ctx=NULL;
	char *certPath;
	int certNum;
	struct stat buf;
#ifdef USER_CERT_REQUIRE_PASSWD
	char *password="realtek";
#endif

	if(fxs == 0 && proxy == 0){
		certPath=CERT1_PATH;
		certNum=1;
	}else if(fxs == 0 && proxy == 1){
		certPath=CERT2_PATH;
		certNum=2;
	}else if(fxs == 1 && proxy == 0){
		certPath=CERT3_PATH;
		certNum=3;
	}else if(fxs == 1 && proxy == 1){
		certPath=CERT4_PATH;
		certNum=4;
	}
	/* if tmp file not found, generate it*/
	if(stat(certPath, &buf) == -1 && errno == ENOENT)
			flash_cert_read(certNum);

	printf("+++++%s+++++\n",__FUNCTION__);
	if(!bio_err){	
		/* Global system initialization*/
		SSL_library_init();		
		SSL_load_error_strings();
		/* An error write context */
		bio_err=BIO_new_fp(stderr,BIO_NOCLOSE);
	}		
	/* Set up a SIGPIPE handler */	
	signal(SIGPIPE,sigpipe_handle);		
	/* Create our context*/	
	meth=TLSv1_method();	
	ctx=SSL_CTX_new(meth);
	/*client certificate*/
#ifdef CLIENT_CERTIFICATE
	if(!(SSL_CTX_use_certificate_chain_file(ctx, "/var/web/realtek-cert.pem")))
		berr_exit("Can't read certificate file");
#ifdef USER_CERT_REQUIRE_PASSWD
	pass=password;
	SSL_CTX_set_default_passwd_cb(ctx, password_cb);
#endif
	if(!(SSL_CTX_use_PrivateKey_file(ctx, "/var/web/realtek-privkey.pem",SSL_FILETYPE_PEM)))
		berr_exit("Can't read key file");
#endif /*CLIENT_CERTIFICATE*/
	/* Load the CAs we trust*/	
	if(!(SSL_CTX_load_verify_locations(ctx,certPath,0)))
		berr_exit("Can't read CA list");
	#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
		SSL_CTX_set_verify_depth(ctx,1);
	#endif    	
	return ctx;
}

SSL *initialize_client_ssl(SSL_CTX *ctx, int socket){
	SSL *ssl=NULL;
	BIO *sbio;

	printf("+++++%s+++++\n",__FUNCTION__);
	/*check the ctx and socket*/
	if(ctx==NULL || socket ==-1)
		return NULL;
	
	/* Connect the SSL socket */	
	ssl=SSL_new(ctx);	
	sbio=BIO_new_socket(socket,BIO_NOCLOSE);	
	SSL_set_bio(ssl, sbio, sbio);
	
	return ssl;
}

int SSL_Handshake(SSL *ssl){
	printf("+++++%s+++++\n",__FUNCTION__);
	if(SSL_connect(ssl)<=0){
		berr_exit("SSL connect error");
		return 0;
	}

	return 1;
}

int tls_send_message(SSL *ssl, char *msg, int len){
	int ret;

	if(ssl == NULL)
		return 0;
	
	ret=SSL_write(ssl, msg, len);
	return ret;
}

int tls_read_message(SSL *ssl, char *msg, int len){
	int ret;

	if(ssl == NULL)
		return 0;
	
	ret=SSL_read(ssl, msg, len);
	return ret;
}

SSL_SESSION *tls_get_session(SSL *ssl){
	if(ssl == NULL)
		return NULL;
	
	return SSL_get1_session(ssl);
}

int tls_resuming_session(SSL *ssl, SSL_SESSION *sess){
	int ret=0;
	
	if(ssl == NULL)
		return 0;
	
	ret=SSL_set_session(ssl,sess);
	return ret;
}

void tls_free(tlsObject_t tlsObj){
	if(tlsObj.ssl_conn)
		SSL_shutdown(tlsObj.ssl_conn);
		//SSL_free(tlsObj.ssl_conn);

	if(tlsObj.ssl_ctx)
		SSL_CTX_free(tlsObj.ssl_ctx);

	if(bio_err!=NULL)
		BIO_free(bio_err);
}