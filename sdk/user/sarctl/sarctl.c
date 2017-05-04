#define AUTO_PVC_SEARCH
#include "sarctl.h"
//ql
#include <rtk/options.h>
#define CONFIG_USER_PORT_MIRROR

uint8	MAC_ADDR[6] = {0x00, 0x00, 0x86, 0x70, 0x00, 0x01};

void DumpOamState(ATMOAMLBState *);
void DumpOamStatefe(ATMOAMLBRXState *);
BOOL Parsing_OAM_id(ATMOAMLBID *myid, char **spp);
BOOL Parsing_OAM_req(ATMOAMLBReq *req, char **spp);
BOOL Parsing_OAM_status(ATMOAMLBState *state, char **spp);
BOOL Parsing_OAM_statusfe(ATMOAMLBRXState *statefe, char **spp);

BOOL parseQosCfg(struct SAR_IOCTL_CFG *cfg, char **spp)
{
	int	loopback;
	*spp++;	
	//cfg->ch_no	= s2i(*(spp++));

	if (!strncmp(*spp, "vc", 2)) {
		cfg->ch_no = s2i(*spp+2);
	} else {
		printf("QoS channel mode error, %s\n", *spp);
		goto ERROR;
	}
	spp++;
		
	/* QoS Type */
	if(!strcmp(*spp, "ubr"))
		cfg->QoS.Type = QoS_UBR;
	else if(!strcmp(*spp, "cbr"))
		cfg->QoS.Type = QoS_CBR;
	else if(!strcmp(*spp, "nrt-vbr"))
		cfg->QoS.Type = QoS_nrtVBR;
	else if(!strcmp(*spp, "rt-vbr"))
		cfg->QoS.Type = QoS_rtVBR;
	else {
		printf("QoS type error, %s\n", *spp);
		goto ERROR;
	}
	spp++;
	
	cfg->QoS.PCR = (uint16)s2i(*(spp++));
	cfg->QoS.SCR = (uint16)s2i(*(spp++));
	cfg->QoS.MBS = (uint8)s2i(*(spp++));
	return TRUE;

ERROR:
	return FALSE;
}

static void usage(void)
{
	char usage_desc_now[] = 
"Usage:\n\
  [getstats [<ch_no>]]\n\
  [resetstats [<ch_no>]]\n\
  [enable]\n\
  [disable]\n\
  [getconfig <ch_no>]\n\
  [setconfig <ch_no> <vpi> <vci> <rfc> <framing> <QoS> <PCR> <SCR> <MBR> <CDVT>]\n\
  [createvc <ch_no> <vpi> <vci> <rfc> <framing> <QoS> <PCR> <SCR> <MBR> <CDVT>]\n\
  [qos <ifname> <QoS> <PCR> <SCR> <MBR>]\n\
  [deletevc <ch_no>]\n\
  [oamsetid <vpi> <vci> <sourceID>]\n\
  [oamstart <vpi> <vci> <scope> <locationID> <sourceID>]\n\
  [oamstop <vpi> <vci> <tag>]\n\
  [oamtest <vpi> <vci> <scope> <locationID>]\n\
  [oamteststop]\n\
  [oamstatus <vpi> <vci> <tag>]\n\
  [oamstatusfe <vpi> <vci>]\n\
   : ch_no      = 0 ~ 15\n\
   : vpi        = 0 ~ 255\n\
   : vci        = 0 ~ 65535\n\
   : rfc        = 1483_br, 1483_rt, 1577, 2364, 2516\n\
   : framing    = llc, vcmux\n\
   : QoS_Type   = ubr, cbr, nrt-vbr, rt-vbr\n\
   : PCR        = 0 ~ 65535\n\
   : SCR        = 0 ~ 65535\n\
   : MBR        = 0 ~ 255\n\
   : CDVT       = 0 ~ 4294967295\n\
  [slow]\n\
  [fast]\n\
  [loopback <1/0>]\n\
  [pvcnumber <vc_num>]\n\
  [debug size <num>]\n\
  [debug status]\n\
  [debug pvc <flag> <options>]\n\
   flag\n\
   : 0  = disabled\n\
   : 1  = both\n\
   : 2  = input\n\
   : 3  = output\n\
   options\n\
   : -c channel\n\
   	Specify pvc channel number, default is all channels.\n\
   : -e type\n\
   	Specify EtherType value of Ethernet header,\n\
   		ex. 0x0800, 0x0806, 0x8864 ...\n\
   : -p protocol\n\
   	Specify IP Protocol value of IP header,\n\
   		ex. 0x01, 0x06, 0x11 ...\n\
   : -m\n\
   	Use human-readable format.\n\
  [debug obaal5 [0/1]]\n\
  [debug obcell [0/1]]\n\
  [debug eoc [0/1]]\n\
  [debug eoc_lbrx [0/1]]\n\
  [debug eoc_lbtx [0/1]]\n\
  [tc <maxtraffic(kBps)>]\n\
  [qos_imq <1/0>]\n\
  [mirror <vpi> <vci> <mirror_flag>]\n\
   : mirror_flag      = 0: not mirror, 1: mirror sar input, 2: mirror sar output, 3: mirror both\n\
   : vpi/vci is optional\n\
   : dst mac's 3,4th byte of unicast mirror packets will be plus 1\n\
   : for pppoa and ipoa packets, fake pppoe header or mac header(dst mac: 00-06-68-89-90-06, src mac: 00-66-88-99-00-66) will be added\n\
  [mirror_ob <mirror_flag>]\n\
   : mirror AAL5 for outband channel.\n\
   : mirror_flag      = 0: not mirror, 1: mirror sar input, 2: mirror sar output, 3: mirror both\n\
";

	printf(usage_desc_now);
	//free(usage_desc);
	//free(usage_desc_now);
	return;
}

int main(int argc, char *argv[])
{
	
	int skfd,eocsd,i,j;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBID lbId;
	ATMOAMLBState lbState;
	ATMOAMLBRXState lbStatefe;
	struct ifreq				ifr;
	struct SAR_IOCTL_CFG	cfg, total_cfg,cfg_1;
	struct ch_stat				stat;

//	char	*dev_name = "eth1";
	char	**spp;

	macaddr_t 	mac_P;
	char		tmp_mac;

	argc--;
	argv++;

	if (argc == 0) goto USAGE_END;

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		exit(1);
	}
	
	eocsd = socket(PF_DSLEOC, SOCK_DGRAM, 0);
#if 0
 	if (eocsd < 0) {
  		perror("EOC socket open error\n");
  		exit (1);
 	}
#endif
//	strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);

//	ifr.ifr_data = (void *)&cfg;
	spp = argv;
	mysio.number = 0;

	/* ATM_SAR_GETSTAT */
	if (!strcmp(*spp, "getstats")) {
		if(*(spp+1)!=NULL){				
			cfg.ch_no = s2i(*(spp+1));
			mysio.arg = (void *)&cfg;
			if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)	goto ERROR_END;
			printf("channel %d statistics\n", cfg.ch_no);
			//DumpStat(cfg);
			DumpStat_2(cfg);
			goto END;
		}else{

			memset(&total_cfg.stat, 0, sizeof(ch_stat));
			mysio.arg = (void *)&cfg;
			for(i=0;i<16;i++){
				cfg.ch_no=i;
				if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)	goto ERROR_END;
				AccumCfg(&total_cfg, cfg);
			}
			printf("Statistics of All enabled channels\n");
			DumpStat(total_cfg);
			goto END;
		}
	}
#ifdef CONFIG_RTL8672	
	/* SAR_SET_SARHDR */
	if (!strcmp(*spp, "set_header")) {
		int ch_no,encap_mode,*ptr,vpi,vci;

		if(argc==2) {
			encap_mode=0;	vpi=0;vci=0;
		}else {
			*spp++;
			encap_mode=s2i(*spp);
			*spp++;
			vpi=s2i(*spp);
			*spp++;
			vci=s2i(*spp);
		}
		if (!(ptr=malloc(12))) {
			printf("memory read malloc failed!\n");
			goto ERROR_END;
		}
		ptr[0]=encap_mode;
		ptr[1]=vpi;
		ptr[2] =vci;

		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)ptr;		

		if ((ioctl(skfd, SAR_SET_SARHDR, &mysio))<0)	
			goto ERROR_END;
		goto END;
	}		
	
	if (!strcmp(*spp, "set_pkta")) {
		int ch_no,encap_mode,*ptr,vpi,vci;

		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)ptr;		

		if ((ioctl(skfd, SAR_SET_PKTA, &mysio))<0)	
			goto ERROR_END;
		goto END;
	}		
#endif	
	/* SAR_SET_BRIDGE_MODE */
	if (!strcmp(*spp, "direct_bridge")) {
#if 0
		int ch_no;
		if(argc!=2)	goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&ch_no;		
		ch_no = s2i(*(spp+1));
#else
		int ch_no,*ptr,vpi,vci;

		*spp++;
		ch_no = s2i(*spp);
		if(argc==2) {
			j=0;	vpi=0;vci=0;
		}else {
			*spp++;
			j=s2i(*spp);
			*spp++;
			vpi=s2i(*spp);
			*spp++;
			vci=s2i(*spp);
		}
		if (!(ptr=malloc(16))) {
			printf("memory read malloc failed!\n");
			goto ERROR_END;
		};
		ptr[0]=ch_no;
		ptr[1]=j;
		ptr[2] =vpi;
		ptr[3]=vci;

		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)ptr;		
#endif  //#if 0
		if ((ioctl(skfd, SAR_SET_BRIDGE_MODE, &mysio))<0)	
			goto ERROR_END;
		goto END;
}
	
	//ql_xu:
	if (!strcmp(*spp, "qos_imq")) {
		int enable;
		if(argc!=2)	goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&enable;
		//mysio.arg = (void *)ptr;
		enable = s2i(*(spp+1));
		if ((ioctl(skfd, QOS_ENABLE_IMQ, &mysio))<0)	goto ERROR_END;
		goto END;
	}
	
	//ql_xu:
	// qos_imq = qosstate . Mason Yu

	if (!strcmp(*spp, "qos")) {
		struct SAR_IOCTL_CFG qosCfg;
		
		if(argc < 6) goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&qosCfg;
		/* Set Configuration */
		parseQosCfg(&qosCfg, spp);
		if(ioctl(skfd, PVC_QOS_TYPE, &mysio)<0)	goto ERROR_END;

		goto END;
	}
	
	/* SAR_ENABLE */
	if (!strcmp(*spp, "enable")) {
		if(ioctl(skfd, SAR_ENABLE, &ifr)<0)	goto ERROR_END;
		printf("SAR Enabled\n");
		goto END;
	}
	
	/* SAR_DISABLE */
	if (!strcmp(*spp, "disable")) {
		if(ioctl(skfd, SAR_DISABLE, &ifr)<0)	goto ERROR_END;
		printf("SAR Disabled\n");
		goto END;
	}
	
	/* SAR_GET_CONFIG */
	if (!strcmp(*spp, "getconfig")) {
		if(*(spp+1)!=NULL)
			cfg.ch_no = s2i(*(spp+1));
		else goto USAGE_END;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETCONFIG, &mysio)<0)	goto ERROR_END;
		//if(ioctl(skfd, SAR_GET_CONFIG, &ifr)<0)	goto ERROR_END;

		if(cfg.created!=VC_CREATED)	printf("Channel %d is not Created\n", cfg.ch_no);
		else					DumpCfg(cfg);
		goto END;
	}
	
	/* SAR_SET_CONFIG */
	if (!strcmp(*spp, "setconfig")) {
		
		if(argc < 11) goto USAGE_END;
		/* Query if this Channel is created */
		if(Parsing_Cfg(&cfg, spp)==FALSE)	goto ERROR_END;
		if(ioctl(skfd, SAR_GET_CONFIG, &ifr)<0) goto ERROR_END;
		if(cfg.created!=VC_CREATED){
			printf("Channel %d is not Created\n", cfg.ch_no);
			goto ERROR_END;
		}
		
		/* Set Configuration */
		Parsing_Cfg(&cfg, spp);	
		if(ioctl(skfd, SAR_SET_CONFIG, &ifr)<0)	goto ERROR_END;
		printf("channel %d vc modified\n", cfg.ch_no);
		
		if(ioctl(skfd, SAR_GET_CONFIG, &ifr)<0) goto ERROR_END;
		DumpCfg(cfg);
		
		goto END;
	}
	
	/* ATM_OAM_SET_ID */
	if (!strcmp(*spp, "oamsetid")) {
		if(argc < 4) goto USAGE_END;
		memset(&lbId, 0, sizeof(ATMOAMLBID));
		if(Parsing_OAM_id(&lbId, spp)==FALSE)	goto END;
		mysio.arg = (void *)&lbId;
		if(ioctl(skfd, ATM_OAM_SET_ID, &mysio)<0) goto ERROR_END;
		goto END;
	}
	
	/* ATM_OAM_START */
	if (!strcmp(*spp, "oamstart")) {
		if(argc < 4) goto USAGE_END;
		memset(&lbReq, 0, sizeof(ATMOAMLBReq));
		if(Parsing_OAM_req(&lbReq, spp)==FALSE)	goto END;
		mysio.arg = (void *)&lbReq;
		if(ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) goto ERROR_END;
		goto END;
	}
	
	/* SAR_ATM_OAM_RPT_LB */
	if (!strcmp(*spp, "oamtest")) {
		if(argc < 4) goto USAGE_END;
		memset(&lbReq, 0, sizeof(ATMOAMLBReq));
		if(Parsing_OAM_req(&lbReq, spp)==FALSE)	goto END;
		mysio.arg = (void *)&lbReq;
		if(ioctl(skfd, SAR_ATM_OAM_RPT_LB, &mysio)<0) goto ERROR_END;
		goto END;
	}
	
	/* ATM_OAM_LB_AGAIN */
//	if (!strcmp(*spp, "oamtxagain")) {
//		if(ioctl(skfd, SAR_ATM_OAM_LB_AGAIN, &ifr)<0)	goto ERROR_END;
//		goto END;
//	}
	
	/* ATM_OAM_LB_STATUS */
	if (!strcmp(*spp, "oamstatus")) {
		if(argc < 4) goto USAGE_END;
		memset(&lbState, 0, sizeof(ATMOAMLBState));
		if(Parsing_OAM_status(&lbState, spp)==FALSE)	goto END;
		mysio.arg = (void *)&lbState;
		if(ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0)	goto ERROR_END;
		DumpOamState(&lbState);
		goto END;
	}
	
	/* ATM_OAM_STATUS_FE */
	if (!strcmp(*spp, "oamstatusfe")) {
		if(argc < 3) goto USAGE_END;
		memset(&lbStatefe, 0, sizeof(ATMOAMLBRXState));
		if(Parsing_OAM_statusfe(&lbStatefe, spp)==FALSE)	goto END;
		mysio.arg = (void *)&lbStatefe;
		if(ioctl(skfd, ATM_OAM_STATUS_FE, &mysio)<0)	goto ERROR_END;
		DumpOamStatefe(&lbStatefe);
		goto END;
	}
	
	/* SAR_ATM_OAM_STOP */
	if (!strcmp(*spp, "oamstop")) {
		if(argc < 4) goto USAGE_END;
		memset(&lbReq, 0, sizeof(ATMOAMLBReq));
		if(Parsing_OAM_req(&lbReq, spp)==FALSE)	goto END;
		mysio.arg = (void *)&lbReq;
		if(ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) goto ERROR_END;
		goto END;
	}
	
	/* SAR_ATM_OAM_STOP_LB */
	if (!strcmp(*spp, "oamteststop")) {
		if(argc < 1) goto USAGE_END;
		memset(&lbReq, 0, sizeof(ATMOAMLBReq));
		if(Parsing_OAM_req(&lbReq, spp)==FALSE)	goto END;
		mysio.arg = (void *)&lbReq;
		if(ioctl(skfd, SAR_ATM_OAM_STOP_LB, &mysio)<0) goto ERROR_END;
		goto END;
	}
	
	/* SAR_CREATE_VC */
	if (!strcmp(*spp, "createvc")) {
		
		if(argc < 11) goto USAGE_END;
		
		/* Query if this Channel is created */
		if(Parsing_Cfg(&cfg, spp)==FALSE)       goto ERROR_END;
		if(ioctl(skfd, SAR_GET_CONFIG, &ifr)<0) goto ERROR_END;
		if(cfg.created==VC_CREATED){
			printf("Channel %d is already Created\nPlease use setconfig to change the configuration\n", cfg.ch_no);
			goto ERROR_END;
		}
								
		if(Parsing_Cfg(&cfg, spp) == FALSE)	goto USAGE_END;

		if(ioctl(skfd, SAR_CREATE_VC, &ifr)<0)	goto ERROR_END;
		printf("channel %d vc created\n", cfg.ch_no);
		goto END;
	}
	
	/* SAR_DELETE_VC */
	if (!strcmp(*spp, "deletevc")) {

		if(*(spp+1)!=NULL)
			cfg.ch_no = s2i(*(spp+1));
		else goto USAGE_END;

		/* Query if this Channel is created */
		if(ioctl(skfd, SAR_GET_CONFIG, &ifr)<0) goto ERROR_END;
		if(cfg.created!=VC_CREATED){
			printf("Channel %d is not Created\n", cfg.ch_no);
			goto ERROR_END;
		}
		
		if(ioctl(skfd, SAR_DELETE_VC, &ifr)<0)	goto ERROR_END;
		printf("channel %d vc deleted\n", cfg.ch_no);
		goto END;
	}

	/* SAR_DELETE_ALL_VC */
	if (!strcmp(*spp, "deleteallvc")) {
		
		for(i=0;i<16;i++){
			cfg.ch_no=i;
			/* Query if this Channel is created */
			if(ioctl(skfd, SAR_GET_CONFIG, &ifr)<0) goto ERROR_END;
			if(cfg.created==VC_CREATED)
				ioctl(skfd, SAR_DELETE_VC, &ifr);
		}
		goto END;
	}
	
	/* SAR_SETMAC */ /* for QoS Test Only */
	if (!strcmp(*spp, "setmac")){
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&cfg;		
		cfg.ch_no=0;
		*spp++;
		s2Mac(&mac_P, *spp);
		memcpy(&cfg.MAC, &mac_P, 6);
		//for(i=0;i<3;i++){
		//	tmp_mac = cfg.MAC[i];
		//	cfg.MAC[i] = cfg.MAC[5-i];
		//	cfg.MAC[5-i]=tmp_mac;
		//}
		if(ioctl(skfd, SAR_SETMAC, &mysio)<0) goto ERROR_END;
		printf("ch %d MAC = ", cfg.ch_no);
		for(i=0;i<6;i++)
			printf("%02X:", cfg.MAC[i]);
		

		cfg.ch_no=1;
		*spp++;
		s2Mac(&mac_P, *spp);
		memcpy(&cfg.MAC, &mac_P, 6);
		//for(i=0;i<3;i++){
		//	tmp_mac = cfg.MAC[i];
		//	cfg.MAC[i] = cfg.MAC[5-i];
		//	cfg.MAC[5-i]=tmp_mac;
		//}
		if(ioctl(skfd, SAR_SETMAC, &mysio)<0) goto ERROR_END;
		printf("\nch %d MAC = ", cfg.ch_no);
		for(i=0;i<6;i++)
			printf("%02X:", cfg.MAC[i]);
			
		printf("\n");
		
		goto END;
	}

	if(!strcmp(*spp, "utopia")){
		if(ioctl(skfd, SAR_ENABLE_UTOPIA, &ifr)<0) goto ERROR_END;
		else printf("UTOPIA Enabled\n");
		goto END;
	}
	
	if(!strcmp(*spp, "fast")){
		if(ioctl(skfd, SAR_UTOPIA_FAST, &ifr)<0) goto ERROR_END;
		else printf("UTOPIA FAST PATH Enabled\n");
		goto END;
	}	
	
	if(!strcmp(*spp, "slow")){
		if(ioctl(skfd, SAR_UTOPIA_SLOW, &ifr)<0) goto ERROR_END;
		else printf("UTOPIA INTERLEAVE PATH Enabled\n");
		goto END;
	}	

	

	if(!strcmp(*spp, "r")){
		unsigned int addr, *ptr;
		*spp++;
		addr=s2i(*spp);	// Address
		if (argc==2)	j=1;
		else {
			*spp++;
			j=s2i(*spp);	// Length
		}
		if (!(ptr=malloc(8+j))) {
			printf("memory read malloc failed!\n");
			goto ERROR_END;
		};
		ptr[0]=addr;
		ptr[1]=j;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)ptr;		
		if ((ioctl(skfd, SAR_READ_MEM, &mysio))<0)	goto ERROR_END;
		Dump((uint32)ptr,j);
		goto END;
	}	
	
	
	if(!strcmp(*spp, "w")){
		unsigned int ptr[2];
		*spp++;
		ptr[0]=s2i(*spp);	// Address
		*spp++;
		ptr[1]=s2i(*spp);	// Value
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)ptr;		
		if ((ioctl(skfd, SAR_WRITE_MEM, &mysio))<0)	goto ERROR_END;
		goto END;
	}		

	if(!strcmp(*spp, "enablelb")){
		if(ioctl(skfd, SAR_EnableLOOPBACK, &ifr)<0) goto ERROR_END;
		goto END;
	}	

	//ql-- limit upstream traffic
#ifdef UPSTREAM_TRAFFIC_CTL
	if (!strcmp(*spp, "tc")) {
		int uMaxTraffic;
		/*unsigned int ptr[6];
		*spp++;
		ptr[0]=s2i(*spp);
		*spp++;
		ptr[1]=s2i(*spp);
		*spp++;
		ptr[2]=s2i(*spp);
		*spp++;
		ptr[3]=s2i(*spp);
		*spp++;
		ptr[4]=s2i(*spp);
		*spp++;
		ptr[5]=s2i(*spp);*/
		if(argc!=2)	goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&uMaxTraffic;
		//mysio.arg = (void *)ptr;
		uMaxTraffic = s2i(*(spp+1));
		if ((ioctl(skfd, SAR_SET_TRAFFIC_CTL, &mysio))<0)	goto ERROR_END;
		goto END;
	}
#endif
	if(!strcmp(*spp, "disablelb")){
		if(ioctl(skfd, SAR_DisableLOOPBACK, &ifr)<0) goto ERROR_END;
		goto END;
	}	

	//12/30/05' hrchen, for PVC desc number setting
	/* SAR_SET_PVC_NUMBER */
	if (!strcmp(*spp, "pvcnumber")) {
		int ch_num;
		if(argc!=2)	goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&ch_num;		
		ch_num = s2i(*(spp+1));
		if ((ioctl(skfd, SAR_SET_PVC_NUMBER, &mysio))<0)	goto ERROR_END;
		goto END;
	}
		
//#ifdef AUTO_PVC_SEARCH
	if (!strcmp(*spp, "boa_pid")) {
		int ch_num;
		if(argc!=2)	goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&ch_num;		
		ch_num = s2i(*(spp+1));
		if ((ioctl(skfd, SENT_BOA_PID, &mysio))<0)	goto ERROR_END;
		goto END;
	}
//#endif		
	if (!strcmp(*spp, "tr069_pid")) {
		int ch_num;
		if(argc!=2)	goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)&ch_num;		
		ch_num = s2i(*(spp+1));
		if ((ioctl(skfd, SENT_TR069_PID, &mysio))<0)	goto ERROR_END;
		goto END;
	}
	if (!strcmp(*spp, "iptv_intf")) {
		char intf_str[10];
		if(argc!=2)	goto USAGE_END;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (char *)intf_str;		
		strncpy(mysio.arg, *(spp+1), sizeof(intf_str) );
		*((char *)(mysio.arg)+9) = '\0';		
		if ((ioctl(skfd, SENT_IPTV_INTF, &mysio))<0)	goto ERROR_END;
		goto END;
	}
	if(!strcmp(*spp, "ppp_term")){
		if(ioctl(skfd, SENT_TERM_REQ, &ifr)<0) goto ERROR_END;
		goto END;
	}	


	/*if(!strcmp(*spp, "loopback")){
		if (argc<2)	goto USAGE_END;
		*spp++;
		i=s2i(*spp);	// Address
		
		if(i==0){
			(*(volatile uint16 *)(0xb8000032))=0x0000;
			printf("UTOPIA loopback Disabled, loopback Register = %04X\n",(*(volatile uint16 *)(0xb8000032)));
		}else if(i==1){
			(*(volatile uint16 *)(0xb8000032))=0x0300;		
			printf("UTOPIA loopback Enable, loopback Register = %04X\n",(*(volatile uint16 *)(0xb8000032)));
		}else
			goto USAGE_END;
		
		goto END;
	}*/	
	
	if (!strcmp(*spp, "debug")) {
		int num;
		int flag;
		
		if (argc == 2) {
			if (!strcmp(*(spp+1), "status")) {
				if ((ioctl(skfd, SAR_DEBUG_STATUS, &mysio))<0)	goto ERROR_END;
				if (eocsd>=0)
					if ((ioctl(eocsd, DSL_DEBUG_STATUS, &num))<0)	goto ERROR_END;
			}
			goto END;
		}
		if(argc < 3) goto USAGE_END;
		if (!strcmp(*(spp+1), "pvc")) {
			struct SAR_IOCTL_DEBUG dbg;
			memset(&dbg, 0, sizeof(struct SAR_IOCTL_DEBUG));
			dbg.chno = 0xffffffff;
			num = atoi(spp[2]);
			if (num < 0 || num > 3)
				goto USAGE_END;
			dbg.enable = 0;
			if (num == 1) // both
				dbg.enable = 0x03;
			else if (num == 2) // input
				dbg.enable = 0x01;
			else if (num == 3) // output
				dbg.enable = 0x02;
			
			while ((flag = getopt(argc, argv, "c:e:p:m")) != EOF) {
				switch(flag) {
					case 'c': // channel
						if (!optarg)
							break;
						dbg.chno = strtol(optarg, 0, 0);
						break;
					case 'e':
						if (!optarg)
							break;
						dbg.etype = strtol(optarg, 0, 0);
						break;
					case 'p':
						if (!optarg)
							break;
						dbg.ipproto = strtol(optarg, 0, 0);
						break;
					case 'm':
						dbg.hr = 1;
						break;
					default:
						break;
				}
			}
			mysio.arg = (void *)&dbg;
			if ((ioctl(skfd, SAR_DEBUG_PVC_ENABLE, &mysio))<0)	goto ERROR_END;
		}
		else if (!strcmp(*(spp+1), "size")) {
			num = atoi(spp[2]);
			mysio.arg = (void *)&num;
			if ((ioctl(skfd, SAR_DEBUG_NUM2PRINT, &mysio))<0)	goto ERROR_END;
			if (eocsd>=0)
				if ((ioctl(eocsd, DSL_DEBUG_NUM2PRINT, &num))<0)	goto ERROR_END;
		}
		else if (!strcmp(*(spp+1), "obaal5")){
			if (spp[2][0]=='0')
				num = 0;
			else
				num = 1;
			mysio.arg = (void *)&num;
			if ((ioctl(skfd, SAR_DEBUG_OBAAL5_ENABLE, &mysio))<0)	goto ERROR_END;
		}
		else if (!strcmp(*(spp+1), "obcell")){
			if (spp[2][0]=='0')
				num = 0;
			else
				num = 1;
			mysio.arg = (void *)&num;
			if ((ioctl(skfd, SAR_DEBUG_OBCELL_ENABLE, &mysio))<0)	goto ERROR_END;
		}
		else if (!strcmp(*(spp+1), "eoc")) {
			if (eocsd<0) {
				printf("EOC not supported !!\n");
				goto END;
			}
			if (spp[2][0]=='0')
				num = 0;
			else
				num = 1;
			mysio.arg = (void *)&num;
			if ((ioctl(eocsd, DSL_DEBUG_EOC_ENABLE, &num))<0)	goto ERROR_END;
		}
		else if (!strcmp(*(spp+1), "eoc_lbrx")) {
			if (eocsd<0) {
				printf("EOC not supported !!\n");
				goto END;
			}
			if (spp[2][0]=='0')
				num = 0;
			else
				num = 1;
			mysio.arg = (void *)&num;
			if ((ioctl(eocsd, DSL_DEBUG_EOC_LBRX, &num))<0)	goto ERROR_END;
		}
		else if (!strcmp(*(spp+1), "eoc_lbtx")) {
			if (eocsd<0) {
				printf("EOC not supported !!\n");
				goto END;
			}
			if (spp[2][0]=='0')
				num = 0;
			else
				num = 1;
			mysio.arg = (void *)&num;
			if ((ioctl(eocsd, DSL_DEBUG_EOC_LBTX, &num))<0)	goto ERROR_END;
		}
		else goto USAGE_END;
		goto END;
	}

#ifdef CONFIG_USER_PORT_MIRROR
	if(!strcmp(*spp, "mirror"))
	{
		int mirror_para[3]; /* vpi, vci, flag */
		
		*spp++;
		if (4 == argc)
		{
			mirror_para[0] = s2i(*spp);
			*spp++;
			mirror_para[1] = s2i(*spp);
			*spp++;
		}
		else if (2 == argc)
		{
			mirror_para[0] = 0;
			mirror_para[1] = 0;
		}
		else
		{
			goto USAGE_END;
		}
		
		mirror_para[2] = s2i(*spp);
		
		mysio.arg = (void *)mirror_para;
		if ((ioctl(skfd, SAR_PORT_MIRROR, &mysio)) < 0)
		{
			goto ERROR_END;
		}
		
		goto END;
	}	
	if(!strcmp(*spp, "mirror_ob"))
	{
		int mirror_para[3]; /* vpi, vci, flag */
		
		*spp++;
		if (2 == argc)
		{
			// (vpi, vci)==(1, 0) for outband
			mirror_para[0] = 1;
			mirror_para[1] = 0;
		}
		else
		{
			goto USAGE_END;
		}
		
		mirror_para[2] = s2i(*spp);
		
		mysio.arg = (void *)mirror_para;
		if ((ioctl(skfd, SAR_PORT_MIRROR, &mysio)) < 0)
		{
			goto ERROR_END;
		}
		
		goto END;
	}	
#endif
	/* SAR_UPGRADE_FW */
	if (!strcmp(*spp, "upgrade_fw")) {
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		if ((ioctl(skfd, SAR_UPGRADE_FW, &mysio))<0)	goto ERROR_END;
		goto END;
	}
	/* SAR_RESET_STATS */
	if (!strcmp(*spp, "resetstats")) {
		if(*(spp+1)!=NULL)
			j = s2i(*(spp+1));
		else // all channels
			j = -1;
		
		mysio.arg = (void *)&j;
		if ((ioctl(skfd, SAR_RESET_STATS, &mysio))<0)	goto ERROR_END;
		goto END;
	}

USAGE_END:
	usage();

END:
	if(skfd>0)
		close(skfd);
	if(eocsd>0)
		close(eocsd);
	exit(0);

ERROR_END:
	//printf("ioctl error\n"); 
	exit(1);


}


int32 s2Mac(macaddr_t *mac_P, uint8 *str_P)
{
    uint16  val;
    uint32  count;
    
    
    if ( str_P[0] == '0' && (str_P[1] == 'x' || str_P[1] == 'X') )
        str_P += 2;
	
    if ( *str_P == '\0' )
        return -1;
	
	count = 1;
    for (val = 0; count <= 12; str_P++)
    {
        val <<= 4;
        if ( '0' <= *str_P && *str_P <= '9' )
        {
            val += *str_P - '0';
        }
        else if ( 'a' <= *str_P && *str_P <= 'f' )
        {
            val +=  *str_P - 'a' + 10;
        }
        else if ( 'A' <= *str_P && *str_P <= 'F' )
        {
            val +=  *str_P - 'A' + 10;
        }
        else
            continue;
        
        if ( count % 4 == 0 )
        {
            if ( count / 4 == 1 )
            {
                mac_P->mac47_32 = val;
                val = 0;
            }
            else if ( count / 4 == 2 )
            {
                mac_P->mac31_16 = val;
                val = 0;
            }
        }
        
        count++;
    }
    
    mac_P->mac15_0 = val;
    
    return 0;
}


int s2i(char  *str_P)
{
    uint32  val;
    int minus;
    
    if ( (str_P[0] == '0') && (str_P[1] == 'x') )
    {
        str_P += 2;
        for (val = 0; *str_P; str_P++)
        {
            val *= 16;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else if ( 'a' <= *str_P && *str_P <= 'f' )
                val += *str_P - 'a' + 10;
            else if ( 'A' <= *str_P && *str_P <= 'F' )
                val += *str_P - 'A' + 10;
            else
                break;
        }
    }
    else
    {
    	minus = 0;
    	if (str_P[0] == '-') {
    		minus = 1;
    		str_P++;
    	}
        for (val = 0; *str_P; str_P++)
        {
            val *= 10;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else
                break;
        }
        if (minus)
            val = -val;
    }
    
    return val;
}


void DumpStat(struct SAR_IOCTL_CFG cfg){

	printf("----RX----\n");
	printf("packet    : %d\n", cfg.stat.rx_pkt_cnt);
	printf("bytes     : %d\n", cfg.stat.rx_byte_cnt);
	printf("CRC error : %d\n", cfg.stat.rx_crc_error);

	printf("----TX----\n");
	printf("packet    : %d\n", cfg.stat.tx_pkt_ok_cnt);
	printf("bytes     : %d\n", cfg.stat.tx_byte_cnt);
	printf("dropped   : %d\n", cfg.stat.tx_pkt_fail_cnt);

	return;
	
}



void DumpStat_2(struct SAR_IOCTL_CFG cfg){


	printf("----RX----\n");
	printf("rcv_cnt \t: %d\n", cfg.stat.rcv_cnt);
	printf("rcv_ok \t\t: %d\n", cfg.stat.rcv_ok);
	printf("rx_desc_cnt \t: %d\n", cfg.stat.rx_desc_cnt);
	printf("rx_byte_cnt \t: %d\n", cfg.stat.rx_byte_cnt);
	printf("rx_pkt_cnt \t: %d\n", cfg.stat.rx_pkt_cnt);
	printf("rx_pkt_fail \t: %d\n", cfg.stat.rx_pkt_fail);
	printf("rx_buf_alloc \t: %d\n", cfg.stat.rx_buf_alloc);
	printf("rx_crc_error \t: %d\n", cfg.stat.rx_crc_error);
	printf("rx_lenb_error \t: %d\n", cfg.stat.rx_lenb_error);
	printf("rx_buf_lack \t: %d\n", cfg.stat.rx_buf_lack);
	printf("rx_rbf_count \t: %d\n", cfg.stat.rx_rbf_count);

	printf("----TX----\n");
	printf("send_cnt \t: %d\n", cfg.stat.send_cnt);
	printf("send_ok \t: %d\n", cfg.stat.send_ok);
	printf("send_fail \t: %d\n", cfg.stat.send_fail);
	printf("send_desc_full \t: %d\n", cfg.stat.send_desc_full);
	printf("tx_desc_ok_cnt \t: %d\n", cfg.stat.tx_desc_ok_cnt);
	printf("tx_buf_free \t: %d\n", cfg.stat.tx_buf_free);
	printf("tx_pkt_fail_cnt : %d\n", cfg.stat.tx_pkt_fail_cnt);
	printf("tx_byte_cnt \t: %d\n", cfg.stat.tx_byte_cnt);
	printf("send_desc_lack \t: %d\n", cfg.stat.send_desc_lack);
#ifdef STAI_DEBUG //for SAR software QoS starvation prevention
    printf("other_vc_starvation \t : %d\n", cfg.stat.tx_starvation);
    printf("force_drop \t : %d\n", cfg.stat.forceDrop);
#endif
	return;
	
}

void DumpOamState(ATMOAMLBState *state)
{
	printf("OAM loopback ping status:\n");
	printf("vpi       : %d\n", state->vpi);
	printf("vci       : %d\n", state->vci);
	printf("count     : %u %u %u %u %u %u\n", state->count[0],
		state->count[1], state->count[2], state->count[3],
		state->count[4], state->count[5]);
	printf("rtt       : %u %u %u %u %u %u\n", state->rtt[0],
		state->rtt[1], state->rtt[2], state->rtt[3],
		state->rtt[4], state->rtt[5]);
}

void DumpOamStatefe(ATMOAMLBRXState *statefe)
{
	int i, j;
	
	printf("OAM loopback ping status far-end:\n");
	printf("vpi       : %d\n", statefe->vpi);
	printf("vci       : %d\n", statefe->vci);
	for (i=0; i<6; i++)
	{
		printf("Location ID: ");
		for (j=0; j<16; j++)
			printf("%.02x", statefe->LocID[i][j]);
		printf("   count: %u", statefe->count[i]);
		printf("   status:%u\n", statefe->status[i]);
	}
}

void DumpCfg(struct SAR_IOCTL_CFG cfg){

	printf("Ch_no     : %d\n", cfg.ch_no);
	/* VC State*/
	if(cfg.created==VC_CREATED)
		printf("VC State  : Created\n");
	else if(cfg.created==VC_NOT_CREATED)
		printf("VC State  : Not Created\n");
	else
		printf("VC State  : Unknown\n");
	
	printf("vpi       : %d\n", cfg.vpi);
	printf("vci       : %d\n", cfg.vci);

	
	/* RFC Mode*/
	if(cfg.rfc==RFC1483_BRIDGED)
		printf("RFC Type  : 1483 Bridge\n");
	else if(cfg.rfc==RFC1483_ROUTED)
		printf("RFC Type  : 1483 Routed\n");
	else if(cfg.rfc==RFC1577)
		printf("RFC Type  : 1577\n");
	else if(cfg.rfc==RFC2364)
		printf("RFC Type  : 2364(PPPoA)\n");
	else if(cfg.rfc==RFC2516)
		printf("RFC Type  : 2516(PPPoE)\n");
	else
		printf("RFC Type  : Unknow\n");
	
	/* Framing Mode*/
	if(cfg.framing==LLC_SNAP)
		printf("framing   : LLC SNAP\n");
	else if(cfg.framing==VC_MUX)
		printf("framing   : VC Mux\n");
	else
		printf("framing   : Unknown\n");
		

	/* QoS Type */
	if(cfg.QoS.Type==QoS_UBR)
		printf("Qos Type  : UBR\n");
	else if(cfg.QoS.Type==QoS_CBR)
		printf("Qos Type  : CBR\n");
	else if(cfg.QoS.Type==QoS_nrtVBR)
		printf("Qos Type  : nrt-VBR\n");
	else if(cfg.QoS.Type==QoS_rtVBR)
		printf("Qos Type  : rt-VBR\n");
	else
		printf("Qos Type  : Unknown\n");

	printf("PCR       : %d\n", (uint16)cfg.QoS.PCR);
	printf("SCR       : %d\n", (uint16)cfg.QoS.SCR);
	printf("MBS       : %d\n", (uint16)cfg.QoS.MBS);
	//printf("MBS       : %d\n", (uint8)cfg.QoS.MBS);
	printf("CRD       : %d\n", (uint8)cfg.QoS.CRD);
	printf("CDVT      : %d\n", (uint32)cfg.QoS.CDVT);

	/* Loopback Mode */
	/*if(cfg.loopback==TRUE)
		printf("loopback  : enabled\n");
	else if(cfg.loopback==FALSE)
		printf("loopback  : disabled\n");
	else
		printf("loopback  : unknown\n");*/

	DumpStat(cfg);
	return;
	
}

void AccumCfg(struct SAR_IOCTL_CFG *total_cfg, struct SAR_IOCTL_CFG cfg){
	
	total_cfg->stat.rx_pkt_cnt 		+= cfg.stat.rx_pkt_cnt;
	total_cfg->stat.rx_byte_cnt		+= cfg.stat.rx_byte_cnt;
	total_cfg->stat.rx_crc_error	+= cfg.stat.rx_crc_error;
	
	total_cfg->stat.tx_pkt_ok_cnt	+= cfg.stat.tx_pkt_ok_cnt;
	total_cfg->stat.tx_byte_cnt		+= cfg.stat.tx_byte_cnt;
	total_cfg->stat.tx_pkt_fail_cnt	+= cfg.stat.tx_pkt_fail_cnt;

	return;
}

BOOL	Parsing_Cfg(struct SAR_IOCTL_CFG *cfg, char **spp){

	int	loopback;
	*spp++;	
	cfg->ch_no	= s2i(*(spp++));
	cfg->vpi 	= (uint8)s2i(*(spp++));
	cfg->vci 	= (uint16)s2i(*(spp++));
	
	/* RFC Mode*/
	if(!strcmp(*spp, "1483_br"))
		cfg->rfc = RFC1483_BRIDGED;
	else if(!strcmp(*spp, "1483_rt"))
		cfg->rfc = RFC1483_ROUTED;
	else if(!strcmp(*spp, "1577"))
		cfg->rfc = RFC1577;
	else if(!strcmp(*spp, "2364"))
		cfg->rfc = RFC2364;
	else if(!strcmp(*spp, "2516"))
		cfg->rfc = RFC2516;
	else {
		printf("RFC mode error, %s\n", *spp);
		goto ERROR;
	}
	spp++;
	
	/* Framing*/		
	if(!strcmp(*spp, "llc"))
		cfg->framing = LLC_SNAP;
	else if (!strcmp(*spp, "vcmux"))
		cfg->framing = VC_MUX;
	else {
		printf("Framing Error, %s\n", *spp);
		goto ERROR;
	}
	spp++;
	
	/* QoS Type */
	if(!strcmp(*spp, "ubr"))
		cfg->QoS.Type = QoS_UBR;
	else if(!strcmp(*spp, "cbr"))
		cfg->QoS.Type = QoS_CBR;
	else if(!strcmp(*spp, "nrt-vbr"))
		cfg->QoS.Type = QoS_nrtVBR;
	else if(!strcmp(*spp, "rt-vbr"))
		cfg->QoS.Type = QoS_rtVBR;
	else {
		printf("QoS type error, %s\n", *spp);
		goto ERROR;
	}
	spp++;
		
	cfg->QoS.PCR = (uint16)s2i(*(spp++));
	cfg->QoS.SCR = (uint16)s2i(*(spp++));
	cfg->QoS.MBS = (uint8)s2i(*(spp++));
	cfg->QoS.CDVT	= (uint32)s2i(*(spp++));
	loopback	= (uint32)s2i(*(spp++));

	/*if(loopback == 1)
		cfg->loopback = TRUE;
	else if (loopback == 0)
		cfg->loopback = FALSE;
	else goto ERROR;*/

	return TRUE;

ERROR:
	return FALSE;
}

// convert 32 hex-decimal chars into its 16 octet values
static int convert16(unsigned char *hstring, unsigned char *hvalue)
{
	int i, curidx, len;
	char *tmpStr;
	unsigned char *tmpValue;
	
	len = strlen(hstring);
	if (len > 32) return 0;
	curidx = 16;
	for (i=0; i<32; i+=2)
	{
		curidx--;
		tmpValue = (unsigned char *)&hvalue[curidx];
		if (len > 0)
		{
			len -= 2;
			if (len < 0)
				len = 0;
			tmpStr = hstring + len;
			*tmpValue = strtoul(tmpStr, 0, 16);
			*tmpStr='\0';
		}
		else
			*tmpValue = 0;
	}
	
	return 1;
}

BOOL	Parsing_OAM_id(ATMOAMLBID *myid, char **spp)
{

	int i;
	char ch;
	
	*spp++;	
	myid->vpi 	= (uint32)s2i(*(spp++));
	myid->vci 	= (uint32)s2i(*(spp++));
	
	if (strlen(*spp) != 32)
	{
		printf("Source ID error !\n");
		return FALSE;
	}
	
	convert16(*spp++, myid->LocID);
	
	return TRUE;
}

BOOL	Parsing_OAM_req(ATMOAMLBReq *req, char **spp)
{

	int i;
	int type; // 0: oamstart, 1: oamstop
	char ch;
	
	if (!strcmp(*spp, "oamstart"))
		type = 0;
	else if (!strcmp(*spp, "oamtest"))
		type = 0;
	else if (!strcmp(*spp, "oamstop"))
		type = 1;
	else if (!strcmp(*spp, "oamteststop"))
		type = 1;

	*spp++;	
	req->vpi 	= (uint32)s2i(*(spp++));
	req->vci 	= (uint32)s2i(*(spp++));
	
	if (type == 0) // oamstart, oamtest
	{
		req->Scope 	= (uint8)s2i(*(spp++));
		
		if ((*spp) == NULL)
		{
			for (i=0; i<16; i++)
				req->LocID[i] = 0xff;
		}
		else
		{
			if (strlen(*spp) != 32)
			{
				printf("locationID error !\n");
				return FALSE;
			}
			
			convert16(*spp++, req->LocID);
			
			if ((*spp) == NULL)  //source ID
			{
				for (i=0; i<16; i++)
					req->SrcID[i] = 0; // not set into driver
			}
			else
			{
				if (strlen(*spp) != 32)
				{
					printf("locationID error !\n");
					return FALSE;
				}
				
				convert16(*spp++, req->SrcID);
			}
		}
	}
	else if (type == 1) // oamstop
		req->Tag = (uint32)s2i(*(spp++));
	
	return TRUE;
}

BOOL	Parsing_OAM_status(ATMOAMLBState *state, char **spp)
{

	*spp++;	
	state->vpi = (uint32)s2i(*(spp++));
	state->vci = (uint32)s2i(*(spp++));
	state->Tag = (uint32)s2i(*(spp++));
	
	return TRUE;	
}

BOOL	Parsing_OAM_statusfe(ATMOAMLBRXState *statefe, char **spp)
{

	*spp++;	
	statefe->vpi = (uint32)s2i(*(spp++));
	statefe->vci = (uint32)s2i(*(spp++));
	
	return TRUE;	
}

void Dump(uint32 Buffer, int32 size){

	int	k;	
	if(size%4)	size=size+4-(size%4);

	//Buffer=Buffer&0xFFFFFFFC;
	
	if ((Buffer&0xF0000000)==0x80000000) Buffer |= 0xA0000000;
	printf("Address  : Data");
	for(k=0;k<(size/4);k++){
		if ((k%4)==0) {
			printf ("\n");
			printf("%08X : ",((int *)Buffer)[0]+k*4);
		}		
		printf("%08X  ", (*(volatile uint32 *)((uint32)((Buffer+8)+k*4))));
	}
	printf("\n");


}
