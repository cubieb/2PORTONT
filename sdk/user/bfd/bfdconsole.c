#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "bfdlib.h"

#ifdef _HANDLE_CONSOLE_
static void bfd_handle_console_input( struct bfdsessiondata *pb, unsigned char *b)
{
	if(pb && b)
	{
		struct timeval tv;

		printf( BFDDBG_FMT "input(%d)=%s\n", BFDDBG_ARG(pb), strlen(b), b  );
		
		switch(b[0])
		{
		case 'h':
			printf( "\th: help\n" );
			printf( "\tq: quit\n" );
			printf( "\ts: show settings and status\n" );
			printf( "\tp: poll sequence start\n");
			printf( "\tb[ed]: enable/disable the BFD session\n" );
			printf( "\tt[+-]: change bfd.DesiredMinTxInterval\n" );
			printf( "\tr[+-]: change bfd.RequiredMinRxInterval\n" );
			printf( "\tc[ed]: enable/disable the tx of BFD echo packets\n" );
			printf( "\to[+-]: change bfd.RemoteEchoRxInterval\n" );
			printf( "\tm[ed]: enable/disable the demand mode\n" );
			printf( "\tu[+-]: change bfd.DetectMult\n" );
			break;
		case 'q':
			printf( BFDDBG_FMT "quit\n", BFDDBG_ARG(pb) );
			exit(0);
			break;
		case 's':
			printf( BFDDBG_FMT "show settings and status\n", BFDDBG_ARG(pb) );
			bfd_sessiondata_dump(pb);
			break;
		case 'b':
			if(b[1]=='e')
				bfd_handle_statemachine( pb, EVENT_ENABLE_SESSION);
			else if(b[1]=='d')
			{
				bfd_handle_statemachine( pb, EVENT_DISABLE_SESSION);
				//test for the gracefual shutdown case
				//pb->session.LocalDiag=DC_PATH_DOWN;
				//bfd_pollseq_increase(pb);//pb->PollSeq++;
			}
			break;
		case 'p':
			{
				printf( BFDDBG_FMT "start poll sequence\n", BFDDBG_ARG(pb) );
				bfd_pollseq_increase(pb);//pb->PollSeq++;
			}
			break;
		case 't':
			{
				printf( BFDDBG_FMT "bfd.DesiredMinTxInterval=%u", BFDDBG_ARG(pb), pb->session.DesiredMinTxInterval );
				if( b[1]=='+' )
				{
					if( (pb->session.SessionState==ST_UP)&&(pb->session.OldDesiredMinTxInterval==0) )
					{
						pb->session.OldDesiredMinTxInterval=pb->session.DesiredMinTxInterval;
					}
					pb->session.DesiredMinTxInterval=pb->session.DesiredMinTxInterval+10*BFD_ONESECOND;
					pb->session.UserDesiredMinTxInterval=pb->session.DesiredMinTxInterval;
					bfd_pollseq_increase(pb);//pb->PollSeq++;
				}else if( b[1]=='-' )
				{
					pb->session.DesiredMinTxInterval=pb->session.DesiredMinTxInterval-10*BFD_ONESECOND;
					pb->session.UserDesiredMinTxInterval=pb->session.DesiredMinTxInterval;
					bfd_pollseq_increase(pb);//pb->PollSeq++;
				}
				printf( "==> new value=%u\n", pb->session.DesiredMinTxInterval );				
			}
			break;
		case 'r':
			{
				printf( BFDDBG_FMT "bfd.RequiredMinRxInterval=%u", BFDDBG_ARG(pb), pb->session.RequiredMinRxInterval);
				if( b[1]=='+' )
				{
					pb->session.RequiredMinRxInterval=pb->session.RequiredMinRxInterval+10*BFD_ONESECOND;
					bfd_pollseq_increase(pb);//pb->PollSeq++;
				}else if( b[1]=='-' )
				{
					if( (pb->session.SessionState==ST_UP)&&(pb->session.OldRequiredMinRxInterval==0) )
					{
						pb->session.OldRequiredMinRxInterval=pb->session.RequiredMinRxInterval;
					}
					pb->session.RequiredMinRxInterval=pb->session.RequiredMinRxInterval-10*BFD_ONESECOND;	
					bfd_pollseq_increase(pb);//pb->PollSeq++;
				}
				printf( "==> new value=%u\n", pb->session.RequiredMinRxInterval );				
			}
			break;
		case 'c':
			{
				if(b[1]=='e')
				{
					pb->EchoAdminEnable=1;
				}else if(b[1]=='d') {
					pb->EchoAdminEnable=0;
				}
				printf( BFDDBG_FMT "%s to tx BFD echo packets\n", BFDDBG_ARG(pb), pb->EchoAdminEnable?"enable":"disable" ); 
			}
			break;
		case 'o':
			{
				printf( BFDDBG_FMT "bfd.LocalEchoRxInterval=%u", BFDDBG_ARG(pb), pb->session.LocalEchoRxInterval );
				if( b[1]=='+' )
				{
					pb->session.LocalEchoRxInterval=pb->session.LocalEchoRxInterval+2000000;
					bfd_pollseq_increase(pb);//pb->PollSeq++;
				}else if( b[1]=='-' )
				{
					if(pb->session.LocalEchoRxInterval<=2000000)
					{
						if( (pb->session.SessionState==ST_UP)&&
							(pb->session.OldLocalEchoRxInterval==0)&&
							(pb->session.LocalEchoRxInterval) )
							pb->session.OldLocalEchoRxInterval=pb->session.LocalEchoRxInterval;
						pb->session.LocalEchoRxInterval=0;
					}else
						pb->session.LocalEchoRxInterval=pb->session.LocalEchoRxInterval-2000000;
					bfd_pollseq_increase(pb);//pb->PollSeq++;
				}
				printf( "==> new value=%u\n", pb->session.LocalEchoRxInterval );				
			}
			break;
		case 'm':
			{
				if(b[1]=='e')
				{
					pb->session.DemandMode=1;
				}else if(b[1]=='d') {
					pb->session.DemandMode=0;
				}
				printf( BFDDBG_FMT "%s to bfd DemandMode\n", BFDDBG_ARG(pb), pb->session.DemandMode?"enable":"disable" ); 
			}
			break;
		default:
			printf( BFDDBG_FMT "unknown input\n", BFDDBG_ARG(pb), __FUNCTION__ );
			break;
		case 'u':
			{
				printf( BFDDBG_FMT "bfd.DetectMult=%u", BFDDBG_ARG(pb), pb->session.DetectMult);
				if( b[1]=='+' )
				{
					pb->session.DetectMult++;
					//bfd_pollseq_increase(pb);//pb->PollSeq++;
				}else if( b[1]=='-' )
				{
					if(pb->session.DetectMult>1)
						pb->session.DetectMult--;
					else
						pb->session.DetectMult=1;
					//bfd_pollseq_increase(pb);//pb->PollSeq++;
				}
				printf( "==> new value=%u\n", pb->session.DetectMult );				
			}
			break;
		}
	}
	
	return;
}

void bfd_handle_console(struct bfdsessiondata *pb)
{
	fd_set fd;
	int maxfd;
	int r;
	struct timeval to;

	if(!pb) return;

        FD_ZERO(&fd);
        FD_SET(STDIN_FILENO, &fd);
        maxfd=STDIN_FILENO;
        to.tv_sec=0;
        to.tv_usec=0;
        //r = select( maxfd+1, &fd, NULL, &fd, &to);
        r = select( maxfd+1, &fd, NULL, NULL, &to);
        if(r<0)
        	perror( "bfd_handle_console> select error" );
        else if(r>0)
        {
        	if( FD_ISSET(STDIN_FILENO, &fd) )
        	{
			char	buf[128];
			int	rlen;
        		rlen=read( STDIN_FILENO, buf, sizeof(buf)-1 );
        		if(rlen>0)
        		{
        			char *p;
        			buf[rlen]=0;
        			p = strchr( buf, '\r' );
        			if(p) *p=0;
        			p = strchr( buf, '\n' );
        			if(p) *p=0;
        			bfd_handle_console_input( pb, buf );
        		}
        	}
        }
        
        return;
}
#endif /*_HANDLE_CONSOLE_*/
