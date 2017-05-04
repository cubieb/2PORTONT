/*
 *	socketLayer.c
 *	Release $Name:  $
 *
 *	Sample SSL socket layer for MatrixSSL example exectuables
 */
/*
 *	Copyright (c) PeerSec Networks, 2002-2007. All Rights Reserved.
 *	The latest version of this code is available at http://www.matrixssl.org
 *
 *	This software is open source; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This General Public License does NOT permit incorporating this software 
 *	into proprietary programs.  If you are unable to comply with the GPL, a 
 *	commercial license for this software may be purchased from PeerSec Networks
 *	at http://www.peersec.com
 *	
 *	This program is distributed in WITHOUT ANY WARRANTY; without even the 
 *	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *	See the GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *	http://www.gnu.org/copyleft/gpl.html
 */
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "sslSocket.h"

/******************************************************************************/
/*
	An EXAMPLE socket layer API for the MatrixSSL library.  
*/

/******************************************************************************/
/*
	Server side.  Set up a listen socket.  This code is not specific to SSL.
*/
SOCKET socketListen(short port, int *err)
{
	struct sockaddr_in	addr;
	SOCKET				fd;
	int					rc;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Error creating listen socket\n");
		*err = getSocketError();
		return INVALID_SOCKET;
	}
/*
	Make sure the socket is not inherited by exec'd processes
	Set the REUSE flag to minimize the number of sockets in TIME_WAIT
*/
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	rc = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		fprintf(stderr, 
			"Can't bind socket. Port in use or insufficient privilege\n");
		*err = getSocketError();
		return INVALID_SOCKET;
	}
	if (listen(fd, SOMAXCONN) < 0) {
		fprintf(stderr, "Error listening on socket\n");
		*err = getSocketError();
		return INVALID_SOCKET;
	}
	return fd;
}

/******************************************************************************/
/*
	Server side.  Accept a new socket connection off our listen socket.  
	This code is not specific to SSL.
*/
SOCKET socketAccept(SOCKET listenfd, int *err)
{
	struct sockaddr_in	addr;
	SOCKET				fd;
	int					len;
/*
	Wait(blocking)/poll(non-blocking) for an incoming connection
*/
	len = sizeof(addr);
	if ((fd = accept(listenfd, (struct sockaddr *)&addr, &len)) 
			== INVALID_SOCKET) {
		*err = getSocketError();
		if (*err != WOULD_BLOCK) {
			fprintf(stderr, "Error %d accepting new socket\n", *err);
		}
		return INVALID_SOCKET;
	}
/*
	fd is the newly accepted socket. Disable Nagle on this socket.
	Set blocking mode as default
*/
/*	fprintf(stdout, "Connection received from %d.%d.%d.%d\n", 
		addr.sin_addr.S_un.S_un_b.s_b1,
		addr.sin_addr.S_un.S_un_b.s_b2,
		addr.sin_addr.S_un.S_un_b.s_b3,
		addr.sin_addr.S_un.S_un_b.s_b4);
*/
	setSocketNodelay(fd);
	setSocketBlock(fd);
	return fd;
}

/******************************************************************************/
/*
	Client side. Open a socket connection to a remote ip and port.
	This code is not specific to SSL.
*/
SOCKET socketConnect(char *ip, short port, int *err)
{
	struct sockaddr_in	addr;
	SOCKET				fd;
	int					rc;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Error creating socket\n");
		*err = getSocketError();
		return INVALID_SOCKET;
	}
/*
	Make sure the socket is not inherited by exec'd processes
	Set the REUSEADDR flag to minimize the number of sockets in TIME_WAIT
*/
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	rc = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));
	setSocketNodelay(fd);
/*
	Turn on blocking mode for the connecting socket
*/
	setSocketBlock(fd);

	memset((char *) &addr, 0x0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	rc = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
#if WIN
	if (rc != 0) {
#else
	if (rc < 0) {
#endif
		*err = getSocketError();
		return INVALID_SOCKET;
	}
	return fd;
}

/******************************************************************************/
/*
	Server side.  Accept an incomming SSL connection request.
	'conn' will be filled in with information about the accepted ssl connection

	return -1 on error, 0 on success, or WOULD_BLOCK for non-blocking sockets
*/
int sslAccept(sslConn_t **cp, SOCKET fd, sslKeys_t *keys,
			  int (*certValidator)(void *ssl, psX509Cert_t *certInfo, int32 alert), int flags)
{
	sslConn_t		*conn;
	unsigned char	buf[1024];
	int				status, rc;
/*
	Associate a new ssl session with this socket.  The session represents
	the state of the ssl protocol over this socket.  Session caching is
	handled automatically by this api.
*/
	conn = calloc(sizeof(sslConn_t), 1);
	conn->fd = fd;
	if (matrixSslNewServerSession(&conn->ssl, keys, NULL, flags) < 0) {
		sslFreeConnection(&conn);
		return -1;
	}

	*cp = conn;

	return 0;
}

/******************************************************************************/
/*
	Client side.  Make a socket connection and go through the SSL handshake
	phase in blocking mode.  The last parameter is an optional function
	callback for user-level certificate validation.  NULL if not needed.
*/
int sslConnect(sslConn_t **cp, SOCKET fd, sslKeys_t *keys,
				sslSessionId_t *id, uint32 cipherSuite[],
				int32 (*certValidator)(void *ssl, psX509Cert_t *certInfo, int32 alert), void *cb_data )
{
	sslConn_t	    *conn;
	int				cipherSpecLen;
	int32			rc, transferred, len, status;
	unsigned char	*buf;

/*
	Create a new SSL session for the new socket and register the
	user certificate validator 
*/
	conn = calloc(sizeof(sslConn_t), 1);
	if (conn == NULL) {
		return -1;
	}
	conn->fd = fd;

	if(cipherSuite == NULL)
		cipherSpecLen = 0;
	else
		cipherSpecLen = sizeof(cipherSuite)/sizeof(uint32);
	
	if (matrixSslNewClientSession(&conn->ssl, keys, id, cipherSuite, cipherSpecLen, 
								certValidator, NULL, NULL, NULL, 0) < 0)
	{
		//printf("%s %d matrixSslNewClientSession fail.\n", __func__, __LINE__);
		sslFreeConnection(&conn);
		return -1;
	}
	matrixSslSetCertValidator(conn->ssl, certValidator);

	*cp = conn;

WRITE_MORE:
	while ((len = matrixSslGetOutdata(conn->ssl, &buf)) > 0)
	{
		transferred = send(fd, buf, len, MSG_DONTWAIT);
		if (transferred <= 0)
		{
			//printf("%s %d send fail.\n", __func__, __LINE__);
			sslFreeConnection(&conn);
			return -1;
		}
		else
		{
			/* Indicate that we've written > 0 bytes of data */
			if ((rc = matrixSslSentData(conn->ssl, transferred)) < 0)
			{
				//printf("%s %d matrixSslSentData fail.\n", __func__, __LINE__);
				sslFreeConnection(&conn);
				return -1;
			}
			if (rc == MATRIXSSL_REQUEST_CLOSE) {
				//printf("%s %d matrixSslSentData fail.\n", __func__, __LINE__);
				sslFreeConnection(&conn);
				return -1;
			}
		}
	}
	
READ_MORE:
	rc = sslRead(conn, buf, sizeof(buf), &status);
	if (rc == 0)
	{
		if (status == SSLSOCKET_EOF || status == SSLSOCKET_CLOSE_NOTIFY)
		{
//			sslFreeConnection(&conn);
			//printf("%s %d sslRead fail, rc=%d.\n", __func__, __LINE__, rc);
			return -1;
		}
	}
	else if (rc < 0)
	{
		//jiunming, for non-blocking, 
		if( (status==EINTR) || (status==EAGAIN) || (status==EWOULDBLOCK) )
		{ 
			fprintf( stderr, "sslConnect: skip EINTR/EAGAIN/EWOULDBLOCK(%d)\n", status );
			goto READ_MORE;
		}
		//sslFreeConnection(&conn);
		//printf("%s %d read fail, rc=%d.\n", __func__, __LINE__, rc);
		return -1;
	}
	
	return 0;
}

#define	GOTO_SANITY			32	/* Must be <= 255 */

#define _CHECKTO_
#ifdef _CHECKTO_
#define CK_TIME_VALUE	10
 
int check_rsk_timeout( int check_sk, int *st )
{
	struct timeval timeout;
	fd_set fd;
	
	if(st) *st=0;

	timeout.tv_sec = CK_TIME_VALUE;
	timeout.tv_usec = 0;

	FD_ZERO(&fd);
	FD_SET( check_sk, &fd);
	for (;;)
	{
		int r;
		r = select((check_sk + 1), &fd, NULL, &fd, &timeout);
		if (r > 0) break;
		
		if (!r)
		{
			//fprintf( stderr, "<%s:%d>time-out\n", __FUNCTION__, __LINE__ );
			return 1;
		}
		if (errno != EINTR && errno != EAGAIN )
		{
			//fprintf( stderr, "<%s:%d> errno:%d\n", __FUNCTION__, __LINE__, errno );
			if(st) *st = getSocketError();
			return -1;
		}
	}
	//fprintf( stderr, "<%s:%d> can read\n", __FUNCTION__, __LINE__ );

	return 0;
}


int check_wsk_timeout( int check_sk, int *st )
{
	struct timeval timeout;
    fd_set fd;

	if(st) *st=0;
		
	timeout.tv_sec = CK_TIME_VALUE;
    timeout.tv_usec = 0;

    FD_ZERO(&fd);
    FD_SET(check_sk, &fd);
    for (;;)
    {
    	int r;
    	r = select((check_sk + 1), NULL, &fd, &fd, &timeout);
      	if (r > 0) break;
      	
      	if (!r)
      	{ 
		//fprintf( stderr, "<%s:%d>time-out\n", __FUNCTION__, __LINE__ );
        		return 1;
      	}
      	if (errno != EINTR && errno != EAGAIN )
      	{
			//fprintf( stderr, "<%s:%d> errno:%d\n", __FUNCTION__, __LINE__, errno );
			if(st) *st=getSocketError();
    		return -1;
      	}
    }
	//fprintf( stderr, "<%s:%d> can write\n", __FUNCTION__, __LINE__ );

	return 0;
}
#endif //_CHECKTO_



/******************************************************************************/
/*
	An example socket sslRead implementation that handles the ssl handshake
	transparently.  Caller passes in allocated buf and length. 
	
	Return codes are as follows:

	-1 return code is an error.  If a socket level error, error code is
		contained in status parameter.  If using a non-blocking socket
		implementation the caller should check for non-fatal errors such as
		WOULD_BLOCK before closing the connection.  A zero value
		in status indicates an error with this routine.  

	A positive integer return code is the number of bytes successfully read
		into the supplied buffer.  User can call sslRead again on the updated
		buffer is there is more to be read.

	0 return code indicates the read was successful, but there was no data
		to be returned.  If status is set to zero, this is a case internal
		to the sslAccept and sslConnect functions that a handshake
		message has been exchanged.  If status is set to SOCKET_EOF
		the connection has been closed by the other side.

*/
int sslRead(sslConn_t *cp, char *buf, int len, int *status)
{
	unsigned char *ssl_buf;
	unsigned int  ssl_len;
	int32		  rc, transferred, retLen;
	unsigned char rSanity, wSanity, acceptSanity;
	ssl_t *ssl = (ssl_t *)cp->ssl;

	*status = 0;

	if ((NULL == cp->ssl) || (len <= 0)) {
		printf("[%s %d] parameter error[req->ssl %#x buflen %d].\n", 
				__func__, __LINE__, (unsigned int)cp->ssl, len);
		*status = MATRIXSSL_ERROR;
		return -1;
	}

	rc = matrixSslGetPlain(cp->ssl, &ssl_buf, &ssl_len);
	if ((PS_SUCCESS == rc) && (ssl_len > 0))
	{
		retLen = min(ssl_len, len);
		memcpy(buf, ssl_buf, retLen);
		//printf("read %d byte from %p\n", retLen, ssl_buf);
		//printf("buflen %d\n", buflen);

		ssl_buf += retLen;
		ssl_len -= retLen;
		matrixSslSetPlain(cp->ssl, ssl_buf, ssl_len);

		if (0 == ssl_len)//no plain text remained, then renew ssl->inbuf
			matrixSslProcessedData(cp->ssl, &ssl_buf, (uint32*)&ssl_len);
		
		return retLen;
	}

	/* if there is remaining data in inbuf, just return it */
	if (ssl->inlen > 0)
	{
		if ((rc = matrixSslReceivedData(cp->ssl, 0, &ssl_buf, 
										(uint32*)&ssl_len)) < 0) {
			printf("[%s %d] matrixSslReceivedData fail.\n", __func__, __LINE__);
			return 0;
		}
		/* just care data here */
		if ((MATRIXSSL_APP_DATA == rc) || (MATRIXSSL_APP_DATA_COMPRESSED == rc))
			goto PROCESS_MORE;
	}

	rSanity = wSanity = 0;
	
	/*
			See if there's pending data to send on this connection
			We could use FD_ISSET, but this is more reliable for the current
				state of data to send.
	 */
WRITE_MORE:
	if ((ssl_len = matrixSslGetOutdata(cp->ssl, &ssl_buf)) > 0) {
		/* Could get a EWOULDBLOCK since we don't check FD_ISSET */
		transferred = send(cp->fd, ssl_buf, ssl_len, MSG_DONTWAIT);
		if (transferred <= 0)
		{
			printf("[%s %d] send fail.\n", __func__, __LINE__);
			*status = getSocketError();
			
			if( (*status!=EINTR) && (*status!=EAGAIN) && (*status!=EWOULDBLOCK) )
				return -1;
		}
		else
		{
			//printf("[%s %d] send %d bytes\n", __func__, __LINE__, transferred);
			
			/* Indicate that we've written > 0 bytes of data */
			if ((rc = matrixSslSentData(cp->ssl, transferred)) < 0)
			{
				printf("[%s %d] matrixSslSentData fail.\n", __func__, __LINE__);
				return -1;
			}
			if (rc == MATRIXSSL_REQUEST_CLOSE)
			{
				printf("[%s %d] matrixSslSentData fail.\n", __func__, __LINE__);
				return -1;
			}
			else if (rc == MATRIXSSL_HANDSHAKE_COMPLETE)
			{
				/* If the protocol is server initiated, send data here */
//#ifdef ENABLE_FALSE_START
				/* OR this could be a Chrome browser using 
					FALSE_START and the application data is already
					waiting in our inbuf for processing */
				if ((rc = matrixSslReceivedData(cp->ssl, 0,
							&ssl_buf, (uint32*)&ssl_len)) < 0)
				{
					printf("[%s %d] matrixSslReceivedData fail.\n", __func__, __LINE__);
					return -1;
				}
				if (rc > 0) { /* There was leftover data */
					printf("[%s %d] process more\n", __func__, __LINE__);
					goto PROCESS_MORE;
				}
//#endif /* ENABLE_FALSE_START	*/
			}

			/* Try to send again if more data to send */
			if (rc == MATRIXSSL_REQUEST_SEND || transferred < ssl_len)
			{
				//printf("[%s %d] send more data.\n", __func__, __LINE__);
				if (wSanity++ < GOTO_SANITY) goto WRITE_MORE;
			}
		}
	}

	/*
	 Check the file descriptor returned from select to see if the connection
	 has data to be read
	 */
READ_MORE:
	/* Get the ssl buffer and how much data it can accept */
	/* Note 0 is a return failure, unlike with matrixSslGetOutdata */
	if ((ssl_len = matrixSslGetReadbuf(cp->ssl, &ssl_buf)) <= 0) {
		printf("[%s %d] matrixSslGetReadbuf fail.\n", __func__, __LINE__);
		*status = MATRIXSSL_ERROR;
		return 0;
	}
	
#ifdef _CHECKTO_
	{
		int ch_st, ch_ret;
		ch_st=0;
		ch_ret = check_rsk_timeout( cp->fd, &ch_st );
		if( ch_ret==1 ) //timeout
		{
			printf("[%s %d] timeout\n", __func__, __LINE__);
			*status = SSLSOCKET_EOF;
			return 0;
		}
		else if( ch_ret==-1) //error
		{
			printf("[%s %d] read error\n", __func__, __LINE__);
			*status = ch_st;
			return -1;
		}//else normal
	}
#endif //#ifdef _CHECKTO_
	if ((transferred = recv(cp->fd, ssl_buf, ssl_len, MSG_DONTWAIT)) < 0) {
		printf("[%s %d] recv fail.\n", __func__, __LINE__);
		/* We could get EWOULDBLOCK despite the FD_ISSET on goto  */
		if (errno != EWOULDBLOCK) {
			return -1;
		}
	}
	/* If EOF, remote socket closed. This is semi-normal closure.
	   Officially, we should close on closure alert. */
	if (transferred == 0) {
		printf("[%s %d] recv fail SSLSOCKET_EOF.\n", __func__, __LINE__);
		*status = SSLSOCKET_EOF;
		return 0;
	}

	/*
	 Notify SSL state machine that we've received more data into the
	 ssl buffer retreived with matrixSslGetReadbuf.
	*/
	if ((rc = matrixSslReceivedData(cp->ssl, (int32)transferred, &ssl_buf, 
									(uint32*)&ssl_len)) < 0) {
		printf("[%s %d] matrixSslReceivedData fail.\n", __func__, __LINE__);
		return 0;
	}

PROCESS_MORE:
	//printf("[%s %d] matrixSslReceivedData ret %d.\n", __func__, __LINE__, rc);
	/* Process any incoming plaintext application data */
	switch (rc) {
		case MATRIXSSL_SUCCESS:
		case MATRIXSSL_HANDSHAKE_COMPLETE:
			/* If the protocol is server initiated, send data here */
			//printf("[%s %d] read more data.\n", __func__, __LINE__);
			//goto READ_MORE;
			return 0;
		case MATRIXSSL_APP_DATA:
		case MATRIXSSL_APP_DATA_COMPRESSED:
			/* if buffer size is smaller recv data len, then only read buffer size content */
			retLen = min(ssl_len, len);
			memcpy(buf, ssl_buf, retLen);

			ssl_buf += retLen;
			ssl_len -= retLen;
			matrixSslSetPlain(cp->ssl, ssl_buf, ssl_len);
			
			/* We processed a partial HTTP message */
			if (0 == ssl_len)//no plain text remained, then renew ssl->inbuf
				matrixSslProcessedData(cp->ssl, &ssl_buf, (uint32*)&ssl_len);
			
			return retLen;
			
		case MATRIXSSL_REQUEST_SEND:
			//printf("[%s %d] send more data.\n", __func__, __LINE__);
			if (wSanity++ < GOTO_SANITY) goto WRITE_MORE;
			break;
		case MATRIXSSL_REQUEST_RECV:
			//printf("[%s %d] read more data.\n", __func__, __LINE__);
			if (rSanity++ < GOTO_SANITY) goto READ_MORE; 
			break;
		case MATRIXSSL_RECEIVED_ALERT:
			printf("[%s %d] received alert.\n", __func__, __LINE__);
			/* The first byte of the buffer is the level */
			/* The second byte is the description */
			if (*ssl_buf == SSL_ALERT_LEVEL_FATAL) {
				//printf("[%s %d] SSL_ALERT_LEVEL_FATAL.\n", __func__, __LINE__);
				return 0;
			}
			/* Closure alert is normal (and best) way to close */
			if (*(ssl_buf + 1) == SSL_ALERT_CLOSE_NOTIFY) {
				//printf("[%s %d] SSL_ALERT_CLOSE_NOTIFY.\n", __func__, __LINE__);
				*status = SSLSOCKET_CLOSE_NOTIFY;
				return 0;
			}
			
			if ((rc = matrixSslProcessedData(cp->ssl, &ssl_buf, (uint32*)&ssl_len)) == 0) {
				/* No more data in buffer. Might as well read for more. */
				//printf("[%s %d] read more.\n", __func__, __LINE__);
				goto READ_MORE;
			}
			//printf("[%s %d] process more.\n", __func__, __LINE__);
			goto PROCESS_MORE;

		default:
			/* If rc <= 0 we fall here */
			printf("[%s %d] fail.\n", __func__, __LINE__);
			*status = MATRIXSSL_ERROR;
			return 0;
	}
	/* Always try to read more if we processed some data */
	//printf("[%s %d] read more data.\n", __func__, __LINE__);
	if (rSanity++ < GOTO_SANITY) goto READ_MORE;

	return 0;
}


/******************************************************************************/
/*
	Example sslWrite functionality.  Takes care of encoding the input buffer
	and sending it out on the connection.

	Return codes are as follows:

	-1 return code is an error.  If a socket level error, error code is
		contained in status.  If using a non-blocking socket
		implementation the caller should check for non-fatal errors such as
		WOULD_BLOCK before closing the connection.  A zero value
		in status indicates an error with this routine.

	A positive integer return value indicates the number of bytes succesfully
		written on the connection.  Should always match the len parameter.

	0 return code indicates the write must be called again with the same
		parameters.
*/
int sslWrite(sslConn_t *cp, char *buf, int len, int *status)
{
	unsigned char *ssl_buf;
	uint32		  requested;
	int32		  available;
	int32		  transferred, ssl_len;
	int32		  rc, sendOK = 0;
	unsigned char wSanity;

	*status = 0;
	if ((NULL == cp->ssl) || (len <= 0)) {
		printf("[%s %d] parameter error[req->ssl %#x buflen %d].\n", 
				__func__, __LINE__, (unsigned int)cp->ssl, len);
		*status = MATRIXSSL_ERROR;
		return -1;
	}

	wSanity = 0;
	
WRITE_MORE:
	while ((ssl_len = matrixSslGetOutdata(cp->ssl, &ssl_buf)) > 0)
	{
		//printf("[%s %d] send buf:%p len:%d\n", __func__, __LINE__, ssl_buf, ssl_len);
		transferred = send(cp->fd, ssl_buf, ssl_len, MSG_DONTWAIT);
		if (transferred <= 0)
		{
			printf("[%s %d] send fail.\n", __func__, __LINE__);
			*status = errno;
			
			if( (*status!=EINTR) && (*status!=EAGAIN) && (*status!=EWOULDBLOCK) )
				return -1;
		}
		else
		{
			//printf("[%s %d] send %d bytes, total %d\n", __func__, __LINE__, transferred, ssl_len);
			
			/* Indicate that we've written > 0 bytes of data */
			if ((rc = matrixSslSentData(cp->ssl, transferred)) < 0)
			{
				printf("[%s %d] matrixSslSentData fail.\n", __func__, __LINE__);
				return -1;
			}
			if (rc == MATRIXSSL_REQUEST_CLOSE)
			{
				printf("[%s %d] matrixSslSentData fail.\n", __func__, __LINE__);
				return -1;
			}
			else if (rc == MATRIXSSL_HANDSHAKE_COMPLETE)
			{
				printf("[%s %d] write more.\n", __func__, __LINE__);
				goto WRITE_MORE;
			}

			/* Try to send again if more data to send */
			if (rc == MATRIXSSL_REQUEST_SEND || transferred < ssl_len)
			{
				printf("[%s %d] send more data.\n", __func__, __LINE__);
				if (wSanity++ < GOTO_SANITY) goto WRITE_MORE;
			}
		}
	}

	if (sendOK)
		return len;

	requested = len;
	if ((available = matrixSslGetWritebuf(cp->ssl, &ssl_buf, requested)) < 0)
	{
		printf("[%s %d] matrixSslGetWritebuf fail.\n", __func__, __LINE__);
		return -1;
	}
	//printf("[%s %d] ssl_buf %p available %d\n", __func__, __LINE__, ssl_buf, available);

	if (available < requested)
	{
		printf("[%s %d] no more available buffer.\n", __func__, __LINE__);
		return -1;
	}
	
	memcpy((char *)ssl_buf, (char *)buf, len);
	if (matrixSslEncodeWritebuf(cp->ssl, len) < 0)
	{
		printf("[%s %d] matrixSslEncodeWritebuf fail.\n", __func__, __LINE__);
		return -1;
	}

	sendOK = 1;

	goto WRITE_MORE;
}


/******************************************************************************/
/*
	Send a close alert
*/
void sslWriteClosureAlert(sslConn_t *cp)
{
	if (cp != NULL)
	{
		matrixSslEncodeClosureAlert(cp->ssl);
		/*ping_zhang:20090929 Set block socket avoid infinite while loop in send_cwmp*/
		//setSocketNonblock(cp->fd);
		setSocketBlock(cp->fd); 
		send(cp->fd, cp->ssl->outbuf,
			cp->ssl->outlen, MSG_NOSIGNAL);
	}
}

/******************************************************************************/
/*
	Close a seesion that was opened with sslAccept or sslConnect and
	free the insock and outsock buffers
*/
void sslFreeConnection(sslConn_t **cp)
{
	sslConn_t	*conn;

	conn = *cp;
	if (NULL == conn)
		return;
	
	matrixSslDeleteSession(conn->ssl);
	conn->ssl = NULL;
	free(conn);
	*cp = NULL;
}

/******************************************************************************/
/*
	Set the socket to non blocking mode and perform a few extra tricks
	to make sure the socket closes down cross platform
*/
void socketShutdown(SOCKET sock)
{
	char	buf[32];

	if (sock != INVALID_SOCKET) {
		setSocketNonblock(sock);
		if (shutdown(sock, 1) >= 0) {
			while (recv(sock, buf, sizeof(buf), 0) > 0);
		}
		closesocket(sock);
	}
}

/******************************************************************************/
/*
	Turn on socket blocking mode (and set CLOEXEC on LINUX for kicks).
*/
void setSocketBlock(SOCKET sock)
{
#if _WIN32
	int		block = 0;
	ioctlsocket(sock, FIONBIO, &block);
#elif LINUX
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) & ~O_NONBLOCK);
	fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif
}

/******************************************************************************/
/*
	Turn off socket blocking mode.
*/
void setSocketNonblock(SOCKET sock)
{
#if _WIN32
	int		block = 1;
	ioctlsocket(sock, FIONBIO, &block);
#elif LINUX
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
#endif
}

/******************************************************************************/
/*
	Disable the Nagle algorithm for less latency in RPC
	http://www.faqs.org/rfcs/rfc896.html
	http://www.w3.org/Protocols/HTTP/Performance/Nagle/
*/
void setSocketNodelay(SOCKET sock)
{
#if _WIN32
	BOOL	tmp = TRUE;
#else
	int		tmp = 1;
#endif /* WIN32 */
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp, sizeof(tmp));
}

/******************************************************************************/
/*
	Set a breakpoint in this function to catch asserts.
	This function is called whenever an assert is triggered.  Useful because
	VisualStudio often won't show the right line of code if DebugBreak() is 
	called directly, and abort() may not be desireable on LINUX.
*/
void breakpoint()
{
#ifdef DEBUG
	static int preventInline = 0;
#if _WIN32
	DebugBreak();
#elif LINUX
	abort();
#endif
#endif //DEBUG
}


/******************************************************************************/
/*
 	Parse an ASCII command line string.  Assumes a NULL terminated space 
 	separated list of command line arguments.  Uses this info to create an argv
 	array.
 
 	Notes:
 		handles double quotes
 		args gets hacked up!  can't pass in static string!
 		not thread safe, so should be called b4 any thread creation
 		we currently hardcode argv[0] cause none of our apps need it
 */

#if WINCE || VXWORKS

void parseCmdLineArgs(char *args, int *pargc, char ***pargv)
{
	char			**argv;
	char			*ptr;
	int				size, i;

/*
 *	Figure out the number of elements in our argv array.  
 *	We know we need an argv array of at least 3, since we have the
 *	program name, an argument, and a NULL in the array.
 */
	for (size = 3, ptr = args; ptr && *ptr != '\0'; ptr++) {
		if (isspace(*ptr)) {
			size++;
			while (isspace(*ptr)) {
				ptr++;
			}
			if (*ptr == '\0') {
				break;
			}
		}
	}
/*
 *	This is called from main, so don't use psMalloc here or
 *	all the stats will be wrong.
 */
	argv = (char**) malloc(size * sizeof(char*));
	*pargv = argv;

	for (i = 1, ptr = args; ptr && *ptr != '\0'; i++) {
		while (isspace(*ptr)) {
			ptr++;
		}
		if (*ptr == '\0')  {
			break;
		}
/*
 *		Handle double quoted arguments.  Treat everything within
 *		the double quote as one arg.
 */
		if (*ptr == '"') {
			ptr++;
			argv[i] = ptr;
			while ((*ptr != '\0') && (*ptr != '"')) {
				ptr++;
			}
		} else {
			argv[i] = ptr;
			while (*ptr != '\0' && !isspace(*ptr)) {
				ptr++;
			}
		}
		if (*ptr != '\0') {
			*ptr = '\0';
			ptr++;
		}
	}
	argv[i] = NULL;
	*pargc = i ;

	argv[0] = "PeerSec";
	for (ptr = argv[0]; *ptr; ptr++) {
		if (*ptr == '\\') {
			*ptr = '/';
		}
	}
}
#endif /* WINCE || VXWORKS */

#ifdef WINCE

/******************************************************************************/
/*
 	The following functions implement a unixlike time() function for WINCE.

	NOTE: this code is copied from the os layer in win.c to expose it for use
	in example applications.
 */

static FILETIME YearToFileTime(WORD wYear)
{	
	SYSTEMTIME sbase;
	FILETIME fbase;

	sbase.wYear         = wYear;
	sbase.wMonth        = 1;
	sbase.wDayOfWeek    = 1; //assumed
	sbase.wDay          = 1;
	sbase.wHour         = 0;
	sbase.wMinute       = 0;
	sbase.wSecond       = 0;
	sbase.wMilliseconds = 0;

	SystemTimeToFileTime( &sbase, &fbase );

	return fbase;
}

time_t time() {

	__int64 time1, time2, iTimeDiff;
	FILETIME fileTime1, fileTime2;
	SYSTEMTIME  sysTime;

/*
	Get 1970's filetime.
*/
	fileTime1 = YearToFileTime(1970);

/*
	Get the current filetime time.
*/
	GetSystemTime(&sysTime);
	SystemTimeToFileTime(&sysTime, &fileTime2);


/* 
	Stuff the 2 FILETIMEs into their own __int64s.
*/	
	time1 = fileTime1.dwHighDateTime;
	time1 <<= 32;				
	time1 |= fileTime1.dwLowDateTime;

	time2 = fileTime2.dwHighDateTime;
	time2 <<= 32;				
	time2 |= fileTime2.dwLowDateTime;

/*
	Get the difference of the two64-bit ints.

	This is he number of 100-nanosecond intervals since Jan. 1970.  So
	we divide by 10000 to get seconds.
 */
	iTimeDiff = (time2 - time1) / 10000000;
	return (int)iTimeDiff;
}
#endif /* WINCE */

/******************************************************************************/





