/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Some changes Copyright (C) 1996,97 Larry Doolittle <ldoolitt@jlab.org>
 *  Some changes Copyright (C) 1996,97 Jon Nelson <nels0988@tc.umn.edu>
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

/* boa: request.c */

#include "asp_page.h"
#include "boa.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#ifdef SERVER_SSL
#ifdef USES_MATRIX_SSL
#include <sslSocket.h>
#else /*!USES_MATRIX_SSL*/
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif /*USES_MATRIX_SSL*/
#endif

#include "syslog.h"

#ifdef EMBED
#include "./LINUX/options.h"
#if defined(WLAN_WEB_REDIRECT) || defined(WEB_REDIRECT_BY_MAC) || defined(_SUPPORT_CAPTIVEPORTAL_PROFILE_)
#include "./LINUX/mib.h"
#endif
#else
#include "./LINUX/options.h"
#if defined(WLAN_WEB_REDIRECT) || defined(WEB_REDIRECT_BY_MAC) || defined(_SUPPORT_CAPTIVEPORTAL_PROFILE_)
#include "./LINUX/mib.h"
#endif
#endif
//ql
#ifdef ENABLE_SIGNATURE_ADV
#include "defs.h"
#endif


request *get_sock_request(int sock_fd);


int sockbufsize = SOCKETBUF_SIZE;
int g_upgrade_firmware=FALSE;

extern int server_s;			/* boa socket */
extern int do_sock;				/*Do normal sockets??*/
extern time_t time_counter;

#ifdef SERVER_SSL
extern int server_ssl;			/*the socket to listen for ssl requests*/
#ifdef USES_MATRIX_SSL
#else /* !USES_MATRIX_SSL */
extern SSL_CTX *ctx;			/*The global connection context*/
extern int do_ssl;				/*do ssl sockets??*/
#endif /*USES_MATRIX_SSL */
#endif /*SERVER_SSL*/


/*
 * the types of requests we can handle
 */

static struct {
	char	*str;
	int		 type;
} request_types[] = {
	{ "GET ",	M_GET },
	{ "POST ",	M_POST },
	{ "HEAD ",	M_HEAD },
	{ NULL,		0 }
};

/*
 * return the request type for a request,  short or invalid
 */

int request_type(request *req)
{
	int i, n, max_out = 0;

	for (i = 0; request_types[i].str; i++) {
		n = strlen(request_types[i].str);
		if (req->client_stream_pos < n) {
			max_out = 1;
			continue;
		}
		if (!memcmp(req->client_stream, request_types[i].str, n))
			return(request_types[i].type);
	}
	return(max_out ? M_SHORT : M_INVALID);
}


/*
 * Name: new_request
 * Description: Obtains a request struct off the free list, or if the
 * free list is empty, allocates memory
 *
 * Return value: pointer to initialized request
 */

request *new_request(void)
{
	request *req;

	if (request_free) {
		req = request_free;		/* first on free list */
		dequeue(&request_free, request_free);	/* dequeue the head */
	} else {
		req = (request *) malloc(sizeof(request));
		if (!req)
			die(OUT_OF_MEMORY);
	}

#ifdef SERVER_SSL
	req->ssl = NULL;
#endif /*SERVER_SSL*/

	memset(req, 0, sizeof(request) - NO_ZERO_FILL_LENGTH);

#ifdef SUPPORT_ASP
		req->max_buffer_size=CLIENT_STREAM_SIZE;
		req->buffer=(char *)malloc(req->max_buffer_size+1);
		if(!req->buffer)
			die(OUT_OF_MEMORY);
#endif

	return req;
}


/*
 * Name: get_request
 *
 * Description: Polls the server socket for a request.  If one exists,
 * does some basic initialization and adds it to the ready queue;.
 */

void get_request(void)
{
	get_sock_request(server_s);
}


#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
extern int redir_server_s;
void get_redir_request(void)
{
	request *new_req;
	new_req = get_sock_request(redir_server_s);
	if(new_req)
		new_req->request_from=1;
}
#endif

#ifdef WEB_REDIRECT_BY_MAC
extern int mac_redir_server_s;
void get_mac_redir_request(void)
{
	request *new_req;
	new_req = get_sock_request(mac_redir_server_s);
	if(new_req)
		new_req->request_mac=1;
}
int handle_mac_for_webredirect( char *mac )
{
	unsigned char tmp[6];
	unsigned int tmp1[6];

	if(mac==NULL) return -1;

	if( sscanf( mac, "%x:%x:%x:%x:%x:%x",
			&tmp1[0], &tmp1[1], &tmp1[2], &tmp1[3], &tmp1[4], &tmp1[5] )!=6 )
	{
		printf( "\n paser the mac format error: %s\n", mac );
		return -1;
	}


	if( tmp1[0]==0 && tmp1[1]==0 && tmp1[2]==0 &&
		tmp1[3]==0 && tmp1[4]==0 && tmp1[5]==0 )
	{
		printf( "\n the mac address is zero: %s\n", mac );
		return -1;
	}

	tmp[0]=tmp1[0];tmp[1]=tmp1[1];tmp[2]=tmp1[2];
	tmp[3]=tmp1[3];tmp[4]=tmp1[4];tmp[5]=tmp1[5];
	//printf( "\n the parsed mac address is %02X:%02X:%02X:%02X:%02X:%02X\n", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5] );

	{
		int num;
		unsigned char tmp2[18];
		MIB_WEB_REDIR_BY_MAC_T	wrm_entry;

		num = mib_chain_total( MIB_WEB_REDIR_BY_MAC_TBL );
		//printf( "\nori num=%d\n", num );
		if( num>=MAX_WEB_REDIR_BY_MAC )
		{
			//delete the first one
			if( mib_chain_get( MIB_WEB_REDIR_BY_MAC_TBL, 0, (void*)&wrm_entry )!=0 )
			{
				sprintf( tmp2, "%02X:%02X:%02X:%02X:%02X:%02X",
					wrm_entry.mac[0], wrm_entry.mac[1], wrm_entry.mac[2], wrm_entry.mac[3], wrm_entry.mac[4], wrm_entry.mac[5] );
				//printf( "delete old one mac: %s \n", tmp2 );
				// iptables -A macfilter -i eth0  -m mac --mac-source $MAC -j ACCEPT/DROP
				va_cmd("/bin/iptables", 10, 1, "-t", "nat", "-D", "WebRedirectByMAC", "-m", "mac", "--mac-source", tmp2, "-j", "RETURN");
			}

			mib_chain_delete( MIB_WEB_REDIR_BY_MAC_TBL, 0 );
			//num = mib_chain_total( MIB_WEB_REDIR_BY_MAC_TBL );
			//printf( "\after del nnum=%d\n", num );
		}

		//add the new one
		memcpy( wrm_entry.mac, tmp, 6 );
		mib_chain_add( MIB_WEB_REDIR_BY_MAC_TBL, (unsigned char*)&wrm_entry );
		// iptables -A macfilter -i eth0  -m mac --mac-source $MAC -j ACCEPT/DROP
		va_cmd("/bin/iptables", 10, 1, "-t", "nat", "-I", "WebRedirectByMAC", "-m", "mac", "--mac-source", mac, "-j", "RETURN");
		//num = mib_chain_total( MIB_WEB_REDIR_BY_MAC_TBL );
		//printf( "\after add nnum=%d\n", num );

		//update to the flash
		#if 0
		itfcfg("sar", 0);
		itfcfg("eth0", 0);
		#endif
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
		#if 0
		itfcfg("sar", 1);
		itfcfg("eth0", 1);
		#endif
	}

	return 0;
}
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
extern int captiveportal_server_s;
void get_captiveportal_request(void)
{
	request *new_req;
	new_req = get_sock_request(captiveportal_server_s);

	if(new_req)
		new_req->request_captiveportal = 1;
	else
		new_req->request_captiveportal = 0;
}
#endif

#ifdef SERVER_SSL
#ifdef USES_MATRIX_SSL
	extern sslConn_t	*mtrx_cp;
	extern sslKeys_t	*mtrx_keys;
void get_ssl_request(void)
{
	request *conn;
	int	rc,flags=0;


	conn = get_sock_request(server_ssl);
	if (!conn)
		return;

	if ((rc = sslAccept(&mtrx_cp, conn->fd, mtrx_keys, NULL, flags)) != 0) {
		socketShutdown(conn->fd);
		return;
	}
	conn->ssl = mtrx_cp;
}
#else /*!USES_MATRIX_SSL */
void get_ssl_request(void)
{
	request *conn;

	conn = get_sock_request(server_ssl);
	if (!conn)
		return;
	conn->ssl = SSL_new (ctx);
	if(conn->ssl == NULL){
		printf("Couldn't create ssl connection stuff\n");
		return;
	}
	SSL_set_fd (conn->ssl, conn->fd);
	if(SSL_accept(conn->ssl) <= 0){
		ERR_print_errors_fp(stderr);
		return;
	}
	else{/*printf("SSL_accepted\n");*/}
}
#endif /*USES_MATRIX_SSL*/
#endif /*SERVER_SSL*/

/* This routine works around an interesting problem encountered with IE
 * and the 2.4 kernel (i.e. no problems with netscape or with the 2.0 kernel).
 * The hassle is that the connection socket has a couple of crap bytes sent to
 * it by IE after the HTTP request.  When we close a socket with some crap waiting
 * to be read, the 2.4 kernel shuts down with a reset whereas earlier kernels
 * did the standard fin stuff.  IE complains about the reset and aborts page
 * rendering immediately.
 *
 * We must not loop here otherwise a DoS will have us for breakfast.
 */
static void safe_close(int fd) {
	fd_set rfd;
	struct timeval to;
	char buf[32];

	to.tv_sec = 0;
	to.tv_usec = 100;
	FD_ZERO(&rfd);
	FD_SET(fd, &rfd);
	if ((select(fd+1, &rfd, NULL, NULL, &to)) > 0 && FD_ISSET(fd, &rfd))
		read(fd, buf, sizeof buf);
	close(fd);
}

/*
 * Name: free_request
 *
 * Description: Deallocates memory for a finished request and closes
 * down socket.
 */

void free_request(request ** list_head_addr, request * req)
{
	if (req->buffer_end)
		return;

	dequeue(list_head_addr, req);	/* dequeue from ready or block list */

	if (req->buffer_end)
		FD_CLR(req->fd, &block_write_fdset);
	else {
		switch (req->status) {
		case PIPE_WRITE:
		case WRITE:
			FD_CLR(req->fd, &block_write_fdset);
			break;
		case PIPE_READ:
			FD_CLR(req->data_fd, &block_read_fdset);
			break;
		case BODY_WRITE:
			FD_CLR(req->post_data_fd, &block_write_fdset);
			break;
		default:
			FD_CLR(req->fd, &block_read_fdset);
		}
	}

	if (req->logline)			/* access log */
		log_access(req);

	if (req->data_mem)
		munmap(req->data_mem, req->filesize);

	if (req->data_fd)
		close(req->data_fd);

	if (req->response_status >= 400)
		status.errors++;

	if ((req->keepalive == KA_ACTIVE) &&
		(req->response_status < 400) &&
		(++req->kacount < ka_max)) {

		request *conn;

		conn = new_request();
		conn->fd = req->fd;
		conn->status = READ_HEADER;
		conn->header_line = conn->client_stream;
		conn->time_last = time_counter;
		conn->kacount = req->kacount;
#ifdef SERVER_SSL
		conn->ssl = req->ssl; /*MN*/
#endif /*SERVER_SSL*/

		/* we don't need to reset the fd parms for conn->fd because
		   we already did that for req */
		/* for log file and possible use by CGI programs */

		strcpy(conn->remote_ip_addr, req->remote_ip_addr);

		/* for possible use by CGI programs */
		conn->remote_port = req->remote_port;

		if (req->local_ip_addr)
			conn->local_ip_addr = strdup(req->local_ip_addr);

		status.requests++;

		if (conn->kacount + 1 == ka_max)
			SQUASH_KA(conn);

		conn->pipeline_start = req->client_stream_pos -
								req->pipeline_start;

		if (conn->pipeline_start) {
			memcpy(conn->client_stream,
				req->client_stream + req->pipeline_start,
				conn->pipeline_start);
			enqueue(&request_ready, conn);
		} else
			block_request(conn);
	} else{
		if (req->fd != -1) {
			status.connections--;
			safe_close(req->fd);
		}
		req->fd = -1;
#ifdef SERVER_SSL
#ifdef USES_MATRIX_SSL
		if (req->ssl)
		sslFreeConnection((sslConn_t**)&req->ssl);
#else /*!USES_MATRIX_SSL*/
		SSL_free(req->ssl);
#endif /*USES_MATRIX_SSL*/
#endif /*SERVER_SSL*/
	}

	if (req->cgi_env) {
		int i = COMMON_CGI_VARS;
		req->cgi_env[req->cgi_env_index]=0;
		while (req->cgi_env[i])
		{
			free(req->cgi_env[i++]);
		}
		free(req->cgi_env);
	}
	if (req->pathname)
		free(req->pathname);
	if (req->path_info)
		free(req->path_info);
	if (req->path_translated)
		free(req->path_translated);
	if (req->script_name)
		free(req->script_name);
	if (req->query_string)
		free(req->query_string);
	if (req->local_ip_addr)
		free(req->local_ip_addr);
/*
 *	need to clean up if anything went wrong
 */
 // Mason Yu
 	if (g_upgrade_firmware==FALSE) {
		if (req->post_file_name) {
			unlink(req->post_file_name);
			free(req->post_file_name);
			close(req->post_data_fd);
			req->post_data_fd = -1;
			req->post_file_name = NULL;
		}
	} else {
		g_upgrade_firmware=FALSE;
	}

	if(req->buffer) free(req->buffer);

	enqueue(&request_free, req);	/* put request on the free list */

	return;
}


/*
 * Name: process_requests
 *
 * Description: Iterates through the ready queue, passing each request
 * to the appropriate handler for processing.  It monitors the
 * return value from handler functions, all of which return -1
 * to indicate a block, 0 on completion and 1 to remain on the
 * ready list for more procesing.
 */

void process_requests(void)
{
	int retval = 0;
	request *current, *trailer;
	current = request_ready;

	while (current) {
#ifdef CRASHDEBUG
		crashdebug_current = current;
#endif

		if (current->buffer_end) {
			req_flush(current);
			if (current->status == CLOSE)
				retval = 0;
			else
				retval = 1;
		} else {
			switch (current->status) {
			case READ_HEADER:
			case ONE_CR:
			case ONE_LF:
			case TWO_CR:
				retval = read_header(current);
				break;
			case BODY_READ:
				retval = read_body(current);
				break;
			case BODY_WRITE:
				retval = write_body(current);
				break;
			case WRITE:
				retval = process_get(current);
				break;
			case PIPE_READ:
				retval = read_from_pipe(current);
				break;
			case PIPE_WRITE:
				retval = write_from_pipe(current);
				break;
			default:
				retval = 0;
#if 0
				fprintf(stderr, "Unknown status (%d), closing!\n",
						current->status);
#endif
				break;
			}
		}

		if (lame_duck_mode)
			SQUASH_KA(current);
		// Mason Yu
		//printf("retval=%d\n", retval);
		switch (retval) {
		case -1:				/* request blocked */
			trailer = current;
			current = current->next;
			block_request(trailer);
			break;
		default:			/* everything else means an error, jump ship */
			send_r_error(current);
			/* fall-through */
		case 0:				/* request complete */
			trailer = current;
			current = current->next;
			free_request(&request_ready, trailer);
#ifdef SUPPORT_ASP
			if(trailer->status==REBOOT)
			{
				cmd_reboot();
			}
#endif
			break;
		case 1:				/* more to do */
			current->time_last = time_counter;
			current = current->next;
			break;
		}
	}
#ifdef CRASHDEBUG
		crashdebug_current = current;
#endif
}

/*
 * Name: process_logline
 *
 * Description: This is called with the first req->header_line received
 * by a request, called "logline" because it is logged to a file.
 * It is parsed to determine request type and method, then passed to
 * translate_uri for further parsing.  Also sets up CGI environment if
 * needed.
 */

int process_logline(request * req)
{
	char *stop, *stop2;
	static char *SIMPLE_HTTP_VERSION = "HTTP/0.9";

	req->logline = req->header_line;
	req->method = request_type(req);
	if (req->method == M_INVALID || req->method == M_SHORT) {
#ifdef BOA_TIME_LOG
		log_error_time();
		fprintf(stderr, "malformed request: \"%s\"\n", req->logline);
#endif
		syslog(LOG_ERR, "malformed request: \"%s\" from %s\n", req->logline, req->remote_ip_addr);
		send_r_bad_request(req);
		return 0;
	}

	/* Guaranteed to find ' ' since we matched a method above */
	stop = req->logline + 3;
	if (*stop != ' ')
		++stop;

	/* scan to start of non-whitespace */
	while (*(++stop) == ' ');

	stop2 = stop;

	/* scan to end of non-whitespace */
	while (*stop2 != '\0' && *stop2 != ' ')
		++stop2;

	if (stop2 - stop > MAX_HEADER_LENGTH) {
#ifdef BOA_TIME_LOG
		log_error_time();
		fprintf(stderr, "URI too long %d: \"%s\"\n", MAX_HEADER_LENGTH,
				req->logline);
#endif
		syslog(LOG_ERR, "URI too long %d: \"%s\" from %s\n", MAX_HEADER_LENGTH,
				req->logline, req->remote_ip_addr);
		send_r_bad_request(req);
		return 0;
	}
	memcpy(req->request_uri, stop, stop2 - stop);
	req->request_uri[stop2 - stop] = '\0';

	if (*stop2 == ' ') {
		/* if found, we should get an HTTP/x.x */
		int p1, p2;

		if (sscanf(++stop2, "HTTP/%d.%d", &p1, &p2) == 2 && p1 >= 1) {
			req->http_version = stop2;
			req->simple = 0;
		} else {
#ifdef BOA_TIME_LOG
			log_error_time();
			fprintf(stderr, "bogus HTTP version: \"%s\"\n", stop2);
#endif
			syslog(LOG_ERR, "bogus HTTP version: \"%s\" from %s\n", stop2, req->remote_ip_addr);
			send_r_bad_request(req);
			return 0;
		}

	} else {
		req->http_version = SIMPLE_HTTP_VERSION;
		req->simple = 1;
	}

	if (req->method == M_HEAD && req->simple) {
		syslog(LOG_ERR, "Simple HEAD request not allowed from %s\n", req->remote_ip_addr);
		send_r_bad_request(req);
		return 0;
	}
	create_env(req);    /* create cgi env[], we don't know if url is cgi */
	return 1;
}

/*
 * Name: process_header_end
 *
 * Description: takes a request and performs some final checking before
 * init_cgi or init_get
 * Returns 0 for error or NPH, or 1 for success
 */

#define NO_REFERER_LOG
int process_header_end(request * req)
{
#ifdef CHECK_IP_MAC
	char mac[18];  /*Naughty, assuming we are on ethernet!!!*/
#endif /*CHECK_IP_MAC*/

	if (!req->logline) {
		send_r_error(req);
		return 0;
	}

	/*MATT2 - I figured this was a good place to check for the MAC address*/
#ifdef CHECK_IP_MAC
	if(get_mac_from_IP(mac, req->remote_ip_addr))
		do_mac_crap(req->remote_ip_addr, mac);
	else;
		/*they could be on a remote lan, or just not in the arp cache*/
#endif



#ifdef USE_BROWSERMATCH
	browser_match_request(req);
#endif

#ifndef NO_REFERER_LOG
	if (req->referer)
		log_referer(req);
#endif

#ifndef NO_AGENT_LOG
	if (req->user_agent)
		log_user_agent(req);
#endif

	if (translate_uri(req) == 0) {  /* unescape, parse uri */
		SQUASH_KA(req);
		return 0;               /* failure, close down */
	}

#ifdef WLAN_WEB_REDIRECT //jiunming,web_redirect
	if(req->request_from==1)
	{
		char tmpbuf[MAX_URL_LEN];

		if( !mib_get(MIB_WLAN_WEB_REDIR_URL, (void*)tmpbuf) )
		{
			send_r_error(req);
		}else{
			char tmpbuf2[MAX_URL_LEN+10], *phttp;

			phttp = strstr(tmpbuf, "http://");
			if( phttp==NULL )
				sprintf(tmpbuf2,"http://%s", tmpbuf);
			else
				sprintf(tmpbuf2,"%s", phttp);
			send_redirect_temp(req,tmpbuf2);
		}
		return 0;
	}
#endif

#ifdef WEB_REDIRECT_BY_MAC
	if(req->request_mac==1)
	{
		char tmpbuf[MAX_URL_LEN];
		char mac[18];

		memset( mac, 0, sizeof(mac) );
		if( get_mac_from_IP(mac, req->remote_ip_addr) )
		{
			//printf( "\na new connection from %s:%s \n", req->remote_ip_addr, mac );
			if( handle_mac_for_webredirect( mac )<0 )
			{
				send_r_error(req);
			}else{
				if( !mib_get(MIB_WEB_REDIR_BY_MAC_URL, (void*)tmpbuf) )
				{
					send_r_error(req);
				}else{
					char tmpbuf2[MAX_URL_LEN+10], *phttp;

					phttp = strstr(tmpbuf, "http://");
					if( phttp==NULL )
						sprintf(tmpbuf2,"http://%s", tmpbuf);
					else
						sprintf(tmpbuf2,"%s", phttp);
					//send_redirect_temp(req,tmpbuf2);
					send_popwin_and_reload( req,tmpbuf2 );
				}
			}
		}else
			send_r_error(req);
		return 0;
	}
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	if(req->request_captiveportal == 1)
	{
		char cp_url[MAX_URL_LEN+10];
		struct hostent *hp;

		if(mib_get(MIB_CAPTIVEPORTAL_URL, (void *)cp_url))
			send_redirect_temp(req, cp_url);
		else
			send_r_error(req);

		return 0;
	}
#endif

#ifdef USE_NLS
#ifdef USE_NLS_REFERER_REDIR
				if (!req->cp_name)
				{
					if (!nls_try_redirect(req))
						return 0;
				}
#endif
				nls_set_codepage(req);
#endif

#ifdef USE_AUTH
				closelog();
				openlog("boa", LOG_PID, LOG_AUTHPRIV);
				if (!auth_authorize(req)) {
					openlog("boa", LOG_PID, 0);
					return 0;
				}
#endif

	if (req->method == M_POST) {
		#ifdef CONFIG_HTTP_FILE
		char *tmpfilep = "/proc/rtk/http_buf";
		#else
		char tmpfilep[] = "/tmp/rtktmpXXXXXX";
		#endif
		if (!tmpfilep) {
#if 0
			boa_perror(req, "tmpnam");
#endif
			return 0;
		}
		/* open temp file for post data */
		if ((req->post_data_fd = mkstemp(tmpfilep)) == -1) {
#if 0
			boa_perror(req, "tmpfile open");
#endif
			return 0;
		}
		req->post_file_name = strdup(tmpfilep);
		return 1;
	}
	if (req->is_cgi)
#ifdef SUPPORT_ASP
		return init_get2(req);
#else
		return init_cgi(req);
#endif
	req->status = WRITE;
	return init_get(req);		/* get and head */
}

/*
 * Name: process_option_line
 *
 * Description: Parses the contents of req->header_line and takes
 * appropriate action.
 */

int process_option_line(request * req)
{
	char c, *value, *line = req->header_line;
	int eat_line = 0;

/* Start by aggressively hacking the in-place copy of the header line */

#ifdef FASCIST_LOGGING
	fprintf(stderr, "\"%s\"\n", line);
#endif

	value = strchr(line, ':');
	if (value == NULL)
		return 0;
	*value++ = '\0';			/* overwrite the : */
	to_upper(line);				/* header types are case-insensitive */
	while ((c = *value) && (c == ' ' || c == '\t'))
		value++;

	if (!memcmp(line, "IF_MODIFIED_SINCE", 18) && !req->if_modified_since)
		req->if_modified_since = value;

	else if (!memcmp(line, "CONTENT_TYPE", 13) && !req->content_type)
		req->content_type = value;

	else if (!memcmp(line, "CONTENT_LENGTH", 15) && !req->content_length && atoi(value) >= 0)
		req->content_length = value;

	else if (!memcmp(line, "HOST",5) && !req->host)
		req->host = value;

#ifndef NO_REFERER_LOG
	else if (!memcmp(line, "REFERER", 8) && !req->referer)
		req->referer = value;
#endif

#ifdef USE_AUTH
	else if (!memcmp(line,"AUTHORIZATION",14) && !req->authorization)
		req->authorization = value;
#endif

#ifndef NO_AGENT_LOG
        else if (!memcmp(line, "USER_AGENT", 11) && !req->user_agent)
                req->user_agent = value;
#endif

#ifndef NO_COOKIES
	else if (!memcmp(line, "COOKIE", 6) && !req->cookie) {
		req->cookie = value;
	}
#endif

	else if (!memcmp(line, "CONNECTION", 11) &&
			 ka_max &&
			 req->keepalive != KA_STOPPED)
#ifdef SUPPORT_ASP
		req->keepalive = KA_STOPPED;
#else
		req->keepalive = (!strncasecmp(value, "Keep-Alive", 10) ?
						  KA_ACTIVE : KA_STOPPED);
#endif

#ifdef ACCEPT_ON
	else if (!memcmp(line, "ACCEPT", 7)) {
		add_accept_header(req, value);
		eat_line = 1;
	}
#endif
	/* Silently ignore unknown header lines unless is_cgi */

	else  {
		add_cgi_env(req, line, value);
		eat_line = 1;
	}

	if (eat_line) {
		int throw = (req->header_end - req->header_line) + 1;
		memmove(req->header_line, req->header_end + 1,
				CLIENT_STREAM_SIZE -
						((req->header_end + 1) - req->client_stream));
		req->client_stream_pos -= throw;
		return(throw);
	}

	return 0;
}

/*
 * Name: add_accept_header
 * Description: Adds a mime_type to a requests accept char buffer
 *   silently ignore any that don't fit -
 *   shouldn't happen because of relative buffer sizes
 */

void add_accept_header(request * req, char *mime_type)
{
#ifdef ACCEPT_ON
	int l = strlen(req->accept);

	if ((strlen(mime_type) + l + 2) >= MAX_HEADER_LENGTH)
		return;

	if (req->accept[0] == '\0')
		strcpy(req->accept, mime_type);
	else {
		sprintf(req->accept + l, ", %s", mime_type);
	}
#endif
}

void free_requests(void)

{
	request *ptr, *next;

	ptr = request_free;
	while(ptr != NULL) {
		next = ptr->next;
		/*Free the socket stuff if it exists*/
#ifdef SUPPORT_ASP
		if(ptr->buffer) free(ptr->buffer);
#endif
		free(ptr);
		ptr = next;
	}
	request_free = NULL;
}

/*
 * Name: dump_request
 *
 * Description: Prints request to stderr for debugging purposes.
 */
void dump_request(request*req)
{
#if 0
	fputs("-----[ REQUEST DUMP ]-----\n",stderr);
	if (!req)
	{
		fputs("no request!\n",stderr);
		return;
	}
	fprintf(stderr,"Logline: %s\n",req->logline);
	fprintf(stderr,"request_uri: %s\n",req->request_uri);
	fprintf(stderr,"Pathname: %s\n",req->pathname);
	fprintf(stderr,"Status: %u\n",req->status);
	fprintf(stderr,"Host: %s\n",req->host);
	fprintf(stderr,"local_ip_addr: %s\n",req->local_ip_addr);
	fprintf(stderr,"remote_ip_addr: %s\n",req->remote_ip_addr);
#ifdef USE_NLS
	fprintf(stderr,"cp_name: %s\n",req->cp_name);
#endif
	fputs("---------------------------\n\n",stderr);
#endif
}


request *get_sock_request(int sock_fd)
{
	int fd;						/* socket */
#ifdef IPV6
	struct sockaddr_in6 remote_addr;
#else
	struct sockaddr_in remote_addr;		/* address */
#endif
	int remote_addrlen = sizeof(remote_addr);
	request *conn;				/* connection */

	if (max_connections != -1 && status.connections >= max_connections)
		return NULL;

#ifdef IPV6
	remote_addr.sin6_family = 0xdead;
#else
  remote_addr.sin_family = 0xdead;
#endif
	fd = accept(sock_fd, (struct sockaddr *) &remote_addr, &remote_addrlen);

	if (fd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)	/* no requests */
			return NULL;
		else {					/* accept error */
			log_error_time();
#if 0
			perror("accept");
#endif
			return NULL;
		}
	}
#ifdef DEBUGNONINET
	/*  This shows up due to race conditions in some Linux kernels
	 *  when the client closes the socket sometime between
	 *  the select() and accept() syscalls.
	 *  Code and description by Larry Doolittle <ldoolitt@jlab.org>
	 */
#define HEX(x) (((x)>9)?(('a'-10)+(x)):('0'+(x)))
	if (remote_addr.sin_family != AF_INET) {
		struct sockaddr *bogus = (struct sockaddr *) &remote_addr;
		char *ap, ablock[44];
		int i;
		close(fd);
#ifdef BOA_TIME_LOG
		log_error_time();
#endif
		for (ap = ablock, i = 0; i < remote_addrlen && i < 14; i++) {
			*ap++ = ' ';
			*ap++ = HEX((bogus->sa_data[i] >> 4) & 0x0f);
			*ap++ = HEX(bogus->sa_data[i] & 0x0f);
		}
		*ap = '\0';
#ifdef BOA_TIME_LOG
		fprintf(stderr, "non-INET connection attempt: socket %d, "
				"sa_family = %hu, sa_data[%d] = %s\n",
				fd, bogus->sa_family, remote_addrlen, ablock);
#endif
		return NULL;
	}
#endif

	if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &sock_opt,
		sizeof(sock_opt))) == -1){
			die(NO_SETSOCKOPT);
			return NULL;
	}

	conn = new_request();
	conn->fd = fd;
	conn->status = READ_HEADER;
	conn->header_line = conn->client_stream;
	conn->time_last = time_counter;
#ifdef USE_CHARSET_HEADER
	conn->send_charset = 1;
#endif

	/* nonblocking socket */
	if (fcntl(conn->fd, F_SETFL, NOBLOCK) == -1) {
#ifdef BOA_TIME_LOG
		log_error_time();
		perror("request.c, fcntl");
#endif
	}
	/* set close on exec to true */
	if (fcntl(conn->fd, F_SETFD, 1) == -1) {
#ifdef BOA_TIME_LOG
		log_error_time();
		perror("request.c, fcntl-close-on-exec");
#endif
	}

	/* large buffers */
	if (setsockopt(conn->fd, SOL_SOCKET, SO_SNDBUF, (void *) &sockbufsize,
				   sizeof(sockbufsize)) == -1)
		die(NO_SETSOCKOPT);

	/* for log file and possible use by CGI programs */
#ifdef IPV6
	if (getnameinfo((struct sockaddr *)&remote_addr,
									sizeof(remote_addr),
									conn->remote_ip_addr, 20,
								 	NULL, 0, NI_NUMERICHOST)) {
#ifdef _DEBUG_MESSAGE
			fprintf(stderr, "[IPv6] getnameinfo failed\n");
#endif
			conn->remote_ip_addr[0]=0;
		}
#else
	strncpy(conn->remote_ip_addr, (char *) inet_ntoa(remote_addr.sin_addr), 20);
#endif

	/* for possible use by CGI programs */
#ifdef IPV6
	conn->remote_port = ntohs(remote_addr.sin6_port);
#else
	conn->remote_port = ntohs(remote_addr.sin_port);
#endif

	if (virtualhost) {
#ifdef IPV6
		char host[20];
		struct sockaddr_in6 salocal;
		int dummy;

		dummy = sizeof(salocal);
		if (getsockname(conn->fd, (struct sockaddr *) &salocal, &dummy) == -1)
									      die(SERVER_ERROR);
			if (getnameinfo((struct sockaddr *)&salocal, sizeof(salocal),
			          host, 20, NULL, 0, NI_NUMERICHOST)) {
#ifdef _DEBUG_MESSAGE
				fprintf(stderr, "[IPv6] getnameinfo failed\n");
#endif
			}else
				conn->local_ip_addr = strdup(host);
#else
		struct sockaddr_in salocal;
		int dummy;

		dummy = sizeof(salocal);
		if (getsockname(conn->fd, (struct sockaddr *) &salocal, &dummy) == -1){
			die(SERVER_ERROR);
			return NULL;
		}
		conn->local_ip_addr = strdup(inet_ntoa(salocal.sin_addr));
#endif
	}
	status.requests++;
	status.connections++;

	/* Thanks to Jef Poskanzer <jef@acme.com> for this tweak */
	{
		int one = 1;
		if (setsockopt(conn->fd, IPPROTO_TCP, TCP_NODELAY, (void *) &one,
			sizeof(one)) == -1){
			die(NO_SETSOCKOPT);
			return NULL;
		}
	}
	enqueue(&request_ready, conn);
	return conn;
}
