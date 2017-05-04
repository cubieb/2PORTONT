#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

/* included in rtk_voip/include/ */
#include "voip_manager.h"
#include "voip_ioctl.h"
#define IVR_G722_ENCODED_BYTES_PER_FRAME	( 80 )
#define IVR_G722_MS_PER_FRAME		( 10 )

int rtk_SetIvrPlayG722( unsigned int nCount, const unsigned char *pData, IvrPlayDir_t dir )
{
	TstVoipPlayIVR_G722 stVoipPlayIVR;
	
	if( nCount > MAX_FRAMES_OF_G722 )
		nCount = MAX_FRAMES_OF_G722;
	
	stVoipPlayIVR.ch_id = 0;
	stVoipPlayIVR.type = IVR_PLAY_TYPE_G722;
	stVoipPlayIVR.dir = dir;
	stVoipPlayIVR.nFramesCount = nCount;
	
	memcpy( stVoipPlayIVR.data, pData, nCount * IVR_G722_ENCODED_BYTES_PER_FRAME );

	SETSOCKOPT(VOIP_MGR_PLAY_IVR, &stVoipPlayIVR, TstVoipPlayIVR_G722, 1);
    
	//printf( "IVR playing time: %u0 ms\n", stVoipPlayIVR.playing_period_10ms );
	//printf( "\tCopied data:%d\n", stVoipPlayIVR.nRetCopiedFrames );
    
	return stVoipPlayIVR.nRetCopiedFrames;
}

int main( int argc, char **argv )
{
	int dir = 0;
	FILE *fp;
	unsigned char buffer[ IVR_G722_ENCODED_BYTES_PER_FRAME * IVR_G722_MS_PER_FRAME ];
	unsigned int cRead, cWritten, shift;
	payloadtype_config_t codec_config;
	
	if( ( fp = fopen( "722_raw", "rb" ) ) == NULL ) {
		printf( "Open error\n" );
		return 1;
	}

	if (dir == 0) 
		printf("playing G.722 to local\n");
	else if (dir ==1)
		printf("playing G.722 to remote\n");
	else
		return 1;

	printf( "722 IVR playing...\n" );

	// PCM mode MUST be set to wideband	
	codec_config.uLocalPktFormat = rtpPayloadG722;
	codec_config.uRemotePktFormat = rtpPayloadG722;
	codec_config.nPcmMode = 3;	// wide-band
	rtk_SetRtpPayloadType( &codec_config );
	//set to pcm ch0 to wide band
	rtk_enablePCM( 0, 0 );	// ch0. disable pcm
	rtk_enablePCM( 0, 2 );	// ch0. wide band
	
	while( 1 ) {
		
		cRead = fread( buffer, 80, IVR_G722_MS_PER_FRAME, fp );	// 10frames* 80bytes/frame
		shift = 0;
		
		if( cRead == 0 )	/* seen as eof */
			break;

lable_put_g722_data:
		cWritten = rtk_SetIvrPlayG722( cRead, buffer + shift, dir );
		//printf( "Write:%d, %d\n", cRead, shift );
	
		/* buffer is full */
		if( cWritten < cRead ) {			
			printf( "Buffer is full.. Wait one second...\n" );
			//printf( "[%d:%d:%d]\n", cRead, cWritten, shift );
			rtk_SetIvrPlayG722( 0, buffer, dir );	/* show current playing time */
			
			sleep( 1 );	
			
			cRead -= cWritten;
			shift += cWritten * IVR_G722_ENCODED_BYTES_PER_FRAME;
			printf( "[%d:%d:%d]\n", cRead, cWritten, shift );
			
			goto lable_put_g722_data;
		}
	}
	
	fclose( fp );
	// set to pcm ch0 to narrow band
	sleep( 1 );
	rtk_enablePCM( 0, 0 );	// ch0. disable pcm
	rtk_enablePCM( 0, 1 );	// ch0. narrow band
	
	return 0;
}
