#include <unistd.h>
#include "../common/operation_mode_defs.h"
#include "si_dect_callcontrol.h"
#include "cvmcon.h"
#include "si_dect_api.h"


// -------------------------------------------------------- 
// DECT API - SIP to RTX 
// -------------------------------------------------------- 

/* copy from si_ua_readMACAddress() in SiPhoneUA/si_ua_init.c */
#include <net/if.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

static int readMACAddress( const char * ifname, unsigned char *MACAddress )
{
	int sfd;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

	memset(&ifr, 0, sizeof ifr);

	if (0 > (sfd = socket(AF_INET, SOCK_STREAM, 0))) {
		//si_print(PRINT_LEVEL_ERR, "Error opening socket to read MAC Address \n");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	sin->sin_family = AF_INET;

	if (0 > ioctl(sfd, SIOCGIFHWADDR, &ifr)) {
		//si_print(PRINT_LEVEL_ERR, "Unable to get MAC Address \n");
		return -1;
	}

	memcpy(MACAddress, (unsigned char *) &ifr.ifr_addr.sa_data, 6);

	if (MACAddress[0] + MACAddress[1] + MACAddress[2] + MACAddress[3] + MACAddress[4] + MACAddress[5]) {
		//si_print(PRINT_LEVEL_INFO, "\n\nHW Address: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n\n",
		//						MACAddress[0], MACAddress[1], MACAddress[2], 
		//						MACAddress[3], MACAddress[4], MACAddress[5]);

	}else memcpy(MACAddress, "00:01:02:03:04:05", 6);

	return 0;
}

void dect_api_S2R_init_with_args( const dect_api_init_args_t * args )
{
	unsigned char SystemIPAddress[64];
	const char *ifname = { "br0" };
	
	// RFPI  
	if( args == NULL || args ->ifname == NULL )
		;	// default value 
	else
		ifname = args ->ifname;
		
	readMACAddress( ifname, SystemIPAddress );
	sc1445x_dect_change_rfpi( SystemIPAddress );
	
	// firmware filename
	if( args )
		dect_api_S2R_set_fwu( args ->fwname );
	
	// start dect main 
	dect_main();
}

void dect_api_S2R_set_fwu( const char *file )
{
	if( file )
		FwuSetFile( file );
}

void dect_api_S2R_get_info( dect_api_info_t *info )
{
	info ->VersionHex = ref_VersionHex;
	info ->AccessCode[ 0 ] = ref_AccessCode[ 0 ];
	info ->AccessCode[ 1 ] = ref_AccessCode[ 1 ];
}

#if !defined( SA_CVM_NO_MAIN )
int main( int argc, char *argv[] )
{
	dect_api_init_args_t args;
	
	if( argc >= 2 ) {
		args.ifname = "br0";
		if( strcmp( argv[ 1 ], "112" ) == 0 )
			args.fwname = "/mnt/nfs/dect/Cvm480Fp_v0112.fwu";
		else
			args.fwname = "/mnt/nfs/dect/Cvm480Fp_v0113.fwu";
		dect_api_S2R_init_with_args( &args );
	} else
		dect_api_S2R_init();
	
	printf("\n-Sleeping %s..\n", __FUNCTION__);
#if 1
	extern void ConsoleEnterKeyMessage( unsigned char ch );
	unsigned char ch;
	
	while( 1 ) {
		//printf( "> " );
		if( scanf( "%c", &ch ) == 1 )
			ConsoleEnterKeyMessage( ch );
	}	
#else
	while( 1 )
		sleep( 5 );	
#endif
	
	return 0;
}
#endif

int dect_api_S2R_call_setup( int wideband, unsigned long chid, const char *dialednum, const char *callerid )
{
	return HandleSendConnectReq( ( wideband ? 1 : 0 ),	// codec: narrow band = 0, wide band = 1
								 ( unsigned char )chid,
								 dialednum,
								 callerid );
}

void dect_api_S2R_call_release( unsigned long chid )
{
	HandleSendReleaseReq( ( unsigned char )chid );
}

int dect_api_S2R_check_handset_busy( unsigned long chid )
{
	return CheckIfCallStateIsBusy( ( unsigned char )chid );
}

int dect_api_S2R_check_handset_ringing( unsigned long chid )
{
	return CheckIfCallStateIsRinging( ( unsigned char )chid );
}

// -------------------------------------------------------- 
// DECT API - RTX to SIP 
// -------------------------------------------------------- 

void dect_api_R2S_pressed_key( int accountid, int port, char input )
{
#ifdef CONFIG_RTK_VOIP
	extern int dect_event_in(unsigned long ch_id, char input);
	
	dect_event_in( accountid, input );
#endif
}

