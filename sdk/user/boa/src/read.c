/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Some changes Copyright (C) 1996, 97 Larry Doolittle <ldoolitt@jlab.org>
 *  Some changes Copyright (C) 1997 Jon Nelson <nels0988@tc.umn.edu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "asp_page.h"
#include "boa.h"

#ifdef SUPPORT_ASP
//#include <string.h>
#endif

#ifdef SERVER_SSL
#ifdef USES_MATRIX_SSL
#include <sslSocket.h>
#else /*!USES_MATRIX_SSL*/
#include <openssl/ssl.h>
#endif /*USES_MATRIX_SSL*/
#endif
//ql
#include <stdio.h>

// Added by Mason Yu for limit filesize
#ifdef EMBED
#include <rtk/utility.h>
#else
#include "LINUX/utility.h"
#endif
int g_filesize;
int g_max_upload_size;
#ifdef ENABLE_SIGNATURE_ADV
int upgrade=0;
#endif

/*
 * Name: read_header
 * Description: Reads data from a request socket.  Manages the current
 * status via a state machine.  Changes status from READ_HEADER to
 * READ_BODY or WRITE as necessary.
 *
 * Return values:
 *  -1: request blocked, move to blocked queue
 *   0: request done, close it down
 *   1: more to do, leave on ready list
 */
static unsigned int loop_counter=0;
int read_header(request * req)
{
	int restart = 0, bytes, buf_bytes_left;
	char *check, *buffer;

restart_tag:

	if (req->pipeline_start){
		buffer = req->client_stream;
		bytes = req->client_stream_pos = req->pipeline_start;
		req->pipeline_start = 0;
	} else { /* first from free_request */
		buffer = req->client_stream + req->client_stream_pos;
		buf_bytes_left = CLIENT_STREAM_SIZE - req->client_stream_pos;
		if (buf_bytes_left < 0) {
#ifdef BOA_TIME_LOG
			log_error_time();
			fputs("buffer overrun - read.c, read_header - closing\n", stderr);
#endif
			syslog(LOG_ERR, "buffer overrun in read_header1 - closing");
			return 0;
		}
		/*MN stuff to support ssl*/
#ifdef SERVER_SSL
		if(req->ssl == NULL){
#endif /*SERVER_SSL*/
	//	if (!strncmp(buffer,"GET /favicon.ico",16)){
	//		SQUASH_KA(req);
	//		printf("%s catched....\n",__FUNCTION__);
	//		return 0;
	//	}
			bytes = read(req->fd, buffer, buf_bytes_left);
#ifdef SERVER_SSL
		}
		else{
#ifdef USES_MATRIX_SSL
                      int     mtrx_status;
                      int     t = 6; // prevent forever loop
                      bytes = sslRead(req->ssl, buffer, buf_bytes_left,&mtrx_status);
                      while ((t--) && (bytes==0) &&(mtrx_status == 0))
                      {
                              bytes = sslRead(req->ssl, buffer, buf_bytes_left,&mtrx_status);
                      }
#else
			bytes = SSL_read(req->ssl, buffer, buf_bytes_left);
#endif
#if 0
			printf("SSL_read 1\n");
			if(bytes > 0){
				char tmp;
				tmp = buffer[bytes-1];
				buffer[bytes-1] = '\0';
				printf("buffer-\n %s\n----------\n", buffer);
				buffer[bytes-1] = tmp;
			}
#endif /*0*/
		}
#endif /*SERVER_SSL*/

		if (bytes == -1) {
			if (errno == EINTR){
				loop_counter = 0;
				return 1;
			}
			if (errno == EAGAIN || errno == EWOULDBLOCK)	/* request blocked */{
				loop_counter++;
				// workaround for request blocking.
				// threshhold 20 may mistakely drop normal connection, changed to 200 instead.
				//if (loop_counter>=20){
				if (loop_counter>=200){
					printf("Request blocked over 100 times, free this request !\n");
					SQUASH_KA(req);
					loop_counter =0;
					return 0;
				}
					return -1;
			}else if (errno == EBADF || errno == EPIPE) {
				loop_counter = 0;
				SQUASH_KA(req);		/* force close fd */
				return 0;
			} else {
#if 0
				boa_perror(req, "header read");
#endif
				loop_counter =0;
				return 0;
			}
		} else if (bytes == 0) {
			// if (buf_bytes_left == 0)
			//	syslog(LOG_ERR, "buffer overrun in read_header2 - closing");
			return 0;
		}
		loop_counter = 0;
		req->client_stream_pos += bytes;
	}

	check = buffer;

	while (check < (buffer + bytes)) {
		switch (req->status) {
		case READ_HEADER:
			if (*check == '\r') {
				req->status = ONE_CR;
				req->header_end = check;
			} else if (*check == '\n') {
				req->status = ONE_LF;
				req->header_end = check;
			}
			break;

		case ONE_CR:
			if (*check == '\n')
				req->status = ONE_LF;
			else
				req->status = READ_HEADER;
			break;

		case ONE_LF:			/* if here, we've found the end (for sure) of a header */
			if (*check == '\r')	/* could be end o headers */
				req->status = TWO_CR;
			else if (*check == '\n')
				req->status = BODY_READ;
			else
				req->status = READ_HEADER;
			break;

		case TWO_CR:
			if (*check == '\n')
				req->status = BODY_READ;
			else
				req->status = READ_HEADER;
			break;

		default:
			break;
		}

		++check;

		if (req->status == ONE_LF) {
			int count;
			*req->header_end = '\0';
			/* terminate string that begins at req->header_line */
			/* (or at req->data_mem, if we've never been here before */

			/* the following logic still needs work, esp. after req->simple */
			if (req->logline) {
				count = process_option_line(req);
				bytes -= count;
				check -= count;
				if (count)
					restart = 1;
			} else {
				if (process_logline(req) == 0)
					return 0;
				if (req->simple)
					return process_header_end(req);
			}
			req->header_line = check; /* start of unprocessed data */
		} else if (req->status == BODY_READ) {
			int retval= process_header_end(req);
			/* process_header_end inits non-POST cgi's */

			req->pipeline_start = (check - req->client_stream);

			if (retval && req->method == M_POST) {

				/* rest of non-header data is contained in
				   the area following check

				   check now points to data
				 */

				if (req->content_length) {
					req->filesize = atoi(req->content_length);
					//printf("req->filesize=%d\n", req->filesize);
					// Added by Mason Yu for limit filesize
					g_filesize = req->filesize;

					//ql-- in 831II, if signature error, the system should not restart,
					//       so process should not be killed unless signature is OK.
#ifndef ENABLE_SIGNATURE_ADV
					if ( req->filesize > MIN_UPLOAD_FILESIZE ) {
						cmd_killproc(ALL_PID & ~(1<<PID_WEB));
					}
#else
					if (req->filesize > MIN_UPLOAD_FILESIZE)
						upgrade = 1;
#endif
				}
				else {
#ifdef BOA_TIME_LOG
					log_error_time();
					fprintf(stderr, "Unknown Content-Length POST\n");
#endif
				}

				/* buffer + bytes is 1 past the end of the data */
				req->filepos = (buffer + bytes) - check;

				/* copy the remainder into req->buffer, otherwise
				 * we don't have a full BUFFER_SIZE to play with,
				 * and buffer overruns occur */
				memcpy(req->buffer, check, req->filepos);
				req->header_line = req->buffer;
				req->header_end = req->buffer + req->filepos;

				if (req->filepos >= req->filesize)
					req->cgi_status = CGI_CLOSE;	/* close after write */
			}
			return retval;		/* 0 - close it done, 1 - keep on ready */
		}
	}

	/* check for bogus requests */
	if (req->status == READ_HEADER && request_type(req) == M_INVALID) {
		syslog(LOG_ERR, "malformed request: \"%s\"\n", req->logline);
		send_r_bad_request(req);
		return 0;
	}

	if (restart)
		goto restart_tag;

	/* only reached if request is split across more than one packet */
	return 1;
}

/*
 * Name: read_body
 * Description: Reads body from a request socket for POST CGI
 *
 * Return values:
 *
 *  -1: request blocked, move to blocked queue
 *   0: request done, close it down
 *   1: more to do, leave on ready list
 *
 *   At the moment, we are lenient with a POST request that comes
 *   in without a content length.  We read the network socket
 *   until we get an EWOUDLBOCK or EAGAIN, then assume that's the end.
 *   This works for short requests (less than a packet), but fails
 *   miserably for multi-packet requests.  Of course, it's their fault
 *   for not including the content-length so we can know when to quit.
 *   (Modified from Larry Doolittle's comment on the 0.92 POST patch)
 *   Note that filesize, if present, denotes the converted Content-Length
 */

int read_body(request * req)
{
	int bytes_read, bytes_to_read;

	bytes_to_read = BUFFER_SIZE - (req->header_end - req->header_line);

	if (req->filesize) {
		int bytes_left = req->filesize - req->filepos;
		if (bytes_left < bytes_to_read)
			bytes_to_read = bytes_left;
	}
	if (bytes_to_read <= 0) {
		req->status = BODY_WRITE;	/* go write it */
		return 1;
	}
#ifdef SERVER_SSL
	if(req->ssl == NULL){
#endif /*SERVER_SSL*/
		bytes_read = read(req->fd, req->header_end,	bytes_to_read);
#ifdef SERVER_SSL
	}
	else{
#ifdef USES_MATRIX_SSL
		int	mtrx_status;
		int	t = 6; // prevent sslRead loop
		bytes_read = sslRead(req->ssl, req->header_end, bytes_to_read,&mtrx_status);
		while ((t--) && (bytes_read==0) &&(mtrx_status == 0))
		{
			bytes_read = sslRead(req->ssl, req->header_end, bytes_to_read,&mtrx_status);
		}
#else /*! USES_MATRIX_SSL*/
		bytes_read = SSL_read(req->ssl, req->header_end, bytes_to_read);
#endif /*USES_MATRIX_SSL*/
#if 0
		printf("SSL_read 2\n");
		if(bytes_read >0){
			char tmp;
			tmp = req->header_end[bytes_read-1];
			req->header_end[bytes_read-1] = '\0';
			printf("buffer-\n %s\n----------\n", req->header_end);
			req->header_end[bytes_read-1] = tmp;
		}
#endif /*0*/
	}
#endif /*SERVER_SSL*/

	if (bytes_read == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			/*
			 * avoid a busy loop swapping from READ/WRITE and block
			 * for more read data
			 */
			if (req->filesize) {
				return -1;
			}
			req->status = BODY_WRITE;
			return 1;
		} else {
			boa_perror(req, "read body");
			return 0;
		}
	} else if (bytes_read == 0) {
		req->status = BODY_WRITE;
		req->cgi_status = CGI_CLOSE;	/* init cgi when write finished */
	}
	req->header_end += bytes_read;
	req->filepos += bytes_read;

	if (bytes_read == bytes_to_read)
		req->status = BODY_WRITE;

	return 1;
}

/*
 * Name: write_body
 * Description: Writes a chunk of data to a file
 *
 * Return values:
 *  -1: request blocked, move to blocked queue
 *   0: EOF or error, close it down
 *   1: successful write, recycle in ready queue
 */

int write_body(request * req)
{
	int bytes_written, bytes_to_write = req->header_end - req->header_line;
#ifdef ENABLE_SIGNATURE_ADV
	IMGHDR imgHdr;
	SIGHDR sigHdr;
	unsigned int hdrChksum;
	unsigned char *imgStr;
	unsigned int len;
	unsigned int startPos, i=0;
	unsigned char c;
#endif
	/*ql: 20080729 START: check image key according to IC version*/
#ifdef MULTI_IC_SUPPORT
	int key;
#endif
	/*ql: 20080729 END*/
#ifdef CONFIG_RTL8686
	int err = -1;
#endif

	if (bytes_to_write == 0) {	/* nothing left in buffer to write */
		req->header_line = req->header_end = req->buffer;
		if (req->cgi_status == CGI_CLOSE || req->filepos >= req->filesize){

#ifdef SUPPORT_ASP
			return init_form(req);	/* init_cgi should change status approp. */
#else
			return init_cgi(req);	/* init_cgi should change status approp. */
#endif
		}
		/* if here, we can safely assume that there is more to read */
		req->status = BODY_READ;
		return 1;
	}

	// Modified by Mason Yu for limit filesize
	if (req->filepos <= g_max_upload_size) {
#ifdef ENABLE_SIGNATURE_ADV
		if ( ((req->filesize > MIN_UPLOAD_FILESIZE) && upgrade) ||
			req->filesize <= MIN_UPLOAD_FILESIZE) {//check if is not config file
#endif
			bytes_written = write(req->post_data_fd,	req->header_line, 	bytes_to_write);

			if (bytes_written == -1) {
				if (errno == EWOULDBLOCK || errno == EAGAIN){
					return -1;			/* request blocked at the pipe level, but keep going */
				}else {
#if 0
					boa_perror(req, "write body");	/* OK to disable if your logs get too big */
#endif
					return -2; /* exit and error */
				}
			}
#ifdef ENABLE_SIGNATURE_ADV
		}
#endif
		//printf("write body bytes_to_write=%d bytes_written=%d\n",bytes_to_write,bytes_written);
	}

	//ql--when upgrade the image, if received over 300 bytes, check the signature.
	//if signature OK, then kill some process to release memory, otherwise stop upgrade.
#ifdef ENABLE_SIGNATURE_ADV
	if ( req->filesize > MIN_UPLOAD_FILESIZE ) {//check if is not config file
		if (upgrade == 1) {
			if (req->filepos > 300) {
				imgStr = (unsigned char *)malloc(300);

				lseek(req->post_data_fd, 0, SEEK_SET);
				//read 300 bytes from tmp file
				if ((len=read(req->post_data_fd, imgStr, 300)) < 300) {
					printf("read post data error! read len:%d\n", len);
					goto FAIL;
				}
				lseek(req->post_data_fd, 0, SEEK_END);
				//seek the header of the image
				do
				{
					if (i >= 300) {
						printf("Cannot find start of file\n");
						goto FAIL;
					}
					if ((c=imgStr[i++]) != 0xd)
						continue;
					if ((c=imgStr[i++]) != 0xa)
						continue;
					if ((c=imgStr[i++]) != 0xd)
						continue;
					if ((c=imgStr[i++]) != 0xa)
						continue;
					break;
				}while(1);
				startPos = i;
#ifdef ENABLE_SIGNATURE
				//get signature header
				memcpy(&sigHdr, imgStr+startPos, sizeof(sigHdr));
				//get image header
				memcpy(&imgHdr, imgStr+startPos+sizeof(sigHdr), sizeof(imgHdr));

				if (sigHdr.sigLen > SIG_LEN) {
					printf("signature length error! sig len:%d\n", sigHdr.sigLen);
					goto FAIL;
				}
				for (i=0; i<sigHdr.sigLen; i++)
					sigHdr.sigStr[i] = sigHdr.sigStr[i] - 10;
				if (strcmp(sigHdr.sigStr, SIGNATURE)) {
					printf("signature error\n");
					goto FAIL;
				}

				hdrChksum = sigHdr.chksum;
				hdrChksum = ipchksum(&imgHdr, sizeof(imgHdr), hdrChksum);
				if (hdrChksum) {
					printf("Checksum failed(read), size=%d, csum=%04xh\n", sigHdr.sigLen, hdrChksum);
					goto FAIL;
				}
				/*ql:20080729 START: different ic matched with different image key*/
#ifdef MULTI_IC_SUPPORT
				key = getImgKey();

				if ((key != (imgHdr.key & key)) || (((imgHdr.key>>28)&0xf) != ((key>>28)&0xf))) {
					printf("image header error!\n");
					goto FAIL;
				}
#endif
				/*ql:20080729 END*/
#else
				//check image header
				memcpy(&imgHdr, imgStr+startPos, sizeof(imgHdr));
				/*ql:20080729 START: different ic matched with different image key*/
#ifdef MULTI_IC_SUPPORT
				key = getImgKey();

				if ((key != (imgHdr.key & key)) || (((imgHdr.key>>28)&0xf) != ((key>>28)&0xf))) {
					printf("image header error!\n");
					goto FAIL;
				}
#else
#ifdef CONFIG_RTL8686
				switch(imgHdr.key){
					case APPLICATION_UBOOT:
					case APPLICATION_UIMAGE:		
					case APPLICATION_ROOTFS:
						printf("%s-%d, got header::%x\n",__func__,__LINE__,imgHdr.key);
						err = 0;
						break;
					default:
						printf("%s-%d, Unknown header::%x\n",__func__,__LINE__,imgHdr.key);
						err = 1;
						break;
				}
				if(err)
					goto FAIL;
#else
				if (imgHdr.key != APPLICATION_IMAGE) {
					printf("image header error!\n");
					goto FAIL;
				}
#endif /*CONFIG_RTL8686*/				
#endif
				/*ql:20080729 END*/
#endif
				upgrade = 2;	//check OK
				// Kaohj -- disable ip forwarding
// Mason Yu.  do not interrupt service
#if 0
				setup_ipforwarding(0);
				cmd_killproc(ALL_PID & ~(1<<PID_WEB));
				// Notify driver/kernel to release resources.
				va_cmd("/bin/sarctl", 1, 1, "upgrade_fw");
				va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
#endif
				cmd_killproc(ALL_PID & ~(1<<PID_WEB));
				goto SUCC;
FAIL:
				upgrade = 0;
SUCC:
				free(imgStr);
				//req->status = BODY_WRITE;
				//return 3;	//ql defined
			}
		}
	}
#endif

#ifdef ENABLE_SIGNATURE_ADV
	if ( ((req->filesize > MIN_UPLOAD_FILESIZE) && upgrade) ||
		req->filesize <= MIN_UPLOAD_FILESIZE) {
#endif
		// Modified by Mason Yu for limit filesoze
		if (req->filepos <= g_max_upload_size) {
			req->header_line += bytes_written;
		}
		else {
			req->header_line += bytes_to_write;
		}
#ifdef ENABLE_SIGNATURE_ADV
	} else {
		req->header_line += bytes_to_write;
	}
#endif

	return 1;					/* more to do */
}
