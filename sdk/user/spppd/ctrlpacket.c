/*
 * ctrlpacket.c
 *
 * PPTP Control Message packet reading, formatting and writing.
 *
 * $Id: ctrlpacket.c,v 1.1 2012/08/27 07:44:19 ql Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "pptpdefs.h"
#include "ctrlpacket.h"
#include "pptpmanager.h"
#include "if_sppp.h"


/* Local function prototypes */
static ssize_t read_pptp_header(struct sppp *sp, unsigned char *packet, int *ctrl_message_type);
static void deal_start_ctrl_conn(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size);
static void deal_stop_ctrl_conn(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size);
static void deal_out_call(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size);
static void deal_echo(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size);
static void deal_call_clr(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size);
static void deal_set_link_info(unsigned char *packet);
static u_int16_t getcall();
static u_int16_t freecall();

#if notyet
static int make_out_call_rqst(unsigned char *rply_packet, ssize_t * rply_size);
#endif

/*
 * read_pptp_packet
 *
 * Sees if a packet can be read and if so what type of packet it is. The
 * method then calls the appropriate function to examine the details of the
 * packet and form a suitable reply packet.
 *
 * args:        clientFd (IN) - Client socket to read from.
 *              packet (OUT) - Packet read from the client.
 *              rply_packet (OUT) - Reply packet for the client.
 *              rply_size (OUT) - Size of the reply packet.
 *
 * retn:        PPTP control message type of the packet on success.
 *              -1 on retryable error.
 *              0 on error to abort on.
 */
int read_pptp_packet(struct sppp *sp, unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size)
{
	size_t bytes_read=0;
	int pptp_ctrl_type;	/* Control Message Type */

	/* read a packet and parse header */
	if ((bytes_read = read_pptp_header(sp, packet, &pptp_ctrl_type)) <= 0) {
		/* error reading packet */
		syslog(LOG_ERR, "CTRL: couldn't read packet header (%s)", bytes_read ? "retry" : "exit");
		return bytes_read;
	}

	/* launch appropriate method to form suitable reply to the packet */
	switch (pptp_ctrl_type) {
	case START_CTRL_CONN_RQST:	/* Start Control Connection Request */
		deal_start_ctrl_conn(packet, rply_packet, rply_size);
		break;

	case STOP_CTRL_CONN_RQST:
		deal_stop_ctrl_conn(packet, rply_packet, rply_size);
		break;

	case OUT_CALL_RQST:		/* Outgoing Call Request */
		deal_out_call(packet, rply_packet, rply_size);
		break;

	case ECHO_RQST:			/* Echo Request */
		deal_echo(packet, rply_packet, rply_size);
		break;

	case CALL_CLR_RQST:		/* Call Clear Request (Disconnect Request) */
		deal_call_clr(packet, rply_packet, rply_size);
		break;

	case SET_LINK_INFO:		/* Set Link Info */
		/* no reply packet but process it */
		deal_set_link_info(packet);
		break;

	case ECHO_RPLY:			/* Echo Reply */
	case STOP_CTRL_CONN_RPLY:	/* Stop Control Connection Reply */
	case CALL_DISCONN_NTFY:		/* Call Disconnect Notify */
		/* no reply packet */
		break;

	default:
		syslog(LOG_ERR, "CTRL: PPTP Control Message type %d not supported.", pptp_ctrl_type);
		pptp_ctrl_type = -1;
	}

	return pptp_ctrl_type;
}


/*
 * send_pptp_packet
 *
 * Sends a PPTP packet to a file descriptor.
 *
 * args:        clientFd (IN) - file descriptor to write the packet to.
 *              packet (IN) - the packet data to write.
 *              packet_size (IN) - the packet size.
 *
 * retn:        Number of bytes written on success.
 *              -1 on write failure.
 */
size_t send_pptp_packet(struct sppp *sp, unsigned char *packet, size_t packet_size)
{
	PPTPD_DRV_CTRL *p;
	int clientFd;
	size_t bytes_written;

	if ((p = sp->pp_lowerp) == NULL)
		return -1;

	clientFd = p->ctrl_sock;
	
	if ((bytes_written = write(clientFd, packet, packet_size)) == -1) {
		/* write failed */
		syslog(LOG_ERR, "CTRL: Couldn't write packet to client.");
		return -1;

	} else {
		/* debugging */
		//if (pptpctrl_debug) {
			syslog(LOG_DEBUG, "CTRL: I wrote %d bytes to the client.", packet_size);
			syslog(LOG_DEBUG, "CTRL: Sent packet to client");
		//}
		return bytes_written;
	}
}

/*
 * ignoreErrno
 *
 * Check if an errno represents a read error which should be ignored, and
 * put back to be select()ed on again later.
 *
 * Very similar to the function in Squid by Duane Wessels (under GPL).
 *
 * args: an errno value
 *
 * retn: 1 if the error is unimportant
 *       0 if the error is important
 */
static int ignoreErrno(int ierrno) {
	switch (ierrno) {
	case EAGAIN:		/* nothing to read */
	case EINTR:		/* signal received */
#ifdef ERESTART
#if ERESTART != EINTR
	case ERESTART:		/* signal received, should restart syscall */
#endif
#endif
#if EWOULDBLOCK != EAGAIN
	case EWOULDBLOCK:	/* shouldn't get this one but anyway, just in case */
#endif
		return 1;
	default:
		return 0;
	}
}

/*
 * read_pptp_header
 *
 * Reads a packet from a file descriptor and determines whether it is a
 * valid PPTP Control Message. If a valid PPTP Control Message is detected
 * it extracts the Control Message type from the packet header.
 *
 * args:        sp (IN) - get Clients file descriptor.
 *              packet (OUT) - Packet we read from the client.
 *              pptp_ctrl_type (OUT) - PPTP Control Message type of the packet.
 *
 * retn:        Number of bytes read on success.
 *              -1 on retryable error.
 *              0 on error to exit on.
 */
ssize_t read_pptp_header(struct sppp *sp, unsigned char *packet, int *pptp_ctrl_type)
{
	PPTPD_DRV_CTRL *p;
	int clientFd;
	ssize_t bytes_ttl, bytes_this;	/* quantities read (total and this read) */
	u_int16_t length;		/* length of this packet */
	struct pptp_header *header;	/* the received header */
	size_t bad_bytes = 0;
	size_t retval;

	if ((p = sp->pp_lowerp) == NULL)
		return 0;
	clientFd = p->ctrl_sock;

	/* read any previously buffered data */
	if (p->read_size == p->read_alloc) { /* need to alloc more memory */
		char *new_buffer=realloc(p->read_buffer, 
			sizeof(*(p->read_buffer)) * p->read_alloc * 2);
		if (new_buffer == NULL) {
			printf("Out of memory\n"); return;
		}
		p->read_alloc*=2;
		p->read_buffer = new_buffer;
	}
	retval = read(clientFd, p->read_buffer + p->read_size, 
					p->read_alloc  - p->read_size);
	if (retval < 0) {
		if (errno == EINTR || errno == EAGAIN)
			/* ignore */ ;
		else { /* a real error */
			return 0;
		}
	}
	p->read_size += retval;
	
	/* Give up unless there are at least sizeof(pptp_header) bytes */
	while ((p->read_size - bad_bytes) >= sizeof(struct pptp_header)) {
		/* Throw out bytes until we have a valid header. */
		header = (struct pptp_header *) (p->read_buffer + bad_bytes);
		if (ntoh32(header->magic) != PPTP_MAGIC_COOKIE) goto throwitout;
		if (ntoh16(header->length) < sizeof(struct pptp_header)) goto throwitout;
		if (ntoh16(header->length) > PPTP_MAX_CTRL_PCKT_SIZE) goto throwitout;

		/* At least one PPTP server implementation (Cisco) ignores RFC 2637's
		* requirement that Reserved0 "MUST be 0". In the interests of
		* interoperability, we'll just log this and pretend it never happened.
		*/
		if (ntoh16(header->reserved0)!=0) {
			printf("Protocol violation: header->reserved0(0x%x) != 0",
						ntoh16(header->reserved0));
		}

		/* well.  I guess it's good. Let's see if we've got it all. */
		if (ntoh16(header->length) > (p->read_size-bad_bytes))
			/* nope.  Let's wait until we've got it, then. */
			goto flushbadbytes;
		/* One last check: */
		if ((ntoh16(header->pptp_type) == PPTP_CTRL_MESSAGE) &&
			(ntoh16(header->length) != PPTP_CTRL_SIZE(ntoh16(header->ctrl_type))))
			goto throwitout;

		/* well, I guess we've got it. */
		memcpy(packet, p->read_buffer, header->length);
		/* Delete this packet from the read_buffer. */
		p->read_size -= (bad_bytes + header->length);
		memmove(p->read_buffer, p->read_buffer+bad_bytes+header->length, p->read_size);

		*pptp_ctrl_type = (int) (ntohs(header->ctrl_type));
		
		return header->length;

throwitout:
		bad_bytes++;
	}
flushbadbytes:
	/* no more packets.  Let's get rid of those bad bytes */
	p->read_size -= bad_bytes;
	memmove(p->read_buffer, p->read_buffer+bad_bytes, p->read_size);
	if (bad_bytes > 0) 
		printf("%lu bad bytes thrown away.", (unsigned long) bad_bytes);
	
	return -1;
}

/* Macros to use in making response packets */

#define MAKE_CTRL_HEADER(where, what) \
	where.header.length = htons(sizeof(where)); \
	where.header.pptp_type = htons(PPTP_CTRL_MESSAGE); \
	where.header.magic = htonl(PPTP_MAGIC_COOKIE); \
	where.header.ctrl_type = htons(what); \
	where.header.reserved0 = htons(RESERVED)

#define COPY_CTRL_PACKET(from, to, size) \
	memcpy(to, &from, ((*size) = sizeof(from)))

#define DEBUG_PACKET(what) \
		printf("CTRL: Made a " what " packet\n")

/*
 * deal_start_ctrl_conn
 *
 * This method 'deals' with a START-CONTROL-CONNECTION-REQUEST. After
 * stripping down the connection request a suitable reply is formed and
 * stored in 'rply_packet' ready for sending.
 *
 * args: packet (IN) - the packet that we have to deal with (should be a
 *                     START-CONTROL-CONNECTION-REQUEST packet)
 *       rply_packet (OUT) - suitable reply to the 'packet' we got.
 *       rply_size (OUT) - size of the reply packet
 */
void deal_start_ctrl_conn(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size)
{
	struct pptp_start_ctrl_conn_rqst *start_ctrl_conn_rqst;
	struct pptp_start_ctrl_conn_rply start_ctrl_conn_rply;

	start_ctrl_conn_rqst = (struct pptp_start_ctrl_conn_rqst *) packet;

	MAKE_CTRL_HEADER(start_ctrl_conn_rply, START_CTRL_CONN_RPLY);
	start_ctrl_conn_rply.version = htons(PPTP_VERSION);
	start_ctrl_conn_rply.result_code = CONNECTED;
	start_ctrl_conn_rply.error_code = NO_ERROR;
	start_ctrl_conn_rply.framing_cap = htons(OUR_FRAMING);
	start_ctrl_conn_rply.bearer_cap = htons(OUR_BEARER);
	start_ctrl_conn_rply.max_channels = htons(MAX_CHANNELS);
	start_ctrl_conn_rply.firmware_rev = htons(PPTP_FIRMWARE_VERSION);
	bzero(start_ctrl_conn_rply.hostname, MAX_HOSTNAME_SIZE);
	strncpy(start_ctrl_conn_rply.hostname, PPTP_HOSTNAME, MAX_HOSTNAME_SIZE);
	bzero(start_ctrl_conn_rply.vendor, MAX_VENDOR_SIZE);
	strncpy(start_ctrl_conn_rply.vendor, PPTP_VENDOR, MAX_VENDOR_SIZE);
	COPY_CTRL_PACKET(start_ctrl_conn_rply, rply_packet, rply_size);
	DEBUG_PACKET("START CTRL CONN RPLY");
}

/*
 * deal_stop_ctrl_conn
 *
 * This method response to a STOP-CONTROL-CONNECTION-REQUEST with a
 * STOP-CONTROL-CONNECTION-REPLY.
 */
void deal_stop_ctrl_conn(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size)
{
	struct pptp_stop_ctrl_conn_rply stop_ctrl_conn_rply;

	MAKE_CTRL_HEADER(stop_ctrl_conn_rply, STOP_CTRL_CONN_RPLY);
        stop_ctrl_conn_rply.result_code = DISCONNECTED;
        stop_ctrl_conn_rply.error_code = NO_ERROR;
        stop_ctrl_conn_rply.reserved1 = htons(RESERVED);
	COPY_CTRL_PACKET(stop_ctrl_conn_rply, rply_packet, rply_size);
	DEBUG_PACKET("STOP CTRL CONN RPLY");
}

/*
 * deal_out_call
 *
 * This method 'deals' with a OUT-GOING-CALL-REQUEST. After
 * stripping down the request a suitable reply is formed and stored in
 * 'rply_packet' ready for sending.
 *
 * args: packet (IN) - the packet that we have to deal with (should be a
 *                      OUT-GOING-CALL-REQUEST packet)
 *       rply_packet (OUT) - suitable reply to the 'packet' we got.
 *       rply_size (OUT) - size of the reply packet
 *
 */
void deal_out_call(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size)
{
	u_int16_t pac_call_id;
	struct pptp_out_call_rqst *out_call_rqst;
	struct pptp_out_call_rply out_call_rply;

	out_call_rqst = (struct pptp_out_call_rqst *) packet;

	if ((pac_call_id = getcall()) == htons(-1)) {
		/* XXX should reject call */
		syslog(LOG_ERR, "CTRL: No free Call IDs!");
		pac_call_id = 0;
	}
	MAKE_CTRL_HEADER(out_call_rply, OUT_CALL_RPLY);
	/* call_id is used for ctrl, call_id_peer is used for GRE
	 * call_id_peer is what we were sent by the other end in ctrl initilization
	 */
	out_call_rply.call_id = pac_call_id;
	out_call_rply.call_id_peer = out_call_rqst->call_id;
	out_call_rply.result_code = CONNECTED;
	out_call_rply.error_code = NO_ERROR;
	out_call_rply.cause_code = NO_ERROR;
	/* maybe limit to pppd speed? but pppd doesn't accept 10Mbps as a speed and yet
	 * still performs at over 115200, eg, 60kbyte/sec and higher observed.
	 */
	out_call_rply.speed = out_call_rqst->max_bps;
	/* lets match their window size for now... was htons(PCKT_RECV_WINDOW_SIZE)
	 */
	out_call_rply.pckt_recv_size = out_call_rqst->pckt_recv_size;
	//if(pptpctrl_debug)
		syslog(LOG_DEBUG, "CTRL: Set parameters to %d maxbps, %d window size",
			ntohs(out_call_rply.speed), ntohs(out_call_rply.pckt_recv_size));
	out_call_rply.pckt_delay = htons(PCKT_PROCESS_DELAY);
	out_call_rply.channel_id = htonl(CHANNEL_ID);
	COPY_CTRL_PACKET(out_call_rply, rply_packet, rply_size);
	DEBUG_PACKET("OUT CALL RPLY");
}


/*
 * deal_echo
 *
 * This method 'deals' with a ECHO-REQUEST. After stripping down the
 * connection request a suitable reply is formed and stored in
 * 'rply_packet' ready for sending.
 *
 * args: packet (IN) - the packet that we have to deal with (should be a
 *                      ECHO-REQUEST packet)
 *       rply_packet (OUT) - suitable reply to the 'packet' we got.
 *       rply_size (OUT) - size of the reply packet
 *
 */
void deal_echo(unsigned char *packet, unsigned char *rply_packet, ssize_t * rply_size)
{
	struct pptp_echo_rqst *echo_rqst;
	struct pptp_echo_rply echo_rply;

	echo_rqst = (struct pptp_echo_rqst *) packet;

	MAKE_CTRL_HEADER(echo_rply, ECHO_RPLY);
	echo_rply.identifier = echo_rqst->identifier;
	echo_rply.result_code = CONNECTED;
	echo_rply.error_code = NO_ERROR;
	echo_rply.reserved1 = htons(RESERVED);
	COPY_CTRL_PACKET(echo_rply, rply_packet, rply_size);
	DEBUG_PACKET("ECHO RPLY");
}

/*
 * deal_call_clr
 *
 * This method 'deals' with a CALL-CLEAR-REQUEST. After stripping down the
 * connection request a suitable reply is formed and stored in
 * 'rply_packet' ready for sending.
 *
 * args: packet (IN) - the packet that we have to deal with (should be a
 *                      CALL-CLEAR-REQUEST packet)
 *       rply_packet (OUT) - suitable reply to the 'packet' we got.
 *       rply_size (OUT) - size of the reply packet
 *
 */
void deal_call_clr(unsigned char *packet, unsigned char *rply_packet, ssize_t *rply_size)
{
	struct pptp_call_disconn_ntfy call_disconn_ntfy;
	u_int16_t pac_call_id;

	/* Form a reply
	 * The reply packet is a CALL-DISCONECT-NOTIFY
	 * In single call mode we don't care what peer's call ID is, so don't even bother looking
	 */
	if ((pac_call_id = freecall()) == htons(-1)) {
		/* XXX should return an error */
		syslog(LOG_ERR, "CTRL: Could not free Call ID [call clear]!");
	}
	MAKE_CTRL_HEADER(call_disconn_ntfy, CALL_DISCONN_NTFY);
	call_disconn_ntfy.call_id = pac_call_id;
	call_disconn_ntfy.result_code = CALL_CLEAR_REQUEST;	/* disconnected by call_clr_rqst */
	call_disconn_ntfy.error_code = NO_ERROR;
	call_disconn_ntfy.cause_code = htons(NO_ERROR);
	call_disconn_ntfy.reserved1 = htons(RESERVED);
	memset(call_disconn_ntfy.call_stats, 0, 128);
	COPY_CTRL_PACKET(call_disconn_ntfy, rply_packet, rply_size);
	DEBUG_PACKET("CALL DISCONNECT RPLY");
}

/*
 * deal_set_link_info
 *
 * @FIXME This function is *not* completed
 *
 * This method 'deals' with a SET-LINK-INFO. After stripping down the
 * connection request a suitable reply is formed and stored in
 * 'rply_packet' ready for sending.
 *
 * args: packet (IN) - the packet that we have to deal with (should be a
 *                      SET-LINK-INFO packet)
 *       rply_packet (OUT) - suitable reply to the 'packet' we got.
 *       rply_size (OUT) - size of the reply packet
 *
 */
void deal_set_link_info(unsigned char *packet)
{
	struct pptp_set_link_info *set_link_info;

	set_link_info = (struct pptp_set_link_info *) packet;
	if(set_link_info->send_accm != 0xffffffff || set_link_info->recv_accm != 0xffffffff)
		syslog(LOG_ERR, "CTRL: Ignored a SET LINK INFO packet with real ACCMs!");
	else// if(pptpctrl_debug)
		syslog(LOG_DEBUG, "CTRL: Got a SET LINK INFO packet with standard ACCMs");
}

void make_echo_req_packet(unsigned char *rply_packet, ssize_t * rply_size, u_int32_t echo_id)
{
	struct pptp_echo_rqst echo_packet;

	MAKE_CTRL_HEADER(echo_packet, ECHO_RQST);
	echo_packet.identifier = echo_id;
	COPY_CTRL_PACKET(echo_packet, rply_packet, rply_size);
	DEBUG_PACKET("ECHO REQ");
}

void make_stop_ctrl_req(unsigned char *rply_packet, ssize_t * rply_size)
{
	struct pptp_stop_ctrl_conn_rqst stop_ctrl;

	MAKE_CTRL_HEADER(stop_ctrl, STOP_CTRL_CONN_RQST);
	stop_ctrl.reason = GENERAL_STOP_CTRL;
	stop_ctrl.reserved1 = RESERVED;
	stop_ctrl.reserved2 = htons(RESERVED);
	COPY_CTRL_PACKET(stop_ctrl, rply_packet, rply_size);
	DEBUG_PACKET("STOP CTRL REQ");
}

void make_call_admin_shutdown(unsigned char *rply_packet, ssize_t * rply_size)
{
	struct pptp_call_disconn_ntfy call_disconn_ntfy;
	u_int16_t pac_call_id;

	/* Form a reply
	 * The reply packet is a CALL-DISCONECT-NOTIFY
	 * In single call mode we don't care what peer's call ID is, so don't even bother looking
	 */
	if ((pac_call_id = freecall()) == htons(-1)) {
		/* XXX should return an error */
		syslog(LOG_ERR, "CTRL: Could not free Call ID [admin shutdown]!");
	}
	MAKE_CTRL_HEADER(call_disconn_ntfy, CALL_DISCONN_NTFY);
	call_disconn_ntfy.call_id = pac_call_id;
	call_disconn_ntfy.result_code = ADMIN_SHUTDOWN;		/* disconnected by admin shutdown */
	call_disconn_ntfy.error_code = NO_ERROR;
	call_disconn_ntfy.cause_code = htons(NO_ERROR);
	call_disconn_ntfy.reserved1 = htons(RESERVED);
	memset(call_disconn_ntfy.call_stats, 0, 128);
	COPY_CTRL_PACKET(call_disconn_ntfy, rply_packet, rply_size);
	DEBUG_PACKET("CALL DISCONNECT RPLY");
}

#if PNS_MODE
/* out of date.  really PNS isn't 'trivially different', it's quite different */

#define C_BITS (sizeof(unsigned int) * 8)
#define C_SEG(x) (x/C_BITS)
#define C_BIT(x) ((1U)<<(x%C_BITS))
static unsigned int activeCalls[(MAX_CALLS / C_BITS) + 1];

/*
 * get_call_id
 *
 * Assigns a call ID and peer call ID to the session.
 *
 * args: call_id (OUT) - the call ID for the session
 * retn: 0 on success, -1 on failure
 */
int get_call_id(u_int16_t * loc)
{
	for (i = 0; i < MAX_CALLS; i++) {
		if (!(activeCalls[C_SEG(i)] & C_BIT(i))) {
			activeCalls[C_SEG(i)] |= C_BIT(i);
			*loc = i;
			return 0;
		}
	}
	return -1;
}

/*
 * free_call_id
 *
 * args: call_id (IN) - the call ID for a terminated session
 * retn: 0 on success, -1 on failure
 */
int free_call_id(u_int16_t call_id)
{
	if (!(activeCalls[C_SEG(i)] & C_BIT(i)))
		return -1;
	activeCalls[C_SEG(i)] &= ~C_BIT(i);
	return 0;
}
#else
static int _pac_call_id;
static u_int16_t _pac_init = 0;

/*
 * getcall
 *
 * Assigns a call ID to the session and stores/returns it
 *
 * we only permit one call at a time, so the chance of wrapping 65k on one
 * control connection is zero to none...
 */
u_int16_t getcall()
{
	static u_int16_t i = 0;

	if(!_pac_init) {
		srandom(time(NULL));
		i = random();
		_pac_call_id = htons(-1);
		_pac_init = 1;
	}
	if(_pac_call_id != htons(-1))
		syslog(LOG_ERR, "CTRL: Asked to allocate call id when call open, not handled well");
	_pac_call_id = i;
	return i++;
}

/*
 * freecall
 *
 * Notes termination of current call
 *
 * retn: -1 on failure, PAC call ID on success
 */
u_int16_t freecall()
{
	u_int16_t ret;

	if(!_pac_init) {
		_pac_call_id = htons(-1);
	}
	ret = _pac_call_id;
	if(_pac_call_id == htons(-1))
		syslog(LOG_ERR, "CTRL: Asked to free call when no call open, not handled well");
	_pac_call_id = htons(-1);
	return ret;
}
#endif
