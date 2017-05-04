/*
 *  Main module for WiFi Simple-Config Daemon
 *
 *	Copyright (C)2006, Realtek Semiconductor Corp. All rights reserved.
 *
 *	$Id: wsc.c,v 1.21 2012/02/23 07:19:45 cathy Exp $
 */

/*================================================================*/
/* Include Files */

#include "wsc.h"
#include "built_time"


/*================================================================*/
/* Local Variables */

static const char rnd_seed[] = "Realtek WiFi Simple-Config Daemon program 2006-05-15";
 CTX_Tp pGlobalCtx;

#ifdef OUTPUT_LOG
FILE *outlog_fp=NULL;
unsigned char StringbufferOut[80];
#endif

/*================================================================*/
/* Global Variables */

unsigned char WSC_VENDOR_ID[3] = {0x00, 0x37, 0x2a};
unsigned char wsc_prime_num[]={
0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xC9,0x0F,0xDA,0xA2, 0x21,0x68,0xC2,0x34, 0xC4,0xC6,0x62,0x8B, 0x80,0xDC,0x1C,0xD1,
0x29,0x02,0x4E,0x08, 0x8A,0x67,0xCC,0x74, 0x02,0x0B,0xBE,0xA6, 0x3B,0x13,0x9B,0x22, 0x51,0x4A,0x08,0x79, 0x8E,0x34,0x04,0xDD,
0xEF,0x95,0x19,0xB3, 0xCD,0x3A,0x43,0x1B, 0x30,0x2B,0x0A,0x6D, 0xF2,0x5F,0x14,0x37, 0x4F,0xE1,0x35,0x6D, 0x6D,0x51,0xC2,0x45,
0xE4,0x85,0xB5,0x76, 0x62,0x5E,0x7E,0xC6, 0xF4,0x4C,0x42,0xE9, 0xA6,0x37,0xED,0x6B, 0x0B,0xFF,0x5C,0xB6, 0xF4,0x06,0xB7,0xED,
0xEE,0x38,0x6B,0xFB, 0x5A,0x89,0x9F,0xA5, 0xAE,0x9F,0x24,0x11, 0x7C,0x4B,0x1F,0xE6, 0x49,0x28,0x66,0x51, 0xEC,0xE4,0x5B,0x3D,
0xC2,0x00,0x7C,0xB8, 0xA1,0x63,0xBF,0x05, 0x98,0xDA,0x48,0x36, 0x1C,0x55,0xD3,0x9A, 0x69,0x16,0x3F,0xA8, 0xFD,0x24,0xCF,0x5F,
0x83,0x65,0x5D,0x23, 0xDC,0xA3,0xAD,0x96, 0x1C,0x62,0xF3,0x56, 0x20,0x85,0x52,0xBB, 0x9E,0xD5,0x29,0x07, 0x70,0x96,0x96,0x6D,
0x67,0x0C,0x35,0x4E, 0x4A,0xBC,0x98,0x04, 0xF1,0x74,0x6C,0x08, 0xCA,0x23,0x73,0x27, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF,
};


/*================================================================*/
static void process_event(CTX_Tp pCtx, int evt);
static int init_config(CTX_Tp pCtx, int def);

// patch-100804
#ifdef WSC_1SEC_TIMER
static int wsc_alarm(void);
static int sigHandler_alarm(int signo);
#else
static void sigHandler_alarm(int signo);
#endif

/*================================================================*/
/* Implementation Routines */
#ifdef AUTO_LOCK_DOWN
void InOut_auto_lock_down(CTX_Tp pCtx , int enter)
{
	//struct sysinfo info ;
	unsigned char tmpbuf[64];
	
	if(enter){	// enter lock state

		printf("\n\n	AUTO_LOCK_DOWN(locked)\n");
#ifdef ALD_BRUTEFORCE_ATTACK_MITIGATION
		if(pCtx->ADL_pin_attack_count < ALD_INDEFINITE_TH) {
			WSC_DEBUG("during (%d) seconds,occur (%d) times auth fail,\n", AUTH_FAIL_TIME_TH, AUTH_FAIL_TIMES);
			WSC_DEBUG("locked [%d] seconds\n\n",pCtx->auto_lock_down);
		} else {
			printf("indefinitely auto-lock-down until user intervenes to unlock!!\n" );
		}
#endif

		/* update beacon and probe-resp content*/
		if (pCtx->is_ap && pCtx->use_ie
			#ifdef FOR_DUAL_BAND
			&& pCtx->wlan0_wsc_disabled==0
			#endif
		){
			update_ie(pCtx,0, 0);
		}
		
		#ifdef FOR_DUAL_BAND
		if (pCtx->is_ap && pCtx->use_ie && pCtx->wlan1_wsc_disabled==0){
			update_ie2(pCtx,0, 0);
		}
		#endif

		// echo  1 > /tmp/wscd_lock_stat
		sprintf(tmpbuf, "echo 1 > %s", WSCD_LOCK_STAT);
		system(tmpbuf);

		// led blink into error state
		#ifdef	DET_WPS_SPEC
		if (wlioctl_set_led(pCtx->wlan_interface_name, TURNKEY_LED_LOCK_DOWN) < 0) {
			DEBUG_ERR("issue wlan ioctl set_led error!\n");	
		}
		#else	// if is turnkey
		if (wlioctl_set_led(pCtx->wlan_interface_name, TURNKEY_LED_WSC_NOP) < 0) {
			DEBUG_ERR("issue wlan ioctl set_led error!\n");	
		}
		#endif

	}
	else{	// leave lock state

		printf("\n\n	AUTO_LOCK_DOWN(unlocked)\n\n");

		// leave led error state
		if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_END) < 0) {
			DEBUG_ERR("issue wlan ioctl set_led error!\n");
		}

		/* update IE in beacon and probe-resp */
		//init_wlan(pCtx , 1);
		if (pCtx->is_ap && pCtx->use_ie
			#ifdef FOR_DUAL_BAND
			&& pCtx->wlan0_wsc_disabled==0
			#endif
		){
			update_ie(pCtx,0, 0);
		}
		
		#ifdef FOR_DUAL_BAND
		if (pCtx->is_ap && pCtx->use_ie && pCtx->wlan1_wsc_disabled==0){
			update_ie2(pCtx,0, 0);
		}
		#endif
		
		// remove "/tmp/wscd_lock_stat"
		unlink(WSCD_LOCK_STAT);

	}
}

#endif

static void do_bcopy(unsigned char *dst, unsigned char *src, int len)
{
	int i;
	for (i=0; i<len; dst++, src++, i++)
		*dst = *src;
}

static void convert_hex_to_ascii(unsigned long code, char *out)
{
	*out++ = '0' + ((code / 10000000) % 10);
	*out++ = '0' + ((code / 1000000) % 10);
	*out++ = '0' + ((code / 100000) % 10);
	*out++ = '0' + ((code / 10000) % 10);
	*out++ = '0' + ((code / 1000) % 10);
	*out++ = '0' + ((code / 100) % 10);
	*out++ = '0' + ((code / 10) % 10);
	*out++ = '0' + ((code / 1) % 10);
	*out = '\0';
}

static int compute_pin_checksum(unsigned long int PIN)
{
	unsigned long int accum = 0;
	int digit;
	
	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10);
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10);
	accum += 3 * ((PIN / 1000) % 10);
	accum += 1 * ((PIN / 100) % 10);
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	return (10 - digit) % 10;
}

static int get_mac_addr(int sk, char *interface, char *addr)
{
	struct ifreq ifr;

	DBFENTER;

	strcpy(ifr.ifr_name, interface);

	if ( ioctl(sk, SIOCGIFHWADDR, &ifr) < 0) {
		printf("ioctl(SIOCGIFHWADDR) failed!\n");
		return -1;
	}

	memcpy(addr, ifr.ifr_hwaddr.sa_data, 6);
	return 0;
}

static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0)
		printf("Unable to open pidfile %s\n", pidfile);
	else
		lockf(pid_fd, F_LOCK, 0);

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

static unsigned char convert_atob(char *data, int base)
{
	char tmpbuf[10];
	int bin;

	memcpy(tmpbuf, data, 2);
	tmpbuf[2]='\0';
	if (base == 16)
		sscanf(tmpbuf, "%02x", &bin);
	else
		sscanf(tmpbuf, "%02d", &bin);
	return((unsigned char)bin);
}

#ifndef USE_MINI_UPNP
static char *get_token(char *data, char *token)
{
	char *ptr=data;
	int len=0, idx=0;

	while (*ptr && *ptr != '\n' ) {
		if (*ptr == '=') {
			if (len <= 1)
				return NULL;
			memcpy(token, data, len);

			/* delete ending space */
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx] !=  ' ')
					break;
			}
			token[idx+1] = '\0';

			return ptr+1;
		}
		len++;
		ptr++;
	}
	return NULL;
}

static int get_value(char *data, char *value)
{
	char *ptr=data;
	int len=0, idx, i;

	while (*ptr && *ptr != '\n' && *ptr != '\r') {
		len++;
		ptr++;
	}

	/* delete leading space */
	idx = 0;
	while (len-idx > 0) {
		if (data[idx] != ' ')
			break;
		idx++;
	}
	len -= idx;

	/* delete bracing '"' */
	if (data[idx] == '"') {
		for (i=idx+len-1; i>idx; i--) {
			if (data[i] == '"') {
				idx++;
				len = i - idx;
			}
			break;
		}
	}

	if (len > 0) {
		memcpy(value, &data[idx], len);
		value[len] = '\0';
	}
	return len;
}
#endif

static int init_socket(CTX_Tp pCtx)
{
	struct ifreq ifr;
	struct sockaddr_ll addr;
	int ifindex;

	DBFENTER;
	
	pCtx->socket = socket(PF_PACKET, SOCK_RAW, 0);
	if (pCtx->socket < 0) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		return -1;
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pCtx->wlan_interface_name, sizeof(ifr.ifr_name));
	while (ioctl(pCtx->socket , SIOCGIFINDEX, &ifr) != 0) {
		WSC_DEBUG("ioctl(SIOCGIFINDEX) failed!\n");
		sleep(1);
	}
	ifindex = ifr.ifr_ifindex;
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifindex;
	if (bind(pCtx->socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		WSC_DEBUG("bind[PACKET] fail\n");
		return -1;
	}

#ifdef FOR_DUAL_BAND

	if(!pCtx->wlan1_wsc_disabled){
		int countxx=0;
		pCtx->socket2 = socket(PF_PACKET, SOCK_RAW, 0);
		if (pCtx->socket2 < 0) {
			perror("socket[PF_PACKET,SOCK_RAW]");
			WSC_DEBUG("bind[PACKET] fail\n");
			return -1;
		}
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, pCtx->wlan_interface_name2, sizeof(ifr.ifr_name));
		while (ioctl(pCtx->socket2 , SIOCGIFINDEX, &ifr) != 0) {
			WSC_DEBUG("ioctl(SIOCGIFINDEX) failed!\n");
			countxx++;
			if(countxx>5){
				printf("ioctl(SIOCGIFINDEX) failed!\n");
				break;
			}
			sleep(1);
		}
		ifindex = ifr.ifr_ifindex;
		memset(&addr, 0, sizeof(addr));
		addr.sll_family = AF_PACKET;
		addr.sll_ifindex = ifindex;
		if (bind(pCtx->socket2, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			printf("bind PACKET2 fail\n");
			return -1;
		}
	}
#endif //FOR_DUAL_BAND

	return 0;
}

static STA_CTX_Tp search_sta_entry(CTX_Tp pCtx, unsigned char *addr)
{
	int i, idx=-1;

	for (i=0; i<MAX_STA_NUM; i++) {
		if (!pCtx->sta[i] || pCtx->sta[i]->used == 0) {
			if (idx < 0)
				idx = i;
			continue;
		}
		if (!memcmp(pCtx->sta[i]->addr, addr, ETHER_ADDRLEN))
			break;
	}

	if ( i != MAX_STA_NUM)
		return (pCtx->sta[i]);
	

	if (idx >= 0) {
		pCtx->sta[idx] = calloc(1, sizeof(STA_CTX));
		return (pCtx->sta[idx]);
	}

	_DEBUG_PRINT("wscd : search_sta_entry : station table full!\n");
	return NULL; // table full
}

static int check_wsc_packet(CTX_Tp pCtx, struct eap_wsc_t *wsc)
{
	if (memcmp(wsc->vendor_id, WSC_VENDOR_ID, 3)) {
		_DEBUG_PRINT("Invalid WSC vendor id!,0x%02x-%02x-%02x\n",
			wsc->vendor_id[0],wsc->vendor_id[1],wsc->vendor_id[2]);
		return -1;
	}

	#ifdef DEBUG		// for debug
	if (wsc->vendor_type != ntohl(WSC_VENDOR_TYPE)) {
		_DEBUG_PRINT("Invalid WSC vendor type!\n");
		//return -1;
	}
	#endif

	if (wsc->op_code < WSC_OP_START || wsc->op_code > WSC_OP_FRAG_ACK) {
		WSC_DEBUG("Invalid WSC OP-Code!\n");
		return -1;
	}
	return 0;
}

static int get_wlan_evt(CTX_Tp pCtx)
{
	int evt;
	unsigned char type;

	type = *(pCtx->rx_buffer + FIFO_HEADER_LEN);
	if (type == (unsigned char)DOT11_EVENT_WSC_ASSOC_REQ_IE_IND)
		evt = EV_ASSOC_IND;
	else if (type == (unsigned char)DOT11_EVENT_WSC_PIN_IND)
		evt = EV_PIN_INPUT;
#ifdef P2P_SUPPORT
	else if (type == (unsigned char)DOT11_EVENT_WSC_SWITCH_MODE)
		evt = EV_P2P_SWITCH_MODE;
	else if (type == (unsigned char)DOT11_EVENT_WSC_STOP)
		evt = EV_STOP;
#endif
	else if (type == (unsigned char)DOT11_EVENT_WSC_SET_MY_PIN)
		evt = EV_CHANGE_MY_PIN;
	else if (type == (unsigned char)DOT11_EVENT_WSC_SPEC_SSID)
		evt = EV_SPEC_SSID;
	else if (type == (unsigned char)DOT11_EVENT_WSC_SPEC_MAC_IND)
		evt = EV_SET_SPEC_CONNECT_MAC;
	else if (type == DOT11_EVENT_EAP_PACKET)
		evt = EV_EAP;
#ifdef SUPPORT_UPNP
	else if (type == (unsigned char)DOT11_EVENT_WSC_PROBE_REQ_IND)
		evt = EV_PROBEREQ_IND;
#endif
	else {
		_DEBUG_PRINT("Rx error! event type is not recognized [%d]\n", type);
		return -1;
	}
	return evt;
}

#if !defined(NO_IWCONTROL) || defined(USE_MINI_UPNP)
static void listen_and_process_event(CTX_Tp pCtx)
{
	int selret=0;
	fd_set netFD;

#ifndef NO_IWCONTROL
	int evt;
	int nRead;
#endif
#ifdef USE_MINI_UPNP
	struct upnphttp * e = 0;
	struct upnphttp * next = 0;
	struct EvtRespElement *EvtResp=NULL;
	struct EvtRespElement *EvtResp_next=NULL;
#endif
	// patch-100804
#ifndef WSC_1SEC_TIMER
	struct timeval timeout, time1, time2;
	int timePass;
#endif

	DBFENTER;

	// patch-100804 -- 1 sec timer
#ifndef WSC_1SEC_TIMER
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
#endif
	while(1) {
		FD_ZERO(&netFD);
#ifndef NO_IWCONTROL

#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
#endif
		{
			FD_SET(pCtx->fifo, &netFD);
		}

#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan1_wsc_disabled){
			FD_SET(pCtx->fifo2, &netFD);
		}
#endif //FOR_DUAL_BAND
#endif // !NO_IWCONTROL

		gettimeofday(&time1, 0);
		if (pCtx->upnp) {
#ifdef USE_MINI_UPNP
#ifdef STAND_ALONE_MINIUPNP
			if (pCtx->upnp_info.sudp >= 0)
				FD_SET(pCtx->upnp_info.sudp, &netFD);
#endif
			if (pCtx->upnp_info.shttpl >= 0)
				FD_SET(pCtx->upnp_info.shttpl, &netFD);

			for(e = pCtx->upnp_info.upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
			{
				if((e->socket >= 0) && (e->state <= 2))
				{
					FD_SET(e->socket, &netFD);
				}
			}

			for(EvtResp = pCtx->upnp_info.subscribe_list.EvtResp_head.lh_first; EvtResp != NULL; EvtResp = EvtResp->entries.le_next)
			{
				if (EvtResp->socket >= 0)
					FD_SET(EvtResp->socket, &netFD);
			}
			
			// patch-100804
			//selret = select(FD_SETSIZE, &netFD, NULL, NULL, NULL);
			selret = select(FD_SETSIZE, &netFD, NULL, NULL, &timeout);
#endif //USE_MINI_UPNP	
		}
#ifndef NO_IWCONTROL		
		else
			// patch-100804
			//selret = select(pCtx->fifo+1, &netFD, NULL, NULL, NULL);
			selret = select(pCtx->fifo+1, &netFD, NULL, NULL, &timeout);
#endif		
		
		if (selret >= 0) {
			// patch-100804
#ifndef WSC_1SEC_TIMER
			if (selret == 0) { // timeout
				sigHandler_alarm(0);
				// reset timer
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
				continue;
			}
#endif
#ifndef NO_IWCONTROL
			/* Polling FIFO event */
			/* get event from wlan0 interface */

#ifdef FOR_DUAL_BAND
			if( pCtx->wlan0_wsc_disabled==0	)
#endif
			{
				if (FD_ISSET(pCtx->fifo, &netFD)) {

					nRead = read(pCtx->fifo, pCtx->rx_buffer, MAX_MSG_SIZE);
					if (nRead > 0){
						
						evt = get_wlan_evt(pCtx);
#ifdef FOR_DUAL_BAND
						if(evt == EV_EAP && pCtx->inter1only == 1){
							WSC_DEBUG("drop EAP from wlan0\n");
							continue;
						}
						if(pCtx->is_ap){
							if( evt == EV_EAP || evt == EV_ASSOC_IND ){
								pCtx->InterFaceComeIn = COME_FROM_WLAN0 ;
								_DEBUG_PRINT("<<<===from wlan0\n");
							}
						}
#endif //FOR_DUAL_BAND
						if (evt >= 0)
							process_event(pCtx, evt);
					}
				}
			}

#ifdef FOR_DUAL_BAND		/* get event from wlan1 interface */
			//if(pCtx->is_ap && !pCtx->wlan1_wsc_disabled){
			if(	pCtx->wlan1_wsc_disabled==0 )
			{
				if (FD_ISSET(pCtx->fifo2, &netFD)) {
					nRead = read(pCtx->fifo2, pCtx->rx_buffer, MAX_MSG_SIZE);
					if (nRead > 0 )
					{
						evt = get_wlan_evt(pCtx);
						if(evt == EV_EAP && pCtx->inter0only == 1){
							WSC_DEBUG("drop EAP from wlan1\n");
							continue;
						}

						if( evt == EV_EAP || evt == EV_ASSOC_IND){
							pCtx->InterFaceComeIn = COME_FROM_WLAN1 ;
							_DEBUG_PRINT("<<<===from wlan1\n");
						}

						if (evt >= 0)
							process_event(pCtx, evt );
					}
				}
			}
#endif //FOR_DUAL_BAND
#endif // !NO_IWCONTROL
			if (!pCtx->upnp)
				continue;
			
#ifdef USE_MINI_UPNP
#ifdef STAND_ALONE_MINIUPNP
			/* process SSDP packets */
			if(pCtx->upnp_info.sudp >= 0 && FD_ISSET(pCtx->upnp_info.sudp, &netFD))
			{
				/*syslog(LOG_INFO, "Received UDP Packet");*/
				WSC_DEBUG("\n\n");
				ProcessSSDPRequest(pCtx->upnp_info.sudp, pCtx->upnp_info.lan_ip_address, 
					(unsigned short)pCtx->upnp_info.port, &pCtx->upnp_info.SSDP);
			}
#endif
			
			/* process incoming HTTP connections */
			if(pCtx->upnp_info.shttpl >= 0 && FD_ISSET(pCtx->upnp_info.shttpl, &netFD))
			{
				int shttp;
				socklen_t	clientnamelen;
				struct sockaddr_in clientname;
				struct upnphttp * tmp = 0;
				clientnamelen = sizeof(struct sockaddr_in);
				char *IP=NULL;
				shttp = accept(pCtx->upnp_info.shttpl, (struct sockaddr *)&clientname, &clientnamelen);
				if(shttp<0)
				{
					syslog(LOG_ERR, "accept: %m");
				}
				else
				{
					_DEBUG_PRINT("\n\n");
					WSC_DEBUG("HTTP connection from %s:%d\n",inet_ntoa(clientname.sin_addr),ntohs(clientname.sin_port) );
					/*if (fcntl(shttp, F_SETFL, O_NONBLOCK) < 0) {
						syslog(LOG_ERR, "fcntl F_SETFL, O_NONBLOCK");
					}*/
					/* Create a new upnphttp object and add it to
					 * the active upnphttp object list */
					tmp = New_upnphttp(shttp);
					if (tmp) {
						IP = inet_ntoa(clientname.sin_addr);
						memcpy(tmp->IP, IP, strlen(IP));
						tmp->IP[strlen(IP)] = '\0';
						tmp->soapMethods = pCtx->upnp_info.soapMethods;
						tmp->sendDesc = pCtx->upnp_info.sendDesc;
						tmp->subscribe_list = &pCtx->upnp_info.subscribe_list;
					}
					LIST_INSERT_HEAD(&pCtx->upnp_info.upnphttphead, tmp, entries);
				}
			}
			/* process active HTTP connections */
			/* LIST_FOREACH is not available under linux */
			for(e = pCtx->upnp_info.upnphttphead.lh_first; e != NULL; e = e->entries.le_next)
			{
				if(  (e->socket >= 0) && (e->state <= 2)
			   		&&(FD_ISSET(e->socket, &netFD)) )
					Process_upnphttp(e);
			}
			/* delete finished HTTP connections */
			for(e = pCtx->upnp_info.upnphttphead.lh_first; e != NULL; )
			{
				next = e->entries.le_next;
				if(e->state >= 100)
				{
					LIST_REMOVE(e, entries);
					Delete_upnphttp(e);
				}
				e = next;
			}
			/* process eventing response */
			for(EvtResp = pCtx->upnp_info.subscribe_list.EvtResp_head.lh_first; EvtResp != NULL; )
			{
				EvtResp_next = EvtResp->entries.le_next;
				if(FD_ISSET(EvtResp->socket, &netFD)) {
					ProcessEventingResp(EvtResp);
					LIST_REMOVE(EvtResp, entries);
					if (EvtResp->socket >= 0)
						close(EvtResp->socket);
					free(EvtResp);
				}
				EvtResp = EvtResp_next;
			}
#endif // !USE_MINI_UPNP
		}
		else {
			// Re-calculate the timeout
			gettimeofday(&time2, 0);
			if (time2.tv_sec > time1.tv_sec)
				time2.tv_usec+=1000000L;
			timePass=time2.tv_usec-time1.tv_usec;
			if (timePass > 0) {
				// It's specific for 1 sec. timer
				if (timeout.tv_sec)
					timeout.tv_usec = 1000000L;
				timeout.tv_sec=0;
				timeout.tv_usec -= timePass;
				if (timeout.tv_usec <= 0) {
					sigHandler_alarm(0);
					timeout.tv_sec = 1;
					timeout.tv_usec = 0;
				}
			}
		}
	}
}
#endif // !NO_IWCONTROL || USE_MINI_UPNP


//#define SET_SELECT_REG 1
int update_ie(CTX_Tp pCtx, unsigned char selected , int type)
{
	int len;
	unsigned char tmpbuf[1024];
	WSC_DEBUG("\n\n");
	len = build_beacon_ie(pCtx, selected, type, pCtx->config_method, tmpbuf);

		//under WSP2.0+ version should not provisioning WEP
	if ((pCtx->encrypt_type == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) // add provisioning service ie
	{
		len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));
	}

	if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0){
			_DEBUG_PRINT("<< EV_START error, IOCTL set beacon IE failed >>\n");
			return -1;
	}
	len = build_probe_rsp_ie(pCtx, selected, type, pCtx->config_method, tmpbuf);

	/* add provisioning service ie ;
	   by microsoft suggest ,for avoid WEP IOT issue when VS MS device*/

		//under WSP2.0+ version shoult not provisioning WEP
	if ((pCtx->encrypt_type == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) {
		len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));
	}

	if (len > MAX_WSC_IE_LEN) {
		_DEBUG_PRINT("<< EC_START error, Length of IE exceeds %d>>\n", MAX_WSC_IE_LEN);
		return -1;
	}

	if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0){
		_DEBUG_PRINT("<< EC_START error, IOCTL set probe IE failed >>\n");
		return -1;
	}
	return 0;
}

#ifdef FOR_DUAL_BAND
int update_ie2(CTX_Tp pCtx, unsigned char selected , int type)
{

	int len;
	unsigned char tmpbuf[1024];

	WSC_DEBUG("\n\n");
	len = build_beacon_ie(pCtx, selected, type, pCtx->config_method, tmpbuf);		

	if ((pCtx->encrypt_type2 == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) // add provisioning service ie
	{
		WSC_DEBUG("!!!	under WSP2.0+ version shoult not provisioning WEP , check!!!\n");
		len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));
	}

	if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0){
		_DEBUG_PRINT("<< EV_START error, IOCTL set beacon IE failed >>\n");
		return -1;
	}



	len = build_probe_rsp_ie(pCtx, selected, type, pCtx->config_method, tmpbuf);

	/* add provisioning service ie ;
	   by microsoft suggest ,for avoid WEP IOT issue when VS MS device*/
	if ((pCtx->encrypt_type2 == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) {
		WSC_DEBUG("!!!	under WSP2.0+ version shoult not provisioning WEP , check!!!\n");
		len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));
	}

	if (len > MAX_WSC_IE_LEN) {
		_DEBUG_PRINT("<< EC_START error, Length of IE exceeds %d>>\n", MAX_WSC_IE_LEN);
		return -1;
	}


	if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0){
		_DEBUG_PRINT("<< EV_START error, IOCTL set probe IE failed >>\n");
		return -1;
	}

	return 0;
}
#endif //FOR_DUAL_BAND

static int reset_pin_procedure(CTX_Tp pCtx, int interval)
{
	unsigned char tmpbuf[1024];

#ifdef P2P_SUPPORT
	if(pCtx->p2p_trigger_type==P2P_PRE_CLIENT){
		WSC_DEBUG("p2p GC mode timeout is 30 secs\n");
		pCtx->pin_timeout = 30;
	}else
#endif
	pCtx->pin_timeout = interval;

	WSC_DEBUG("LED START...\n");
	if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_START) < 0) {
		printf("issue wlan ioctl set_led error!\n");
	}

	if (pCtx->is_ap)
		pCtx->wps_triggered = 1;
	
	if(pCtx->pb_pressed || pCtx->pb_timeout) {
		WSC_DEBUG("Clear PBC stuff!\n");
		pCtx->pb_pressed = 0;
		pCtx->pb_timeout = 0;
	}
	if (pCtx->setSelectedRegTimeout) {
		DEBUG_PRINT("Clear setSelectedReg stuff!\n");
		pCtx->setSelectedRegTimeout = 0;
	}
	return 0;
}

static int evHandler_pin_input(CTX_Tp pCtx, char *pin)
{
	int code, len;
#ifdef CLIENT_MODE
	unsigned char tmpbuf[1024];
#endif
	DOT11_WSC_PIN_IND *pIndEvt = (DOT11_WSC_PIN_IND *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
	char *pcode;

	DBFENTER;

	if (!IS_PIN_METHOD(pCtx->config_method)) {
		WSC_DEBUG("PIN method is not configured!\n");
		return -1;
	}
	
	printf("<< Got PIN-code Input event >>\n");
	if (pin)
		pcode =  pin;
	else
		pcode =  pIndEvt->code;

#ifdef DEBUG
	if (pCtx->debug)
		printf("PIN code: %s\n", pcode);
#endif

	#if 0
	//WSC_pthread_mutex_lock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d Lock mutex\n", __FUNCTION__, __LINE__);
	if (pCtx->registration_on >= 1) {
		WSC_DEBUG("Registration protocol is already in progress; ignore PIN!\n");
		//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
		//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
		return -1;
	}
	//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
	#endif

	len = strlen(pcode);
	if (len != PIN_LEN && len != 4) {
		printf("Invalid pin_code length!\n");
		return -1;
	}
		
#if defined(CLIENT_MODE) && defined(SUPPORT_REGISTRAR)
	if (!pCtx->is_ap) {
		if (pCtx->role == REGISTRAR) {
			pCtx->start = 1;
			client_set_WlanDriver_WscEnable(pCtx, 1);
		}
		else {
			DEBUG_ERR("Enrollee PIN could not be supported in client mode!\n");
			return -1;
		}
	}
#endif

	code = atoi(pcode);
	if (len == PIN_LEN) {
		if (!validate_pin_code(code)) {
			DEBUG_PRINT("Checksum error! Assume PIN is user specified.\n");
			pCtx->peer_pin_id = PASS_ID_USER;
		}
		else
			pCtx->peer_pin_id = PASS_ID_DEFAULT;
	}
	pCtx->pin_assigned = 1;
	
	reset_pin_procedure(pCtx, PIN_WALK_TIME);

	report_WPS_STATUS(PROTOCOL_START);
	
	if (pin == NULL){
		strcpy(pCtx->peer_pin_code, pIndEvt->code);
	}
#ifdef P2P_SUPPORT
	else
	{
		/*modify  when add P2P_SUPPORT  ; take care!!*/

		strcpy(pCtx->peer_pin_code, pcode);
	}
#endif

	if (pCtx->is_ap) {

#ifdef FOR_DUAL_BAND
		pCtx->inter0only  = 0;
		pCtx->inter1only  = 0;

		/*2011-11 add for support under dual band mode just trigger single interface*/
		struct stat cancel_status;
		if (stat(WSCD_IND_ONLY_INTERFACE0, &cancel_status) == 0) {
			WSC_DEBUG("%s\n",WSCD_IND_ONLY_INTERFACE0);
			pCtx->inter0only  = 1;
			unlink(WSCD_IND_ONLY_INTERFACE0);
		}
		if (stat(WSCD_IND_ONLY_INTERFACE1, &cancel_status) == 0) {
			WSC_DEBUG("%s\n",WSCD_IND_ONLY_INTERFACE1);
			pCtx->inter1only  = 1;
			unlink(WSCD_IND_ONLY_INTERFACE1);
		}

		WSC_DEBUG("inter0only = %d\n",pCtx->inter0only);
		WSC_DEBUG("inter1only = %d\n",pCtx->inter1only);
		/*2011-11 add for support under dual band mode just trigger single interface*/
#endif
		{
#ifdef WPS2DOTX
			if (pCtx->current_wps_version == WPS_VERSION_V2) {
				registrar_remove_all_authorized_mac(pCtx);
				registrar_add_authorized_mac(pCtx , BroadCastMac);	// 2010-0719
			}
#endif
			if (pCtx->use_ie) {
#ifdef FOR_DUAL_BAND
				if(pCtx->wlan0_wsc_disabled == 0
					&& pCtx->inter1only != 1	/*ap pin mode support single trigger*/
					)
#endif	
				{
					WSC_DEBUG("PIN mode update wlan0 IE\n");
					if( update_ie(pCtx, 1, PASS_ID_DEFAULT) < 0 )
						return -1;
				}
#ifdef FOR_DUAL_BAND

				if(pCtx->wlan1_wsc_disabled==0
						&& pCtx->inter0only != 1	/*ap pin mode support single trigger*/
					)
				{
					WSC_DEBUG("PIN mode update wlan1 IE\n");
					if( update_ie2(pCtx, 1, PASS_ID_DEFAULT) < 0 )
						return -1;
				}
#endif //FOR_DUAL_BAND
		   	}
		}
	}
#if defined(CLIENT_MODE) && defined(SUPPORT_REGISTRAR)
	else {
		//struct sysinfo info ;

		if (pCtx->use_ie) {
			len = build_probe_request_ie(pCtx, PASS_ID_DEFAULT, tmpbuf);

			if (len == 0)
				DEBUG_ERR("Length of probe request IE exceeds %d\n", MAX_WSC_IE_LEN);
			
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_REQ) < 0)
				DEBUG_ERR("Set probe request IE failed\n");

			len = build_assoc_request_ie(pCtx, tmpbuf);
			if (len == 0)
				DEBUG_ERR("Length of assoc request IE exceeds %d\n", MAX_WSC_IE_LEN);
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_ASSOC_REQ) < 0)
				DEBUG_ERR("Set assoc request IE failed\n");		
		}
		//sysinfo(&info);
		//pCtx->start_time = (unsigned long)info.uptime;
		pCtx->start_time = time(NULL);
		
		DEBUG_PRINT("%s %d Issue scan\n", __FUNCTION__, __LINE__);
		issue_scan_req(pCtx, CONFIG_METHOD_PIN);		
	}
#endif

#ifdef BLOCKED_ROGUE_STA
	if (pCtx->is_ap && pCtx->blocked_expired_time)
		disassociate_blocked_list(pCtx);
#endif

	return 0;
}

static int evHandler_pb_press(CTX_Tp pCtx)
{
	unsigned char tmpbuf[1024];
	int len=0;

	DBFENTER;

	if (!IS_PBC_METHOD(pCtx->config_method)) {
		printf("\n\n!!!Config-method no include PBC method , pls check!\n\n\n");
		return -1;
	}
	
	printf("<< Got PB-Pressed event >>\n");

#ifdef CLIENT_MODE
	if (!pCtx->is_ap) {
		if (pCtx->role == REGISTRAR) {
			DEBUG_ERR("PBC could not be supported in external registrar mode!\n");
			return -1;
		}
		else {
		pCtx->start = 1;
			client_set_WlanDriver_WscEnable(pCtx, 1);
		}
	}
#endif

	if (!pCtx->start) {
		DEBUG_PRINT("Not started yet!\n");
		return 0;
	}
		
	#if 0
	//WSC_pthread_mutex_lock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d Lock mutex\n", __FUNCTION__, __LINE__);
	if (pCtx->registration_on >= 1) {
		WSC_DEBUG("Registration protocol is already in progress; ignore PBC!\n" );
		//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
		//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
		return -1;
	}
	//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
	#endif

#ifdef MUL_PBC_DETECTTION
	if (!pCtx->disable_MulPBC_detection && pCtx->active_pbc_sta_count > 1) {
		WSC_DEBUG("\n\n		!!! Multiple PBC sessions [%d] detected!\n\n\n", pCtx->active_pbc_sta_count);
		SwitchSessionOverlap_LED_On(pCtx);
		return -1;
	}
#endif

	WSC_DEBUG("LED START...\n");
	if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_START) < 0) {
		printf("issue wlan ioctl set_led error!\n");
		return -1;				
	}

	if (pCtx->is_ap) //add by peteryu for WZC in WEP
		pCtx->wps_triggered = 1; 

	pCtx->pb_pressed = 1;
#ifdef P2P_SUPPORT
    if(pCtx->p2p_trigger_type==P2P_PRE_CLIENT){
        WSC_DEBUG("p2p GC mode timeout is 30 secs\n");
        pCtx->pb_timeout = 30;
    }else
#endif
	pCtx->pb_timeout = PBC_WALK_TIME;
	WSC_DEBUG("update pb_timeout to %d\n",pCtx->pb_timeout);
	
	report_WPS_STATUS(PROTOCOL_START);

	if (pCtx->pin_timeout) {
		pCtx->pin_timeout = 0; //clear PIN timeout
		DEBUG_PRINT("Clear PIN stuff!\n");
	}

	if (pCtx->setSelectedRegTimeout) {
		DEBUG_PRINT("Clear setSelectedReg stuff!\n");
		pCtx->setSelectedRegTimeout = 0;
	}
	
	if (pCtx->role == REGISTRAR) {
		strcpy(pCtx->peer_pin_code, "00000000");
		pCtx->pin_assigned = 1;
	} else {
		strcpy(pCtx->pin_code, "00000000");
	}

	if (pCtx->is_ap) {
		if (pCtx->use_ie) {
#ifdef WPS2DOTX
			if (pCtx->current_wps_version == WPS_VERSION_V2) {
				registrar_remove_all_authorized_mac(pCtx);
				registrar_add_authorized_mac(pCtx , BroadCastMac);	// 2010-0719
			}
#endif
#ifdef FOR_DUAL_BAND
			if(pCtx->wlan0_wsc_disabled == 0
				&& pCtx->inter1only!=1	)
#endif
			{
				WSC_DEBUG("PBC mode update wlan0 IE\n");
				if( update_ie(pCtx, 1, PASS_ID_PB) < 0 ){
					return -1;
				}
			}
			
#ifdef FOR_DUAL_BAND
			if(pCtx->wlan1_wsc_disabled == 0
				&& pCtx->inter0only!=1)
			{
				WSC_DEBUG("PBC mode update wlan1 IE\n");
				if( update_ie2(pCtx, 1, PASS_ID_PB) < 0 ){
					return -1;
				}
			}
#endif //FOR_DUAL_BAND
		}
	}
#ifdef CLIENT_MODE	
	else {
			//struct sysinfo info ;
		
			//pCtx->start = 1;
			if (pCtx->use_ie) {
			len = build_probe_request_ie(pCtx, PASS_ID_PB, tmpbuf);
			if (len == 0)
				DEBUG_ERR("Length of probe request IE exceeds %d\n", MAX_WSC_IE_LEN);
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
				DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_REQ) < 0)
					DEBUG_ERR("Set probe request IE failed\n");

			len = build_assoc_request_ie(pCtx, tmpbuf);
			if (len == 0)
				DEBUG_ERR("Length of assoc request IE exceeds %d\n", MAX_WSC_IE_LEN);
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_ASSOC_REQ) < 0)
				DEBUG_ERR("Set assoc request IE failed\n");		
			}
			//sysinfo(&info);
			//pCtx->start_time= (unsigned long)info.uptime;
			pCtx->start_time = time(NULL);
			
			DEBUG_PRINT("%s %d Issue scan\n", __FUNCTION__, __LINE__);
			issue_scan_req(pCtx, CONFIG_METHOD_PBC);
	}
#endif

#ifdef BLOCKED_ROGUE_STA
	if (pCtx->is_ap && pCtx->blocked_expired_time)
		disassociate_blocked_list(pCtx);
#endif

	return 0;
}

static int evHandler_eap(CTX_Tp pCtx)
{
	DOT11_EAP_PACKET *pIndEvt = (DOT11_EAP_PACKET *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
	struct ethernet_t *eth;
	struct eapol_t *eapol;
	struct eap_t *eap;
	struct eap_wsc_t *wsc;
	STA_CTX_Tp pSta;
	int packet_len = (int)pIndEvt->packet_len;
	int ret = 0;
#ifdef WPS2DOTX
	unsigned char* MsgPtr;
	int tmplength;
//	unsigned char* pData;
//	int tag_len;
	int EAP_Length	;
#endif

	//2011-0727 SGS
#ifdef DEBUG
#ifdef SUPPORT_UPNP
			unsigned char *pMsg=NULL;
			unsigned char *pData=NULL;
			int msg_len=0;
			int tag_len = 0;
#endif
#endif
	//2011-0727 SGS

	DBFENTER;

#ifdef DEBUG
	if (pCtx->debug2) {
		debug_out("Rx EAP packet", pIndEvt->packet, packet_len);
	}
#endif

	if (!pCtx->start) {
		WSC_DEBUG("Not started yet!\n");
		return 0;
	}

	eth = (struct ethernet_t *)pIndEvt->packet;
	eapol = (struct eapol_t *)(((unsigned char *)eth) + ETHER_HDRLEN);
	pSta = search_sta_entry(pCtx, eth->ether_shost);

	if (pSta == NULL)
		return 0;
	
	if (!pSta->used) {
		//pSta->used = 1;
		pCtx->num_sta++;

		if(pCtx->debug){
			WSC_DEBUG("Rx a EAP packet;from unused STA[%02x:%02x:%02x:%02x:%02x:%02x]\n",
				eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
				eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);

			if (eapol->packet_type == EAPOL_KEY ) {
				WSC_DEBUG("EAP type == EAPOL-KEY \n");
			}
		}
		
		pSta->do_not_rescan = 1;
		reset_sta(pCtx, pSta, 1);
		return 0;
	}
	
#ifdef CLIENT_MODE
	if (!pCtx->is_ap)
		pCtx->sta_to_clear = pSta;
#endif

	if (packet_len < (ETHER_HDRLEN + EAPOL_HDRLEN)) {
		WSC_DEBUG("Rx EAPOL packet size too small!\n");
		return -1;
	}

	if (eapol->packet_type != EAPOL_START && eapol->packet_type != EAPOL_EAPPKT) {
		WSC_DEBUG("Invalid EAPOL packet type [0x%x]!\n", eapol->packet_type);
		return -1;
	}
	if (eapol->packet_type == EAPOL_START) {

		_DEBUG_PRINT("\n>>Rx EAPOL_START!\n");

		report_WPS_STATUS(RECV_EAPOL_START);

		if (pCtx->is_ap) {
			if (pSta->state == ST_WAIT_EAPOL_START) {
				struct timeval tod;
				gettimeofday(&tod , NULL);
				srand(tod.tv_sec);
				pSta->eap_reqid = (rand() % 50) + 1;
				send_eap_reqid(pCtx, pSta);
				pSta->state = ST_WAIT_RSP_ID;
				pSta->tx_timeout = pCtx->tx_timeout;
				pSta->retry = 0;
				return 0;
			}
			else {
				WSC_DEBUG("Invalid state![%d]\n", pSta->state);
				return 0;
			}
		}//client mode
		else {
			WSC_DEBUG("!!!Rx EAPOL_START, but ourself is not enable to config. Discard it!\n");
			return -1;
		}
	}
	if (packet_len < (ETHER_HDRLEN + EAPOL_HDRLEN + EAP_HDRLEN)) {
		WSC_DEBUG("!!!Rx EAP packet size too small!\n");
		return -1;
	}
	eap = (struct eap_t *)(((unsigned char *)eapol) + EAPOL_HDRLEN);

	// for debug, for intel sdk

	if (packet_len != (ETHER_HDRLEN + EAPOL_HDRLEN + ntohs(eapol->packet_body_length))) {
		WSC_DEBUG("!!!Rx EAP packet size mismatch with eapol length!\n");
		return -1;
	}

	if (packet_len != (ETHER_HDRLEN + EAPOL_HDRLEN + ntohs(eap->length))) {
		WSC_DEBUG("!!!Rx EAP packet size mismatch with eap length!\n");
		return -1;
	}


	if (eapol->packet_type != EAPOL_EAPPKT &&
				eapol->packet_type != EAPOL_START) {
		WSC_DEBUG("Invalid EAPOL type!\n");
		return -1;
	}

	if (eap->code != EAP_REQUEST &&
		eap->code != EAP_RESPONSE &&
			eap->code != EAP_FAIL) {
		WSC_DEBUG("Invalid EAP code [%d]!!!\n", eap->code);
		return -1;
	}

	if (eap->code == EAP_FAIL)
		return pktHandler_eap_fail(pCtx, pSta);

	wsc = (struct eap_wsc_t *)(((unsigned char *)eap) + EAP_HDRLEN);

	if (wsc->type != EAP_TYPE_IDENTITY && wsc->type != EAP_TYPE_EXPANDED) {
		WSC_DEBUG("Invalid WSC type!\n");
		return -1;
	}
	
	//2011-0727 SGS
#ifdef DEBUG
#ifdef SUPPORT_UPNP
	if(pCtx->is_ap){
		if (pSta->used & IS_UPNP_CONTROL_POINT) {
			msg_len = ntohs(eap->length)-EAP_HDRLEN;
			pMsg = ((struct WSC_packet *)wsc)->rx_buffer;
		}
		
		pData = search_tag(pMsg, TAG_MSG_TYPE, msg_len, &tag_len);
		if (pData != NULL) {

			if (pData[0] == MSG_TYPE_M1) {
				WSC_DEBUG("RX M1\n");
			}else
			if (pData[0] == MSG_TYPE_M2) {
				WSC_DEBUG("RX M2\n");
			}else
			if (pData[0] == MSG_TYPE_M2D) {
				WSC_DEBUG("RX M2D\n");
			}else
			if (pData[0] == MSG_TYPE_M3) {
				WSC_DEBUG("RX M3\n");
			}else
			if (pData[0] == MSG_TYPE_M4) {
				WSC_DEBUG("RX M4\n");
			}else
			if (pData[0] == MSG_TYPE_M5) {
				WSC_DEBUG("RX M5\n");
			}else
			if (pData[0] == MSG_TYPE_M6) {
				WSC_DEBUG("RX M6\n");
			}else
			if (pData[0] == MSG_TYPE_M7) {
				WSC_DEBUG("RX M7\n");
			}else
			if (pData[0] == MSG_TYPE_M8) {
				WSC_DEBUG("RX M8\n");
			}else
			if (pData[0] == MSG_TYPE_ACK) {
				WSC_DEBUG("RX ACK\n");
			}else
			if (pData[0] == MSG_TYPE_NACK) {
				WSC_DEBUG("RX NACK\n");
			}else
			if (pData[0] == MSG_TYPE_DONE) {
				WSC_DEBUG("RX DONE\n");
			}

		}

	}

#endif		
#endif
	//2011-0727 SGS

	
	if (pCtx->is_ap) {
		if (eap->code != EAP_RESPONSE) {
			DEBUG_ERR("Invalid EAP code [%x]\n", eap->code);
			return -1;
		}

		if (eap->identifier != pSta->eap_reqid) {
			WSC_DEBUG("	!!!EAP identifier is not matched! [in eap=0x%x, in keep =0x%x]!!!\n",eap->identifier, pSta->eap_reqid);
			return -1;
		}

		if (wsc->type == EAP_TYPE_IDENTITY)
			return pktHandler_rspid(pCtx, pSta, wsc->vendor_id, (int)(ntohs(eap->length)-EAP_HDRLEN-1));
	}
	else { // client mode
		pSta->eap_reqid = ntohs(eap->identifier);
		
#ifdef SUPPORT_ENROLLEE
		if (eap->code != EAP_REQUEST) {
			WSC_DEBUG("Invalid EAP code [0x%x]!\n", eap->code);
			return -1;
		}
		if (wsc->type == EAP_TYPE_IDENTITY) {
			return pktHandler_reqid(pCtx, pSta, eap->identifier);
		}

		if (eap->identifier != pSta->eap_reqid) {
			WSC_DEBUG("	!!!EAP identifier is not matched! [in eap=0x%x, in keep=0x%x]!!!\n",eap->identifier, pSta->eap_reqid);
			return -1;
		}
#endif
	}

	if (check_wsc_packet(pCtx, wsc) < 0)
		return -1;

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) { /*process reassembly first*/
#ifdef EAP_REASSEMBLY
		// now MsgPtr point to Message data
		MsgPtr = ((unsigned char *)wsc) + sizeof(struct eap_wsc_t);
		EAP_Length = ntohs(eap->length);

		/*tmplength = EAP_Length - 4 - 10*/
		tmplength = EAP_Length - EAP_HDRLEN - sizeof(struct eap_wsc_t) ;


		if (wsc->flags & EAP_FR_MF){
			if (wsc->flags & EAP_FR_LF){

				// when EAP_FR_LF== true ; now MsgPtr point to Message Length (2 bytes)
				pSta->total_message_len = (int)*(unsigned short *)MsgPtr;
				_DEBUG_PRINT("\n<<Receive First Frag EAP identifier=0x%x,total message length=%d\n",
					eap->identifier ,pSta->total_message_len);

				tmplength -= MSG_LEN_LEN;  // if(MF&&LF)the EPA include MessageLength(2B);Reduce that

				WSC_DEBUG("Frag EAP_Length =%d, 0x%x\n" ,EAP_Length,EAP_Length);
				WSC_DEBUG("Msg Data Length =%d, 0x%x \n",tmplength,tmplength);
				
				/* MsgPtr+MSG_LEN_LEN point to Message Data */
				memset(pSta->ReassemblyData ,'\0', EAP_FRAMENT_LEN);
				memcpy(pSta->ReassemblyData, MsgPtr+MSG_LEN_LEN ,tmplength);

				/* count msg_data_len */
				pSta->each_message_len = tmplength ;

				pSta->frag_state = 1 ; // indicate process frag message now
				send_wsc_frag_ack(pCtx , pSta);
				return 0;

			}
			else{
				_DEBUG_PRINT("\n<<Receive Middle Frag EAP identifier=0x%x\n",eap->identifier );

				WSC_DEBUG("Frag EAP_Length =%d, 0x%x\n" ,EAP_Length,EAP_Length);
				WSC_DEBUG("Msg Data Length =%d, 0x%x \n",tmplength,tmplength);
				
				/*now , MsgPtr point to Message Data */
				memcpy(&(pSta->ReassemblyData[pSta->each_message_len]), MsgPtr ,tmplength);

				/*count msg_data_len*/
				pSta->each_message_len += tmplength;
				
				pSta->frag_state = 1 ; // indicate process frag message now
				send_wsc_frag_ack(pCtx , pSta);
				return 0;
			}
		}
		else{

			if(pSta->frag_state==1){

				/* completed EAP reassembly*/
				_DEBUG_PRINT("\n<<Receive Last Frag EAP ,identifier:0x%x\n",eap->identifier);
				WSC_DEBUG("EAP_Length=%d, 0x%x\n",EAP_Length,EAP_Length);
				WSC_DEBUG("MsgDataLength =%d,0x%x\n",tmplength,tmplength);

				/* copy last EAP frag frame's msg data  */
				memcpy(&(pSta->ReassemblyData[pSta->each_message_len]), MsgPtr ,tmplength);						

				/*count msg_data_len*/
				pSta->each_message_len += tmplength;
				
				WSC_DEBUG("verify msglen ;length by count=%d,total message length=%d\n",
					pSta->each_message_len , pSta->total_message_len);
				
				if(pSta->each_message_len != pSta->total_message_len){
					WSC_DEBUG("		!!!EAP reassem ;total_message_len no match!!\n");
				}

				memcpy(MsgPtr , pSta->ReassemblyData, pSta->total_message_len);	

				/* modify EAP's Length */
				tmplength = EAP_HDRLEN + sizeof(struct eap_wsc_t) + pSta->total_message_len ;
				eap->length = htons(tmplength);
				
				pSta->frag_state = 0 ; // indicate process Last frag message now

				debug_out("after reassembly EAP message", pSta->ReassemblyData , pSta->total_message_len);
				

			}
			else{
				/* just for trace ;  this EAP Message is normal ; never do Reassembly */
				/*
				pData = search_tag(MsgPtr, TAG_MSG_TYPE, tmplength, &tag_len);
				if(pData){
					
					if (pData[0] >= MSG_TYPE_M1 && pData[0] <= MSG_TYPE_M8) {
						WSC_DEBUG("Rec Normal M%d msg length=%d\n",pData[0]-3,tmplength);
					}
					
				}
				*/
			}
		}
#endif
	}
#endif

	if (wsc->op_code == WSC_OP_ACK){
		return pktHandler_wsc_ack(pCtx, pSta, wsc);
	}
	else if (wsc->op_code == WSC_OP_NACK) {
#ifdef SUPPORT_UPNP
		if (pCtx->role == PROXY && (pSta->state == ST_UPNP_PROXY ||
			pSta->state == ST_UPNP_WAIT_DONE)){
			
			/* forward WSC_NACK to the external registrar */
			pktHandler_wsc_msg(pCtx, pSta, wsc, ntohs(eap->length)-EAP_HDRLEN);
			
		}
#endif
		return pktHandler_wsc_nack(pCtx, pSta, wsc);
	}
	else if (wsc->op_code == WSC_OP_MSG) {
		
		DEBUG_PRINT2("pCtx->role == %d; pSta->state == %d\n",pCtx->role , pSta->state);
		ret = pktHandler_wsc_msg(pCtx, pSta, wsc, ntohs(eap->length)-EAP_HDRLEN);
		if (ret > 0) {
			send_wsc_nack(pCtx, pSta, ret);
			if (!pCtx->is_ap)
				pSta->state = ST_WAIT_EAP_FAIL;
			pSta->tx_timeout = pCtx->tx_timeout;
			pSta->retry = 0;
		}
	
		// Fix the WLK v1.2 test issue ----------
		if (ret < 0) {
			send_wsc_nack(pCtx, pSta, ret);
			pSta->tx_timeout = pCtx->tx_timeout;
			pSta->retry = 0;
		}
		//-----------------david+2008-05-27

		return ret;
	}
	else if (wsc->op_code == WSC_OP_DONE) {
#ifdef SUPPORT_UPNP
		DEBUG_PRINT2("pCtx->role == %d; pSta->state == %d\n",pCtx->role , pSta->state);

		/*forward WSC_DONE to the external registrar*/
		/* for fix DTM M1-M2D proxy testing fail ; now we don't forward STA's DONE message to ERs*/
		if (pSta->state == ST_UPNP_WAIT_DONE && pCtx->ERisDTM==0 ) {
			WSC_DEBUG("	>>>Forward DONE msg to ER\n");
			pktHandler_wsc_msg(pCtx, pSta, wsc, ntohs(eap->length)-EAP_HDRLEN);
		}
#endif
		return pktHandler_wsc_done(pCtx, pSta);
	}
#ifdef SUPPORT_ENROLLEE
	else if (wsc->op_code == WSC_OP_START) {
		if (!pCtx->is_ap)
			return pktHandler_wsc_start(pCtx, pSta);
		else {
			DEBUG_ERR("Rx WSC_OP_START, but ourself is not a client!\n");
			return -1;
		}
	}
#endif
#ifdef WPS2DOTX	/*process when rec Fragment ack*/
	else if ((wsc->op_code == WSC_OP_FRAG_ACK) && (pCtx->current_wps_version == WPS_VERSION_V2)) {
		WSC_DEBUG("Rec ACK eap->identifier=0x%x \n",eap->identifier);

		/*enrollee mode sync registrar's EAP id here*/
		if(!pCtx->is_ap){
			pSta->eap_reqid = eap->identifier;
		}
		
		switch(pSta->state){
			case ST_WAIT_EAPOL_FRAG_ACK_M1:
				WSC_DEBUG("Rx M1_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M1 , 0);
				
				break;
			case ST_WAIT_EAPOL_FRAG_ACK_M2:
				WSC_DEBUG("Rx M2_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M2 , 0);
				break;
				
			case ST_WAIT_EAPOL_FRAG_ACK_M3:
				WSC_DEBUG("Rx M3_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M3 , 0);
				break;
			case ST_WAIT_EAPOL_FRAG_ACK_M4:
				WSC_DEBUG("Rx M4_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M4 , 0);
				break;
				
			case ST_WAIT_EAPOL_FRAG_ACK_M5:
				WSC_DEBUG("Rx M5_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M5 , 0);
				break;
			case ST_WAIT_EAPOL_FRAG_ACK_M6:
				WSC_DEBUG("Rx M6_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M6 , 0);
				break;
				
			case ST_WAIT_EAPOL_FRAG_ACK_M7:
				WSC_DEBUG("Rx M7_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M7 , 0);
				break;
			case ST_WAIT_EAPOL_FRAG_ACK_M8:
				WSC_DEBUG("Rx M8_FRAG_ACK \n");
				send_frag_msg(pCtx , pSta , ST_WAIT_EAPOL_FRAG_ACK_M8 , 0);
				break;
			default:
				WSC_DEBUG("rec unknow type %d \n",pSta->state);				
				break;
				
		}
		printf("\n");
	}
#endif
	else {
		DEBUG_ERR("Invalid WSC OP code [0x%x]!\n", wsc->op_code);
		return -1;
	}
		
	return 0;
}

static int evHandler_assocInd(CTX_Tp pCtx, unsigned char assoc_type)
{
	STA_CTX_Tp pSta=NULL;
	DOT11_WSC_ASSOC_IND *msg=NULL;
	unsigned char MACAddr[6];
	unsigned char wscIE_included=0;

	DBFENTER;
	
	
	msg = (DOT11_WSC_ASSOC_IND *)(pCtx->rx_buffer + FIFO_HEADER_LEN);

#ifdef FOR_DUAL_BAND
	if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
		if (((pCtx->fix_wzc_wep==1 && !pCtx->config_by_ext_reg) ||pCtx->fix_wzc_wep ==2) && pCtx->is_ap
				&& pCtx->encrypt_type == ENCRYPT_WEP && !pCtx->wps_triggered) {
			DEBUG_ERR("Discard Assoc-Ind for WZC issue in WEP\n");
			return -1;
		}
	}else if(pCtx->InterFaceComeIn == COME_FROM_WLAN1){
		if (((pCtx->fix_wzc_wep==1 && !pCtx->config_by_ext_reg) ||pCtx->fix_wzc_wep ==2) && pCtx->is_ap
				&& pCtx->encrypt_type2 == ENCRYPT_WEP && !pCtx->wps_triggered) {
			DEBUG_ERR("Discard Assoc-Ind for WZC issue in WEP\n");
			return -1;
		}
	}
#else //FOR_DUAL_BAND
	if (((pCtx->fix_wzc_wep==1 && !pCtx->config_by_ext_reg) ||pCtx->fix_wzc_wep ==2) && pCtx->is_ap
			&& pCtx->encrypt_type == ENCRYPT_WEP && !pCtx->wps_triggered) {
		DEBUG_ERR("Discard Assoc-Ind for WZC issue in WEP\n");
		return -1;
	}
#endif //FOR_DUAL_BAND

	/*fix unaligment issue, 2007-09-24+david
	  memcpy(MACAddr, msg->MACAddr, ETHER_ADDRLEN);*/
	do_bcopy(MACAddr, msg->MACAddr, ETHER_ADDRLEN);

	wscIE_included = msg->wscIE_included;
	
#ifdef DEBUG
	if (pCtx->debug) {
		char msg_buffer[200];

		_DEBUG_PRINT("<<Got Assoc Event>>\n");	

		convert_bin_to_str(MACAddr, ETHER_ADDRLEN, msg_buffer);
		if (!pCtx->is_ap)
			_DEBUG_PRINT("from AP(%s)\n", msg_buffer);
		else
			_DEBUG_PRINT("from STA(%s)\n", msg_buffer);
	}
#endif

#ifdef BLOCKED_ROGUE_STA
	if (pCtx->is_ap && pCtx->blocked_expired_time) {
		if (search_blocked_list(pCtx, MACAddr))
			return -1;
	}
#endif

	//WSC_pthread_mutex_lock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d Lock mutex\n", __FUNCTION__, __LINE__);

	if (!pCtx->start) {
		DEBUG_PRINT("Not started yet!\n");
		goto err_handle;
	}

	pSta = search_sta_entry(pCtx, (unsigned char *)MACAddr);
	if (pSta == NULL) {
		DEBUG_ERR("STA table full, can't do simple-config!\n");
		goto err_handle;
	}

	if (pCtx->registration_on >= 1 && pCtx->sta_invoke_reg != pSta) {
		if (!pSta->used)
			pCtx->num_sta++;
		
		WSC_DEBUG("Registration_on = %d\n", pCtx->registration_on);		
		WSC_DEBUG("Registration protocol is already in progress; ignore this association\n");

		goto err_handle;
	}
		
	if (pSta->used) {
		DEBUG_PRINT("STA entry existed\n");
#ifdef CLIENT_MODE
		if (!pCtx->is_ap) {
			DEBUG_PRINT("Ignore this association with AP\n");
			return 0;
		}
#endif
		if (pSta->locked) {
			DEBUG_ERR("STA is locked, ignore it!\n");
			goto err_handle;
		}

// Comment-out the following code by SD1 Mars advise ---------
#if 0
		if (pCtx->is_ap && (pSta->ap_role != ENROLLEE)) {
			pSta->allow_reconnect_count++;
			if (pSta->allow_reconnect_count >= 2) {
				DEBUG_PRINT("Exceeds re-connect limit, add into the block list\n");
				add_into_blocked_list(pCtx, pSta);
				reset_sta(pCtx, pSta, 1);
				return -1;
			}
		}
#endif
//-------------------------------------- david+2007-12-07
		
		reset_sta(pCtx, pSta, 0);
	}

	pCtx->num_sta++;
#ifdef CLIENT_MODE
	if (!pCtx->is_ap) {
		if (pCtx->num_sta >= 2) {
			DEBUG_ERR("Only associate with one AP, ignore this!\n");
			goto err_handle;
		}
	}
#endif
	pSta->used = 1;
	memcpy(pSta->addr, MACAddr, ETHER_ADDRLEN);
	DEBUG_PRINT("%s %d number of station = %d\n", __FUNCTION__, __LINE__, pCtx->num_sta);

	if (!pCtx->is_ap) {
#ifdef SUPPORT_ENROLLEE
		send_eapol_start(pCtx, pSta);
		pSta->state = ST_WAIT_REQ_ID;
		pSta->tx_timeout = pCtx->tx_timeout;
#ifdef CLIENT_MODE
		pCtx->connect_fail = 0;
		//pCtx->wait_assoc_ind = 0;
#endif
#else
		return -1;
#endif
	} else {
		/*
		  case 1:  make sure STA include WSC IE
  		  case 2:  because auth == open & no SSN or RSN IE ;so we 
	  		  treat this case as WSC IE included*/

		pSta->Assoc_wscIE_included = wscIE_included;
		pSta->tx_timeout = pCtx->tx_timeout;
		pSta->state = ST_WAIT_EAPOL_START;
	}

	pSta->reg_timeout = pCtx->reg_timeout;
	pSta->retry = 0;

	//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
	
	return 0;

err_handle:
	if (pSta) {
#ifdef CLIENT_MODE
		if (!pCtx->is_ap)
			pSta->do_not_rescan = 1;
#endif

		reset_sta(pCtx, pSta, 1);
	}
	
	if (pCtx->is_ap && msg->wscIE_included==1){
		WSC_DEBUG("IssueDisconnect\n");
		IssueDisconnect(pCtx->wlan_interface_name, msg->MACAddr, 1);
	}
	
	//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
	return -1;
}

#ifdef SUPPORT_UPNP
#ifdef PREVENT_PROBE_DEADLOCK
static struct probe_node *search_probe_sta(CTX_Tp pCtx, unsigned char *addr)
{
	int i, idx=-1;

	for (i=0; i<MAX_WSC_PROBE_STA; i++) {
		if (pCtx->probe_list[i].used == 0) {
			if (idx < 0)
				idx = i;
			continue;
		}
		if (!memcmp(pCtx->probe_list[i].ProbeMACAddr, addr, ETHER_ADDRLEN))
			break;
	}

	if ( i != MAX_STA_NUM)
		return (&pCtx->probe_list[i]);

	if (idx >= 0)
		return (&pCtx->probe_list[idx]);
	else{
		unsigned int largest=0;
		idx=0;
		for (i=0; i<MAX_WSC_PROBE_STA; i++) {
			//cathy, difftime return a double value, our LIB has problem to access float or double value
			//unsigned int time_offset = difftime(time(NULL), pCtx->probe_list[i].time_stamp);
			unsigned int time_offset = time(NULL) - pCtx->probe_list[i].time_stamp;
			if (time_offset > largest) {
				idx = i;
				largest = time_offset;
			}
		}
		memset(&pCtx->probe_list[idx], 0, sizeof(struct probe_node));
		pCtx->probe_list_count--;
		return (&pCtx->probe_list[idx]);
	}
}
#endif //PREVENT_PROBE_DEADLOCK

static int evHandler_probe_req_Ind(CTX_Tp pCtx)
{
	DOT11_PROBE_REQUEST_IND *msg = (DOT11_PROBE_REQUEST_IND *)(pCtx->rx_buffer + FIFO_HEADER_LEN);
#ifdef MUL_PBC_DETECTTION
	unsigned char *pData;
	int tag_len;
	unsigned short sVal;
#endif
	int ie_len;

#ifdef DEBUG
	char msg_buffer[15];
#endif
	
#ifdef WPS2DOTX
	unsigned char *Ptr;
	int IS_V2 =0 ;
	int Req_Enroll=0;
	unsigned char *StmpPtr ;
	unsigned char *EVPtr ;
	int msg_len_tmp ;
	int wvfound = 0;
#endif
	DBFENTER;

	if (!pCtx->start) {
		DEBUG_PRINT("Not started yet!\n");
		return -1;
	}

#ifdef DEBUG
	convert_bin_to_str(msg->MACAddr, 6, msg_buffer);
	_DEBUG_PRINT("\n<<Got Probe_Req,from Sta(%s)>>\n",msg_buffer);
	if(pCtx->debug2)
		debug_out("PROBE_REQUEST WSC IE", msg->ProbeIE, msg->ProbeIELen);
#endif

	memcpy(&sVal, pCtx->rx_buffer+FIFO_HEADER_LEN+8, 2);
	ie_len = (int)sVal;

	unsigned char *MsgStart = (msg->ProbeIE+2+4);
	int MsgLen = (ie_len-2-4);


#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2) {
		StmpPtr = MsgStart;
		msg_len_tmp = MsgLen;

		pData = search_tag(StmpPtr, TAG_VENDOR_EXT, msg_len_tmp, &tag_len);
		if (pData) {
			if(!memcmp(pData , WSC_VENDOR_OUI ,3 ))
			{
				//WSC_DEBUG("WFA-OUI\n");
				debug_out("verdor ext:",pData,tag_len );
				wvfound = 1 ;
			}
		}
		
		if(wvfound){
			int lent2=0;
			EVPtr = search_VendorExt_tag(pData ,VENDOR_VERSION2 , tag_len , &lent2);
			if(EVPtr){
				IS_V2 = 1;
				RX_DEBUG("Rev version2(0x%x) at  ProReq\n",EVPtr[0]);
			}
		}
		pData = search_tag(MsgStart, TAG_REQ_TO_ENROLL, MsgLen, &tag_len);
		if (pData != NULL && tag_len == 1) {
			Ptr = msg->MACAddr;
			
			if(pData[0]){
				Req_Enroll = 1;
				//WSC_DEBUG("REQ_TO_ENROLL=%x \n",pData[0]);
			}
		}else{
			Req_Enroll =0 ;
		}
		pData = search_tag(MsgStart, TAG_REQ_DEV_TYPE, MsgLen, &tag_len);
		if (pData != NULL && tag_len == 8) {
			WSC_DEBUG("Requested Device Type:\n	 len=%d, category_id=0x%x, oui=%02x%02x%02x%02x, sub_category_id=0x%x\n",
			tag_len, ntohs(*((unsigned short *)pData)), pData[2],pData[3],pData[4],pData[5],ntohs(*((unsigned short *)&pData[6])));
			if(ntohs(*((unsigned short *)pData))!=6 || 
				!( pData[2]==0x0 && pData[3]==0x50 && pData[4]==0xF2 && pData[5]==0x04)){
				WSC_DEBUG("Requested Device Type not our inster");
			}
		}
	}
#endif

/*
	if (pCtx->debug) {
		char msg_buffer[30];
		convert_bin_to_str(msg->MACAddr, 6, msg_buffer);
		_DEBUG_PRINT("from client mac: %s\n\n\n", msg_buffer);
	}
*/


#ifdef MUL_PBC_DETECTTION
	if (pCtx->is_ap && IS_PBC_METHOD(pCtx->config_method) &&
		!pCtx->disable_MulPBC_detection) {
		pData = search_tag(MsgStart, TAG_DEVICE_PASSWORD_ID, MsgLen, &tag_len);
		if (pData != NULL) {
			memcpy(&sVal, pData, tag_len);
			sVal = ntohs(sVal);
			if (sVal == PASS_ID_PB) {
				pData = search_tag(MsgStart, TAG_UUID_E, MsgLen, &tag_len);
				if (pData != NULL && tag_len == UUID_LEN) {
					WSC_pthread_mutex_lock(&pCtx->PBCMutex);
					//DEBUG_PRINT("%s %d Lock PBC mutex\n", __FUNCTION__, __LINE__);
										
					search_active_pbc_sta(pCtx, msg->MACAddr, pData);
					WSC_pthread_mutex_unlock(&pCtx->PBCMutex);
					//DEBUG_PRINT("%s %d unlock PBC mutex\n", __FUNCTION__, __LINE__);
				}
			}
		}
	}
#endif






#ifdef PREVENT_PROBE_DEADLOCK // scheduled by one second timer ( by sigHandler_alarm() &  PREVENT_PROBE_DEADLOCK)
	if (pCtx->is_ap && (pCtx->role == REGISTRAR || pCtx->role == PROXY) &&
		(pCtx->original_role != ENROLLEE) &&
		!pCtx->pb_pressed && !pCtx->pin_assigned && 
		pCtx->upnp && pCtx->TotalSubscriptions) {
		struct probe_node* probe_sta;
		
		probe_sta = search_probe_sta(pCtx, msg->MACAddr);
		probe_sta->ProbeIELen = MsgLen;
		memcpy(probe_sta->ProbeIE, ie_len+2+4, probe_sta->ProbeIELen);
		if (!probe_sta->used) {
			probe_sta->used = 1;
			memcpy(probe_sta->ProbeMACAddr, msg->MACAddr, 6);
			pCtx->probe_list_count++;
		}
		probe_sta->time_stamp = time(NULL);
		probe_sta->sent = 0;
	}
#else // will cause eventing problems in UPnP SDK
	if (pCtx->is_ap && (pCtx->role == REGISTRAR || pCtx->role == PROXY) &&
			(pCtx->original_role != ENROLLEE) &&
			!pCtx->pb_pressed && !pCtx->pin_assigned && 
			pCtx->upnp && pCtx->TotalSubscriptions) {
		struct WSC_packet packet;
		
		packet.EventType = WSC_PROBE_FRAME;
		packet.EventID = WSC_PUTWLANREQUEST;
		convert_bin_to_str_UPnP(msg->MACAddr, 6, packet.EventMac);
		packet.tx_buffer = MsgStart;
		packet.tx_size = MsgLen;

		if (WSCUpnpTxmit(&packet) != WSC_UPNP_SUCCESS) {
			DEBUG_ERR("WSCUpnpTxmit() return error!\n");
			return -1;
		}
	}
#endif

	return 0;
}
#endif

#ifdef CLIENT_MODE
#ifdef DEBUG

/**
 *	@brief  get WPA/WPA2 information
 *
 *	use 1 timestamp (32-bit variable) to carry WPA/WPA2 info \n
 *	1st 16-bit:                 WPA \n
 *  |          auth       |              unicast cipher              |              multicast cipher            |	\n
 *     15    14    13   12      11      10     9     8       7      6      5       4      3     2       1      0	\n
 *	+-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 *	| Rsv | PSK | 1X | Rsv | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp |	\n
 *	+-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 *	2nd 16-bit:                 WPA2 \n
 *            auth       |              unicast cipher              |              multicast cipher            |	\n
 *	  15    14    13   12      11      10     9     8       7      6      5       4      3     2       1      0		\n
 *  +-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 *	| Rsv | PSK | 1X | Rsv | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp | WEP104 | CCMP | Rsv | TKIP | WEP40 | Grp |	\n
 *  +-----+-----+----+-----+--------+------+-----+------+-------+-----+--------+------+-----+------+-------+-----+	\n
 */
// reference from wlan driver get_security_info()
static void print_bss(CTX_Tp pCtx, int idx)
{
	BssDscr *pBss;
	char allInfoBuf[256];
	char InfoBuf[10];
	pBss = &pCtx->ss_status.bssdb[idx];
	memset(allInfoBuf , '\0' , sizeof(allInfoBuf));
	memset(InfoBuf , '\0' , sizeof(InfoBuf));
	
	sprintf(allInfoBuf,"AP info: idx(%d), %02x:%02x:%02x:%02x:%02x:%02x", idx,
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);

	strcat(allInfoBuf , "\n\t type:");
	if(pBss->bdCap & cIBSS)
		strcat(allInfoBuf , "Ad hoc");
	else
		strcat(allInfoBuf , "AP");

	strcat(allInfoBuf , "\n\t ssid:");
	strcat(allInfoBuf ,pBss->bdSsIdBuf );
	
	strcat(allInfoBuf, "\n\t band:");
	if (pBss->network & BAND_11B)
		strcat(allInfoBuf, "(B)");

	if (pBss->network & BAND_11G)
		strcat(allInfoBuf, "(G)");
	
	if (pBss->network & BAND_11N)
		strcat(allInfoBuf, "(N)");

	if (pBss->network & BAND_11A)
		strcat(allInfoBuf, "(A)");

	strcat(allInfoBuf , "\n\t channel:");
	sprintf(InfoBuf , "%d" , pBss->ChannelNumber);
	strcat(allInfoBuf , InfoBuf);

	strcat(allInfoBuf , "\n\t security:");
	
	if ((pBss->bdCap & cPrivacy) == 0){
		strcat(allInfoBuf , "no");
	}else{
	
		if (pBss->bdTstamp[0] == 0){
			strcat(allInfoBuf , "WEP");
		}else{
			int wpa_exist = 0 ;
			if (pBss->bdTstamp[0] & 0x0000ffff) {
				strcat(allInfoBuf , "WPA");
				if(pBss->bdTstamp[0] & (1<<10))
					strcat(allInfoBuf , "-PSK");
				wpa_exist = 1;

				if(pBss->bdTstamp[0] & (1<<8))
					strcat(allInfoBuf , "+TKIP");
				if(pBss->bdTstamp[0] & (1<<10))
					strcat(allInfoBuf , "+AES");
			}
			
			if (pBss->bdTstamp[0] & 0xffff0000) {
				if (wpa_exist)
					strcat(allInfoBuf , "/");
				strcat(allInfoBuf , "WPA2");
				if(pBss->bdTstamp[0] & (1<<(14+16)))
					strcat(allInfoBuf , "-PSK");

				if(pBss->bdTstamp[0] & (1<<(8+16)))
					strcat(allInfoBuf , "+TKIP");
				if(pBss->bdTstamp[0] & (1<<(10+16)))
					strcat(allInfoBuf , "+AES");
				
			}
		}
	}

	WSC_DEBUG("%s\n", allInfoBuf);
	if(pCtx->debug){
		if (pCtx->ss_ie.ie[idx].data[0] == 221 && pCtx->ss_ie.ie[idx].data[1] > 0)
			debug_out("WSC IE", &pCtx->ss_ie.ie[idx].data[2], (int)pCtx->ss_ie.ie[idx].data[1]);
	}
}
#endif

#ifdef MUL_PBC_DETECTTION
static void Search_PBC_AP(CTX_Tp pCtx)
{
	int i, len, tag_len;
	unsigned char *pData;
	unsigned short usVal;

	for (i=0; i<pCtx->ss_status.number && pCtx->ss_status.number!=0xff; i++) {
		if (pCtx->ss_ie.ie[i].data[0] == WSC_IE_ID && pCtx->ss_ie.ie[i].data[1] > 0) {
			len = (int)pCtx->ss_ie.ie[i].data[1] - 4;
			pData = search_tag(&pCtx->ss_ie.ie[i].data[6], TAG_SELECTED_REGITRAR, len, &tag_len);
			if (pData == NULL || pData[0] != 1)
				continue;
			pData = search_tag(&pCtx->ss_ie.ie[i].data[6], TAG_DEVICE_PASSWORD_ID, len, &tag_len);
			if (pData == NULL || tag_len != 2)
				continue;
			memcpy(&usVal, pData, 2);
			usVal = ntohs(usVal);
			if (usVal == PASS_ID_PB) {
				pData = search_tag(&pCtx->ss_ie.ie[i].data[6], TAG_UUID_E, len, &tag_len);
				if (pData) {
					if (tag_len != 16)
						continue;
					search_active_pbc_sta(pCtx, pCtx->ss_status.bssdb[i].bdBssId, pData);
				}
				else {
					unsigned char uuid[16];
					memset(uuid, 0, 16);
					search_active_pbc_sta(pCtx, pCtx->ss_status.bssdb[i].bdBssId, uuid);
				}
			}
		}
	}
}
#endif
#define	_SPEC_MAC_ 0x001
#define	_SPEC_SSID_ 0x010
static int connect_wps_ap(CTX_Tp pCtx, int method)
{
	int i, len, tag_len, found, wait_time;
	unsigned char *pData, res;
	unsigned short usVal;
#ifdef WPS2DOTX
	unsigned char *StmpPtr ;
	int wvfound = 0;
#endif

	unsigned char *wsc_ie_start;
	BssDscr *pBss;

	//get_next:
	DEBUG_PRINT("ss_status.number =%d\n", pCtx->ss_status.number);
	if( strlen(pCtx->SPEC_SSID) ){
		WSC_DEBUG("pCtx->SPEC_SSID=%s\n",pCtx->SPEC_SSID);
	}

	if( strlen(pCtx->SPEC_MAC) ){
		WSC_DEBUG("pCtx->SPEC_SSID=%s\n",pCtx->SPEC_SSID);
		MAC_PRINT(pCtx->SPEC_MAC);
	}

	for (found=0, i=++pCtx->join_idx; i<pCtx->ss_status.number && pCtx->ss_status.number!=0xff; i++) {

#ifdef CONNECT_PROXY_AP
		if(pCtx->ss_status.number<=MAX_RETRY_AP_NUM && strlen(pCtx->SPEC_SSID)==0  && is_zero_ether_addr(pCtx->SPEC_MAC)){
			if ((method == CONFIG_METHOD_PIN) && (pCtx->role == ENROLLEE) && (!pCtx->is_ap) )
				if(search_blocked_ap_list(pCtx, i, 1))
					continue;
		}
#endif

		if (pCtx->ss_ie.ie[i].data[0] == WSC_IE_ID && pCtx->ss_ie.ie[i].data[1] > 0) {
			len = (int)pCtx->ss_ie.ie[i].data[1] - 4;
			wsc_ie_start = (&pCtx->ss_ie.ie[i].data[6]) ;
			
			pData = search_tag(wsc_ie_start, TAG_SELECTED_REGITRAR, len, &tag_len);
			if (pCtx->role == REGISTRAR) {
				if (pData)
					continue;
				else {
					found = 1;
					break;
				}
			}
			else
			{

				pBss = &pCtx->ss_status.bssdb[i];
				/*we have assigned special SSID to connect,must match*/
				/* support  special SSID , 2011-0505 */
				if( strlen(pCtx->SPEC_SSID) ){
					if(!strcmp(pBss->bdSsIdBuf,pCtx->SPEC_SSID)){
						found = 5;
						break;
					}else{
						continue;
					}
				}

				/*we have assigned special MacAddr to connect,must match*/
				/* support  special MAC Addr , 2011-0505 */
				if( !is_zero_ether_addr(pCtx->SPEC_MAC) ){
					if(!memcmp(pBss->bdBssId,pCtx->SPEC_MAC,6)){
						found = 6;
						break;
					}else{
						continue;
					}
				}
				

				/*	skip check TAG_SELECTED_REGITRAR ; for MicroSsft softAp IOT issue  ; don't apply under WPS2x for stable*/ 
				if (pData == NULL)
					continue;
						
				if(pData[0] == 0){
					pData = search_tag(wsc_ie_start, TAG_MANUFACTURER, len, &tag_len);
					if(tag_len == strlen("Microsoft")){
						if(strncmp(pData ,"Microsoft", tag_len)!=0)
							continue;
					}else{
						continue;
					}
				}

				pData = search_tag(wsc_ie_start, TAG_DEVICE_PASSWORD_ID, len, &tag_len);
				if (pData == NULL || tag_len != 2)
					continue;
				memcpy(&usVal, pData, 2);
				usVal = ntohs(usVal);
				if (method == CONFIG_METHOD_PBC && usVal == PASS_ID_PB)
				{
					found = 2;
					break;
				}
				if (method == CONFIG_METHOD_PIN && 
						(usVal == PASS_ID_DEFAULT || usVal == PASS_ID_USER))
				{
					found = 3;
					break;
				}

#ifdef WPS2DOTX		
				if (pCtx->current_wps_version == WPS_VERSION_V2) {
					pData = search_tag(wsc_ie_start, TAG_VENDOR_EXT, len, &tag_len);	
					if (pData != NULL) {
						if(!memcmp(pData , WSC_VENDOR_OUI ,3 ))
						{
							WSC_DEBUG("found WFA-vendor-OUI!!\n");
							debug_out("verdor ext:",pData,tag_len );				
							wvfound = 1 ;
						}
					}
			
					if(wvfound){
						int lent2=0;
						StmpPtr = search_VendorExt_tag(pData ,VENDOR_VERSION2 , tag_len , &lent2);
						if(StmpPtr){
							RX_DEBUG("version2(0x%x) EAP\n",StmpPtr[0]);

							StmpPtr = search_VendorExt_tag(pData ,VENDOR_AUTHMAC, tag_len , &lent2);
							if(StmpPtr){
								if(!memcmp(StmpPtr,BroadCastMac,6) || !memcmp(StmpPtr,pCtx->our_addr,6)){
									RX_DEBUG("		AuthMac match!!!:");
									MAC_PRINT(StmpPtr);
									found = 7;
									break;
								}
							}
						}
					}
				}
#endif
			}
		}
	}
#ifdef CONNECT_PROXY_AP
	if(pCtx->ss_status.number<=MAX_RETRY_AP_NUM 
		&& strlen(pCtx->SPEC_SSID)==0  
		&& is_zero_ether_addr(pCtx->SPEC_MAC))
	{
		if ((method == CONFIG_METHOD_PIN) && (pCtx->role == ENROLLEE) && (!pCtx->is_ap)) {
		
		if (found == 0) {

			pCtx->join_idx = -1;

			for (found=0, i=++pCtx->join_idx; i<pCtx->ss_status.number && pCtx->ss_status.number!=0xff; i++) {
				unsigned char *config_state; 

				if(pCtx->blocked_unselected_ap > MAX_RETRY_AP_NUM)
					break;

				if(search_blocked_ap_list(pCtx, i, 0))
					continue;
				
				len = (int)pCtx->ss_ie.ie[i].data[1] - 4;
				wsc_ie_start = (&pCtx->ss_ie.ie[i].data[6]) ;
				
				pData = search_tag(wsc_ie_start, TAG_SIMPLE_CONFIG_STATE, len, &tag_len);

				if(pData != NULL)
					config_state = pData;
				else
					continue;

				if(*config_state == 0x2){
					found = 8;
					break;
				}
				else{
					continue;
				}
			}

		}


		if(found == 0) {
			WSC_DEBUG("proxy-like NEXT LOOP ~~~~ \n");
			clear_blocked_ap_list(pCtx);
			i = pCtx->ss_status.number;
		}
		else if(found == 8) {
			WSC_DEBUG("Try to connect to an configured(proxy-like) WPS AP\n\n");
			add_into_blocked_ap_list(pCtx, i, 0);
		}
		else{
			add_into_blocked_ap_list(pCtx, i, 1);
		}

		}
	}
#endif

	pCtx->join_idx = i;
	pCtx->connect_method = method;

	if (found) {
		if(found==2){
			WSC_DEBUG("Found AP support WPS PBC\n\n");
		}
		else if(found==3){
			WSC_DEBUG("Found AP support WPS PIN\n\n");
		}
#ifdef P2P_SUPPORT
		else if(found==4){
			WSC_DEBUG("Found P2P-AP support WPS PIN\n\n");
		}
#endif
		else if(found==5){
			/* support  special SSID , 2011-0505 */
			WSC_DEBUG("connect to Assigned SSID(%s) AP\n",pCtx->SPEC_SSID);
		}
		else if(found==6){
			/* support  special MAC , 2011-0505 */
			WSC_DEBUG("connect to Assigned MAC AP\n");
			MAC_PRINT(pCtx->SPEC_MAC);
		}else if(found==7){
			WSC_DEBUG("2.0 AP include authorized Mac\n");
		}

#ifdef DEBUG
		print_bss(pCtx, i);
#endif
		//pCtx->join_idx = i;

		res = i;
		wait_time = 0;

#if 	1	// 0728
		IssueDisconnect(pCtx->wlan_interface_name,pCtx->ss_status.bssdb[i].bdBssId, 1);
		WSC_DEBUG("issue disconnect to ");
		MAC_PRINT(pCtx->ss_status.bssdb[i].bdBssId);
#endif
		
		while (1) {
			if (!pCtx->start)
				return 0;
			
			WSC_DEBUG("issue join-req!\n");
			
			if (getWlJoinRequest(pCtx->wlan_interface_name, &pCtx->ss_status.bssdb[i], &res) < 0) {
				WSC_DEBUG("Join request failed!\n");
				//goto get_next;
				return 0;
			}
			if (res == 1) { // wait
				if (wait_time++ > 5) {
					WSC_DEBUG("connect-request timeout!\n");
					//goto get_next;
					return 0;
				}
				sleep(1);
				continue;
			}
			break;
		}

		if (res == 2) { // invalid index
			WSC_DEBUG("Connect failed, invalid index!\n");
			//goto get_next;
			return 0;
		}
		else {
			wait_time = 0;
			while (1) {
				if (!pCtx->start)
					return 0;
				if (getWlJoinResult(pCtx->wlan_interface_name, &res) < 0) {
					WSC_DEBUG("Get Join result failed!\n");
					//goto get_next;
					return 0;
				}
				if (res != 0xff) { // completed
					/*if (wait_time++ > 10) {
						DEBUG_ERR("connect timeout!\n");
						goto get_next;
						return 0;
					}*/
					break;
				}
				if (wait_time++ > 5) {
					WSC_DEBUG("connect timeout!\n");
					//goto get_next;
					return 0;
				}
				sleep(1);
			}

			if (res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active) {
				WSC_DEBUG("Connect failed!\n");
				//goto get_next;
				return 0;
			}
			else{
				WSC_DEBUG("\n	>>>connect to AP(channel:%d)\n",pCtx->ss_status.bssdb[i].ChannelNumber);
				
				if(pCtx->ss_status.bssdb[i].ChannelNumber > 14){
					pCtx->STAmodeNegoWith = COMEFROM5G;
				}else{
					pCtx->STAmodeNegoWith = COMEFROM24G;
				}
			}

			pCtx->wait_reinit = 0;
			pCtx->connect_fail = 0;

			//pCtx->wait_assoc_ind = 3;// bug fixed
			//pCtx->connect_method = method;
			return 1;
		}
	}
	return 0;
}

int issue_scan_req(CTX_Tp pCtx, int method)
{
	int status, wait_time=0;
	int wait_time_define=30;
	//struct sysinfo info;

	// issue scan-request
//scan_next:
#if 0
	sysinfo(&info);
	if (pCtx->target_time < info.uptime) {
		pCtx->start = 0;
		return 0;
	}
#endif

#ifdef P2P_SUPPORT
    if(pCtx->p2p_trigger_type==P2P_PRE_CLIENT){
        wait_time_define=10;
    }
#endif

	while (1) {
		if (!pCtx->start)
			return 0;

		WSC_DEBUG("issue scan-req!, wlan_interface_name is %s\n", pCtx->wlan_interface_name);
		
		if (wlioctl_scan_reqest(pCtx->wlan_interface_name, &status) < 0) {
			WSC_DEBUG("Site-survey request failed!\n");
			return -1;
		}
		if (status != 0) {	// not ready
			if (wait_time++ > 5) {
				WSC_DEBUG("scan request timeout!\n");
				return -1;
			}
			sleep(1);
		}
		else{
			break;
		}
	}

	// get scan result, wait until scan completely
	wait_time = 0;
	while (1) {
		if (!pCtx->start)
			return 0;
		pCtx->ss_status.number = 1;	// only request request status
		if (wlioctl_scan_result(pCtx->wlan_interface_name, &pCtx->ss_status) < 0) {
			WSC_DEBUG("Read site-survey status failed!\n");
			return -1;
		}
		if (pCtx->ss_status.number == 0xff) {   // in progress
			if (wait_time++ > 10) {
				WSC_DEBUG("scan timeout!\n");
				return -1;
			}
			sleep(1);
		}
		else
			break;
	}

	memset(&pCtx->ss_status, '\0', sizeof(SS_STATUS_T));
	memset(&pCtx->ss_ie, '\0', sizeof(SS_IE_T));

	pCtx->ss_status.number = 0; // request BSS DB
	if (wlioctl_scan_result(pCtx->wlan_interface_name, &pCtx->ss_status) < 0) {
		WSC_DEBUG("Read site-survey DB failed!\n");
		return -1;
	}

	pCtx->ss_ie.number = 2; // request WPS IE
	if (wlioctl_scan_result(pCtx->wlan_interface_name, (SS_STATUS_Tp)&pCtx->ss_ie) < 0) {
		WSC_DEBUG("Read site-survey IE failed!\n");
		return -1;
	}

#ifdef MUL_PBC_DETECTTION
	if (pCtx->role == ENROLLEE && method == CONFIG_METHOD_PBC &&
		!pCtx->disable_MulPBC_detection) {
		Search_PBC_AP(pCtx);
		
		if (pCtx->active_pbc_sta_count > 1) {
			WSC_DEBUG("\n\n		!!!Multiple PBC sessions [%d] detected!\n\n", pCtx->active_pbc_sta_count);
			SwitchSessionOverlap_LED_On(pCtx);
			return -1;
		}
	}
#endif

	pCtx->join_idx = -1;
	
	if (!connect_wps_ap(pCtx, method)) { // fail
		//if (pCtx->start)
			//goto scan_next;
		pCtx->connect_fail = 1;
		return -1;
	}
	
	return 0;
}
#endif // CLIENT_MODE

#ifdef NO_IWCONTROL
#define MAXDATALEN      1560	// jimmylin: org:256, enlarge for pass EAP packet by event queue
typedef struct _DOT11_REQUEST{
        unsigned char   EventId;
}DOT11_REQUEST;

static void wlanIndEvt(int sig_no)
{
	CTX_Tp pCtx = pGlobalCtx;
	struct iwreq wrq;
	unsigned char tmpbuf[MAXDATALEN];
	DOT11_REQUEST *req;
	int skfd, evt, next_evt;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("wscd: socket() error!\n");
		return;
	}
	
get_next:
	strcpy(wrq.ifr_name, pCtx->wlan_interface_name);
	req = (DOT11_REQUEST *)tmpbuf;
	wrq.u.data.pointer = tmpbuf;
	req->EventId = DOT11_EVENT_REQUEST;
	wrq.u.data.length = sizeof(DOT11_REQUEST);
  	if (ioctl(skfd, SIOCGIWIND, &wrq) < 0) {
		printf("wscd: ioctl(SIOCGIWIND) error!\n");
		return;
	}
	memcpy(pCtx->rx_buffer + FIFO_HEADER_LEN, wrq.u.data.pointer, wrq.u.data.length);
	evt = get_wlan_evt(pCtx);
	next_evt = *(pCtx->rx_buffer + FIFO_HEADER_LEN + 1);
	if (evt >= 0 && (pCtx->start || (evt == EV_PIN_INPUT &&
		!pCtx->is_ap && pCtx->role == REGISTRAR)))
		process_event(pCtx, evt);

	if (next_evt != 0)
		goto get_next;
	
	close(skfd);
}

#define SIOCSAPPPID     0x8b3e
int register_PID(char *ifname)
{
	struct iwreq	wrq;
	pid_t	 pid;
	int	skfd;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		return -1;
	}

  	/* Get wireless name */
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
  	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

	pid = getpid();
	wrq.u.data.pointer = (caddr_t)&pid;
	wrq.u.data.length = sizeof(pid_t);

  	if(ioctl(skfd, SIOCSAPPPID, &wrq) < 0)
	{
    	// If no wireless name : no wireless extensions
//		strerror(errno);
		close(skfd);
		return(-1);
	}
	close(skfd);
	return 1;
}
#endif // NO_IWCONTROL

int init_wlan(CTX_Tp pCtx, int reinit)
{
	char tmpbuf[1024];
	int len;
#ifdef NO_IWCONTROL
	int fdflags;
#else
	struct stat status;
#endif

	DBFENTER;

	if (!reinit) {
#ifdef NO_IWCONTROL
#ifndef USE_POLLING
		sprintf(tmpbuf, "/dev/wl_chr%c", pCtx->wlan_interface_name[strlen(pCtx->wlan_interface_name)-1]);
		if ((pCtx->wl_chr_fd = open(tmpbuf, O_RDWR, 0)) < 0) {
			int retval;
			retval = register_PID(pCtx->wlan_interface_name);
			if (retval > 0)
				signal(SIGIO, wlanIndEvt);
			else {
				printf("wscd: unable to open an wl_chr device and PID registration fail.\n");
				return -1;
			}
		}
		else {
			signal(SIGIO, wlanIndEvt);
			fcntl(pCtx->wl_chr_fd, F_SETOWN, getpid());
			fdflags = fcntl(pCtx->wl_chr_fd, F_GETFL);
			fcntl(pCtx->wl_chr_fd, F_SETFL, fdflags | FASYNC);
		}
#endif
#else
		/*we are use this case now 0922*/

#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
#endif
		{
			/* Create fifo */
			if (stat(pCtx->fifo_name, &status) == 0)
				unlink(pCtx->fifo_name);
			
			if (mkfifo(pCtx->fifo_name, FIFO_MODE) < 0) {
				WSC_DEBUG("mkfifo %s fail!\n",pCtx->fifo_name);
				return -1;
			}

			/* Open fifo, indication event queue of wlan driver */
			// Kaohj -- it may read-block listen_and_process_event(), so set to noblocking
		  	//pCtx->fifo = open(pCtx->fifo_name, O_RDONLY, 0);
		  	pCtx->fifo = open(pCtx->fifo_name, O_RDONLY|O_NONBLOCK, 0);
		  	if (pCtx->fifo < 0) {
		  		DEBUG_ERR("open fifo(%s) fail!\n",pCtx->fifo_name);
			  	return -1;
			}
		}

#ifdef FOR_DUAL_BAND
		/*for pid naming sync ; any mode will create fifo2  */
		if(pCtx->wlan1_wsc_disabled == 0)
		{
			if (stat(pCtx->fifo_name2, &status) == 0)
				unlink(pCtx->fifo_name2);
			
			if (mkfifo(pCtx->fifo_name2, FIFO_MODE) < 0) {
				DEBUG_ERR("mkfifo %s fail!\n",pCtx->fifo_name2);
				return -1;
			}

			/* Open fifo, indication event queue of wlan driver */
			pCtx->fifo2 = open(pCtx->fifo_name2, O_RDONLY|O_NONBLOCK , 0);
		  	if (pCtx->fifo2 < 0) {
				DEBUG_ERR("open fifo %s fail!\n",pCtx->fifo_name2);
		  		return -1;
			}
		}
					
#endif //FOR_DUAL_BAND
#endif	//NO_IWCONTROL
	}
	
	
	
	if (!pCtx->is_ap) {	/* CLIENT mode */
#ifdef CLIENT_MODE
		if (pCtx->use_ie && pCtx->start) {
			len = build_probe_request_ie(pCtx, 0, tmpbuf);
			if (len == 0)
				DEBUG_ERR("Length of probe request IE exceeds %d\n", MAX_WSC_IE_LEN);
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_REQ) < 0){
		  		WSC_DEBUG("update IE fail\n");
				return -1;
			}

			len = build_assoc_request_ie(pCtx, tmpbuf);
			if (len == 0)
				DEBUG_ERR("Length of assoc request IE exceeds %d\n", MAX_WSC_IE_LEN);
			if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_ASSOC_REQ) < 0){
		  		WSC_DEBUG("update IE fail\n");
				return -1;
			}
		}
#endif
	}
	else {	/* AP mode */
		if (pCtx->use_ie) {
			len = build_beacon_ie(pCtx, 0, 0, 0, tmpbuf);

			if (((!pCtx->fix_wzc_wep || pCtx->config_by_ext_reg) && pCtx->fix_wzc_wep != 2) 
					&& (pCtx->encrypt_type == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) // add provisioning service ie 
				len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));

#ifdef FOR_DUAL_BAND
			if(!pCtx->wlan0_wsc_disabled)
#endif
			{
				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0)
					return -1;
			}

#ifdef FOR_DUAL_BAND
			if(!pCtx->wlan1_wsc_disabled){
				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
						DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_BEACON) < 0){
			  		WSC_DEBUG("update IE fail\n");
					return -1;
				}
			}
#endif //FOR_DUAL_BAND
			len = build_probe_rsp_ie(pCtx, 0, 0, 0, tmpbuf);

			if (((!pCtx->fix_wzc_wep || pCtx->config_by_ext_reg) && pCtx->fix_wzc_wep != 2) 
					&& (pCtx->encrypt_type == ENCRYPT_WEP) && (pCtx->current_wps_version == WPS_VERSION_V1)) // add provisioning service ie 
				len += build_provisioning_service_ie((unsigned char *)(tmpbuf+len));

			if (len > MAX_WSC_IE_LEN) {
				DEBUG_ERR("Length of IE exceeds %d\n", MAX_WSC_IE_LEN);
				return -1;
			}
#ifdef FOR_DUAL_BAND
			if(!pCtx->wlan0_wsc_disabled)
#endif
			{
				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
						DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0){
					WSC_DEBUG("update IE fail\n");
					return -1;
				}
			}
			
#ifdef FOR_DUAL_BAND
			if(!pCtx->wlan1_wsc_disabled){
				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
						DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_RSP) < 0){
			  		WSC_DEBUG("update IE fail\n");
					return -1;
				}
			}
#endif //FOR_DUAL_BAND

#ifdef WPS2DOTX		//under WPS2.0 we must include WSC IE at assoc rsp
			if (pCtx->current_wps_version == WPS_VERSION_V2) {
				len = build_assoc_response_ie(pCtx, tmpbuf);
				if (len == 0)
					DEBUG_ERR("Length of assoc rsp IE exceeds %d\n", MAX_WSC_IE_LEN);

				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
						DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_ASSOC_RSP) < 0){
					WSC_DEBUG("wlioctl_set_wsc_ie fail\n");
					return -1;
				}
#ifdef FOR_DUAL_BAND
				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name2, tmpbuf, len, 
						DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_ASSOC_RSP) < 0){
					WSC_DEBUG("wlioctl_set_wsc_ie fail\n");
					return -1;
				}
#endif //FOR_DUAL_BAND
			}
#endif
		}
	}
	
	if (pCtx->is_ap) {

#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
#endif
		{
			sprintf(tmpbuf,"iwpriv %s set_mib wsc_enable=3", pCtx->wlan_interface_name);		
			system(tmpbuf);
		}
#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan1_wsc_disabled){
			sprintf(tmpbuf,"iwpriv %s set_mib wsc_enable=3", pCtx->wlan_interface_name2);
			system(tmpbuf);
		}
#endif	//FOR_DUAL_BAND

#ifdef WINDOW7
		/*after update wlan's ie info ; release wlan's tx */
#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
#endif
		{
			func_on_wlan_tx(pCtx,pCtx->wlan_interface_name);
		}
#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan1_wsc_disabled ) {
			func_on_wlan_tx(pCtx,pCtx->wlan_interface_name2);
		}
#endif	//FOR_DUAL_BAND
#endif	//WINDOW7
	}
	DBFEXIT;
	return 0;
}

static void process_event(CTX_Tp pCtx, int evt)
{
	int i, ret;

	DBFENTER;
	
	switch (evt) {
		case EV_START:
#if defined(DEBUG) && defined(OUTPUT_LOG)
			if(outlog_fp==NULL){
				outlog_fp = fopen( LOG_PATH ,"a+" );
			}
#endif
#ifdef CLIENT_MODE
			if (!pCtx->is_ap) {
				char tmpbuf[1024];
				int len;
				//struct sysinfo info;

				_DEBUG_PRINT("<< Got EV_START >><< Got PIN-code Input event >>\n");
#ifdef SUPPORT_REGISTRAR
				if (pCtx->role == REGISTRAR) {
					DEBUG_ERR("Self-PIN could not be supported in external registrar mode!\n");
					return;
				}
#endif
				
				pCtx->start = 1;
				client_set_WlanDriver_WscEnable(pCtx, 1);

				if(pCtx->pb_pressed || pCtx->pb_timeout) {
					WSC_DEBUG("Clear PBC stuff!\n");
					pCtx->pb_pressed = 0;
					pCtx->pb_timeout = 0;
				}

				if (pCtx->use_ie) {
				len = build_probe_request_ie(pCtx, 0, tmpbuf);
				if (len == 0)
					DEBUG_ERR("Length of probe request IE exceeds %d\n", MAX_WSC_IE_LEN);
				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_PROBE_REQ) < 0)
					DEBUG_ERR("Set probe request IE failed\n");

				len = build_assoc_request_ie(pCtx, tmpbuf);
				if (len == 0)
					DEBUG_ERR("Length of assoc request IE exceeds %d\n", MAX_WSC_IE_LEN);
				if (wlioctl_set_wsc_ie(pCtx->wlan_interface_name, tmpbuf, len, 
					DOT11_EVENT_WSC_SET_IE, SET_IE_FLAG_ASSOC_REQ) < 0)
					DEBUG_ERR("Set assoc request IE failed\n");
				}
				if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_START) < 0) {
					DEBUG_ERR("issue wlan ioctl set_led error!\n");
				}

				pCtx->pin_timeout = PIN_WALK_TIME;
				report_WPS_STATUS(PROTOCOL_START);
				
				//sysinfo(&info);
				//pCtx->start_time = (unsigned long)info.uptime;
				pCtx->start_time = time(NULL);
				
				DEBUG_PRINT("%s %d Issue scan\n", __FUNCTION__, __LINE__);
				issue_scan_req(pCtx, CONFIG_METHOD_PIN);
			}
			else
#endif
				_DEBUG_PRINT("<< Got EV_START >>\n");
			break;
			
		case EV_STOP:
			_DEBUG_PRINT("<< Got EV_STOP >>\n");
			for (i=0; i<MAX_STA_NUM; i++) {
				if (pCtx->sta[i] && pCtx->sta[i]->used)
					reset_sta(pCtx, pCtx->sta[i], 1);
			}
			reset_ctx_state(pCtx);
			if (!pCtx->is_ap)
			pCtx->start = 0;

#ifdef OUTPUT_LOG
			if(outlog_fp){
				fclose(outlog_fp);
			}
#endif
			break;
			
		case EV_ASSOC_IND:
			evHandler_assocInd(pCtx, DOT11_EVENT_WSC_ASSOC_REQ_IE_IND);
			break;
			
		case EV_EAP:
			ret = evHandler_eap(pCtx);
#ifdef CLIENT_MODE
			if (!pCtx->is_ap && ret != 0) {
				reset_sta(pCtx, pCtx->sta_to_clear, 1);
			}
#endif
			break;
			
		case EV_PIN_INPUT:
#if defined(DEBUG) && defined(OUTPUT_LOG)
			if(outlog_fp==NULL){
				outlog_fp = fopen( LOG_PATH ,"a+" );
			}
#endif
			evHandler_pin_input(pCtx, NULL);
			break;

		case EV_PB_PRESS:
#if defined(DEBUG) && defined(OUTPUT_LOG)
			if(outlog_fp==NULL){
				outlog_fp = fopen( LOG_PATH ,"a+" );
			}
#endif
			evHandler_pb_press(pCtx);
			break;
#ifdef SUPPORT_UPNP
		case EV_PROBEREQ_IND:
			evHandler_probe_req_Ind(pCtx);
			break;
#endif
#ifdef	AUTO_LOCK_DOWN
		case EV_UN_AUTO_LOCK_DOWN:
			if(pCtx->auto_lock_down){
				pCtx->auto_lock_down = 0;
#ifdef ALD_BRUTEFORCE_ATTACK_MITIGATION				
				pCtx->ADL_pin_attack_count=0;	
#endif
				InOut_auto_lock_down(pCtx,0);
				printf("user has intervened unlock\n");
			}
			break;
#endif
#ifdef P2P_SUPPORT
			case EV_P2P_SWITCH_MODE:
				{
					DOT11_P2P_INDICATE_WSC *pIndEvt;
					char tmpbuf2[64];
					pIndEvt = (DOT11_P2P_INDICATE_WSC *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
		
					WSC_DEBUG("\n\n<< P2P event WPS >>\n\n");
		
					// mode
					if(pIndEvt->modeSwitch){
		
						if(pIndEvt->modeSwitch==P2P_PRE_GO){
							
							pCtx->mode = MODE_AP_PROXY_REGISTRAR ;
							pCtx->p2p_trigger_type=P2P_PRE_GO;										  
							
						}else if(pIndEvt->modeSwitch==P2P_TMP_GO){
						
							pCtx->mode = MODE_AP_PROXY_REGISTRAR ;				  
							
						}else if(pIndEvt->modeSwitch==P2P_PRE_CLIENT){
						
							pCtx->mode = MODE_CLIENT_UNCONFIG ; 	 
		
							pCtx->p2p_trigger_type=P2P_PRE_CLIENT;					  
						}
						
						P2P_DEBUG("mode switch to %d\n" ,pCtx->mode);
		
						pCtx->mode_switch=1;
						init_config(pCtx, 0);
		
						if (pCtx->is_ap) {
							pCtx->start = 1;
						} else {
							pCtx->start = 0;
							client_set_WlanDriver_WscEnable(pCtx, 0);
						}
		
						init_wlan(pCtx, 1);
					}else{
						WSC_DEBUG("mode no need change\n");
					}

#ifdef FOR_DUAL_BAND
					if(pCtx->mode == MODE_AP_PROXY_REGISTRAR){
						if(pCtx->InterFaceComeIn == COME_FROM_WLAN0){
							//	PSK
							if(strlen(pIndEvt->network_key)){
								strcpy(pCtx->network_key , pIndEvt->network_key );
								pCtx->network_key_len = strlen(pCtx->network_key);
								P2P_DEBUG("PSK switch to [%s]\n" ,pCtx->network_key);
							}
							
							
							// SSID
							strcpy(pCtx->SSID, pIndEvt->gossid);
							
							// Auth  Eencrypt
							pCtx->auth_type_flash = WSC_AUTH_WPA2PSK;
							pCtx->encrypt_type_flash = WSC_ENCRYPT_AES;
		
						}else{
							//	PSK
							if(strlen(pIndEvt->network_key)){
								strcpy(pCtx->network_key2 , pIndEvt->network_key );
								pCtx->network_key_len2 = strlen(pCtx->network_key2);
								P2P_DEBUG("PSK switch to [%s]\n" ,pCtx->network_key2);
							}
							
							
							// SSID
							strcpy(pCtx->SSID2, pIndEvt->gossid);
							
							// Auth  Eencrypt
							pCtx->auth_type_flash2 = WSC_AUTH_WPA2PSK;
							pCtx->encrypt_type_flash2 = WSC_ENCRYPT_AES;
		
						}
						//system("udhcpd /var/udhcpd.conf");
						//printf("\n\nsystem: udhcpd /var/udhcpd.conf\n\n");
		
					}
#endif		
		
					if(pIndEvt->trigger_method == P2P_PIN_METHOD){
						P2P_DEBUG("PIN method use\n");
		
						if(pCtx->mode == MODE_AP_PROXY_REGISTRAR){
		
							if(pIndEvt->whosPINuse == USE_MY_PIN){
								P2P_DEBUG("USE_MY_PIN(%s)\n",pCtx->pin_code);
		
								if(pIndEvt->requestor)
									pCtx->device_password_id = PASS_ID_REG ;
		
		
								evHandler_pin_input(pCtx,pCtx->pin_code);
		
							}else if(pIndEvt->whosPINuse == USE_TARGET_PIN){
		
								P2P_DEBUG("USE_TARGET_PIN(%s)\n",pIndEvt->PINCode);
		
								if(pIndEvt->requestor)
									pCtx->device_password_id = PASS_ID_DEFAULT ;
		
								evHandler_pin_input(pCtx,pIndEvt->PINCode);
		
							}else{
		
								P2P_DEBUG("unknow type\n");
							}
		
		
						}
						else if(pCtx->mode == MODE_CLIENT_UNCONFIG){
		
							if(pIndEvt->whosPINuse == USE_MY_PIN){
								if(pIndEvt->requestor)
									pCtx->device_password_id = PASS_ID_DEFAULT ;
		
								sprintf(tmpbuf2,"wscd -sig_start %s",GET_CURRENT_INTERFACE);
								system(tmpbuf2);						
								P2P_DEBUG("USE_MY_PIN[%s], issue [system cmd[%s]]\n",pCtx->pin_code,tmpbuf2);
		
							}else if(pIndEvt->whosPINuse == USE_TARGET_PIN){
								strcpy(pCtx->pin_code,pIndEvt->PINCode);
								if(pIndEvt->requestor)
									pCtx->device_password_id = PASS_ID_REG ;
		
								sprintf(tmpbuf2,"wscd -sig_start %s",GET_CURRENT_INTERFACE);
								system(tmpbuf2);											  
								P2P_DEBUG("USE_TARGET_PIN[%s] issue [system cmd[%s]]\n",pIndEvt->PINCode,tmpbuf2);						  
							}else{
								P2P_DEBUG("unknow type\n");
							}
		
						}
		
		
					}
					else if(pIndEvt->trigger_method == P2P_PBC_METHOD){
						evHandler_pb_press(pCtx);
					}else{
						WSC_DEBUG("\njust change mode\n");
					}
		
				}
				break;
		
#endif

		/* support  change STA PIN code, 2011-0505 */
		case EV_CHANGE_MY_PIN:
		{
			DOT11_WSC_PIN_IND *pIndEvt = (DOT11_WSC_PIN_IND *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
			int code=0;
			if(!pCtx->is_ap){
				if (strlen(pIndEvt->code) == PIN_LEN) {
					code = atoi(pIndEvt->code);
					if (!validate_pin_code(code)) {
						WSC_DEBUG("PIN Checksum error!\n");
					}
					else{
						strcpy(pCtx->peer_pin_code, pIndEvt->code);
						strcpy(pCtx->pin_code, pIndEvt->code);
						strcpy(pCtx->original_pin_code, pIndEvt->code);
					}
				}
			}
		}
		break;

		/* support  Assigned SSID, 2011-0505 */
		case EV_SPEC_SSID:
		{
			DOT11_WSC_PIN_IND *pIndEvt = (DOT11_WSC_PIN_IND *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
			if(!pCtx->is_ap){
				if (strlen(pIndEvt->code) && strlen(pIndEvt->code) <= 32) {
					strcpy(pCtx->SPEC_SSID , pIndEvt->code);
				}
			}
		}
		break;
		 
		/* support  Assigned MAC Addr, 2011-0505 */
		case EV_SET_SPEC_CONNECT_MAC:
		{
			DOT11_WSC_PIN_IND *pIndEvt = (DOT11_WSC_PIN_IND *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
			if(!pCtx->is_ap){
				if(strlen(pIndEvt->code) == 12){
					if(string_to_hex(pIndEvt->code , pCtx->SPEC_MAC, 12)){
						WSC_DEBUG("	Spec Mac Addr:%s\n",pIndEvt->code);
					}else{
						WSC_DEBUG("	invaild Spec Mac Addr:%s\n",pIndEvt->code);
						memset(pCtx->SPEC_MAC,0,6);
					}
				}
			}
			
		}
		break;
	 
		default:
			break;
	}	

#if 0
//#ifdef CLIENT_MODE
	if (!pCtx->is_ap && pCtx->start) {
		if (pCtx->connect_fail) {
			DEBUG_PRINT("%s %d Issue re-connect\n", __FUNCTION__, __LINE__);
			if (!connect_wps_ap(pCtx, pCtx->connect_method))
				issue_scan_req(pCtx, pCtx->connect_method);
		}
	}
#endif
}

#ifdef TEST
#include <string.h>
#include <openssl/hmac.h>

typedef struct {
	const char *key, *iv;
	unsigned char kaval[EVP_MAX_MD_SIZE];
} HMAC_KAT;

static const HMAC_KAT vector= {
	"0123456789:;<=>?@ABC",
	"Sample #2",
	{ 0xb8,0xf2,0x0d,0xb5,0x41,0xea,0x43,0x09,
	  0xca,0x4e,0xa9,0x38,0x0c,0xd0,0xe8,0x34,
	  0xf7,0x1f,0xbe,0x91,0x74,0xa2,0x61,0x38,
	  0x0d,0xc1,0x7e,0xae,0x6a,0x34,0x51,0xd9 }
};

static int run_test(CTX_Tp pCtx)
{
	int sel, intv[10], i;
	char tmpbuf[100];
	struct timeval tod;
	unsigned long num;
	DOT11_WSC_ASSOC_IND *pAssocInd;
	DOT11_WSC_PIN_IND *pPinInd;

display_cmd:
	printf("\nTest commands:\n");
	printf("\t1. generate PIN code\n");
	printf("\t2. set local mac address\n");
	printf("\t3. set remote mac address\n");
	printf("\t4. generate asso_ind evnet\n");
	printf("\t5. assign PIN code\n");
	printf("\t6. generate PB pressed\n\n");
	
	printf("\t7. send EAP rsp-id\n");
	printf("\t8. send EAP M1\n");
	printf("\t9. send EAP M2\n\n");
	printf("\t10. hmac-sha256 test\n");
#ifdef CLIENT_MODE
	printf("\t11. scan-request\n");
#endif

	printf("\t12. return & continue\n");
	printf("\t0. exit\n");

	printf("\n\tSelect? ");

	scanf("%d", &sel);

	switch(sel) {
	case 1:
		gettimeofday(&tod , NULL);
		srand(tod.tv_sec);
		num = rand() % 10000000;
		num = num*10 + compute_pin_checksum(num);
		convert_hex_to_ascii((unsigned long)num, tmpbuf);
		printf("PIN: %s\n", tmpbuf);
		strcpy(pCtx->pin_code, tmpbuf);
		break;

	case 2:
		printf("\nsrc mac addr? ");
		scanf("%s", tmpbuf);
		sscanf(tmpbuf, "%02x%02x%02x%02x%02x%02x",
				&intv[0], &intv[1], &intv[2],
				&intv[3], &intv[4], &intv[5]);
		for (i=0; i<6; i++)
			pCtx->our_addr[i] = (unsigned char)intv[i];
		break;

	case 3:
		printf("\nremote mac addr? ");
		scanf("%s", tmpbuf);
		sscanf(tmpbuf, "%02x%02x%02x%02x%02x%02x",
				&intv[0], &intv[1], &intv[2],
				&intv[3], &intv[4], &intv[5]);
		for (i=0; i<6; i++)
			pCtx->sta[0]->addr[i] = (unsigned char)intv[i];
		pCtx->sta[0]->used = 1;
		break;
		
	case 4: // generate assoc_ind
		printf("\npeer mac addr? ");
		scanf("%s", tmpbuf);
		sscanf(tmpbuf, "%02x%02x%02x%02x%02x%02x",
				&intv[0], &intv[1], &intv[2],
				&intv[3], &intv[4], &intv[5]);
		pAssocInd = (DOT11_WSC_ASSOC_IND *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
		for (i=0; i<6; i++)
			pAssocInd->MACAddr[i] = (unsigned char)intv[i];
		process_event(pCtx, EV_ASSOC_IND);
		break;

	case 5:
		printf("\nInput PIN code? ");
		scanf("%s", tmpbuf);
		pPinInd = (DOT11_WSC_PIN_IND *)&pCtx->rx_buffer[FIFO_HEADER_LEN];
		strcpy(pPinInd->code, tmpbuf);
		process_event(pCtx, EV_PIN_INPUT);
		break;

	case 6:
		process_event(pCtx, EV_PB_PRESS);
		break;

	case 7: // send rsp-id
		printf("\nidentifier? ");
		scanf("%d", &i);
		pCtx->sta[0]->eap_reqid = i;
		send_eap_rspid(pCtx, &pCtx->sta[0]);
		break;

	case 8: // send M1
		send_wsc_M1(pCtx, &pCtx->sta[0]);
		break;

	case 9: // send M2
		send_wsc_M2(pCtx, &pCtx->sta[0]);
		break;

	case 10:
//		hmac_sha256(vector.iv, strlen(vector.iv), vector.key, strlen(vector.key), tmpbuf,  &i);
//		if (memcmp(tmpbuf, vector.kaval, BYTE_LEN_256B))
//			printf("test failed!\n");
//		else
//			printf("test ok.\n");
		break;

#ifdef CLIENT_MODE
	case 11:
		issue_scan_req(pCtx ,pCtx->connect_method);
		break;
#endif
		
	case 12:
		return 0;
	case 0:
		return -1;
	}
	goto display_cmd;
}
#endif // TEST

#ifdef SUPPORT_UPNP
static STA_CTX_Tp search_registra_entry_by_IP(CTX_Tp pCtx, char *addr)
{
	int i, idx=-1;

	for (i=0; i<MAX_STA_NUM; i++) {
		if (!pCtx->sta[i] || pCtx->sta[i]->used == 0) {
			if (idx < 0)
				idx = i;
			continue;
		}
		if (strcmp(pCtx->sta[i]->ip_addr, addr) == 0)
			break;
	}

	if ( i != MAX_STA_NUM)
		return (pCtx->sta[i]);

	if (idx >= 0 && pCtx->num_ext_registrar < MAX_EXTERNAL_REGISTRAR_NUM) {
		pCtx->sta[idx] = calloc(1, sizeof(STA_CTX));
		return pCtx->sta[idx];
	}
	else {
		unsigned int largest=0;
		idx=0;
		
		for (i=0; i<MAX_STA_NUM; i++) {
			if (!pCtx->sta[i] || !(pCtx->sta[i]->used & IS_UPNP_CONTROL_POINT))
				continue;
			if (strcmp(pCtx->sta[i]->ip_addr, pCtx->SetSelectedRegistrar_ip) == 0)
				continue;
			//cathy, difftime return a double value, our LIB has problem to access float or double value
			//unsigned int time_offset = difftime(time(NULL), pCtx->sta[i]->time_stamp);
			unsigned int time_offset = time(NULL) - pCtx->sta[i]->time_stamp;
			if (time_offset > largest) {
				idx = i;
				largest = time_offset;
			}
		}
		DEBUG_PRINT("Registrar table full; replace IP[%s]\n", pCtx->sta[idx]->ip_addr);
		if (pCtx->sta[idx]->dh_enrollee){
			DH_free(pCtx->sta[idx]->dh_enrollee);
			pCtx->sta[idx]->dh_enrollee = NULL;
		}
		
		if (pCtx->sta[idx]->dh_registrar){
			DH_free(pCtx->sta[idx]->dh_registrar);
			pCtx->sta[idx]->dh_registrar = NULL;
		}
		memset(pCtx->sta[idx], 0, sizeof(STA_CTX));

		if(pCtx->num_ext_registrar)
			pCtx->num_ext_registrar--;
		WSC_DEBUG("ER count=%d\n", pCtx->num_ext_registrar);
		
		return (pCtx->sta[idx]);
	}
}

int PWSCUpnpCallbackEventHandler(struct WSC_packet *packet, void *Cookie)
{
	CTX_Tp pCtx = (CTX_Tp)Cookie;
	STA_CTX_Tp pSta;
	STA_CTX_Tp pSta_enrollee=NULL;
	int tag_len;
	unsigned char *pData;	
	int ret = WSC_UPNP_FAIL;
	int num_sta=0;
	int i=0;
	unsigned char tmp[ETHER_ADDRLEN];
	char *ptr=(packet->EventMac);
	unsigned char *DTMPtr=NULL;

	//WSC_pthread_mutex_lock(&pCtx->RegMutex);
	//DEBUG_PRINT("%s %d Lock mutex\n", __FUNCTION__, __LINE__);
	if (pCtx->registration_on >= 1) {	
		//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
		//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);

		if (packet->EventID == WSC_GETDEVINFO) {
			DEBUG_PRINT("%s %d Registration protocol is already in progress; uses cached sta context\n", __FUNCTION__, __LINE__);
			if (pCtx->cached_sta.dh_enrollee){
				DH_free(pCtx->cached_sta.dh_enrollee);
				pCtx->cached_sta.dh_enrollee=NULL;
			}
			if (pCtx->cached_sta.dh_registrar){
				DH_free(pCtx->cached_sta.dh_registrar);
				pCtx->cached_sta.dh_registrar=NULL;
			}
			memset(&pCtx->cached_sta, 0, sizeof(STA_CTX));
			pCtx->cached_sta.used = (1 | IS_UPNP_CONTROL_POINT);
			if (0 == (ret = send_wsc_M1(pCtx, &pCtx->cached_sta))) {
				// under IS_UPNP_CONTROL_POINT =1 ;not real send M1 ; plus note
				pCtx->cached_sta.state = ST_WAIT_M2;
				pCtx->cached_sta.tx_timeout = 0;
				pCtx->cached_sta.retry = 0;
			}
			else{
				send_wsc_nack(pCtx, &pCtx->cached_sta, CONFIG_ERR_CANNOT_CONNECT_TO_REG);
			}
			packet->tx_buffer = pCtx->cached_sta.tx_buffer;
			packet->tx_size = pCtx->cached_sta.last_tx_msg_size;
			return WSC_UPNP_SUCCESS;
		}
		else if (packet->EventID == WSC_M2M4M6M8) {
			pData = search_tag(packet->rx_buffer, TAG_MSG_TYPE, packet->rx_size, &tag_len);
			if (pData == NULL) {
				WSC_DEBUG("ERROR, Can't find MessageType tag in UPNP msg!\n");
				packet->tx_buffer = NULL;
				packet->tx_size = 0;
				return WSC_UPNP_SUCCESS;
			}
			else {
				if (pData[0] == MSG_TYPE_M2) {
					pData = search_tag(packet->rx_buffer, TAG_REGISTRAR_NONCE, packet->rx_size, &tag_len);
					if (pData == NULL) {
						DEBUG_ERR("ERROR, Can't find REGISTRAR_NONCE tag in UPNP msg!\n");
						packet->tx_buffer = NULL;
						packet->tx_size = 0;
						return WSC_UPNP_SUCCESS;
					}
					memcpy(&pCtx->cached_sta.nonce_registrar, pData, tag_len);
					if (pCtx->cached_sta.used) {
						send_wsc_nack(pCtx, &pCtx->cached_sta, CONFIG_ERR_DEV_BUSY);
						packet->tx_buffer = pCtx->cached_sta.tx_buffer;
						packet->tx_size = pCtx->cached_sta.last_tx_msg_size;
					}
					else {
						packet->tx_buffer = NULL;
						packet->tx_size = 0;
					}

					return WSC_UPNP_SUCCESS;
				}
			}
		}
	}
	//else {
		//WSC_pthread_mutex_unlock(&pCtx->RegMutex);
		//DEBUG_PRINT("%s %d unlock mutex\n", __FUNCTION__, __LINE__);
	//}
	
	pSta = search_registra_entry_by_IP(pCtx, packet->IP);
	if (pSta == NULL) {
		DEBUG_ERR("ERROR in registrar table!\n");
		goto ret_callback;
	}
	if (packet->EventID == WSC_GETDEVINFO ||
		packet->EventID == WSC_SETSELECTEDREGISTRA ||
		packet->EventID == WSC_PUTWLANRESPONSE) {
		if (!pSta->setip) {
			pCtx->num_ext_registrar++;
			pSta->used = 1 | IS_UPNP_CONTROL_POINT;
			strcpy(pSta->ip_addr, packet->IP);
			pSta->setip = 1;
			WSC_DEBUG("Start commu with upnp ER (%s)\n", pSta->ip_addr);
			WSC_DEBUG("ER count=%d\n", pCtx->num_ext_registrar);
		}
	}
	else {
		if (pSta->setip == 0 || pSta->used == 0) {
			if (pSta->setip)
				pCtx->num_ext_registrar--;
			if (pSta->dh_enrollee){
				DH_free(pSta->dh_enrollee);
				pSta->dh_enrollee = NULL;
			}
			if (pSta->dh_registrar){
				DH_free(pSta->dh_registrar);
				pSta->dh_registrar = NULL;
			}
			memset(pSta, 0, sizeof(STA_CTX));
			WSC_DEBUG("ERROR,chk Registrar's table or invalid state!\n");
			WSC_DEBUG("ER count=%d\n", pCtx->num_ext_registrar);

			goto ret_callback;
		}
	}
	pSta->time_stamp = time(NULL);

	switch (packet->EventID) {
		case WSC_GETDEVINFO:
			if (pSta->state != ST_WAIT_M2) {
				reset_sta(pCtx, pSta, 0);
				ret = send_wsc_M1(pCtx, pSta);
			}
			else // use the cached M1 and send to the external registrar
				ret = 0;
			if (ret == 0) {
				pSta->state = ST_WAIT_M2;
				pSta->tx_timeout = 0;
				pSta->retry = 0;
			}
			else {
				send_wsc_nack(pCtx, pSta, CONFIG_ERR_CANNOT_CONNECT_TO_REG);
				reset_sta_UPnP(pCtx, pSta);
			}
			packet->tx_buffer = pSta->tx_buffer;
			packet->tx_size = pSta->last_tx_msg_size;
			pSta->reg_timeout = pCtx->reg_timeout;
			
			ret = WSC_UPNP_SUCCESS;
			break;

		case WSC_SETSELECTEDREGISTRA:
			ret = pktHandler_upnp_select_msg(pCtx, pSta, packet);
			if (ret == 0)
				ret = WSC_UPNP_SUCCESS;
			else
				ret = WSC_UPNP_FAIL;
			break;

		case WSC_M2M4M6M8:
			ret = pktHandler_wsc_msg(pCtx, pSta, (struct eap_wsc_t *)packet, packet->rx_size);
			if ((ret > 0) || (ret == 0)) {
				if (ret > 0) {
					send_wsc_nack(pCtx, pSta, ret);
					reset_sta_UPnP(pCtx, pSta);
				}
				packet->tx_buffer = pSta->tx_buffer;
				packet->tx_size = pSta->last_tx_msg_size;
			}
			else {
				reset_sta_UPnP(pCtx, pSta);
				if (pSta->state == ST_WAIT_ACK) {
					packet->tx_buffer = pSta->tx_buffer;
					packet->tx_size = pSta->last_tx_msg_size;
				}
				else {
					packet->tx_buffer = NULL;
					packet->tx_size = 0;
				}
			}
			
			ret = WSC_UPNP_SUCCESS;
			break;

		case WSC_PUTWLANRESPONSE:
			pData = search_tag(packet->rx_buffer, TAG_MSG_TYPE, packet->rx_size, &tag_len);
			if (pData == NULL) {
				DEBUG_ERR("ERROR, Can't find MessageType tag in UPNP msg!\n");
				return WSC_UPNP_FAIL;
			}

			if (pCtx->setSelectedRegTimeout) {
				if (memcmp(pCtx->SetSelectedRegistrar_ip, packet->IP, IP_ADDRLEN)) {
					DEBUG_PRINT("Ignore UPnP message from external UPnP registrar [%s]\n", packet->IP);
					return WSC_UPNP_SUCCESS;
				}
			}
			
			if (pData[0] != MSG_TYPE_M2 && pData[0] != MSG_TYPE_M2D && 
				pData[0] != MSG_TYPE_M4 && pData[0] != MSG_TYPE_M6 &&
				pData[0] != MSG_TYPE_M8 && pData[0] != MSG_TYPE_NACK 
				)
			{
				DEBUG_ERR("ERROR, invalid MessageType tag [0x%x] in UPNP msg!\n", pData[0]);
				return WSC_UPNP_FAIL;
			}

			if( pData[0] == MSG_TYPE_M2 ) {
				//unsigned char i;
				DTMPtr = search_tag(packet->rx_buffer, TAG_MANUFACTURER, packet->rx_size, &tag_len);
				if (DTMPtr != NULL) {
					unsigned char device[20]={"Manufacturer"}, model_num[10]="TF", serial_num[10]={"TF123456"};

					if( !memcmp(device,DTMPtr,strlen(device))) {

						DTMPtr = search_tag(packet->rx_buffer, TAG_MODEL_NUMBER, packet->rx_size, &tag_len);
						if (DTMPtr != NULL && !memcmp(DTMPtr,model_num,strlen(model_num)))
							i++;

						DTMPtr = search_tag(packet->rx_buffer, TAG_SERIAL_NUM, packet->rx_size, &tag_len);
						if (DTMPtr != NULL && !memcmp(DTMPtr,serial_num,strlen(serial_num)))
							i++;
					}

					pCtx->ERisDTM = i;
				}
			}

			memset(tmp, 0, ETHER_ADDRLEN);
			for (i=0; i<ETHER_ADDRLEN; i++, ptr+=3) {
				if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
					DEBUG_ERR("Invalid Mac address from UPnP registrar!\n");
					return WSC_UPNP_FAIL;
				}
				tmp[i] = convert_atob(ptr, 16);
			}

			for (num_sta=0; num_sta<MAX_STA_NUM; num_sta++) {
				if (!pCtx->sta[num_sta] || !pCtx->sta[num_sta]->used || (pCtx->sta[num_sta]->used & IS_UPNP_CONTROL_POINT))
					continue;
				if (memcmp(pCtx->sta[num_sta]->addr, tmp, ETHER_ADDRLEN) == 0) {
					pSta_enrollee = pCtx->sta[num_sta];
					break;
				}
			}

			if (num_sta >= MAX_STA_NUM) {
				i = -1;
				WSC_DEBUG("Enrollee Mac address not found; trying to search an exist proxyed enrollee!\n");
				for (num_sta=0; num_sta<MAX_STA_NUM; num_sta++) {
					if (!pCtx->sta[num_sta] || !pCtx->sta[num_sta]->used || (pCtx->sta[num_sta]->used & IS_UPNP_CONTROL_POINT))
						continue;
					if ((pCtx->sta[num_sta]->state < ST_UPNP_DONE) || (pCtx->sta[num_sta]->state > ST_UPNP_WAIT_DONE))
						continue;
					i = num_sta;
					WSC_DEBUG("An Enrollee  has been found!\n");
					MAC_PRINT(pCtx->sta[num_sta]->addr);
					break;
				}
				if (i == -1) {
					DEBUG_ERR("Enrollee Mac address not found\n");
					goto ret_callback;
				}
				else
					pSta_enrollee = pCtx->sta[i];
			}

			/* 20101102 -start */
			//WSC_DEBUG("send_upnp_to_wlan\n");
			ret = send_upnp_to_wlan(pCtx, pSta_enrollee, packet);

			if(ret == 0){	/*sent to STA success*/
				WSC_DEBUG("ER(%s) Rsp ( ",packet->IP);
				if(pData[0]==MSG_TYPE_M2D){
					pSta_enrollee->state = ST_WAIT_ACK;
					_DEBUG_PRINT("M2D");
				}else if(pData[0]==MSG_TYPE_M2){
					pSta_enrollee->state = ST_UPNP_PROXY;
					_DEBUG_PRINT("M2");
				}else if(pData[0]==MSG_TYPE_M4){
					pSta_enrollee->state = ST_UPNP_PROXY;
					_DEBUG_PRINT("M4");
				}else if(pData[0]==MSG_TYPE_M6){
					pSta_enrollee->state = ST_UPNP_PROXY;
					_DEBUG_PRINT("M6");
				}else if(pData[0]==MSG_TYPE_M8){
					pSta_enrollee->state = ST_UPNP_WAIT_DONE;
					ret = WSC_UPNP_SUCCESS;
					_DEBUG_PRINT("M8");
				}

				_DEBUG_PRINT(" ) to STA\n");
				MAC_PRINT(pSta_enrollee->addr);
			}
			
			/* 20101102 -end */
			break;
#if 0
		case WSC_REBOOT:
			if (pSta->state != ST_UPNP_WAIT_REBOOT) {
				DEBUG_ERR("ERROR, got UPnP WSC_REBOOT evt, but invalid state [%d]!\n", pSta->state);
				ret = WSC_UPNP_FAIL;
			}
			else {
				reset_sta(pCtx, pSta);
				signal_webs(REINIT_SYS);
				
				ret = WSC_UPNP_SUCCESS;
			}
			break;
#endif

   		default:
	    		WSC_DEBUG("Unknown event type(%d)\n",packet->EventID);
	    		break;
    }

ret_callback:
    return ret;
}

static int start_upnp(CTX_Tp pCtx)
{
	int ret=0;
	struct WSC_profile *profile=NULL;

	_DEBUG_PRINT("<< Start_upnp >>\n");
	profile = (struct WSC_profile *)malloc(sizeof(struct WSC_profile));
	if (profile == NULL)
		return -1;
	memset(profile, 0, sizeof(struct WSC_profile));
	memcpy(profile->uuid, pCtx->uuid, UUID_LEN);
	profile->manufacturer = pCtx->manufacturer;
	profile->model_name = pCtx->model_name;
	profile->model_num = pCtx->model_num;
	profile->serial_num = pCtx->serial_num;
	profile->device_name = pCtx->device_name;
	profile->manufacturerURL = pCtx->manufacturerURL;
	profile->modelURL = pCtx->model_URL;
	profile->modelDescription = pCtx->manufacturerDesc;
	if(WSCRegisterCallBackFunc(PWSCUpnpCallbackEventHandler, pCtx) != WSC_UPNP_SUCCESS) {
		free(profile);
		return -1;
	}
	if (pCtx->is_ap)
		ret = WSCUpnpStart(pCtx->lan_interface_name, WSC_AP_MODE, profile);
	else
		ret = WSCUpnpStart(pCtx->lan_interface_name, WSC_STA_MODE, profile);
	free(profile);
	return ret;
}
#endif

#ifdef WSC_1SEC_TIMER
static int wsc_alarm(void)
{
	struct itimerval val;
	
// Fix warning message shown in 2.6 -------
	#if 0
	val.it_interval.tv_sec = 0;
	val.it_interval.tv_usec = BASIC_TIMER_UNIT;
	val.it_value.tv_sec = 0;
	val.it_value.tv_usec = BASIC_TIMER_UNIT;
	#endif
	val.it_interval.tv_sec = 1;
	val.it_interval.tv_usec = 0;
	val.it_value.tv_sec = 1;
	val.it_value.tv_usec = 0;
//-------------------- david+2008-06-11

	if (setitimer( ITIMER_REAL, &val, 0 ) == -1)
	{
		DEBUG_ERR("Error in alarm\n");
		return -1;
	}
	else
		return 0;
}
#endif


#ifdef WSC_1SEC_TIMER
static int sigHandler_alarm(int signo)
#else
static void sigHandler_alarm(int signo)
#endif
{
	int i;
	CTX_Tp pCtx = pGlobalCtx;
	struct stat cancel_status;

#ifdef USE_POLLING
	wlanIndEvt(0);
#endif

#ifdef AUTO_LOCK_DOWN
	if(pCtx->auto_lock_down > 0){
//		if(pCtx->auto_lock_down%30 ==0) {
//			WSC_DEBUG("auto_lock_down [%d]\n",pCtx->auto_lock_down);
//		}
		pCtx->auto_lock_down --;
		
		if(pCtx->auto_lock_down == 0)
			InOut_auto_lock_down(pCtx,0);	// leave lock state
		
	}
#endif
	
	for (i=0; i<MAX_STA_NUM; i++) {
		if (pCtx->sta[i] && pCtx->sta[i]->used) {
			if (pCtx->sta[i]->reg_timeout > 0 &&
					--pCtx->sta[i]->reg_timeout <= 0) {

				DEBUG_ERR("STA's Registration timeout, abort registration!\n");
				MAC_PRINT(pCtx->sta[i]->addr);
				
				if (pCtx->wait_reinit) {
					WSC_DEBUG("signal_webs\n");
					signal_webs(pCtx->wait_reinit);
				}
			
				// Reason code 1 : Unspecified reason
				if (pCtx->is_ap && !(pCtx->sta[i]->used & IS_UPNP_CONTROL_POINT) &&
					(pCtx->sta[i]->Assoc_wscIE_included || (pCtx->sta[i]->state >= ST_WAIT_M1 &&
					pCtx->sta[i]->state <= ST_WAIT_EAP_FAIL))) {
#ifdef BLOCKED_ROGUE_STA
					if (pCtx->blocked_expired_time &&
						(pCtx->sta[i]->state >= ST_WAIT_M4 && pCtx->sta[i]->state <= ST_WAIT_M8) &&
						(pCtx->sta_invoke_reg == pCtx->sta[i] && pCtx->registration_on >= 1) &&
						(pCtx->sta[i]->ap_role != ENROLLEE)) {
						add_into_blocked_list(pCtx, pCtx->sta[i]);
					}
					else
#endif
					{
							WSC_DEBUG("IssueDisconnect\n");
						IssueDisconnect(pCtx->wlan_interface_name, pCtx->sta[i]->addr, 1);
					}
				}
				reset_sta(pCtx, pCtx->sta[i], 1);
				continue;
			}

			/* for delay send eap-fail when ER > 1 ;
			   when ER>1 ,and Enrollee send M1 ,  maybe ER1 Rsp M2D but ER2 Rsp M2
			   then Enrollee's state will change to  ST_UPNP_PROXY
			   then  Enrollee's state != ST_WAIT_ACK , meaning some ER will be regristrar 
			   so don't send  send_eap_fail
			   search related code by 20101102
  			   WPS2DOTX,PF#3  Marvell   0925 request*/

			if (pCtx->sta[i]->ER_RspM2D_delaytime > 0 && --pCtx->sta[i]->ER_RspM2D_delaytime <= 0) 
			{
				if(pCtx->sta[i]->state == ST_WAIT_ACK){
					send_eap_fail(pCtx, pCtx->sta[i]);

					// Reason code 1 : Unspecified reason
					if (pCtx->is_ap){
						IssueDisconnect(pCtx->wlan_interface_name, pCtx->sta[i]->addr, 1);
							WSC_DEBUG("IssueDisconnect\n\n");
					}
	
					reset_sta(pCtx, pCtx->sta[i], 1);
					WSC_DEBUG("ER Rsp M2D and no M2 come by close\n");
					continue;
				}else{
					WSC_DEBUG("ER Rsp M2D and M2 come by close\n");
				}
			}
			/* for delay send eap-fail when ER > 1 ;search related code by 20101102 */

			
			if (pCtx->sta[i]->tx_timeout > 0 &&
					--pCtx->sta[i]->tx_timeout <= 0) {
#ifdef SUPPORT_UPNP
				if (pCtx->sta[i]->used & IS_UPNP_CONTROL_POINT) {
					reset_sta(pCtx, pCtx->sta[i], 1);
					continue;
				}
#endif

				if (pCtx->sta[i]->state == ST_WAIT_EAPOL_START) {
					if (pCtx->encrypt_type == ENCRYPT_WEP
#ifdef FOR_DUAL_BAND
						|| pCtx->encrypt_type2 == ENCRYPT_WEP
#endif
						) {
						if (pCtx->sta[i]->Assoc_wscIE_included) {
							DEBUG_PRINT("\n	wsc IE included : ST_WAIT_EAPOL_START timeout\n");
							WSC_DEBUG("IssueDisconnect!!!\n");
							IssueDisconnect(pCtx->wlan_interface_name, pCtx->sta[i]->addr, 1);
						}
						reset_sta(pCtx, pCtx->sta[i], 1);
					}
					/*why????????*/	
					/* since EAPOL-START no receive seem should not on going ; 0925 plus.
					else { // in case EAPOL-START not received
						struct timeval tod;
						gettimeofday(&tod , NULL);
						srand(tod.tv_sec);
						pCtx->sta[i]->eap_reqid = (rand() % 50) + 1;		
						// 0925 need check
						WSC_DEBUG("\n\n in case EAPOL-START not received \n\n");
						send_eap_reqid(pCtx, pCtx->sta[i]);
						pCtx->sta[i]->state = ST_WAIT_RSP_ID;
						pCtx->sta[i]->tx_timeout = pCtx->tx_timeout;
						pCtx->sta[i]->retry = 1;
					}
					*/
					continue;
				}

				WSC_DEBUG("Sending packet timeout\n");
				MAC_PRINT(pCtx->sta[i]->addr);

				if (pCtx->sta[i]->retry < pCtx->resent_limit) {
					pCtx->sta[i]->retry++;
					DEBUG_PRINT("Resent packet [%d]\n", pCtx->sta[i]->retry);

					send_wlan(pCtx, pCtx->sta[i]->tx_buffer, pCtx->sta[i]->tx_size);
					pCtx->sta[i]->tx_timeout = pCtx->tx_timeout;
				}
				else {
					DEBUG_ERR("Exceed retry limit, abort registration!\n");
		
					if (pCtx->wait_reinit) {
						WSC_DEBUG("signal_webs\n");
						signal_webs(pCtx->wait_reinit);
					}
		
					// Reason code 1 : Unspecified reason
					if (!(pCtx->sta[i]->used & IS_UPNP_CONTROL_POINT) && pCtx->is_ap &&
						(pCtx->sta[i]->state > ST_WAIT_RSP_ID || pCtx->sta[i]->Assoc_wscIE_included)) {
#ifdef BLOCKED_ROGUE_STA
							if (pCtx->blocked_expired_time &&
								(pCtx->sta[i]->state >= ST_WAIT_M4 && pCtx->sta[i]->state <= ST_WAIT_M8) &&
								(pCtx->sta_invoke_reg == pCtx->sta[i] && pCtx->registration_on >= 1) &&
								(pCtx->sta[i]->ap_role != ENROLLEE)) {
								add_into_blocked_list(pCtx, pCtx->sta[i]);
							}
							else
#endif
							{
								WSC_DEBUG("IssueDisconnect\n");
								IssueDisconnect(pCtx->wlan_interface_name, pCtx->sta[i]->addr, 1);
							}

					}
					reset_sta(pCtx, pCtx->sta[i], 1);
				}
			}
		}
	}
//cathy
	if(pCtx->restart_timeout > 0 && --pCtx->restart_timeout <= 0) {
		system("echo R > /proc/gpio");
	}
/////	
	if (IS_PBC_METHOD(pCtx->config_method)) {
		int state;

#ifdef MUL_PBC_DETECTTION
		if (!pCtx->disable_MulPBC_detection) {
			if (pCtx->is_ap
#ifdef CLIENT_MODE
				|| (!pCtx->is_ap && pCtx->role == ENROLLEE)
#endif
				)
			remove_active_pbc_sta(pCtx, NULL, 0);

			if (pCtx->SessionOverlapTimeout > 0 && --pCtx->SessionOverlapTimeout <= 0) {
				DEBUG_PRINT("overlap walk timeout,unlock overlap state\n");
				if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_END) < 0) {
					DEBUG_ERR("issue wlan ioctl set_led error!\n");
				}
			}
		}
#endif
		
		if(pCtx->is_ap){
			if (wlioctl_get_button_state(pCtx->wlan_interface_name, &state) < 0){
				DEBUG_ERR("issue wlan ioctl get_button_state failed!\n");
			}

			if (state > 0) {
				// let hold time can config via wscd.conf
				if ( ++pCtx->pb_pressed_time == pCtx->button_hold_time)
				{
					WSC_DEBUG("pbc pressed time (%d), start PBC...\n",pCtx->button_hold_time);

					process_event(pCtx, EV_PB_PRESS);
					pCtx->pb_pressed_time = 0;
				}
			}
			else{
				pCtx->pb_pressed_time = 0;
			}
		}
		else
		{	/*client mode*/ 
			if (pCtx->pb_pressed_time < pCtx->button_hold_time) {				
				if (wlioctl_get_button_state(pCtx->wlan_interface_name, &state) < 0){
					DEBUG_ERR("issue wlan ioctl get_button_state failed!\n");
				}

				if (state > 0) {
					//printf("rdy rec event\n");
					if ( ++pCtx->pb_pressed_time == pCtx->button_hold_time){
						pCtx->pb_pressed_time = 0;
#if defined(DEBUG) && defined(OUTPUT_LOG)
						if(outlog_fp==NULL){
							outlog_fp = fopen( LOG_PATH ,"a+" );
						}
#endif
					
						WSC_DEBUG("start PBC...\n");
						process_event(pCtx, EV_PB_PRESS);
					}
				}
				else{
					pCtx->pb_pressed_time = 0;
				}
			}
		}

#ifdef CLIENT_MODE
		if (!pCtx->is_ap && (pCtx->pb_timeout || pCtx->pin_timeout) && pCtx->start_time) {
			//struct sysinfo info;
			//sysinfo(&info);
			
			if (pCtx->pb_timeout) {
				//if ((((unsigned long)info.uptime) - pCtx->start_time) > PBC_WALK_TIME) {
				if ((time(NULL) - pCtx->start_time) > PBC_WALK_TIME) {
					pCtx->pb_timeout = 1; // let timeout happen immedeately
					pCtx->start_time = 0;
				}
			}
			else {
				//if ((((unsigned long)info.uptime) - pCtx->start_time) > PIN_WALK_TIME) {
				if ((time(NULL) - pCtx->start_time) > PIN_WALK_TIME) {
					pCtx->pin_timeout = 1; // let timeout happen immedeately
					pCtx->start_time = 0;
				}
			}
		}
#endif

		if (pCtx->pb_timeout > 0 && --pCtx->pb_timeout <= 0) {
#ifdef OUTPUT_LOG
			if(outlog_fp){
				UTIL_DEBUG("close outlog_fp\n");
				fclose(outlog_fp);
			}
#endif	
			{
				_DEBUG_PRINT("PBC walk time-out!\n");
				report_WPS_STATUS(PROTOCOL_TIMEOUT);
			}
			reset_ctx_state(pCtx);
		
			if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_ERROR) < 0) {
				DEBUG_ERR("issue wlan ioctl set_led error!\n");
			}

#ifdef CLIENT_MODE
			if (!pCtx->is_ap)
				pCtx->start  = 0;
#endif
		}
	}

	if (IS_PIN_METHOD(pCtx->config_method)) {
		if (pCtx->pin_timeout > 0 && --pCtx->pin_timeout <= 0) {
#ifdef OUTPUT_LOG
			if(outlog_fp){
				UTIL_DEBUG("close outlog_fp\n");
				fclose(outlog_fp);
			}
#endif
			_DEBUG_PRINT("PIN time-out!\n");
#ifdef WPS2DOTX
			if (pCtx->current_wps_version == WPS_VERSION_V2)
				registrar_remove_authorized_mac(pCtx ,BroadCastMac);	// 2010-0719
#endif
#ifdef CONNECT_PROXY_AP
			clear_blocked_ap_list(pCtx);
#endif
			report_WPS_STATUS(PROTOCOL_TIMEOUT);

			reset_ctx_state(pCtx);
#ifdef CLIENT_MODE
			if (!pCtx->is_ap)
				pCtx->start  = 0;
#endif

			if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_ERROR) < 0) {
				DEBUG_ERR("issue wlan ioctl set_led error!\n");	
			}
		}
	}

	if (pCtx->LedTimeout > 0 && --pCtx->LedTimeout <= 0) {
		if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_END) < 0) {
			DEBUG_ERR("issue wlan ioctl set_led error!\n");
		}
	}


	if (pCtx->wait_reboot > 0 && --pCtx->wait_reboot <= 0) {
		WSC_DEBUG(">>>WSC REBOOT\n");
#ifdef WINDOW7
#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
#endif
		{
			func_off_wlan_tx(pCtx,pCtx->wlan_interface_name);
		}
	#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan1_wsc_disabled )
			func_off_wlan_tx(pCtx,pCtx->wlan_interface_name2);
	#endif //FOR_DUAL_BAND

#endif
		WSC_DEBUG("signal_webs\n");
		signal_webs(REINIT_SYS);
	}

#ifdef SUPPORT_UPNP

	if (pCtx->setSelectedRegTimeout > 0 && --pCtx->setSelectedRegTimeout <= 0) {
		WSC_DEBUG("SetSelectedReg time-out!\n");
		report_WPS_STATUS(PROTOCOL_TIMEOUT);
		reset_ctx_state(pCtx);
	}
	
	if (pCtx->upnp && pCtx->status_changed) {
		struct WSC_packet packet;
		unsigned char tmpbuf[1];
		
		tmpbuf[0] = '1';
		if (pCtx->is_ap)
			packet.EventID = WSC_AP_STATUS;	// ap setting changed
		else
			packet.EventID = WSC_STA_STATUS;
		packet.tx_buffer = tmpbuf;
		packet.tx_size = 1;
		if (WSCUpnpTxmit(&packet) != WSC_UPNP_SUCCESS)
			DEBUG_ERR("WSCUpnpTxmit() return error!\n");
		pCtx->status_changed = 0;
	}
	
	if (pCtx->upnp && pCtx->upnp_wait_reboot_timeout > 0 && --pCtx->upnp_wait_reboot_timeout <= 0) {
		_DEBUG_PRINT("upnp_config wait_reboot ----->%s----->%d\n", __FUNCTION__, __LINE__);
		WSCUpnpStop();
	#ifdef WINDOW7
		#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan0_wsc_disabled)
		#endif
		{
			func_off_wlan_tx(pCtx,pCtx->wlan_interface_name);
		}
		
		#ifdef FOR_DUAL_BAND
		if(!pCtx->wlan1_wsc_disabled )
			func_off_wlan_tx(pCtx,pCtx->wlan_interface_name2);
		#endif //FOR_DUAL_BAND
	#endif
		WSC_DEBUG("signal_webs\n");
		signal_webs(REINIT_SYS);
	}

#ifdef PREVENT_PROBE_DEADLOCK
	int idx=-1;
	unsigned int largest=0;
	
	if (pCtx->is_ap && pCtx->upnp && (pCtx->original_role != ENROLLEE)
		&& pCtx->probe_list_count) {
		for (i=0; i<MAX_WSC_PROBE_STA; i++) {
			if (!pCtx->probe_list[i].used)
				continue;
			else {
				//cathy, difftime return a double value, our LIB has problem to access float or double value
				//unsigned int time_offset=difftime(time(NULL), pCtx->probe_list[i].time_stamp);
				unsigned int time_offset=time(NULL) - pCtx->probe_list[i].time_stamp;
				if (time_offset > PROBE_EXPIRED) {
					memset(&pCtx->probe_list[i], 0, sizeof(struct probe_node));
					pCtx->probe_list_count--;
				}
				else {
					if (!pCtx->probe_list[i].sent && (time_offset > largest)) {
						idx = i;
						largest = time_offset;
					}
				}
			}
		}
	}
	
	if (pCtx->is_ap && (pCtx->role == REGISTRAR || pCtx->role == PROXY) &&
		(pCtx->original_role != ENROLLEE) &&
		!pCtx->pb_pressed && !pCtx->pin_assigned && 
		pCtx->upnp && pCtx->TotalSubscriptions && (idx != -1)) {
		struct WSC_packet packet;

		packet.EventType = WSC_PROBE_FRAME;
		packet.EventID = WSC_PUTWLANREQUEST;
		convert_bin_to_str_UPnP(pCtx->probe_list[idx].ProbeMACAddr, 6, packet.EventMac);
		packet.tx_buffer = pCtx->probe_list[idx].ProbeIE;
		packet.tx_size = pCtx->probe_list[idx].ProbeIELen;

		if (WSCUpnpTxmit(&packet) != WSC_UPNP_SUCCESS)
			DEBUG_ERR("WSCUpnpTxmit() return error!\n");
		pCtx->probe_list[idx].sent = 1;
	}
#endif // PREVENT_PROBE_DEADLOCK


#ifdef USE_MINI_UPNP
	if (pCtx->upnp) {
#ifdef STAND_ALONE_MINIUPNP
		if (pCtx->upnp_info.SSDP.alive_timeout > 0 && --pCtx->upnp_info.SSDP.alive_timeout <= 0) {
			//sending alive
			WSC_DEBUG("!!	sending alive\n\n");
			
			SendSSDPNotifies(pCtx->upnp_info.snotify, pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port,
				&pCtx->upnp_info.SSDP, 0, pCtx->upnp_info.SSDP.max_age);
			pCtx->upnp_info.SSDP.alive_timeout = pCtx->upnp_info.SSDP.max_age/2;
		}
#endif
		
		struct EvtRespElement *EvtResp=NULL;
		struct EvtRespElement *EvtResp_next=NULL;
		for(EvtResp = pCtx->upnp_info.subscribe_list.EvtResp_head.lh_first; EvtResp != NULL; )
		{
			EvtResp_next = EvtResp->entries.le_next;
			if(EvtResp->TimeOut > 0 && --EvtResp->TimeOut <= 0) {
				WSC_DEBUG("Eventing response timeout: remove sid[%s]\n", EvtResp->sid);
				if (EvtResp->socket >= 0)
					close(EvtResp->socket);
				LIST_REMOVE(EvtResp, entries);
				free(EvtResp);
			}
			EvtResp = EvtResp_next;
		}

		if (pCtx->TotalSubscriptions)
		{
			struct upnp_subscription_element *e;
			struct upnp_subscription_element *next;
			struct subscription_info *subscription=NULL;
			
			for(e = pCtx->upnp_info.subscribe_list.subscription_head.lh_first; e != NULL; )
			{
				next = e->entries.le_next;
				
				if(e->subscription_timeout > 0)
				{
					e->subscription_timeout -= 1;

                #ifdef DEBUG_UPNP
					if(e->subscription_timeout % 10 == 0) {
						_DEBUG_PRINT("(upnp)Sid[%s],To=%d\n" , e->sid , e->subscription_timeout);
					}
                #endif
					
					if(e->subscription_timeout<=0){
						
						/*clean upnp*/
						LIST_REMOVE(e, entries);
						pCtx->upnp_info.subscribe_list.total_subscription--;
						WSC_DEBUG("(UPNP)Remove sid[%s]\n",e->sid);
						WSC_DEBUG("(UPNP)Total_subscription[%d]\n",
			       				(int)pCtx->upnp_info.subscribe_list.total_subscription);

						subscription = search_sid_entry(pCtx, e->sid);
						if ((NULL != subscription) && subscription->used) {
							memset(subscription, 0, sizeof(struct subscription_info));
						}
						
						free(e);
						// 2011-0422 add
						pCtx->TotalSubscriptions = (int)pCtx->upnp_info.subscribe_list.total_subscription;										
					}
				}
				e = next;
			}
		}
	}
#endif // USE_MINI_UPNP
#endif //SUPPORT_UPNP

	{
		struct stat status;
		if (stat(REINIT_WSCD_FILE, &status) == 0) { // re-init

#ifdef SUPPORT_UPNP
			if (pCtx->upnp)
				WSCUpnpStop();
#endif

			for (i=0; i<MAX_STA_NUM; i++) {
				if (pCtx->sta[i] && pCtx->sta[i]->used)
					reset_sta(pCtx, pCtx->sta[i], 1);
			}
			init_config(pCtx,0);
			if (pCtx->is_ap) {
				pCtx->start = 1;
			} else {
				pCtx->start = 0;
				client_set_WlanDriver_WscEnable(pCtx, 0);
			}
			
			if (init_wlan(pCtx, 1) < 0) {
				printf("wscd: init_wlan() failed!\n");
			}

#ifdef SUPPORT_UPNP
			if (pCtx->upnp) {
				if (start_upnp(pCtx) < 0) {
					printf("wscd: start_upnp() failed!\n");
				}
			}
#endif
			
           	unlink(REINIT_WSCD_FILE);
		}
	}

#ifdef CLIENT_MODE
	if (!pCtx->is_ap && pCtx->start && pCtx->connect_fail) {
		if (pCtx->join_idx >= pCtx->ss_status.number ||
				pCtx->ss_status.number == 0xff) {
			WSC_DEBUG("	Issue Re-Scan\n");
			issue_scan_req(pCtx, pCtx->connect_method);
		}
		else {
			WSC_DEBUG("	Issue Re-Connect\n");
			connect_wps_ap(pCtx, pCtx->connect_method);
		}
	}
#endif

#ifdef BLOCKED_ROGUE_STA
	if (pCtx->is_ap && pCtx->blocked_expired_time)
		countdown_blocked_list(pCtx);
#endif // BLOCKED_ROGUE_STA
	
	if (stat(WSCD_CANCEL_PROTOCOL, &cancel_status) == 0) {
		WSC_DEBUG("		Received WSC stop event \n\n");
		process_event(pCtx, EV_STOP);
		unlink(WSCD_CANCEL_PROTOCOL);
	}

#ifdef WSC_1SEC_TIMER
	return 1;
#else
	// patch-100804
	//alarm(1);
#endif
}

#ifdef WSC_1SEC_TIMER
static int wsc_init_1sec_Timer(void)
{
	struct sigaction  action;

	action.sa_handler = (void (*)())sigHandler_alarm;
	action.sa_flags = SA_RESTART;

	if (sigaction(SIGALRM,&action,0) == -1)
	{
		DEBUG_ERR("Error in sigaction\n");
		return -1;
	}

	wsc_alarm();
	
	return 0;
}
#endif

static void sigHandler_user(int signo)
{

#ifdef FOR_DUAL_BAND
	CTX_Tp pCtx = pGlobalCtx;
	struct stat cancel_status;
#endif

	if (signo == SIGTSTP){
		process_event(pGlobalCtx, EV_START);
	}
	else if (signo == SIGUSR2){
#ifdef FOR_DUAL_BAND
		/*2011-11 add for support under dual band mode just trigger single interface*/
		pCtx->inter0only  = 0;
		pCtx->inter1only  = 0;

		if (stat(WSCD_IND_ONLY_INTERFACE0, &cancel_status) == 0) {
			WSC_DEBUG("%s\n",WSCD_IND_ONLY_INTERFACE0);
			pCtx->inter0only  = 1;
			unlink(WSCD_IND_ONLY_INTERFACE0);
		}
		if (stat(WSCD_IND_ONLY_INTERFACE1, &cancel_status) == 0) {
			WSC_DEBUG("%s\n",WSCD_IND_ONLY_INTERFACE1);
			pCtx->inter1only  = 1;
			unlink(WSCD_IND_ONLY_INTERFACE1);
		}

		WSC_DEBUG("inter0only = %d\n",pCtx->inter0only);
		WSC_DEBUG("inter1only = %d\n",pCtx->inter1only);

		/*2011-11 add for support under dual band mode just trigger single interface*/
#endif
		process_event(pGlobalCtx, EV_PB_PRESS);
	}
#ifdef AUTO_LOCK_DOWN
	else if (signo == SIGUSR1){
		process_event(pGlobalCtx, EV_UN_AUTO_LOCK_DOWN);
	}
#endif
	else{
		printf("Got an invalid signal [%d]!\n", signo);
	}
}

static int read_config_file(CTX_Tp pCtx, char *filename, int def)
{
	FILE *fp;
	char line[400], token[40], value[300], *ptr;
	int i;
	int idx=0;			/*2011-0419 WEP,HEX type key issue*/
	DBFENTER;

	_DEBUG_PRINT("Read config file[%s], def=%d \n",pCtx->cfg_filename,def);

	fp = fopen(filename, "r");
	if (fp == NULL) {
		DEBUG_ERR("read config file [%s] failed!\n", filename);
		return -1;
	}

#ifdef BLOCKED_ROGUE_STA
	pCtx->blocked_expired_time = DEFAULT_BLOCK_TIME;
#endif

	while ( fgets(line, 200, fp) ) {
		if (line[0] == '#')
			continue;
		ptr = get_token(line, token);
		if (ptr == NULL)
			continue;
		if (get_value(ptr, value)==0){
			continue;
		}
		else if (!strcmp(token, "wlan_fifo0")) {
			if (pCtx->fifo_name[0])
				continue;
			strcpy(pCtx->fifo_name, value);
		}
		else if (!strcmp(token, "wlan_fifo1")) {
#ifdef FOR_DUAL_BAND
			if (pCtx->fifo_name2[0])
				continue;
			strcpy(pCtx->fifo_name2, value);
#endif //FOR_DUAL_BAND
		}

/*		
		else if (!strcmp(token, "role")) {
			if (pCtx->role != -1) // set in argument
				continue;
			pCtx->role = atoi(value);
			if (pCtx->role != PROXY && pCtx->role != ENROLLEE &&
				 pCtx->role != REGISTRAR) {
				DEBUG_ERR("Invalid role value [%d]!\n", pCtx->role);
				return -1;
			}
		}
*/		
		else if (!strcmp(token, "mode")) {
			if (pCtx->mode != -1)
				continue;
			pCtx->mode = atoi(value);
		}
		else if (!strcmp(token, "manual_config")) {
			pCtx->manual_config= atoi(value);
		}
		else if (!strcmp(token, "upnp")) {
			if (pCtx->upnp != -1) // set in argument
				continue;
			pCtx->upnp = atoi(value);
		}
		else if (!strcmp(token, "use_ie")) {
			pCtx->use_ie = atoi(value);
		}
		else if (!strcmp(token, "current_wps_version")) {
			pCtx->current_wps_version = atoi(value);
		}
		else if (!strcmp(token, "config_method")) {
			if ((pCtx->current_wps_version == WPS_VERSION_V1) && (pCtx->config_method != -1)) // set in argument
				continue;

			pCtx->config_method = atoi(value);
#ifdef WPS2DOTX
			if (pCtx->current_wps_version == WPS_VERSION_V2) {
				if((pCtx->config_method & (CONFIG_METHOD_VIRTUAL_PIN | CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC))==0){
					pCtx->config_method = (CONFIG_METHOD_VIRTUAL_PIN |  CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC);
					WSC_DEBUG("For WPS2.0 config_method to %x\n",pCtx->config_method);
				}
			} else
#endif
			{
				if (!(pCtx->config_method & (CONFIG_METHOD_PIN | CONFIG_METHOD_PBC))) {
					DEBUG_ERR("Invalid config_method value [%d]! Use default.\n", pCtx->config_method);
					pCtx->config_method = (CONFIG_METHOD_PIN | CONFIG_METHOD_PBC | CONFIG_METHOD_ETH);
				}
			}
		}
#ifdef WPS2DOTX	// should be no need it's only need for as Test Bed
		//EAP_frag_threshold
		else if (!strcmp(token, "EAP_Frag_Len_TH")) {
			pCtx->EAP_frag_threshold = atoi(value);
			WSC_DEBUG("	!!!assigned EAP message Fragment thresold=%d\n",pCtx->EAP_frag_threshold);
			if(pCtx->EAP_frag_threshold < 200 || pCtx->EAP_frag_threshold > 2048){
				pCtx->EAP_frag_threshold = 0;
				WSC_DEBUG("Fragment thresold too small , no fragment\n");
			}

		}
		/*STA mode ; probe_REQ__WSC_IE_Fragment */
		else if (!strcmp(token, "ProbeReq_wscIE_frag")) {
			pCtx->probeReq_need_wscIE_frag = atoi(value);
		}

		/*AP mode ; probe_RSP__WSC_IE_Fragment */
		else if (!strcmp(token, "ProbeRsp_wscIE_frag")) {
			pCtx->probeRsp_need_wscIE_frag = atoi(value);
		}

		/*AP mode ; extension_tag */
		else if (!strcmp(token, "Extension_Tag_test")) {
			pCtx->extension_tag = atoi(value);
		}
#endif
		else if (!strcmp(token, "pin_code")) {
			int code_len;
			if (strlen(pCtx->pin_code) > 0) // set in argument
				continue;

			code_len = strlen(value);
			if (code_len != PIN_LEN && code_len != 4) {
				DEBUG_ERR("Invalid pin_code length!\n");
				return -1;
			}
			i = atoi(value);
			if (code_len == PIN_LEN) {
				if (!validate_pin_code(i)) {
					DEBUG_ERR("Invalid pin_code value (checksum error)!\n");
					return -1;
				}
			}
			strcpy(pCtx->pin_code, value);
		}
		else if (!strcmp(token, "rf_band")) {
			pCtx->rf_band = atoi(value);
		}
		else if (!strcmp(token, "ssid")) {
			if (pCtx->SSID[0] != 0) // set in argument
				continue;
			if (strlen(value) > MAX_SSID_LEN) {
				DEBUG_ERR("Invalid ssid length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->SSID, value);
		}
#ifdef FOR_DUAL_BAND
		/* FOR_DUAL_BAND --start ; need not under define flag*/
		else if (!strcmp(token, "wlan0_wsc_disabled")) {
			
			pCtx->wlan0_wsc_disabled = atoi(value);			
			#ifdef DEBUG
			if(pCtx->wlan0_wsc_disabled == 1){
				WSC_DEBUG("wlan0 interface wsc is Disabled\n");
			}else{
				WSC_DEBUG("wlan0 interface wsc is Enabled\n");
			}
			#endif
		}
		else if (!strcmp(token, "wlan1_wsc_disabled")) {
			
			pCtx->wlan1_wsc_disabled = atoi(value);			
			#ifdef DEBUG						
			if(pCtx->wlan1_wsc_disabled == 1){
				WSC_DEBUG("wlan1 interface wsc is Disabled\n");
			}else{
				WSC_DEBUG("wlan1 interface wsc is Enabled\n");
			}			
			#endif
		}
		/* FOR_DUAL_BAND  --end  ; need not under define flag*/
		else if (!strcmp(token, "ssid2")) {
			if (pCtx->SSID2[0] != 0) // set in argument
				continue;
			if (strlen(value) > MAX_SSID_LEN) {
				DEBUG_ERR("Invalid ssid length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->SSID2, value);
		}
		else if (!strcmp(token, "auth_type2")) {
			pCtx->auth_type_flash2 = atoi(value);
			if ((pCtx->auth_type_flash2 &
 				 (AUTH_OPEN | AUTH_WPAPSK | AUTH_SHARED | AUTH_WPA
				 | AUTH_WPA2| AUTH_WPA2PSK| AUTH_WPA2PSKMIXED))==0)
			{
				DEBUG_ERR("Invalid auth_type value [0x%x]! Use default.\n", pCtx->auth_type_flash2);
				pCtx->auth_type_flash2 = AUTH_OPEN;
			}

			if (pCtx->auth_type_flash2 == AUTH_WPA2PSKMIXED)
				pCtx->auth_type2 = AUTH_WPA2PSK;
			else
				pCtx->auth_type2 = pCtx->auth_type_flash2;

			WSC_DEBUG("auth_type2:0x%02X\n",pCtx->auth_type2);
		}
		else if (!strcmp(token, "encrypt_type2")) {
			pCtx->encrypt_type_flash2 = atoi(value);
			if ( (pCtx->encrypt_type_flash2 & (ENCRYPT_NONE | ENCRYPT_WEP | ENCRYPT_TKIP | ENCRYPT_AES )) ==0 ) 
			{
				DEBUG_ERR("Invalid encrypt_type2 [0x%x]! Use default.\n", pCtx->encrypt_type_flash2);
				pCtx->encrypt_type_flash2 = ENCRYPT_NONE;
			}
			
			if (pCtx->encrypt_type_flash2 == ENCRYPT_TKIPAES) {
				if (pCtx->auth_type2 == AUTH_WPA2PSK)
					pCtx->encrypt_type2 = ENCRYPT_AES;
				else
					pCtx->encrypt_type2 = ENCRYPT_TKIP;
			}
			else{
				pCtx->encrypt_type2 = pCtx->encrypt_type_flash2;
			}
			
			WSC_DEBUG("encrypt_type_flash2:%d\n",pCtx->encrypt_type_flash2);
			WSC_DEBUG("encrypt_type2:%d\n",pCtx->encrypt_type2);
			
			
		}
		else if (!strcmp(token, "mixedmode2")) {
			pCtx->mixedmode2 = atoi(value);
			WSC_DEBUG("security2 mixed mode= %d\n",pCtx->mixedmode2);
		}
		else if (!strcmp(token, "wep_transmit_key2")) {
			pCtx->wep_transmit_key2 = atoi(value);
			if (pCtx->encrypt_type2 == ENCRYPT_WEP && 
				(pCtx->wep_transmit_key2 < 1 || pCtx->wep_transmit_key2 > 4)) {
				DEBUG_ERR("Invalid wep_transmit_key value [%d]!\n", pCtx->wep_transmit_key2);
				return -1;
			}
		}
		else if (!strcmp(token, "network_key2")) {

			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid network_key length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->network_key2, value);
			pCtx->network_key_len2 = strlen(value);

		}
		else if (!strcmp(token, "wep_key22")) {
			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid wep_key2 length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->wep_key22, value);
		}
		else if (!strcmp(token, "wep_key32")) {
			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid wep_key3 length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->wep_key32, value);
		}
		else if (!strcmp(token, "wep_key42")) {
			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid wep_key4 length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->wep_key42, value);
		}
#endif	/*  END of FOR_DUAL_BAND */
		else if (!strcmp(token, "auth_type")) {
			pCtx->auth_type_flash = atoi(value);

			if ((pCtx->auth_type_flash &
 				 (AUTH_OPEN | AUTH_WPAPSK | AUTH_SHARED | AUTH_WPA
				 | AUTH_WPA2| AUTH_WPA2PSK| AUTH_WPA2PSKMIXED))==0)
			{
				DEBUG_ERR("Invalid auth_type value [0x%x]! Use default.\n", pCtx->auth_type_flash);
				pCtx->auth_type_flash = AUTH_OPEN;
			}

			if (pCtx->auth_type_flash == AUTH_WPA2PSKMIXED)
				pCtx->auth_type = AUTH_WPA2PSK;
			else
				pCtx->auth_type = pCtx->auth_type_flash;
			
			WSC_DEBUG("auth_type:0x%02X\n",pCtx->auth_type);
		}
		else if (!strcmp(token, "auth_type_flags")) {
			pCtx->auth_type_flags = atoi(value);
			if (!(pCtx->auth_type_flags & (AUTH_OPEN | AUTH_WPAPSK | AUTH_SHARED | AUTH_WPA | 
					AUTH_WPA2 | AUTH_WPA2PSK))) {
				DEBUG_ERR("Invalid auth_type_flags value [0x%x]!\n", pCtx->auth_type_flags);
				return -1;
			}

		}
		else if (!strcmp(token, "encrypt_type")) {
			pCtx->encrypt_type_flash = atoi(value);
			if ( (pCtx->encrypt_type_flash & (ENCRYPT_NONE | ENCRYPT_WEP | ENCRYPT_TKIP | ENCRYPT_AES ))==0)
			{
				DEBUG_ERR("Invalid encrypt_type [0x%x]! Use default.\n", pCtx->encrypt_type_flash);
				pCtx->encrypt_type_flash = ENCRYPT_NONE;
			}
			
			if (pCtx->encrypt_type_flash == ENCRYPT_TKIPAES) {
				if (pCtx->auth_type == AUTH_WPA2PSK)
					pCtx->encrypt_type = ENCRYPT_AES;
				else
					pCtx->encrypt_type = ENCRYPT_TKIP;
			}
			else{
				pCtx->encrypt_type = pCtx->encrypt_type_flash;

			}
			WSC_DEBUG("encrypt_type_flash:%d\n",pCtx->encrypt_type_flash);
			WSC_DEBUG("encrypt_type:%d\n",pCtx->encrypt_type);

		}
		else if (!strcmp(token, "encrypt_type_flags")) {
			pCtx->encrypt_type_flags = atoi(value);
			if (!(pCtx->encrypt_type_flags & (ENCRYPT_NONE |ENCRYPT_WEP |ENCRYPT_TKIP|ENCRYPT_AES))) {
				DEBUG_ERR("Invalid encrypt_type_flags value [0x%x]!\n", pCtx->encrypt_type_flags);
				return -1;
			}

		}
		else if (!strcmp(token, "mixedmode")) {
			pCtx->mixedmode = atoi(value);
			WSC_DEBUG("security mixed mode = %d\n",pCtx->mixedmode);
		}
		else if (!strcmp(token, "wep_transmit_key")) {
			pCtx->wep_transmit_key = atoi(value);
			if (pCtx->encrypt_type == ENCRYPT_WEP && 
				(pCtx->wep_transmit_key < 1 || pCtx->wep_transmit_key > 4)) {
				DEBUG_ERR("Invalid wep_transmit_key value [%d]!\n", pCtx->wep_transmit_key);
				return -1;
			}
		}
		else if (!strcmp(token, "connection_type")) {
			if (pCtx->connect_type != -1) // set in argument
				continue;
			pCtx->connect_type = atoi(value);
			if (pCtx->connect_type != CONNECT_TYPE_BSS && pCtx->connect_type != CONNECT_TYPE_IBSS) {
				DEBUG_ERR("Invalid connection_type value [%d]!\n", pCtx->connect_type);
				return -1;
			}
		}
		else if (!strcmp(token, "uuid")) {
			if (strlen(value) != UUID_LEN*2) {
				DEBUG_ERR("Invalid uuid length!\n");
				return -1;
			}
			ptr = value;
			for (i=0; i<UUID_LEN; i++, ptr+=2) {
				if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
					DEBUG_ERR("Invalid uuid vlaue!\n");
					return -1;
				}
				pCtx->uuid[i] = convert_atob(ptr, 16);
			}
			
			// replace the last 6 characters of UUID with HW MAC address
			memcpy(&pCtx->uuid[UUID_LEN-ETHER_ADDRLEN], pCtx->our_addr, ETHER_ADDRLEN);
		}else if (!strcmp(token, "modelDescription")) {
			if (strlen(value) > MAX_MANUFACT_LEN) {
				DEBUG_ERR("Invalid modelDescription length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->manufacturerDesc, value);
		}//Brad add 20090206
		else if (!strcmp(token, "manufacturer")) {
			if (strlen(value) > MAX_MANUFACT_LEN) {
				DEBUG_ERR("Invalid manufacturer length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->manufacturer, value);
		}//Brad add 20080721
		else if (!strcmp(token, "manufacturerURL")) {
			if (strlen(value) > MAX_MANUFACT_LEN) {
				DEBUG_ERR("Invalid manufacturer URL length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->manufacturerURL, value);
		}
		else if (!strcmp(token, "modelURL")) {
			if (strlen(value) > MAX_MANUFACT_LEN) {
				DEBUG_ERR("Invalid model URL length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->model_URL, value);
		}//Brad add end
		else if (!strcmp(token, "model_name")) {
			if (strlen(value) > MAX_MODEL_NAME_LEN) {
				DEBUG_ERR("Invalid model_name length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->model_name, value);
		}
		else if (!strcmp(token, "model_num")) {
			if (strlen(value) > MAX_MODEL_NAME_LEN) {
				DEBUG_ERR("Invalid model_num length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->model_num, value);
		}
		else if (!strcmp(token, "serial_num")) {
			if (strlen(value) > MAX_SERIAL_NUM_LEN) {
				DEBUG_ERR("Invalid serial_num length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->serial_num, value);
		}
		else if (!strcmp(token, "device_category_id")) {
			pCtx->device_category_id = atoi(value);
		}
		else if (!strcmp(token, "device_sub_category_id")) {
			pCtx->device_sub_category_id = atoi(value);
		}
		else if (!strcmp(token, "device_oui")) {
			if (strlen(value) != OUI_LEN*2) {
				DEBUG_ERR("Invalid oui length!\n");
				return -1;
			}
			ptr = value;
			for (i=0; i<OUI_LEN; i++, ptr+=2) {
				if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
					DEBUG_ERR("Invalid OUI vlaue!\n");
					return -1;
				}
				pCtx->device_oui[i] = convert_atob(ptr, 16);
			}
		}
		else if (!strcmp(token, "device_name")) {
			if (strlen(value) > MAX_DEVICE_NAME_LEN) {
				DEBUG_ERR("Invalid device_name length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->device_name, value);
		}
		
		else if (!strcmp(token, "device_password_id")) {
			pCtx->device_password_id = atoi(value);
		}
		else if (!strcmp(token, "disable_configured_by_exReg")) {
			pCtx->disable_configured_by_exReg = atoi(value);

		}
		else if (!strcmp(token, "button_hold_time")) {
			pCtx->button_hold_time = atoi(value);

		}
		else if (!strcmp(token, "network_key")) {
			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid network_key length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->network_key, value);
			pCtx->network_key_len = strlen(value);
			WSC_DEBUG("network_key=%s\n",pCtx->network_key);
		}
		else if (!strcmp(token, "wep_key2")) {
			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid wep_key2 length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->wep_key2, value);
		}
		else if (!strcmp(token, "wep_key3")) {
			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid wep_key3 length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->wep_key3, value);
		}
		else if (!strcmp(token, "wep_key4")) {
			if (strlen(value) > 64) {
				DEBUG_ERR("Invalid wep_key4 length [%d]!\n", strlen(value));
				return -1;
			}
			strcpy(pCtx->wep_key4, value);
		}
		else if (!strcmp(token, "tx_timeout")) {
			pCtx->tx_timeout = atoi(value);
		}
		else if (!strcmp(token, "resent_limit")) {
			pCtx->resent_limit = atoi(value);
		}
		else if (!strcmp(token, "reg_timeout")) {
			pCtx->reg_timeout = atoi(value);
		}

#ifdef BLOCKED_ROGUE_STA
		else if (!strcmp(token, "block_timeout")) {
			pCtx->blocked_expired_time = (unsigned char)atoi(value);
		}
#endif
#ifdef MUL_PBC_DETECTTION
		else if (!strcmp(token, "WPS_PBC_overlapping_GPIO_number")) {
			pCtx->WPS_PBC_overlapping_GPIO_number= atoi(value);
		}
		else if (!strcmp(token, "PBC_overlapping_LED_time_out")) {
			pCtx->PBC_overlapping_LED_time_out = atoi(value);
		}
#endif
		else if (!strcmp(token, "WPS_END_LED_unconfig_GPIO_number")) {
			pCtx->WPS_END_LED_unconfig_GPIO_number = atoi(value);
		}
		else if (!strcmp(token, "WPS_END_LED_config_GPIO_number")) {
			pCtx->WPS_END_LED_config_GPIO_number = atoi(value);
		}
		else if (!strcmp(token, "WPS_START_LED_GPIO_number")) {
			pCtx->WPS_START_LED_GPIO_number = atoi(value);
		}
		else if (!strcmp(token, "No_ifname_for_flash_set")) {
			pCtx->No_ifname_for_flash_set = atoi(value);
		}
		/* --- Forrest added, 2007.10.31. */
		else if (!strcmp(token, "WPS_ERROR_LED_time_out")) {
			pCtx->WPS_ERROR_LED_time_out = atoi(value);
		}
		else if (!strcmp(token, "WPS_ERROR_LED_GPIO_number")) {
			pCtx->WPS_ERROR_LED_GPIO_number = atoi(value);
		}
		else if (!strcmp(token, "WPS_SUCCESS_LED_time_out")) {
			pCtx->WPS_SUCCESS_LED_time_out = atoi(value);
		}
		else if (!strcmp(token, "WPS_SUCCESS_LED_GPIO_number")) {
			pCtx->WPS_SUCCESS_LED_GPIO_number = atoi(value);
		}
		/* Forrest added, 2007.10.31. --- */
		else if (!strcmp(token,"config_by_ext_reg")) { //add by peteryu for WZC in WEP
			pCtx->config_by_ext_reg = atoi(value); 
		}
		else if (!strcmp(token,"fix_wzc_wep")) {  //add by peteryu for WZC in WEP
			pCtx->fix_wzc_wep = atoi(value); 
		}
	}
		
#ifdef MUL_PBC_DETECTTION
	if (pCtx->PBC_overlapping_LED_time_out <= 0)
		pCtx->PBC_overlapping_LED_time_out = 30;
#endif

	if (pCtx->WPS_ERROR_LED_time_out <= 0)
		pCtx->WPS_ERROR_LED_time_out = 30;
	if (pCtx->WPS_SUCCESS_LED_time_out <= 0)
		pCtx->WPS_SUCCESS_LED_time_out = 300;

	if(pCtx->button_hold_time <=0){
		pCtx->button_hold_time = BUTTON_HOLD_TIME ;
	}

	if (pCtx->auth_type == AUTH_OPEN && pCtx->encrypt_type == ENCRYPT_NONE) {
		memset(pCtx->network_key, 0, MAX_NETWORK_KEY_LEN+1);
		pCtx->network_key_len = 0;
	}

	if ((pCtx->auth_type == AUTH_OPEN && pCtx->encrypt_type == ENCRYPT_WEP) ||
		(pCtx->auth_type == AUTH_SHARED && pCtx->encrypt_type == ENCRYPT_WEP)) {
		if (pCtx->wep_transmit_key >= 1 && pCtx->wep_transmit_key <= 4) {
			switch (pCtx->wep_transmit_key)
			{
				case 2:
					memcpy(pCtx->network_key, pCtx->wep_key2, MAX_NETWORK_KEY_LEN+1);
					break;
				case 3:
					memcpy(pCtx->network_key, pCtx->wep_key3, MAX_NETWORK_KEY_LEN+1);
					break;
				case 4:
					memcpy(pCtx->network_key, pCtx->wep_key4, MAX_NETWORK_KEY_LEN+1);
					break;
				default:
					break;
			}
			/*2011-0419 WEP,HEX type key issue-start*/			
			WSC_DEBUG("network_key=%s\n",pCtx->network_key);			
			
			idx = strlen(pCtx->network_key);

			string_to_hex(pCtx->network_key, token, idx);
			
			memcpy(pCtx->network_key,token,idx/2);
			
			pCtx->network_key[idx/2]='\0';
			
			pCtx->network_key_len = idx/2 ;


			/*2011-0419 WEP,HEX type key issue-end*/			
		}
		else {
			DEBUG_ERR("Error wep_transmit_key [%d]!\n", pCtx->wep_transmit_key);
			return -1;
		}
	}

#ifdef FOR_DUAL_BAND
		if (pCtx->auth_type2 == AUTH_OPEN && pCtx->encrypt_type2 == ENCRYPT_NONE) {
			memset(pCtx->network_key2, 0, MAX_NETWORK_KEY_LEN+1);
			pCtx->network_key_len2 = 0;
		}
		if ((pCtx->auth_type2 == AUTH_OPEN && pCtx->encrypt_type2 == ENCRYPT_WEP) ||
			(pCtx->auth_type2 == AUTH_SHARED && pCtx->encrypt_type2 == ENCRYPT_WEP)) {
			if (pCtx->wep_transmit_key2 >= 1 && pCtx->wep_transmit_key2 <= 4) {
				switch (pCtx->wep_transmit_key2)
				{
					case 2:
						memcpy(pCtx->network_key2, pCtx->wep_key22, MAX_NETWORK_KEY_LEN+1);
						break;
					case 3:
						memcpy(pCtx->network_key2, pCtx->wep_key32, MAX_NETWORK_KEY_LEN+1);
						break;
					case 4:
						memcpy(pCtx->network_key2, pCtx->wep_key42, MAX_NETWORK_KEY_LEN+1);
						break;
					default:
						break;
				}
				pCtx->network_key_len2 = strlen(pCtx->network_key2);
				/*2011-0419 WEP,HEX type key issue-start*/
				idx = strlen(pCtx->network_key2);

				string_to_hex(pCtx->network_key2, token, idx);
				
				memcpy(pCtx->network_key2,token,idx/2);
				
				pCtx->network_key2[idx/2]='\0';
				
				pCtx->network_key_len2 = idx/2 ;

				WSC_DEBUG("network_key2=%s\n",pCtx->network_key2);
				/*2011-0419 WEP,HEX type key issue-end*/			
			}
			else {
				DEBUG_ERR("Error wep_transmit_key [%d]!\n", pCtx->wep_transmit_key2);
				return -1;
			}
		}
#endif //FOR_DUAL_BAND

	fclose(fp);

	return 0;
}

static void show_help()
{
	printf("  Usage: %s [argument]...\n", PROGRAM_NAME);
	printf("    Where arguments is optional as:\n");
	printf("\t-c config_filename, config filename, default is %s\n", DEFAULT_CONFIG_FILENAME);
	printf("\t-w wlan_interface, wlan interface\n");
#ifdef SUPPORT_UPNP
	printf("\t-br bridge_interface, lan interface\n");
#endif
#ifndef NO_IWCONTROL
	printf("\t-fi fifo_name, wlan fifo path name\n");
#endif
	printf("\t-method val, 1: PIN, 2: PBC, 3: both\n");
	printf("\t-mode val, 1: ap unconfigured, 2: client unconfigured (enrollee),\n");
	printf("\t\t3: client configured (registrar), 4: ap-proxy configured, 5: ap-proxy registrar\n");
	printf("\t-upnp val, 1 - support upnp, 0 - not support (default)\n");
	printf("\t-gen-pin, generate pin code for local entitiy\n");
	printf("\t-peer_pin, assign pin code for peer entitiy\n");
	printf("\t-local_pin, assign pin code for local device\n");
	printf("\t-sig_start, start wsc protocol\n");
	printf("\t-sig_pbc, signal PBC mode\n");
	printf("\t-start_pbc, start PBC mode\n");
#ifdef DEBUG
	printf("\t-debug, turn on debug message\n");
#endif
#ifdef TEST
	printf("\t-test, go to test mode\n");
#endif

	printf("\n");
}
void issue_signal_to_wscd(CTX_Tp pCtx , int SIG_NUMBER ,char *interName)
{
	int pid;
	FILE *fp;
	unsigned char line[100];


	sprintf(pCtx->pid_filename, "%s-%s.pid",DEFAULT_PID_FILENAME,interName);
	if ((fp = fopen(pCtx->pid_filename, "r")) != NULL) {
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%d", &pid)) {
			if (pid > 1){
				kill(pid, SIG_NUMBER);
				WSC_DEBUG("signal(%d),pid(%d)\n",SIG_NUMBER ,pid);
			}
		}
		fclose(fp);
		return;
	}
		
#ifdef FOR_DUAL_BAND
	sprintf(pCtx->pid_filename, "%s-wlan0-wlan1.pid", 	DEFAULT_PID_FILENAME);
	if ((fp = fopen(pCtx->pid_filename, "r")) != NULL) {
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%d", &pid)) {
			if (pid > 1){
				kill(pid, SIG_NUMBER);
				WSC_DEBUG("signal(%d),pid(%d)\n",SIG_NUMBER ,pid);
			}
		}
		fclose(fp);
		return;
	}
#endif
	WSC_DEBUG("trigger sig(%d),pid(%d)\n",SIG_NUMBER ,pid);

}

static int parse_argument(CTX_Tp pCtx, int argc, char *argv[], int *start_pbc)
{
	int argNum=1;
//	int pid;
//	FILE *fp;
//	unsigned char line[100];
	char tmpbuf[100];

	DBFENTER;
	
	while (argNum < argc) {
		if ( !strcmp(argv[argNum], "-c")) {
			if (++argNum >= argc)
				break;
			strcpy(pCtx->cfg_filename, argv[argNum]);
		}
		else if(!strcmp(argv[argNum], "-both_band_ap")) {
			pCtx->both_band_ap = 1;
		}
		else if ( !strcmp(argv[argNum], "-w")) {
			if (++argNum >= argc)
				break;

			strcpy(pCtx->wlan_interface_name, argv[argNum]);

			if(pCtx->both_band_ap == 1) {
				sprintf(pCtx->pid_filename, "%s-wlan0-wlan1.pid", 	DEFAULT_PID_FILENAME);			
			}else{
				sprintf(pCtx->pid_filename, "%s-%s.pid",
					DEFAULT_PID_FILENAME,	pCtx->wlan_interface_name);
			}

		}

#ifdef SUPPORT_UPNP
		else if ( !strcmp(argv[argNum], "-br")) {
			if (++argNum >= argc)
				break;
			strcpy(pCtx->lan_interface_name, argv[argNum]);
		}
#endif
		else if ( !strcmp(argv[argNum], "-fi")) {
			if (++argNum >= argc)
				break;
			strcpy(pCtx->fifo_name, argv[argNum]);
		}
		else if ( !strcmp(argv[argNum], "-fi2")) {
#ifdef FOR_DUAL_BAND
			if (++argNum >= argc)
				break;
			strcpy(pCtx->fifo_name2, argv[argNum]);
#endif //FOR_DUAL_BAND
		}
		else if ( !strcmp(argv[argNum], "-mode")) {
			if (++argNum >= argc)
				break;
			pCtx->mode = atoi(argv[argNum]);
			if (pCtx->mode != MODE_AP_UNCONFIG &&
					pCtx->mode != MODE_CLIENT_UNCONFIG &&
					pCtx->mode != MODE_CLIENT_CONFIG &&
					pCtx->mode != MODE_AP_PROXY &&
					pCtx->mode != MODE_AP_PROXY_REGISTRAR &&
					pCtx->mode != MODE_CLIENT_UNCONFIG_REGISTRAR) {
				DEBUG_ERR("Invalid mode value [%d]!\n", pCtx->mode);
				return -1;
			}
		}
		else if ( !strcmp(argv[argNum], "-gen-pin")) {
			struct timeval tod;
			unsigned long num;
			
			gettimeofday(&tod , NULL);
			srand(tod.tv_sec);
			num = rand() % 10000000;
			num = num*10 + compute_pin_checksum(num);
			convert_hex_to_ascii((unsigned long)num, tmpbuf);
			printf("PIN: %s\n", tmpbuf);
			strcpy(pCtx->pin_code, tmpbuf);
		}
		else if ( !strcmp(argv[argNum], "-upnp")) {
			if (++argNum >= argc)
				break;
			pCtx->upnp = atoi(argv[argNum]);
		}
		else if ( !strcmp(argv[argNum], "-peer_pin")) {
			if (++argNum >= argc)
				break;
			strcpy(pCtx->peer_pin_code, argv[argNum]);
		}
		else if ( !strcmp(argv[argNum], "-local_pin")) {
			if (++argNum >= argc)
				break;
			strcpy(pCtx->pin_code, argv[argNum]);
		}
		else if ( !strcmp(argv[argNum], "-method")) {
			if (++argNum >= argc)
				break;
			pCtx->config_method = atoi(argv[argNum]);
			WSC_DEBUG("pCtx->config_method=%x\n",pCtx->config_method);
		}
		else if ( !strcmp(argv[argNum], "-start")) {
			pCtx->start = 1;
		}
		else if ( !strcmp(argv[argNum], "-sig_start")) {
			char *interface_name;
			if (++argNum >= argc){
				if (pCtx->wlan_interface_name[0])
					interface_name = pCtx->wlan_interface_name;
				else
					interface_name = "wlan0";
			} else {
				interface_name = argv[argNum];
			}
			issue_signal_to_wscd(pCtx, SIGTSTP, interface_name);
			
			return -1;
		}
		else if ( !strcmp(argv[argNum], "-sig_pbc")) {
			char *interface_name;
			if (++argNum >= argc){		
				if (pCtx->wlan_interface_name[0])
					interface_name = pCtx->wlan_interface_name;
				else
					interface_name = "wlan0";
			} else {
				interface_name = argv[argNum];
			}
			issue_signal_to_wscd(pCtx, SIGUSR2, interface_name);
			
			return -1;
		}
#ifdef AUTO_LOCK_DOWN
		else if ( !strcmp(argv[argNum], "-sig_unlock")) {
			char *interface_name;
			if (++argNum >= argc){
				if (pCtx->wlan_interface_name[0])
					interface_name = pCtx->wlan_interface_name;
				else
					interface_name = "wlan0";
			} else {
				interface_name = argv[argNum];
			}
			issue_signal_to_wscd(pCtx, SIGUSR1, interface_name);
			
			return -1;
		}
#endif
		else if ( !strcmp(argv[argNum], "-start_pbc")) {
			*start_pbc = 1;
		}

#ifdef DEBUG
		else if ( !strcmp(argv[argNum], "-debug")) {
				pCtx->debug = 1;
		}
#endif

#ifdef TEST
		else if ( !strcmp(argv[argNum], "-test")) {
				pCtx->test = 1;
		}
#endif
		else if ( !strcmp(argv[argNum], "-connection_type")) {
			if (++argNum >= argc)
				break;
			pCtx->connect_type = atoi(argv[argNum]);
			if (pCtx->connect_type != CONNECT_TYPE_BSS && pCtx->connect_type != CONNECT_TYPE_IBSS) {
				DEBUG_ERR("Invalid connection_type value [%d]!\n", pCtx->connect_type);
				return -1;
			}
		}
		else if ( !strcmp(argv[argNum], "-ssid")) {
			if (++argNum >= argc)
				break;
			if (strlen(argv[argNum]) > MAX_SSID_LEN) {
				DEBUG_ERR("Invalid ssid length [%d]!\n", strlen(argv[argNum]));
				return -1;
			}
			strcpy(pCtx->SSID, argv[argNum]);
		}
#ifdef MUL_PBC_DETECTTION
		else if ( !strcmp(argv[argNum], "-disable_PBC_detection")) {
			if (++argNum >= argc)
				break;
			pCtx->disable_MulPBC_detection = atoi(argv[argNum]);
			if (pCtx->disable_MulPBC_detection != 0 && pCtx->disable_MulPBC_detection != 1) {
				DEBUG_ERR("Invalid disable_PBC_detection value [%d]!\n", pCtx->disable_MulPBC_detection);
				return -1;
			}
		}
#endif
		else {
			printf("invalid argument - %s\n", argv[argNum]);
			show_help();
			return -1;
		}
		argNum++;
	}
	return 0;
}


static int init_config(CTX_Tp pCtx,int def)
{
	/* Read config file and validate parameters */
	if (read_config_file(pCtx, pCtx->cfg_filename, def) < 0) 	
		return 0;

	if (pCtx->mode == -1) {		
		printf("Parameter \"mode\" must be set!\n");	
		return 0;
	}	
	
#if 0 //def WPS2DOTX
#ifdef FOR_DUAL_BAND
	if(pCtx->wlan0_wsc_disabled == 0)
#endif
	{
		func_off_wlan_acl(pCtx,pCtx->wlan_interface_name);
	}
#ifdef FOR_DUAL_BAND
	if(pCtx->wlan1_wsc_disabled == 0){
		func_off_wlan_acl(pCtx,pCtx->wlan_interface_name2);
	}
#endif //FOR_DUAL_BAND
#endif

	switch (pCtx->mode) {
		case MODE_AP_UNCONFIG:
			pCtx->is_ap = 1;
			pCtx->config_state = CONFIG_STATE_UNCONFIGURED;
			pCtx->role = ENROLLEE;
			pCtx->original_role = ENROLLEE;
			break;
		case MODE_AP_PROXY:
			pCtx->is_ap = 1;
			pCtx->config_state = CONFIG_STATE_CONFIGURED;
			pCtx->role = PROXY;
			pCtx->original_role = PROXY;
			break;
		case MODE_AP_PROXY_REGISTRAR:
			pCtx->is_ap = 1;
			pCtx->config_state = CONFIG_STATE_CONFIGURED;
			pCtx->role = REGISTRAR;
			pCtx->original_role = REGISTRAR;
			break;
#ifdef CLIENT_MODE
		case MODE_CLIENT_UNCONFIG:
			pCtx->is_ap = 0;
			pCtx->config_state = CONFIG_STATE_UNCONFIGURED;
			pCtx->role = ENROLLEE;
			pCtx->original_role = ENROLLEE;
			break;
#ifdef SUPPORT_REGISTRAR
		case MODE_CLIENT_CONFIG:
			pCtx->is_ap = 0;
			pCtx->config_state = CONFIG_STATE_CONFIGURED;
			pCtx->role = REGISTRAR;
			pCtx->original_role = REGISTRAR;
			pCtx->resent_limit = 1;
			pCtx->reg_timeout = PIN_WALK_TIME;
			break;
		case MODE_CLIENT_UNCONFIG_REGISTRAR:
			pCtx->is_ap = 0;
			pCtx->config_state = CONFIG_STATE_UNCONFIGURED;
			pCtx->role = REGISTRAR;
			pCtx->original_role = REGISTRAR;
			pCtx->resent_limit = 1;
			pCtx->reg_timeout = PIN_WALK_TIME;
			break;
#endif
#endif
	}

#ifdef WPS2DOTX
	if (pCtx->current_wps_version == WPS_VERSION_V2)
	{
		pCtx->auth_type_flags &= ~(AUTH_SHARED | AUTH_WPAPSK);
		WSC_DEBUG("auth_type_flags:%02X\n",pCtx->auth_type_flags);


		pCtx->encrypt_type_flags &= ~( ENCRYPT_WEP | ENCRYPT_TKIP );
		WSC_DEBUG("encrypt_type_flags:%02X\n",pCtx->encrypt_type_flags);			
		
		/*
		 case1)
		 Disable WSC if the APUT is manually configured for WEP

		 case2)
		 Disable WSC if the APUT is manually configured for WPA/TKIP only,
		 unless the APUT is configured for mixed mode
		 */

		if (pCtx->is_ap
#ifdef FOR_DUAL_BAND
			&& pCtx->wlan0_wsc_disabled==0
#endif
			)
		{
			if( pCtx->auth_type == AUTH_WPAPSK || pCtx->encrypt_type == ENCRYPT_TKIP ||
				pCtx->encrypt_type == ENCRYPT_WEP)
			{
				WSC_DEBUG("	!!WPS2.x no support WEP or WPA only or TKIP only; disabled wlan0\n");
#ifdef FOR_DUAL_BAND
				pCtx->wlan0_wsc_disabled = 1;
#endif
				return 0;
			}
		}

#ifdef FOR_DUAL_BAND
		if (pCtx->wlan1_wsc_disabled==0 && pCtx->is_ap)
		{
			if( pCtx->auth_type2 == AUTH_WPAPSK || pCtx->encrypt_type2 == ENCRYPT_TKIP ||
				pCtx->encrypt_type2 == ENCRYPT_WEP)
			{
				pCtx->wlan1_wsc_disabled = 1;
				WSC_DEBUG("	!!WPS2.x no support WEP or WPA only or TKIP only; disabled wlan0\n");
			}
		}

		if(pCtx->wlan0_wsc_disabled && pCtx->wlan1_wsc_disabled){
			WSC_DEBUG("!!both two interface are disabled\n\n");
			return 0;
		}
#endif //FOR_DUAL_BAND
	}
#endif

	if (!pCtx->is_ap) {
		pCtx->device_category_id = 1; // Computer
		pCtx->device_sub_category_id = 1; // PC
	}
	
#ifdef DEBUG
	if (pCtx->debug) {
		printf("\n<<====WPS argument list=====>>\n");
#ifdef FOR_DUAL_BAND
		printf("FOR_DUAL_BAND be defined\n");
#endif //FOR_DUAL_BAND
		printf("is_ap = %d\n", pCtx->is_ap);
		printf("role = %s\n", (pCtx->role==PROXY ? "Proxy" : (pCtx->role==ENROLLEE ? "Enrollee" : "Registrar")));
		printf("config_state = %s\n", (pCtx->config_state == CONFIG_STATE_UNCONFIGURED ? "unconfigured" : "configured"));
		printf("config_method = 0x%x\n", pCtx->config_method);
		printf("pin_code = %s\n", pCtx->pin_code);
		printf("use_ie = %d\n", pCtx->use_ie);
#ifdef SUPPORT_UPNP
		printf("upnp = %d\n", pCtx->upnp);
#endif
		printf("---wlan0 setting---\n");
		printf("rf_band = %d; 1=2.4G ;2=5G\n", pCtx->rf_band);
		printf("wep_transmit_key = %d\n", pCtx->wep_transmit_key);
		printf("SSID=%s\n",pCtx->SSID);
		printf("Auth Type=%d\n",pCtx->auth_type);
		printf("Encrypt Type=%d\n",pCtx->encrypt_type);
		printf("WSC_PSK = \"%s\"\n", pCtx->network_key);
		if (pCtx->wep_transmit_key > 1) {
			printf("	wep_key2 = %s\n", pCtx->wep_key2);
			printf("	wep_key3 = %s\n", pCtx->wep_key3);
			printf("	wep_key4 = %s\n", pCtx->wep_key4);
		}
		
#ifdef FOR_DUAL_BAND

		if(pCtx->wlan0_wsc_disabled == 1){
			printf("wlan0 interface wsc is Disabled\n");
		}else{
			printf("wlan0 interface wsc is Enabled\n");
		}
		
		if(pCtx->is_ap){
			if(!pCtx->wlan1_wsc_disabled){
				printf("===wlan1 setting===\n");
				printf("SSID=%s\n",pCtx->SSID2);
				printf("Auth Type=%d\n",pCtx->auth_type2);
				printf("Encrypt Type=%d\n",pCtx->encrypt_type2);
				printf("WSC_PSK = \"%s\"\n", pCtx->network_key2);
			
				if (pCtx->wep_transmit_key2 > 1) {
					printf("wep_key2 = %s\n", pCtx->wep_key22);
					printf("wep_key3 = %s\n", pCtx->wep_key32);
					printf("wep_key4 = %s\n", pCtx->wep_key42);
				}
			}
		}

		if(pCtx->wlan1_wsc_disabled == 1){
			printf("wlan1 interface wsc is Disabled\n");
		}else{
			printf("wlan1 interface wsc is Enabled\n");
		}
#endif //FOR_DUAL_BAND
	/*
#ifdef MUL_PBC_DETECTTION
		printf("disable_MulPBC_detection = %d\n", pCtx->disable_MulPBC_detection);
		printf("PBC_overlapping_LED_time_out = %d\n", pCtx->PBC_overlapping_LED_time_out);
		printf("WPS_PBC_overlapping_GPIO_number = %d\n", pCtx->WPS_PBC_overlapping_GPIO_number);
#endif
		//printf("connect_type = %d\n", pCtx->connect_type);
		//printf("disable_configured_by_exReg = %d\n", pCtx->disable_configured_by_exReg);
		printf("WPS_END_LED_unconfig_GPIO_number = %d\n", pCtx->WPS_END_LED_unconfig_GPIO_number);
		printf("WPS_END_LED_config_GPIO_number = %d\n", pCtx->WPS_END_LED_config_GPIO_number);
		printf("WPS_START_LED_GPIO_number = %d\n", pCtx->WPS_START_LED_GPIO_number);
		printf("No_ifname_for_flash_set = %d\n", pCtx->No_ifname_for_flash_set);
		
		printf("WPS_ERROR_LED_time_out = %d\n", pCtx->WPS_ERROR_LED_time_out);
		printf("WPS_ERROR_LED_GPIO_number = %d\n", pCtx->WPS_ERROR_LED_GPIO_number);
		printf("WPS_SUCCESS_LED_time_out = %d\n", pCtx->WPS_SUCCESS_LED_time_out);
		printf("WPS_SUCCESS_LED_GPIO_number = %d\n", pCtx->WPS_SUCCESS_LED_GPIO_number);
		printf("fix_wzc_wep = %d\n", pCtx->fix_wzc_wep);
	*/

		printf("configured button_hold_time = %d\n",pCtx->button_hold_time);
		if(pCtx->is_ap){
			printf("disable_configured_by_exReg = %d\n", pCtx->disable_configured_by_exReg);
		}

#ifdef WPS2DOTX
		if (pCtx->current_wps_version == WPS_VERSION_V2) {
			if(pCtx->EAP_frag_threshold)
				printf("EAP_frag_threshold = %d\n", pCtx->EAP_frag_threshold);
#ifdef EAP_REASSEMBLY
			printf("support EAP_frag reassembly\n");
#endif

#ifdef WSC_IE_FRAGMENT_STASIDE
			if(!pCtx->is_ap){
				printf("STA mode ;support Probe_Req wsc_ie frag = %s\n", 
					pCtx->probeReq_need_wscIE_frag?"true":"false");
			}
#endif
#ifdef WSC_IE_FRAGMENT_APSIDE
			if(pCtx->is_ap){
				printf("AP mode; Probe_Rsp wsc_ie frag =%s\n",
					pCtx->probeRsp_need_wscIE_frag?"true":"false");
			}
#endif
			if(pCtx->is_ap){
				printf("AP mode;Extension Tag =%s\n",
					pCtx->extension_tag?"true":"false");
			}
		}
#endif

		if(pCtx->upnp){
			unsigned char tmpstr[36];
			convert_bin_to_str(pCtx->uuid, UUID_LEN, tmpstr);	
			printf("uuid=%s\n",tmpstr);
		}
#ifdef AUTO_LOCK_DOWN
		if(pCtx->is_ap){
			printf("auto_lock_down:during[%d],fail[%d]times,locked[%d]seconds\n",
				AUTH_FAIL_TIME_TH , AUTH_FAIL_TIMES ,AUTO_LOCKED_DOWN_TIME);
		}
#endif
#ifdef WPS2DOTX
		if (pCtx->current_wps_version == WPS_VERSION_V2)
			printf("wsc version=2.0\n");
		else
#else
			printf("wsc version=1.0\n");
#endif
		printf("\n<<====WPS argument list=====>>\n");

	}
#endif

	if (pCtx->role == ENROLLEE && pCtx->pin_code[0] == '\0' && (pCtx->config_method&CONFIG_METHOD_PIN)) {
		struct timeval tod;
		int num;

		gettimeofday(&tod , NULL);
		srand(tod.tv_sec);
		num = rand() % 10000000;
		num = num*10 + compute_pin_checksum(num);
		convert_hex_to_ascii((unsigned long)num, pCtx->pin_code);
		
		_DEBUG_PRINT("Auto-generated PIN = %s\n", pCtx->pin_code);
	}

	return 1;
}


/*
 * main program enrty.
 * 	SYNOPSYS:
 *	  autoconf ARGs...
 *	ARGs
 *	  -c config_filename, config filename
 * 	  -w wlan_interface, wlan interface
 *	  -fi fifo_name, wlan fifo path name
 *	  -method val, 1: PIN, 2: PBC, 3: both
 *	  -mode val, 1: ap unconfigured, 2: client unconfigured (enrollee), 3: client configured (registrar)
 *				 4: ap-proxy configured, 5: ap-proxy registrar
 *	  -upnp val, 1 - support upnp, 0 - not support (default)
 *	  -gen-pin, generate pin code for local entitiy
 *	  -peer_pin, assign pin code for peer entitiy
  *	  -local_pin, assign local pin code
 *	  -sig_start, start wsc protocol
 *	  -sig_pbc, start PBC mode
 *	  -start_pbc, start PBC
 *	  -debug, turn on debug message
 *	  -test, go to test mode
 */
int main(int argc, char *argv[])
{
	CTX_Tp pCtx;
	int pid_fd, start_pbc=0;
	int idx=0;

	/* Allocate context */
	pCtx = (CTX_Tp) calloc(1, sizeof(CTX_T));
	if (pCtx == NULL) {
		printf("allocate context failed!\n");
		return 0;
	}
	pGlobalCtx = pCtx;

	/* Parse input argument */
	if (argc == 1) { // no argument
		show_help();
		return 0;
	}

	pCtx->mode = -1;
	pCtx->config_method = -1;
	pCtx->upnp = -1;
	pCtx->connect_type = -1;

#ifdef FOR_DUAL_BAND
	// default as wlan0 and wlan1 is disabled
	pCtx->wlan0_wsc_disabled = 1;
	pCtx->wlan1_wsc_disabled = 1;
	strcpy(pCtx->wlan_interface_name2, "wlan1");
#endif //FOR_DUAL_BAND
	
#ifdef WPS2DOTX
//	pCtx->EAP_frag_threshold = 100;
	pCtx->current_wps_version = WPS_VERSION_V2;
#else
	pCtx->current_wps_version = WPS_VERSION_V1;
#endif

#ifdef DEBUG
	pCtx->debug = 1;
	pCtx->debug2 = 1;

	WSC_DEBUG("daemon parameter:\n");
	for(idx=0;idx<argc ;idx++){
		printf("%s ",argv[idx]);
	}
	printf("\n");
#endif

	if (parse_argument(pCtx, argc, argv, &start_pbc) < 0){
		WSC_DEBUG("!!!return!!!\n\n");
		return 0;
	}
	#ifdef CONFIG_RTL867x
	if (WPS_updateWscConf("/etc/wscd.conf", pCtx->cfg_filename, 0)) {
		printf("fail to generate %s\n", pCtx->cfg_filename);
		return 0;
	}
	#endif

#if defined(DEBUG) && defined(OUTPUT_LOG)
	if(outlog_fp == NULL){
		outlog_fp = fopen( LOG_PATH ,"a+" );
	}
#endif

	memset(pCtx->SSID, 0, MAX_SSID_LEN+1);

#ifdef AUTO_LOCK_DOWN
	unlink(WSCD_LOCK_STAT);
#endif
	
	if (!pCtx->cfg_filename[0]){
		strcpy(pCtx->cfg_filename, DEFAULT_CONFIG_FILENAME);
        }
#ifdef SUPPORT_UPNP
	if (!pCtx->lan_interface_name[0])
		strcpy(pCtx->lan_interface_name, DEFAULT_LAN_INTERFACE);
#endif
	
	/* Initialize UDP socket */
	if (init_socket(pCtx) < 0) {
		free(pCtx);
		return 0;
	}
	
	/* Get local mac address */
	if (get_mac_addr(pCtx->socket, pCtx->wlan_interface_name, pCtx->our_addr) < 0)
		return 0;
#ifdef DEBUG
	if(pCtx->debug2){
		WSC_DEBUG("wlan0 addr: ");
		MAC_PRINT(pCtx->our_addr);
	}
#endif

#ifdef FOR_DUAL_BAND
	if(!pCtx->wlan1_wsc_disabled){
		if (get_mac_addr(pCtx->socket2, pCtx->wlan_interface_name2, pCtx->our_addr2) < 0)
			return 0;
		
		WSC_DEBUG("wlan1 addr: ");
		MAC_PRINT(pCtx->our_addr2);
	}
#endif //FOR_DUAL_BAND

	if (init_config(pCtx,0) == 0) {
		free(pCtx);
		WSC_DEBUG("	!!Exit wsc deamon!!\n\n");
		return 0;
	}

#if	0	//def FOR_DUAL_BAND
	if(!pCtx->is_ap){	// if client mode default use wlan0 as interface
		pCtx->InterFaceComeIn=COME_FROM_WLAN0;
	}
#endif //FOR_DUAL_BAND

	memcpy(pCtx->original_pin_code, pCtx->pin_code, PIN_LEN+1);
	DEBUG_PRINT("pCtx->original_pin_code = %s\n", pCtx->original_pin_code);
	
	RAND_seed(rnd_seed, sizeof(rnd_seed));

#ifdef SUPPORT_UPNP
//	if (pCtx->is_ap && pCtx->upnp && pCtx->role == REGISTRAR) {
//		DEBUG_ERR("Invalid config, ap+internal-registrar+external-upnp-registrar!\n");
//		return 0;
//	}

	WSC_pthread_mutex_init(&pCtx->RegMutex, NULL);
	//DEBUG_PRINT("Initialize Registration protocol mutex...\n");
	
	if (pCtx->upnp) {
		unsigned char init_count=0;

		while (init_count++ <= UPNP_INIT_TIMES) {
			if (start_upnp(pCtx) < 0) {
				printf("start_upnp() error!\n");
				if (init_count == UPNP_INIT_TIMES) {
					printf("wscd init fialed!\n");
					WSC_pthread_mutex_destroy(&pCtx->RegMutex);
					return 0;
				}
				else
					sleep(1);
			}
			else
				break;
		}

#if defined(USE_MINI_UPNP) && defined(DEBUG)
		/* TODO : change LOG_LOCAL0 to LOG_DAEMON */
		int openlog_option;
		int debug_flag = 1;
		openlog_option = LOG_PID|LOG_CONS;
		if(debug_flag)
			openlog_option |= LOG_PERROR;	/* also log on stderr */
		openlog("wscd", openlog_option, LOG_USER/*LOG_LOCAL0*/);
#endif
	}
#endif

#ifdef TEST
	if (pCtx->test) {
		if (run_test(pCtx) < 0) {
			free(pCtx);
			WSC_pthread_mutex_destroy(&pCtx->RegMutex);
			return 0;
		}
	}
#endif

	DISPLAY_BANNER;

	setsid();
	
	/* Create daemon */
	pid_fd = pidfile_acquire(pCtx->pid_filename);
	if (pid_fd < 0) {
		WSC_DEBUG("pidfile_acquire fail!!!\n\n");
		WSC_pthread_mutex_destroy(&pCtx->RegMutex);
		return 0;
	}

	pidfile_write_release(pid_fd);

	/* Issue ioctl to wlan driver to set ssid and enable AUTO_CONFIG */
	if (init_wlan(pCtx, 0) < 0) {
		WSC_DEBUG("init_wlan fail!!!\n\n");
		WSC_pthread_mutex_destroy(&pCtx->RegMutex);
		return 0;
	}

	if (IS_PBC_METHOD(pCtx->config_method)) {

#ifdef MUL_PBC_DETECTTION
		if (!pCtx->disable_MulPBC_detection) {
		// allocate buffer for dummy header
			if (pCtx->is_ap
#ifdef CLIENT_MODE
				|| (!pCtx->is_ap && pCtx->role == ENROLLEE)
#endif
			)
			{
				pCtx->active_pbc_staList = (struct pbc_node_context *) malloc(sizeof(struct pbc_node_context));
				if (pCtx->active_pbc_staList == NULL) {
					printf("%s %d Not enough mempry\n", __FUNCTION__, __LINE__);
					WSC_pthread_mutex_destroy(&pCtx->RegMutex);
					return 0;
				}
				memset(pCtx->active_pbc_staList, 0, sizeof(struct pbc_node_context));
				WSC_pthread_mutex_init(&pCtx->PBCMutex, NULL);
				//DEBUG_PRINT("Initialize PBC station list mutex...\n");
			}
		}
#endif
	}

	if (strlen(pCtx->peer_pin_code) > 0) // peer-pin code is assigned
		evHandler_pin_input(pCtx, pCtx->peer_pin_code);
	
	/* Install signal handler */
#ifdef WSC_1SEC_TIMER
	int ret;
	if ((ret = wsc_init_1sec_Timer()) != 0) {
		printf("<<-------Quit wscd!-------->>\n");
		return 0;
	}
#else
	// patch-100804
	//signal(SIGALRM, sigHandler_alarm);
#endif
	
	signal(SIGUSR2, sigHandler_user);
	signal(SIGTSTP, sigHandler_user);

#ifdef	AUTO_LOCK_DOWN	// process unloced
	signal(SIGUSR1, sigHandler_user);
#endif

#ifndef WSC_1SEC_TIMER
	/* Start one second timer */
	// patch-100804
	//alarm(1);
#endif

	// enable LED
	enable_WPS_LED();
	if (wlioctl_set_led(pCtx->wlan_interface_name, LED_WSC_END) < 0) {
		printf("issue wlan ioctl set_led error!\n");
		return 0;
	}
	
	WSC_DEBUG("alarm(1)---------->\n");

	if (start_pbc)
		process_event(pCtx, EV_PB_PRESS);

#ifdef CLIENT_MODE
	if (!pCtx->is_ap && pCtx->start && !start_pbc)
		process_event(pCtx, EV_START);
#endif

	if (pCtx->upnp) {
#ifdef USE_MINI_UPNP
#ifdef STAND_ALONE_MINIUPNP
		unsigned char byebye_max_age=1;
		//sending byebye
		WSC_DEBUG("!!	sending byebye\n\n");
		SendSSDPNotifies(pCtx->upnp_info.snotify, pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port,
			&pCtx->upnp_info.SSDP, 1, byebye_max_age);

		sleep(byebye_max_age);
		
		//sending alive
		WSC_DEBUG("!!	sending alive\n\n");
		SendSSDPNotifies(pCtx->upnp_info.snotify, pCtx->upnp_info.lan_ip_address, pCtx->upnp_info.port,
			&pCtx->upnp_info.SSDP, 0, pCtx->upnp_info.SSDP.max_age);
	
		pCtx->upnp_info.SSDP.alive_timeout = pCtx->upnp_info.SSDP.max_age/2;
#else

		FILE *fp = NULL;

		if (NULL == (fp = fopen(WSCD_CONFIG_FILE,"w"))) {
			printf("output file [%s] can not open\n", WSCD_CONFIG_FILE);
			WSCUpnpStop();
			return 0;
		}
		else {
			char *buffo=NULL;

			buffo = (char *) malloc(256);
			if (buffo == NULL) {
				WSCUpnpStop();
				fclose(fp);
				return 0;
			}
			
			memset(buffo, 0, 256);
			sprintf(buffo, "port %d\n", pCtx->upnp_info.port);
			fputs(buffo, fp);
			
			memset(buffo, 0, 256);
			sprintf(buffo, "max_age %d\n", pCtx->upnp_info.SSDP.max_age);
			fputs(buffo, fp);

			memset(buffo, 0, 256);
			sprintf(buffo, "uuid %s\n", pCtx->upnp_info.SSDP.uuid);
			fputs(buffo, fp);

			memset(buffo, 0, 256);
			sprintf(buffo, "root_desc_name %s\n", pCtx->upnp_info.SSDP.root_desc_name);
			fputs(buffo, fp);

			int i=0;
			while (pCtx->upnp_info.SSDP.known_service_types[i]) {
				memset(buffo, 0, 256);
				sprintf(buffo, "known_service_types %s\n", pCtx->upnp_info.SSDP.known_service_types[i]);
				fputs(buffo, fp);
				i++;
			}
			
			fclose(fp);
			free(buffo);
		}
#endif	// STAND_ALONE_MINIUPNP
#endif	// USE_MINI_UPNP
	}

#if !defined(NO_IWCONTROL) || defined(USE_MINI_UPNP)
	listen_and_process_event(pCtx); // never return
#else
	// no NO_IWCONTROL is defined
	for(;;) pause();
#endif

	return 0;
}
