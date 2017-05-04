/* pptp_ctrl.c ... handle PPTP control connection.
 *                 C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp_ctrl.c,v 1.3 2012/01/19 11:57:28 ql Exp $
 */

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <syslog.h>
#include <linux/if_ether.h>
#include "pptp_msg.h"
#include "pptp_ctrl.h"
#include "vector.h"
#include "pptp.h"
#include "pptp_options.h"


/* BECAUSE OF SIGNAL LIMITATIONS, EACH PROCESS CAN ONLY MANAGE ONE
 * CONNECTION.  SO THIS 'PPTP_CONN' STRUCTURE IS A BIT MISLEADING.
 * WE'LL KEEP CONNECTION-SPECIFIC INFORMATION IN THERE ANYWAY (AS
 * OPPOSED TO USING GLOBAL VARIABLES), BUT BEWARE THAT THE ENTIRE
 * UNIX SIGNAL-HANDLING SEMANTICS WOULD HAVE TO CHANGE (OR THE
 * TIME-OUT CODE DRASTICALLY REWRITTEN) BEFORE YOU COULD DO A 
 * PPTP_CONN_OPEN MORE THAN ONCE PER PROCESS AND GET AWAY WITH IT.
 */


#define INITIAL_BUFSIZE 512 /* initial i/o buffer size. */


struct PPTP_CONN {
	int inet_sock;

	/* Connection States */
	enum { 
		CONN_IDLE, CONN_WAIT_CTL_REPLY, CONN_WAIT_STOP_REPLY, CONN_ESTABLISHED 
	} conn_state; /* on startup: CONN_IDLE */
	/* Keep-alive states */
	enum { 
		KA_NONE, KA_OUTSTANDING 
	} ka_state;  /* on startup: KA_NONE */
	/* Keep-alive ID; monotonically increasing (watch wrap-around!) */
	u_int32_t ka_id; /* on startup: 1 */

	/* Other properties. */
	u_int16_t version;
	u_int16_t firmware_rev;
	u_int8_t  hostname[64], vendor[64];
	/* XXX these are only PNS properties, currently XXX */

	/* Call assignment information. */
	u_int16_t call_serial_number;

	VECTOR *call;

	void * closure;
	pptp_conn_cb callback;

	/******* IO buffers ******/
	char * read_buffer;//, *write_buffer;
	size_t read_alloc;//,   write_alloc;
	size_t read_size;//,    write_size;

	void *drv_ctl;
};

struct PPTP_CALL {
	/* Call properties */
	enum {
		PPTP_CALL_PAC, PPTP_CALL_PNS
	} call_type;
	union { 
		enum pptp_pac_state {
			PAC_IDLE, PAC_WAIT_REPLY, PAC_ESTABLISHED, PAC_WAIT_CS_ANS
		} pac;
		enum pptp_pns_state {
		  PNS_IDLE, PNS_WAIT_REPLY, PNS_ESTABLISHED, PNS_WAIT_DISCONNECT 
		} pns;
	} state;
	u_int16_t call_id, peer_call_id;
	u_int16_t sernum;
	u_int32_t speed;

	/* For user data: */
	pptp_call_cb callback;
	//void * closure;
};


/* Local prototypes */
static void pptp_reset_timer(PPTP_CONN * conn);
//static void pptp_handle_timer(PPTP_DRV_CTRL *p);
static void pptp_handle_timer(void *p);
/* Write/read as much as we can without blocking. */
//void pptp_write_some(PPTP_CONN * conn);
void pptp_read_some(PPTP_CONN * conn);
/* Make valid packets from read_buffer */
int pptp_make_packet(PPTP_CONN * conn, void **buf, size_t *size);
/* Add packet to write_buffer */
int pptp_send_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size);
/* Dispatch packets (general) */
void pptp_dispatch_packet(PPTP_CONN * conn, void * buffer, size_t size);
/* Dispatch packets (control messages) */
void pptp_dispatch_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size);
/*----------------------------------------------------------------------*/
/* Constructors and Destructors.                                        */

void pptp_start_ctrl_connection(PPTP_DRV_CTRL *p)
{
	PPTP_CONN *conn;
	struct pptp_start_ctrl_conn packet = {
		PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
		hton16(PPTP_VERSION), 0, 0, 
		hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
		hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION), 
		PPTP_HOSTNAME, PPTP_VENDOR
	};

	conn = p->conn;
	
	PPTP_DEBUG("send PPTP_START_CTRL_CONN_RQST");
	if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet)))
		conn->conn_state = CONN_WAIT_CTL_REPLY;

	pptp_change_state(p, PTP_ST_CTRL_INIT);

	TIMEOUT(pptp_handle_timer, p, p->ctrl_timeout, p->ctrl_ch);
}

void pptp_conn_init(PPTP_CONN *conn, PPTP_DRV_CTRL *p, pptp_conn_cb callback)
{
	conn->inet_sock = p->ctrl_sock;
	conn->conn_state= CONN_IDLE;
	conn->ka_state  = KA_NONE;
	conn->ka_id     = 1;
	conn->call_serial_number = 0;
	conn->callback  = callback;
	
	/* Make this socket non-blocking. */
	fcntl(conn->inet_sock, F_SETFL, O_NONBLOCK);

	pptp_start_ctrl_connection(p);
}

/* Open new pptp_connection.  Returns NULL on failure. */
PPTP_CONN * pptp_conn_open(PPTP_DRV_CTRL *p, int isclient, pptp_conn_cb callback)
{
	int inet_sock = p->ctrl_sock;
	PPTP_CONN *conn;
	/* Allocate structure */
	if ((conn = malloc(sizeof(*conn))) == NULL) return NULL;
	if ((conn->call = vector_create()) == NULL) { free(conn); return NULL; }
	
	if (!FD_ISSET(p->ctrl_sock, &pptp_in_fds))
		pptp_add_fd(p->ctrl_sock);
	
	/* Initialize */
	conn->inet_sock = inet_sock;
	conn->conn_state= CONN_IDLE;
	conn->ka_state  = KA_NONE;
	conn->ka_id     = 1;
	conn->call_serial_number = 0;
	conn->callback  = callback;
	/* Create I/O buffers */
	conn->read_size = /*conn->write_size = */0;
	conn->read_alloc= /*conn->write_alloc=*/ INITIAL_BUFSIZE;
	conn->read_buffer = malloc(sizeof(*(conn->read_buffer))*conn->read_alloc);
	//conn->write_buffer= malloc(sizeof(*(conn->write_buffer))*conn->write_alloc);
	if (conn->read_buffer == NULL/* || conn->write_buffer == NULL*/) {
		if (conn->read_buffer !=NULL) free(conn->read_buffer);
		//if (conn->write_buffer!=NULL) free(conn->write_buffer);
		vector_destroy(conn->call); free(conn); return NULL;
	}

	/* Make this socket non-blocking. */
	fcntl(conn->inet_sock, F_SETFL, O_NONBLOCK);

	/* Request connection from server, if this is a client */
	if (isclient) {
		struct pptp_start_ctrl_conn packet = {
			PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
			hton16(PPTP_VERSION), 0, 0, 
			hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
			hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION), 
			PPTP_HOSTNAME, PPTP_VENDOR
		};
		PPTP_DEBUG("send PPTP_START_CTRL_CONN_RQST");
		if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet)))
			conn->conn_state = CONN_WAIT_CTL_REPLY;
		else
			return NULL; /* could not send initial start request. */
	}

	pptp_change_state(p, PTP_ST_CTRL_INIT);

	TIMEOUT(pptp_handle_timer, p, p->ctrl_timeout, p->ctrl_ch);

	/* all done. */
	return conn;
}

/* This currently *only* works for client call requests.
 * We need to do something else to allocate calls for incoming requests.
 */
PPTP_CALL * pptp_call_open(PPTP_CONN * conn,
			   pptp_call_cb callback)
{
	PPTP_CALL * call;
	int i, scan_start, scan_end;
	char *cp;
	// assert(conn && conn->call);

	if (!conn)
		return(NULL);
	if (!conn->call) {
		PPTP_DEBUG("Connection is shutting down");
		return(NULL);
	}

	/* Assign call id */
	scan_start = 0;
	scan_end = PPTP_MAX_CHANNELS-1;

	if (!vector_scan(conn->call, 1, PPTP_MAX_CHANNELS-1, &i)) {
		PPTP_DEBUG("No more calls allowed");
	return NULL;
	}

	/* allocate structure. */
	if ((call = malloc(sizeof(*call)))==NULL) return NULL;
	/* Initialize call structure */
	call->call_type = PPTP_CALL_PNS;
	call->state.pns = PNS_IDLE;
	call->call_id   = (u_int16_t) i;
	call->sernum    = conn->call_serial_number++;
	call->callback  = callback;
	//call->closure   = NULL;
	/* Send off the call request */
	{
		struct pptp_out_call_rqst packet = {
			PPTP_HEADER_CTRL(PPTP_OUT_CALL_RQST),
			hton16(call->call_id), hton16(call->sernum),
			hton32(PPTP_BPS_MIN), hton32(PPTP_BPS_MAX),
			hton32(PPTP_BEARER_CAP), hton32(PPTP_FRAME_CAP), 
			hton16(PPTP_WINDOW), 0, 0, 0, {0}, {0}
		};
		PPTP_DEBUG("send PPTP_OUT_CALL_RQST");
		if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet))) {
			pptp_reset_timer(conn);
			call->state.pns = PNS_WAIT_REPLY;
			/* and add it to the call vector */
			vector_insert(conn->call, i, call);
			return call;
		} else { /* oops, unsuccessful. Deallocate. */
			PPTP_DEBUG("Failed to send control packet");
			free(call);
			return NULL;
		}
	}
}

void pptp_call_close(PPTP_CONN * conn, PPTP_CALL * call) {
	struct pptp_call_clear_rqst rqst = {
		PPTP_HEADER_CTRL(PPTP_CALL_CLEAR_RQST), 0, 0
	};
	
	if (!conn || !conn->call || !call)
		return;
	// assert(conn && conn->call); assert(call);
	assert(vector_contains(conn->call, call->call_id));
	assert(call->call_type == PPTP_CALL_PNS); /* haven't thought about PAC yet */
	assert(call->state.pns != PNS_IDLE);
	rqst.call_id = hton16(call->call_id);

	/* don't check state against WAIT_DISCONNECT... allow multiple disconnect
	* requests to be made.
	*/
	PPTP_DEBUG("send PPTP_CALL_CLEAR_RQST");
	pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
	pptp_reset_timer(conn);
	call->state.pns = PNS_WAIT_DISCONNECT;

	/* call structure will be freed when we have confirmation of disconnect. */
}
/* hard close. */
void pptp_call_destroy(PPTP_CONN *conn, PPTP_CALL *call) {
	// assert(conn && conn->call); assert(call);
	if (!conn || !conn->call || !call)
		return;
	assert(vector_contains(conn->call, call->call_id));
	/* notify */
	if (call->callback!=NULL) call->callback(conn, call, CALL_CLOSE_DONE);
	/* deallocate */
	vector_remove(conn->call, call->call_id);
	memset(call, 0, sizeof(*call));
	free(call);
}
/* this is a soft close */
void pptp_conn_close(PPTP_CONN * conn, u_int8_t close_reason)
{
	struct pptp_stop_ctrl_conn rqst = {
		PPTP_HEADER_CTRL(PPTP_STOP_CTRL_CONN_RQST), 
		hton8(close_reason), 0, 0};
	int i;

	// assert(conn && conn->call);
	if (!conn)
		return;

	/* avoid repeated close attempts */
	if (conn->conn_state==CONN_IDLE || conn->conn_state==CONN_WAIT_STOP_REPLY) 
		return;

	/* close open calls, if any */
	for (i=0; conn->call && i<vector_size(conn->call); i++)
		pptp_call_close(conn, vector_get_Nth(conn->call, i));

	/* now close connection */
	PPTP_DEBUG("Closing PPTP connection");
	PPTP_DEBUG("send PPTP_STOP_CTRL_CONN_RQST");
	pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
	pptp_reset_timer(conn); /* wait 60 seconds for reply */
	conn->conn_state = CONN_WAIT_STOP_REPLY;
	return;
}
/* this is a hard close */
void pptp_conn_destroy(PPTP_CONN * conn, int dispose)
{
	int i;
	// assert(conn != NULL);
	// assert(conn->call != NULL);
	if (!conn)
		return;

	/* destroy all open calls */
	if (dispose) {
		for (i=0; conn->call && i<vector_size(conn->call); i++)
			pptp_call_destroy(conn, vector_get_Nth(conn->call, i));
		/* notify */
		if (conn->callback!=NULL) {
			conn->callback(conn, CONN_CLOSE_DONE);
			conn->callback = NULL;
		}

		if (conn->inet_sock >= 0) {
			PPTP_DEBUG("closing %d (inet)", conn->inet_sock);
			close(conn->inet_sock);
			pptp_remove_fd(conn->inet_sock);
			conn->inet_sock = -1;
		}
		/* deallocate */
		if (conn->call != NULL) {
			vector_destroy(conn->call);
			conn->call = NULL;
		}
		//if (dispose) {
			memset(conn, 0, sizeof(*conn));
			free(conn);
		//}
	}
	else
		pptp_down(conn->drv_ctl);
}

/************** Deal with messages, in a non-blocking manner *************/

/* Add file descriptors used by pptp to fd_set. */
void pptp_fd_set(PPTP_CONN * conn, fd_set * read_set, fd_set * write_set)
{
	// assert(conn && conn->call);
	if (!conn)
		return;
	#if 0
	/* Add fd to write_set if there are outstanding writes. */
	if (conn->inet_sock >= 0 && conn->write_size > 0)
		FD_SET(conn->inet_sock, write_set);
	#endif
	/* Always add fd to read_set. (always want something to read) */
	if (conn->inet_sock >= 0)
		FD_SET(conn->inet_sock, read_set);
}

/* handle any pptp file descriptors set in fd_set, and clear them */
void pptp_dispatch(PPTP_CONN * conn, fd_set * read_set, fd_set * write_set)
{
	// assert(conn && conn->call);
	if (!conn)
		return;
	#if 0
	/* Check write_set could be set. */
	if (conn->inet_sock >= 0 && FD_ISSET(conn->inet_sock, write_set)) {
		FD_CLR(conn->inet_sock, write_set);
		if (conn->write_size > 0)
			pptp_write_some(conn); /* write as much as we can without blocking */
	}
	#endif
	/* Check read_set */
	if (conn->inet_sock >= 0 && FD_ISSET(conn->inet_sock, read_set)) {
		void *buffer; size_t size;
		FD_CLR(conn->inet_sock, read_set);
		pptp_read_some(conn); /* read as much as we can without blocking */
		/* make packets of the buffer, while we can. */
		while (pptp_make_packet(conn, &buffer, &size)) {
			pptp_dispatch_packet(conn, buffer, size);
			free(buffer);
		}
	}
	/* That's all, folks.  Simple, eh? */
}

#if 0
/********************* Non-blocking I/O buffering *********************/
void pptp_write_some(PPTP_CONN * conn)
{
	ssize_t retval;
	// assert(conn && conn->call);
	if (!conn || conn->inet_sock == -1)
		return;
	retval = write(conn->inet_sock, conn->write_buffer, conn->write_size);
	if (retval < 0) { /* error. */
		if (errno == EAGAIN || errno == EINTR) { 
			/* ignore */;
		} else { /* a real error */
			PPTP_DEBUG("write error: %s", strerror(errno));
			pptp_conn_destroy(conn, 0); /* shut down fast. */
		}
		return;
	}
	assert(retval <= conn->write_size);
	conn->write_size-=retval;
	memmove(conn->write_buffer, conn->write_buffer+retval, conn->write_size);
}
#endif

void pptp_read_some(PPTP_CONN * conn)
{
	ssize_t retval;
	// assert(conn && conn->call);
	if (!conn || conn->inet_sock == -1)
		return;
	if (conn->read_size == conn->read_alloc) { /* need to alloc more memory */
		char *new_buffer=realloc(conn->read_buffer, 
			sizeof(*(conn->read_buffer))*conn->read_alloc*2);
		if (new_buffer == NULL) {
			PPTP_DEBUG("Out of memory\n"); return;
		}
		conn->read_alloc*=2;
		conn->read_buffer = new_buffer;
	}
	retval = read(conn->inet_sock, 	conn->read_buffer + conn->read_size, 
		conn->read_alloc  - conn->read_size);
	if (retval < 0) {
		if (errno == EINTR || errno == EAGAIN)
			/* ignore */ ;
		else { /* a real error */
			PPTP_DEBUG("read error %d: %s\n", conn->inet_sock, strerror(errno));
			pptp_conn_destroy(conn, 0); /* shut down fast. */
		}
		return;
	}
	#if 0
	else if (retval == 0) {
		PPTP_DEBUG("Control connection closed by peer\n");
		pptp_conn_destroy(conn, 0); /* shut down fast. */
	}
	#endif
	conn->read_size += retval;
	assert(conn->read_size <= conn->read_alloc);
}

/********************* Packet formation *******************************/
int pptp_make_packet(PPTP_CONN * conn, void **buf, size_t *size)
{
	struct pptp_header *header;
	size_t bad_bytes = 0;
	// assert(conn && conn->call);

	if (!conn || !conn->call)
		return(0);
	assert(buf != NULL); assert(size != NULL);

	/* Give up unless there are at least sizeof(pptp_header) bytes */
	while ((conn->read_size-bad_bytes) >= sizeof(struct pptp_header)) {
		/* Throw out bytes until we have a valid header. */
		header = (struct pptp_header *) (conn->read_buffer+bad_bytes);
		if (ntoh32(header->magic) != PPTP_MAGIC) goto throwitout;
		if (ntoh16(header->length) < sizeof(struct pptp_header)) goto throwitout;
		if (ntoh16(header->length) > PPTP_CTRL_SIZE_MAX) goto throwitout;

		/* At least one PPTP server implementation (Cisco) ignores RFC 2637's
		* requirement that Reserved0 "MUST be 0". In the interests of
		* interoperability, we'll just log this and pretend it never happened.
		*/
		if (ntoh16(header->reserved0)!=0) {
			PPTP_DEBUG("Protocol violation: header->reserved0(0x%x) != 0",
			ntoh16(header->reserved0));
		}

		/* well.  I guess it's good. Let's see if we've got it all. */
		if (ntoh16(header->length) > (conn->read_size-bad_bytes))
			/* nope.  Let's wait until we've got it, then. */
			goto flushbadbytes;
		/* One last check: */
		if ((ntoh16(header->pptp_type) == PPTP_MESSAGE_CONTROL) &&
			(ntoh16(header->length) != PPTP_CTRL_SIZE(ntoh16(header->ctrl_type))))
			goto throwitout;

		/* well, I guess we've got it. */
		*size= ntoh16(header->length);
		*buf = malloc(*size);
		if (*buf == NULL) { PPTP_DEBUG("Out of memory."); return 0; /* ack! */ }
		memcpy(*buf, conn->read_buffer, *size);
		/* Delete this packet from the read_buffer. */
		conn->read_size -= (bad_bytes + *size);
		memmove(conn->read_buffer, conn->read_buffer+bad_bytes+*size, conn->read_size);
		if (bad_bytes > 0) 
			PPTP_DEBUG("%lu bad bytes thrown away.", (unsigned long) bad_bytes);
		return 1;

throwitout:
		bad_bytes++;
	}
flushbadbytes:
	/* no more packets.  Let's get rid of those bad bytes */
	conn->read_size -= bad_bytes;
	memmove(conn->read_buffer, conn->read_buffer+bad_bytes, conn->read_size);
	if (bad_bytes > 0) 
		PPTP_DEBUG("%lu bad bytes thrown away.", (unsigned long) bad_bytes);
	return 0;
}

int pptp_send_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size)
{
	ssize_t retval;
	
	// assert(conn && conn->call);
	if (!conn)
		return 0;
	assert(buffer);
	
	#if 0
	/* Shove this into the write buffer */
	if (conn->write_size + size > conn->write_alloc) { /* need more memory */
		char *new_buffer=realloc(conn->write_buffer, 
				sizeof(*(conn->write_buffer))*conn->write_alloc*2);
		if (new_buffer == NULL) {
			PPTP_DEBUG("Out of memory"); return 0;
		}
		conn->write_alloc*=2;
		conn->write_buffer = new_buffer;
	}
	memcpy(conn->write_buffer + conn->write_size, buffer, size);
	conn->write_size+=size;
	#endif
	if (conn->inet_sock < 0)
		return 0;
	
	retval = write(conn->inet_sock, buffer, size);
	if (retval < 0) { /* error. */
		PPTP_DEBUG("write error: %s", strerror(errno));
	}
	return 1;
}

/********************** Packet Dispatch ****************************/
void pptp_dispatch_packet(PPTP_CONN * conn, void * buffer, size_t size)
{
	struct pptp_header *header = (struct pptp_header *)buffer;
	assert(conn && conn->call); assert(buffer);
	assert(ntoh32(header->magic)==PPTP_MAGIC);
	assert(ntoh16(header->length)==size);

	switch (ntoh16(header->pptp_type)) {
		case PPTP_MESSAGE_CONTROL:
			pptp_dispatch_ctrl_packet(conn, buffer, size);
			break;
		case PPTP_MESSAGE_MANAGE:
			/* MANAGEMENT messages aren't even part of the spec right now. */
			PPTP_DEBUG("PPTP management message received, but not understood.");
			break;
		default:
			PPTP_DEBUG("Unknown PPTP control message type received: %u", 
			(unsigned) ntoh16(header->pptp_type));
			break;
	}
}

void pptp_dispatch_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size)
{
	struct pptp_header *header = (struct pptp_header *)buffer;
	u_int8_t close_reason = PPTP_STOP_NONE;

	assert(conn && conn->call); assert(buffer);
	assert(ntoh32(header->magic)==PPTP_MAGIC);
	assert(ntoh16(header->length)==size);
	assert(ntoh16(header->pptp_type)==PPTP_MESSAGE_CONTROL);

	if (size < PPTP_CTRL_SIZE(ntoh16(header->ctrl_type))) {
		PPTP_DEBUG("Invalid packet received [type: %d; length: %d].",
			(int) ntoh16(header->ctrl_type), (int) size);
		return;
	}

	switch (ntoh16(header->ctrl_type)) {
	/* ----------- STANDARD Start-Session MESSAGES ------------ */
	case PPTP_START_CTRL_CONN_RQST:
		{
			struct pptp_start_ctrl_conn *packet = 
				(struct pptp_start_ctrl_conn *) buffer;
			struct pptp_start_ctrl_conn reply = {
				PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RPLY),
				hton16(PPTP_VERSION), 0, 0,
				hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
				hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION),
				PPTP_HOSTNAME, PPTP_VENDOR };

			PPTP_DEBUG("recv PPTP_START_CTRL_CONN_RQST");
			if (conn->conn_state == CONN_IDLE) {
				if (ntoh16(packet->version) < PPTP_VERSION) {
					/* Can't support this (earlier) PPTP_VERSION */
					reply.version = packet->version;
					reply.result_code = hton8(5); /* protocol version not supported */
					PPTP_DEBUG("send PPTP_START_CTRL_CONN_RPLY");
					pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
					pptp_reset_timer(conn); /* give sender a chance for a retry */
				} else { /* same or greater version */
					PPTP_DEBUG("send PPTP_START_CTRL_CONN_RPLY");
					if (pptp_send_ctrl_packet(conn, &reply, sizeof(reply))) {
						conn->conn_state=CONN_ESTABLISHED;
						pptp_change_state( conn->drv_ctl, PTP_ST_CTRL_ESTABLISH);
						PPTP_DEBUG("server connection ESTABLISHED.");
						pptp_reset_timer(conn);
					}
				}
			}
		break;
		}
	case PPTP_START_CTRL_CONN_RPLY:
		{
			struct pptp_start_ctrl_conn *packet = 
				(struct pptp_start_ctrl_conn *) buffer;
			
			PPTP_DEBUG("recv PPTP_START_CTRL_CONN_RPLY");
			if (conn->conn_state == CONN_WAIT_CTL_REPLY) {
				/* XXX handle collision XXX [see rfc] */
				if (ntoh16(packet->version) != PPTP_VERSION) {
					if (conn->callback!=NULL) conn->callback(conn, CONN_OPEN_FAIL);
					close_reason = PPTP_STOP_PROTOCOL;
					goto pptp_err_conn_close;
				}
				if (ntoh8(packet->result_code)!=1) { /* some problem with start */
					/* if result_code == 5, we might fall back to different version */
					if (conn->callback!=NULL) conn->callback(conn, CONN_OPEN_FAIL);
					close_reason = PPTP_STOP_PROTOCOL;
					goto pptp_err_conn_close;
				}
				conn->conn_state = CONN_ESTABLISHED;
				pptp_change_state( conn->drv_ctl, PTP_ST_CTRL_ESTABLISH);

				/* log session properties */
				conn->version      = ntoh16(packet->version);
				conn->firmware_rev = ntoh16(packet->firmware_rev);
				memcpy(conn->hostname, packet->hostname, sizeof(conn->hostname));
				memcpy(conn->vendor, packet->vendor, sizeof(conn->vendor));

				pptp_reset_timer(conn); /* 60 seconds until keep-alive */
				PPTP_DEBUG("Client connection established.");

				if (conn->callback!=NULL) conn->callback(conn, CONN_OPEN_DONE);
			} /* else goto pptp_err_conn_close; */
			break;
		}
	/* ----------- STANDARD Stop-Session MESSAGES ------------ */
	case PPTP_STOP_CTRL_CONN_RQST:
		{
			struct pptp_stop_ctrl_conn *packet = /* XXX do something with this XXX */
				(struct pptp_stop_ctrl_conn *) buffer; 
			/* conn_state should be CONN_ESTABLISHED, but it could be 
			* something else */

			PPTP_DEBUG("recv PPTP_STOP_CTRL_CONN_RQST");
			struct pptp_stop_ctrl_conn reply = {
				PPTP_HEADER_CTRL(PPTP_STOP_CTRL_CONN_RPLY), 
				hton8(1), hton8(PPTP_GENERAL_ERROR_NONE), 0};

			if (conn->conn_state==CONN_IDLE) break;
			PPTP_DEBUG("send PPTP_STOP_CTRL_CONN_RPLY");
			if (pptp_send_ctrl_packet(conn, &reply, sizeof(reply))) {
				if (conn->callback!=NULL) conn->callback(conn, CONN_CLOSE_RQST);
				conn->conn_state=CONN_IDLE;
				pptp_conn_destroy(conn, 0);
			}
			break;
		}
	case PPTP_STOP_CTRL_CONN_RPLY:
		{
			struct pptp_stop_ctrl_conn *packet = /* XXX do something with this XXX */
				(struct pptp_stop_ctrl_conn *) buffer;
			/* conn_state should be CONN_WAIT_STOP_REPLY, but it 
			* could be something else */

			PPTP_DEBUG("recv PPTP_STOP_CTRL_CONN_RPLY");
			if (conn->conn_state == CONN_IDLE) break;
			conn->conn_state=CONN_IDLE;
			pptp_conn_destroy(conn, 0);
			break;
		}
	/* ----------- STANDARD Echo/Keepalive MESSAGES ------------ */
	case PPTP_ECHO_RPLY:
		{
			struct pptp_echo_rply *packet = 
				(struct pptp_echo_rply *) buffer;
			//PPTP_DEBUG("recv PPTP_ECHO_RPLY");
			if ((conn->ka_state == KA_OUTSTANDING) && 
				(ntoh32(packet->identifier)==conn->ka_id)) {
				conn->ka_id++;
				conn->ka_state=KA_NONE;
				pptp_reset_timer(conn);
			}
			break;
		}
	case PPTP_ECHO_RQST:
		{
			struct pptp_echo_rqst *packet = 
				(struct pptp_echo_rqst *) buffer;
			struct pptp_echo_rply reply = {
				PPTP_HEADER_CTRL(PPTP_ECHO_RPLY), 
				packet->identifier, /* skip hton32(ntoh32(id)) */
				hton8(1), hton8(PPTP_GENERAL_ERROR_NONE), 0};
				
			PPTP_DEBUG("recv PPTP_ECHO_RQST");
			PPTP_DEBUG("send PPTP_ECHO_RPLY");
			pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
			pptp_reset_timer(conn);
			break;
		}
	/* ----------- OUTGOING CALL MESSAGES ------------ */
	case PPTP_OUT_CALL_RQST:
		{
			struct pptp_out_call_rqst *packet =
				(struct pptp_out_call_rqst *)buffer;
			struct pptp_out_call_rply reply = {
				PPTP_HEADER_CTRL(PPTP_OUT_CALL_RPLY),
				0 /* callid */, packet->call_id, 1, PPTP_GENERAL_ERROR_NONE, 0,
				hton32(PPTP_CONNECT_SPEED), 
				hton16(PPTP_WINDOW), hton16(PPTP_DELAY), 0 };
			/* XXX PAC: eventually this should make an outgoing call. XXX */
			PPTP_DEBUG("recv PPTP_OUT_CALL_RQST");
			reply.result_code = hton8(7); /* outgoing calls verboten */
			PPTP_DEBUG("send PPTP_OUT_CALL_RPLY");
			pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
			break;
		}
	case PPTP_OUT_CALL_RPLY:
		{
			struct pptp_out_call_rply *packet =
				(struct pptp_out_call_rply *)buffer;
			struct pptp_set_link_info reply = {
				PPTP_HEADER_CTRL(PPTP_SET_LINK_INFO),
				packet->call_id, 0, 0xFFFFFFFF, 0xFFFFFFFF };
			PPTP_CALL * call;
			u_int16_t callid = ntoh16(packet->call_id_peer);
			PPTP_DEBUG("recv PPTP_OUT_CALL_RPLY");
			if (!vector_search(conn->call, (int) callid, &call)) {
				PPTP_DEBUG("PPTP_OUT_CALL_RPLY received for non-existant call.");
				break;
			}
			if (call->call_type!=PPTP_CALL_PNS) {
				PPTP_DEBUG("Ack!  How did this call_type get here?"); /* XXX? */
				break; 
			}
			if (call->state.pns == PNS_WAIT_REPLY) {
				/* check for errors */
				if (packet->result_code!=1) {
					/* An error.  Log it. */
					PPTP_DEBUG("Error opening call. [callid %d]", (int) callid);
					call->state.pns = PNS_IDLE;
					if (call->callback!=NULL) call->callback(conn, call, CALL_OPEN_FAIL);
					pptp_call_destroy(conn, call);
				} else {
					/* connection established */
					call->state.pns = PNS_ESTABLISHED;
					call->peer_call_id = ntoh16(packet->call_id);
					call->speed        = ntoh32(packet->speed);
					pptp_reset_timer(conn);
					if (call->callback!=NULL) call->callback(conn, call, CALL_OPEN_DONE);
					PPTP_DEBUG("send PPTP_SET_LINK_INFO");
					pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
					PPTP_DEBUG("Outgoing call established.");
				}
			}
			break;
		}
		/* ----------- INCOMING CALL MESSAGES ------------ */
		/* XXX write me XXX */
		/* ----------- CALL CONTROL MESSAGES ------------ */
	case PPTP_CALL_CLEAR_RQST:
		{
			struct pptp_call_clear_rqst *packet =
				(struct pptp_call_clear_rqst *)buffer;
			struct pptp_call_clear_ntfy reply = {
				PPTP_HEADER_CTRL(PPTP_CALL_CLEAR_NTFY), packet->call_id,
				1, PPTP_GENERAL_ERROR_NONE, 0, 0, {0}
				};
			PPTP_DEBUG("recv PPTP_CALL_CLEAR_RQST");
			if (vector_contains(conn->call, ntoh16(packet->call_id))) {
				PPTP_CALL * call;
				vector_search(conn->call, ntoh16(packet->call_id), &call);
				if (call->callback!=NULL) call->callback(conn, call, CALL_CLOSE_RQST);
				PPTP_DEBUG("send PPTP_CALL_CLEAR_NTFY");
				pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
				pptp_call_destroy(conn, call);
				PPTP_DEBUG("Call closed (RQST) (call id %d)", (int) call->call_id);
			}
			break;
		}
	case PPTP_CALL_CLEAR_NTFY:
		{
			struct pptp_call_clear_ntfy *packet =
				(struct pptp_call_clear_ntfy *)buffer;
			PPTP_DEBUG("recv PPTP_CALL_CLEAR_NTFY");
			if (vector_contains(conn->call, ntoh16(packet->call_id))) {
				PPTP_CALL * call;
				vector_search(conn->call, ntoh16(packet->call_id), &call);
				pptp_call_destroy(conn, call);
				PPTP_DEBUG("Call closed (NTFY) (call id %d)", (int) call->call_id);
			}
			/* XXX we could log call stats here XXX */
			break;
		}
	case PPTP_SET_LINK_INFO:
		{
			/* I HAVE NO CLUE WHAT TO DO IF send_accm IS NOT 0! */
			/* this is really dealt with in the HDLC deencapsulation, anyway. */
			struct pptp_set_link_info *packet =
				(struct pptp_set_link_info *)buffer;
			PPTP_DEBUG("recv PPTP_SET_LINK_INFO");
			if (ntoh32(packet->send_accm)==0 && ntoh32(packet->recv_accm)==0)
				break; /* this is what we expect. */
			/* log it, otherwise. */
			PPTP_DEBUG("PPTP_SET_LINK_INFO recieved from peer_callid %u",
				(unsigned) ntoh16(packet->call_id_peer));
			PPTP_DEBUG("  send_accm is %08lX, recv_accm is %08lX",
				(unsigned long) ntoh32(packet->send_accm),
				(unsigned long) ntoh32(packet->recv_accm));
			break;
		}
	default:
		PPTP_DEBUG("Unrecognized Packet %d received.", 
			(int) ntoh16(((struct pptp_header *)buffer)->ctrl_type));
		/* goto pptp_err_conn_close; */
		break;
	}
	return;

pptp_err_conn_close:
	PPTP_DEBUG("pptp_conn_close(%d)", (int) close_reason);
	pptp_conn_close(conn, close_reason);
}


/********************* Get info from call structure *******************/

/* NOTE: The peer_call_id is undefined until we get a server response. */
void pptp_call_get_ids(PPTP_CONN * conn, PPTP_CALL * call,
		       u_int16_t * call_id, u_int16_t * peer_call_id) {
	assert(conn!=NULL); assert(call != NULL);
	*call_id = call->call_id;
	*peer_call_id = call->peer_call_id;
}

#if 0
void   pptp_call_closure_put(PPTP_CONN * conn, PPTP_CALL * call, void *cl) {
	assert(conn != NULL); assert(call != NULL);
	call->closure = cl;
}
void * pptp_call_closure_get(PPTP_CONN * conn, PPTP_CALL * call) {
	assert(conn != NULL); assert(call != NULL);
	return call->closure;
}
#endif

void   pptp_conn_closure_put(PPTP_CONN * conn, void *cl) {
	assert(conn != NULL);
	//printf("%s %d set conn->closure=%x\n", __func__, __LINE__, (unsigned int)cl);
	conn->closure = cl;
}
void * pptp_conn_closure_get(PPTP_CONN * conn) {
	assert(conn != NULL);
	return conn->closure;
}

void pptp_drv_ctl_put(PPTP_CONN *conn, PPTP_DRV_CTRL *p)
{
	assert(conn != NULL); assert(p != NULL);
	conn->drv_ctl = p;
}

int pptp_conn_down(PPTP_CONN * conn) {
	assert(conn != NULL);
	return(!conn->call);
}

int pptp_conn_established(PPTP_CONN * conn) {
	assert(conn != NULL);
	return(conn->conn_state == CONN_ESTABLISHED);
}

/*********************** Handle keep-alive timer ***************/
static void pptp_reset_timer(PPTP_CONN * conn)
{
	PPTP_DRV_CTRL *p;

	p = conn->drv_ctl;
	UNTIMEOUT(pptp_handle_timer, p, p->ctrl_ch);
	TIMEOUT(pptp_handle_timer, p, p->ctrl_timeout, p->ctrl_ch);
}

static void pptp_handle_timer(/*PPTP_DRV_CTRL*/void *p)
{
	PPTP_DRV_CTRL *ptpr = (PPTP_DRV_CTRL *)p;
	PPTP_CONN * conn=NULL;
	int i;

	conn = (PPTP_CONN *)ptpr->conn;
	if (!conn) /* things have disappeared under us */
		return;

	/* "Keep Alives and Timers, 1": check connection state */
	if (conn->conn_state != CONN_ESTABLISHED) {
		if (conn->conn_state == CONN_WAIT_STOP_REPLY) {
			/* hard close. */
			pptp_conn_destroy(conn, 0);
			return; /* not much to do after we destroy it ! */
		} else /* soft close */
			pptp_conn_close(conn, PPTP_STOP_NONE);
	}
	/* "Keep Alives and Timers, 2": check echo status */
	if (conn->ka_state == KA_OUTSTANDING) /*no response to keep-alive*/
		pptp_conn_close(conn, PPTP_STOP_NONE);
	else { /* ka_state == NONE */ /* send keep-alive */
		struct pptp_echo_rqst rqst = {
			PPTP_HEADER_CTRL(PPTP_ECHO_RQST), hton32(conn->ka_id) };
		//PPTP_DEBUG("send PPTP_ECHO_RQST");
		pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
		conn->ka_state = KA_OUTSTANDING;
		/* XXX FIXME: wake up ctrl thread -- or will the SIGALRM do that
		* automagically? XXX
		*/
	}
	/* check incoming/outgoing call states for !IDLE && !ESTABLISHED */
	for (i=0; i<vector_size(conn->call); i++) {
		PPTP_CALL * call = vector_get_Nth(conn->call, i);
		if (call->call_type == PPTP_CALL_PNS) {
			if (call->state.pns==PNS_WAIT_REPLY) {
				/* send close request */
				pptp_call_close(conn, call);
				assert(call->state.pns==PNS_WAIT_DISCONNECT);
			} else if (call->state.pns==PNS_WAIT_DISCONNECT) {
				/* hard-close the call */
				pptp_call_destroy(conn, call);
			}
		} else if (call->call_type == PPTP_CALL_PAC) {
			if (call->state.pac == PAC_WAIT_REPLY) {
				/* XXX FIXME -- drop the PAC connection XXX */
			} else if (call->state.pac == PAC_WAIT_CS_ANS) {
				/* XXX FIXME -- drop the PAC connection XXX */
			}
		}
	}
	pptp_reset_timer(conn);
}

PPTP_DRV_CTRL * pptp_get_drv_ctrl(PPTP_CONN * conn)
{
	return conn->drv_ctl;
}
#if 0
void pptp_handle_state( PPTP_DRV_CTRL *p)
{
	PPTP_CONN * conn;

	conn = p->conn;
	switch (p->callctrlState)
	{
	case PTP_ST_DEAD:
		if (CONN_WAIT_CTL_REPLY == conn->conn_state)
			pptp_change_state( p, PTP_ST_CTRL_INIT );
		break;
	case PTP_ST_CTRL_INIT:
		if (CONN_ESTABLISHED == conn->conn_state)
			pptp_change_state( p, PTP_ST_CTRL_ESTABLISH);
		break;
	case PTP_ST_CTRL_ESTABLISH:
		break;
	case PTP_ST_CALL_INIT:
		{
			PPTP_CALL *call;
			int size = vector_size(conn->call);
			int i;

			for (i=0; i<size; i++) {
				call = vector_get_Nth(conn->call, i);
				if (call->state.pns == PNS_ESTABLISHED)
					pptp_change_state( p, PTP_ST_CALL_ESTABLISH );
			}

			if (0 == size)
				pptp_change_state( p, PTP_ST_CALL_FAIL );
		}
	case PTP_ST_CALL_FAIL:
	case PTP_ST_CALL_ESTABLISH:
	default:
		break;
	}
}
#endif

