#include <stdio.h>
#ifdef EMBED
#include <rtk/utility.h>
#endif //EMBED

#include "bfdutil.h"

void bfdutil_dumpcmd( char *pre, char *argv[] )
{
	int i;
	if(pre) printf( "%s> ", pre );
	printf( "CMD: ", pre );
	for(i=0; argv[i]; i++)
		printf( "%s ", argv[i] );
	printf( "\n" );
		
}


static void bfdutil_iprules( int debug, unsigned char *intfname, unsigned char *localip, unsigned char *remoteip, unsigned char *echoremoteip, int enable )
{
#ifdef EMBED
	char *argv[30];
	char *action;
	int i;

	if(enable) 
		action="-I";
	else
		action="-D";

	//allow udp bfd packets with port 3784 in
	// /bin/iptables -I INPUT -i vc2 -s 192.168.4.1 -d 192.168.4.6 -p UDP --dport 3784 -m limit --limit 40/s -j ACCEPT
	i=0;
	argv[i++]=(char *)IPTABLES;
	argv[i++]=action;
	argv[i++]=(char *)FW_INPUT;
	argv[i++]=(char *)ARG_I;
	argv[i++]=intfname;
	argv[i++]="-s";
	argv[i++]=remoteip;
	argv[i++]="-d";
	argv[i++]=localip;
	argv[i++]="-p";
	argv[i++]=(char *)ARG_UDP;
	argv[i++]=(char *)FW_DPORT;
	argv[i++]="3784";
	argv[i++]="-m";
	argv[i++]="limit";
	argv[i++]="--limit";
	argv[i++]="40/s";
	argv[i++]="-j";
	argv[i++]=(char *)FW_ACCEPT;
	argv[i++]=NULL;
	if(debug) bfdutil_dumpcmd( __FUNCTION__, argv );
	do_cmd( argv[0], argv, 1 );

	//allow udp bfd-echo packets with port 3785 in
	// /bin/iptables -I INPUT -i vc2 -d 192.168.4.6 -p UDP --dport 3785 -m limit --limit 40/s -j ACCEPT
	i=0;
	argv[i++]=(char *)IPTABLES;
	argv[i++]=action;
	argv[i++]=(char *)FW_INPUT;
	argv[i++]=(char *)ARG_I;
	argv[i++]=intfname;
	argv[i++]="-d";
	argv[i++]=localip;
	argv[i++]="-p";
	argv[i++]=(char *)ARG_UDP;
	argv[i++]=(char *)FW_DPORT;
	argv[i++]="3785";
	argv[i++]="-m";
	argv[i++]="limit";
	argv[i++]="--limit";
	argv[i++]="40/s";
	argv[i++]="-j";
	argv[i++]=(char *)FW_ACCEPT;
	argv[i++]=NULL;
	if(debug) bfdutil_dumpcmd( __FUNCTION__, argv );
	do_cmd( argv[0], argv, 1 );

	//forward udp bfd-echo packets with port 3785
	// iptables -I FORWARD -i vc2 -d 192.168.4.1 -p UDP --dport 3785 -m limit --limit 40/s -j ACCEPT
	i=0;
	argv[i++]=(char *)IPTABLES;
	argv[i++]=action;
	argv[i++]=(char *)FW_FORWARD;
	argv[i++]=(char *)ARG_I;
	argv[i++]=intfname;
	argv[i++]="-d";
	argv[i++]=echoremoteip;
	argv[i++]="-p";
	argv[i++]=(char *)ARG_UDP;
	argv[i++]=(char *)FW_DPORT;
	argv[i++]="3785";
	argv[i++]="-m";
	argv[i++]="limit";
	argv[i++]="--limit";
	argv[i++]="40/s";
	argv[i++]="-j";
	argv[i++]=(char *)FW_ACCEPT;
	argv[i++]=NULL;
	if(debug) bfdutil_dumpcmd( __FUNCTION__, argv );
	do_cmd( argv[0], argv, 1 );

	//avoid to do SNAT (do not change the source address when forward the bfd-echo packets)
	// iptables -t nat -I POSTROUTING -o vc2 -d 192.168.4.1 -p UDP --dport 3785 -j ACCEPT
	i=0;
	argv[i++]=(char *)IPTABLES;
	argv[i++]=(char *)ARG_T;
	argv[i++]="nat";
	argv[i++]=action;
	argv[i++]="POSTROUTING"; //(char *)FW_POSTROUTING;
	argv[i++]=(char *)ARG_O;
	argv[i++]=intfname;
	argv[i++]="-d";
	argv[i++]=echoremoteip;
	argv[i++]="-p";
	argv[i++]=(char *)ARG_UDP;
	argv[i++]=(char *)FW_DPORT;
	argv[i++]="3785";
	argv[i++]="-j";
	argv[i++]=(char *)FW_ACCEPT;
	argv[i++]=NULL;
	if(debug) bfdutil_dumpcmd( __FUNCTION__, argv );
	do_cmd( argv[0], argv, 1 );

#endif //EMBED
	return;
}

void bfdutil_set_iprules( int debug, unsigned char *intfname, unsigned char *localip, unsigned char *remoteip, unsigned char *echoremoteip )
{
	bfdutil_iprules( debug, intfname, localip, remoteip, echoremoteip, 1 );
	return;
}
void bfdutil_clear_iprules( int debug, unsigned char *intfname, unsigned char *localip, unsigned char *remoteip, unsigned char *echoremoteip )
{
	bfdutil_iprules( debug, intfname, localip, remoteip, echoremoteip, 0 );
	return;
}

