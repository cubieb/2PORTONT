
#include "midware_intf.h"

static int firmwareRegistereOK = 0;
static int midware_debug = 0;

/*receive a packet*/
/*return -1: error; >0 : receive packet len*/
static int recvMidwarePkt(int conn_fd,char *recvBuf)
{
	int ret;
	int number,i;
	MW_PKT_HDR * pPktHdr;
	MW_DATA_HDR * pDataHdr;
	int bufOffset = 0;
	int recvLen=sizeof(MW_PKT_HDR);

recvPktHdr:
	/* receive packet header*/
	ret = recv(conn_fd,(void *)(recvBuf+bufOffset),recvLen,MSG_WAITALL);
	if(ret < 0 && errno != EINTR){
		return -1;
	}else if(ret != recvLen){
		printf("recv errno: EINTR.\n");
		goto recvPktHdr;
	}

	pPktHdr = (MW_PKT_HDR *)(recvBuf+bufOffset);
	number = pPktHdr->num;
	bufOffset += recvLen;
	recvLen = sizeof(MW_DATA_HDR);

	/*receive data*/
	for(i=0;i<number;i++){
recvDataHdr:
		if((bufOffset + recvLen) > MAX_MIDWARE_PKTLEN)
			return -1;
		ret = recv(conn_fd,(void *)(recvBuf+bufOffset),recvLen,MSG_WAITALL);
		if(ret < 0 && errno != EINTR){
			return -1;
		}else if(ret != recvLen){
			printf("recv errno: EINTR.\n");
			goto recvDataHdr;
		}

		pDataHdr = (MW_DATA_HDR *)(recvBuf+bufOffset);
		bufOffset += recvLen;
		recvLen = R_CHAR2WORD(pDataHdr->length);
recvData:
		if((bufOffset + recvLen) > MAX_MIDWARE_PKTLEN)
			return -1;
		ret = recv(conn_fd,(void *)(recvBuf+bufOffset),recvLen,MSG_WAITALL);
		if(ret < 0 && errno != EINTR){
			return -1;
		}else if(ret != recvLen){
			printf("recv errno: EINTR.\n");
			goto recvData;
		}

		bufOffset += recvLen;
		recvLen = sizeof(MW_DATA_HDR);
	}
	return(bufOffset);
}

static int getSendPktLen(char * sendBuf)
{
	int pktLen = 0;
	int number,i;
	MW_PKT_HDR * pPktHdr;
	MW_DATA_HDR * pDataHdr;

	pPktHdr = (MW_PKT_HDR *)sendBuf;
	number = pPktHdr->num;
	pktLen += sizeof(MW_PKT_HDR);

	for(i=0;i<number;i++)
	{
		pDataHdr = (MW_DATA_HDR *)(sendBuf + pktLen);
		pktLen += (sizeof(MW_DATA_HDR) +  R_CHAR2WORD(pDataHdr->length));
	}
		
	return pktLen;
}
static int sendPkt2Midware(int conn_fd,char * sendBuf)
{
	int ret;
	int sendLen;

	sendLen = getSendPktLen(sendBuf);
sendagain:
	ret = send(conn_fd,sendBuf,sendLen,0);
	if((ret < 0) && (errno != EINTR))
	{
		perror("send packet to midware error!\n");
		return -1;
	}else if((ret<0) && (errno == EINTR)){
		goto sendagain;
	}
		
	return 0;
}

static int sendMsg2Cwmp(struct mwMsg *sendMsg)
{
	int msgid;
	FILE * spidfile;
	int spid;

	/*get cwmpclient pid*/
	if(spidfile = fopen(CWMP_MIDPROC_RUNFILE, "r")) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		printf("Midware_intf: cwmpclient pidfile not exists!\n");
		return MSG_SEND_ERR;
	}
	sendMsg->msg_type = spid;
	sendMsg->msg_datatype = MSG_MIDWARE;

	msgid = msgget((key_t)1357,  0666);
	if(msgid > 0){
		if(msgsnd(msgid, (void *)sendMsg, MW_MSG_SIZE, 0) < 0){
			printf("Midware_intf: send message to cwmpclient error!\n");
			return MSG_SEND_ERR;
		}
		return MSG_SEND_OK;
	}else{
		printf("Midware_intf: get cwmp msgqueue error!\n");
		return MSG_SEND_ERR;
	}
}

#if 0
static int recvMsgFromCwmp(int cpid,struct mwMsg * recvMsg)
{
	int msgid;
	
	msgid = msgget((key_t)1357,  0666);
	if(msgid > 0){
		recvMsg->msg_datatype = PACKET_ERR;
		msgrcv(msgid, (void *)recvMsg,MW_MSG_SIZE, cpid, 0);
		return recvMsg->msg_datatype;
	}else{
		printf("get cwmp msgqueue error!\n");
		return MSG_RECV_ERR;
	}
}
#endif

static void chld_clr_pid()
{
	FILE *f;
	char *pidfile = MW_INTF_RUNFILE;

	if((f = fopen(pidfile, "r")) != NULL){
		unlink(MW_INTF_RUNFILE);
		fclose(f);
	}
}

static void chld_handle_term()
{
	chld_clr_pid();
	exit(0);
}

static int chld_log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = MW_INTF_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return 0;
	fprintf(f, "%d\n", pid);
	fclose(f);
	return pid;
}

static int printPacket(char * recvBuf)
{
	uint8 opcode,number;
	MW_PKT_HDR * pPktHdr;
	MW_DATA_HDR * pDataHdr;
	int i,offset=0;
	char tmpBuf[2048];

	pPktHdr = (MW_PKT_HDR *)recvBuf;
	opcode = pPktHdr->opcode;
	number = pPktHdr->num;
	offset += sizeof(MW_PKT_HDR);

	switch(opcode)
	{
		case OP_Register:
			printf("OP_Register\n");
			break;
		case OP_ParameterSetRet:
			printf("OP_ParameterSetRet\n");
			break;
		case OP_ParameterGetRet:
			printf("OP_ParameterGetRet\n");
			break;
		case OP_ParaInformMW:
			printf("OP_ParaInformMW\n");
			break;
		case OP_ParaInformITMS:
			printf("OP_ParaInformITMS\n");
			break;
		case OP_ParaAttributeSetRet:
			printf("OP_ParaAttributeSetRet\n");
			break;
		case OP_ParaAttributeGetRet:
			printf("OP_ParaAttributeGetRet\n");
			break;
		case OP_DownloadRet:
			printf("OP_DownloadRet\n");
			break;
		case OP_UploadRet:
			printf("OP_UploadRet\n");
			break;
		case OP_OperationDone:
			printf("OP_OperationDone\n");
			break;
		case OP_AddObjectRet:
			printf("OP_AddObjectRet\n");
			break;
		case OP_DeleteObjectRet:
			printf("OP_DeleteObjectRet\n");
			break;
		case OP_GetParanamesRet:
			printf("OP_GetParanamesRet\n");
			break;
		case OP_SetDefaultRet:
			printf("OP_SetDefaultRet\n");
			break;
		case OP_MWExit:
			printf("OP_MWExit\n");
			break;
		case OP_RegisterOK:
			printf("OP_RegisterOK\n");
			break;
		case OP_ParameterSet:
			printf("OP_ParameterSet\n");
			break;
		case OP_ParameterGet:
			printf("OP_ParameterGet\n");
			break;
		case OP_ParaInformMWRet:
			printf("OP_ParaInformMWRet\n");
			break;
		case OP_ParaInformITMSRet:
			printf("OP_ParaInformITMSRet\n");
			break;
		case OP_ParaAttributeSet:
			printf("OP_ParaAttributeSet\n");
			break;
		case OP_ParaAttributeGet:
			printf("OP_ParaAttributeGet\n");
			break;
		case OP_Download:
			printf("OP_Download\n");
			break;
		case OP_Upload:
			printf("OP_Upload\n");
			break;
		case OP_Reboot:
			printf("OP_Reboot\n");
			break;
		case OP_AddObject:
			printf("OP_AddObject\n");
			break;
		case OP_DeleteObject:
			printf("OP_DeleteObject\n");
			break;
		case OP_GetParanames:
			printf("OP_GetParanames\n");
			break;
		case OP_SetDefault:
			printf("OP_SetDefault\n");
			break;
		case OP_MWExitRet:
			printf("OP_MWExitRet\n");
			break;
		default:
			printf("unkown opcode\n");
			return -1;
	}

	
	for(i=0;i<number;i++)
	{
		pDataHdr = (MW_DATA_HDR *)(recvBuf + offset);
		memcpy(tmpBuf,(recvBuf+offset+sizeof(MW_DATA_HDR)),R_CHAR2WORD(pDataHdr->length));
		tmpBuf[(R_CHAR2WORD(pDataHdr->length))] = 0;
		
		switch(pDataHdr->type)
		{
		case TYPE_ModName:
			printf("TYPE_ModName:%s\n",tmpBuf);
			break;
		case TYPE_ParameterNames:
			printf("TYPE_ParameterNames:%s\n",tmpBuf);
			break;
		case TYPE_ParaValues:
			printf("TYPE_ParaValues:%s\n",tmpBuf);
			break;
		case TYPE_Retcode:
			printf("TYPE_Retcode:%s\n",tmpBuf);
			break;
		case TYPE_ParaAttributes:
			printf("TYPE_ParaAttributes:%s\n",tmpBuf);
			break;
		case TYPE_TWM:
			printf("TYPE_TWM:%s\n",tmpBuf);
			break;
		case TYPE_Object:
			printf("TYPE_Object:%s\n",tmpBuf);
			break;
		case TYPE_Instance:
			printf("TYPE_Instance:%s\n",tmpBuf);
			break;
		case TYPE_ParaList:
			printf("TYPE_ParaList:%s\n",tmpBuf);
			break;
		case TYPE_Operation:
			printf("TYPE_Operation:%s\n",tmpBuf);
			break;
		default:
			printf("unkown type\n");
			break;
		}
		offset += (sizeof(MW_DATA_HDR) + R_CHAR2WORD(pDataHdr->length));
	}

	return 0;
}

void recvMsgAndSendPkt2Midware(int conn_fd)
{
	int cpid;
	int msgid;
	struct mwMsg recvMsg;
	char * sendBuf = recvMsg.msg_data;

	/* log this process pid */
	if((cpid = chld_log_pid()) == 0){
		printf("log midware interface pid error!");
		exit(1);
	}

	signal(SIGTERM,chld_handle_term);
		
	while(1)
	{
		sleep(5);
		msgid = msgget((key_t)1357,  0666);
		if(msgid <= 0){
			perror("get midprocess msgq error!\n");
		}else{
			break;
		}
	}
	
	while(1)
	{
		//ret = recvMsgFromCwmp(cpid,&recvMsg);
		recvMsg.msg_type = cpid;
		recvMsg.msg_datatype = PACKET_ERR;
		msgrcv(msgid, (void *)&recvMsg,MW_MSG_SIZE, cpid, 0);

		if(midware_debug == 1){
			printf("***send a packet to midware:\n");
			printPacket(recvMsg.msg_data);
		}

		if(recvMsg.msg_datatype == PACKET_OK){
			sendPkt2Midware(conn_fd,sendBuf);
		}else{
			perror("receive message from cwmp error!\n");
		}
	}

}

static void registerFirmware()
{
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	if(!firmwareRegistereOK){
		*sendBuf = OP_RegisterFirmware;
		sendMsg2Cwmp(&sendMsg);	/*ask tr069 to register firmware*/
		signal(SIGALRM,registerFirmware);
		alarm(20);	/*reset timer*/
	}
}

static void actRegisterOK(char * recvBuf)
{
	MW_DATA_HDR * pDataHdr;
	char * pRetcode;
	int retLen;
	int retValue;

	pDataHdr = (MW_DATA_HDR *)(recvBuf+sizeof(MW_PKT_HDR));
	if(pDataHdr->type == TYPE_Retcode){
		pRetcode = (recvBuf+sizeof(MW_PKT_HDR)+sizeof(MW_DATA_HDR));
		retLen = R_CHAR2WORD(pDataHdr->length);
		*(pRetcode+retLen) = '\0';
		retValue = strtol(pRetcode,NULL,10);
		if(retValue == RET_SUCCESS){
			firmwareRegistereOK = 1;
		}
	}
}

static int read_pid(char *filename)
{
	int fh;
	FILE *in;
	int pid;

	fh = open(filename, O_RDWR);
	if ( fh == -1 ) return -1;
	if ((in = fdopen(fh, "r")) == NULL) return -1;
	fscanf(in, "%d", &pid);
	fclose(in);
	close(fh);

	return pid;
}

static void clr_pid()
{
	FILE *f;
	char *pidfile = MWINTF_MAIN_RUNFILE;

	if((f = fopen(pidfile, "r")) != NULL){
		unlink(MWINTF_MAIN_RUNFILE);
		fclose(f);
	}
}

static void close_chld()
{
	int pid=0;
	
	pid = read_pid((char*)MW_INTF_RUNFILE);
	if(pid > 0)
		kill(pid, SIGTERM);
}

static void handle_term()
{
	close_chld();
	clr_pid();
	exit(0);
}

static int log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = MWINTF_MAIN_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return 0;
	fprintf(f, "%d\n", pid);
	fclose(f);
	return pid;
}

/* exit itself and its' child process now */
static void actExitRet()
{
	printf("midware_intf process exit OK!\n");
	firmwareRegistereOK = 0;
	handle_term();
}

static int processMidwarePkt(int conn_fd,char * recvBuf)
{
	uint8 opcode;
	MW_PKT_HDR * pPktHdr;

	pPktHdr = (MW_PKT_HDR *)recvBuf;
	opcode = pPktHdr->opcode;
	switch(opcode)
	{
		case OP_RegisterOK:
			actRegisterOK(recvBuf);
			return SENDPKT2CWMP;
			//return PROCESS_OK;
		case OP_MWExitRet://receive exitRet from ctadmin and exit itself
			actExitRet();
			return PROCESS_OK;
		case OP_ParaInformMWRet:
		case OP_ParaInformITMSRet:
		case OP_ParameterSet:
		case OP_ParameterGet:
		case OP_ParaAttributeSet:
		case OP_ParaAttributeGet:
		case OP_Download:
		case OP_Reboot:
		case OP_Upload:
		case OP_AddObject:
		case OP_DeleteObject:
		case OP_GetParanames:
		case OP_SetDefault:
			return SENDPKT2CWMP;
		default:
			return PROCESS_ERROR;
			//return SENDPKT2CWMP;
	}
}

int main(int argc, char **argv)
{
	unsigned char vChar;
	int conn_fd;
	int ret;
	int addr_len;
	int chld_pid;
	struct sockaddr_un mw_addr;
	struct mwMsg sendMsg;
	char * recvBuf = sendMsg.msg_data;


	signal(SIGTERM,handle_term);

	log_pid();
	
	/*set debug flag*/
	mib_get(MIB_MIDWARE_FLAG, (void *)&vChar);
	if(vChar & MIDWARE_DEBUG_FLAG)
		midware_debug = 1;

	/*creat socket*/
	conn_fd = socket(PF_UNIX,SOCK_STREAM,0);
	if(conn_fd < 0){
		perror("get midware socket error!\n");
		exit(1);
	}

	/*connect midware*/
	memset(&mw_addr,0,sizeof(mw_addr));
	mw_addr.sun_family = PF_UNIX;
	strcpy(mw_addr.sun_path,MW_DOMAIN_FILE)	;
	addr_len = strlen(mw_addr.sun_path)+sizeof(mw_addr.sun_family);
	while(1){
		if(connect(conn_fd,(struct sockaddr *)&mw_addr,addr_len) != -1)
			break;
		sleep(10);
	}
	printf("socket connect ctadmin successfully\n");

	/*create msgRecv and pktSend child process*/
	if((chld_pid=fork()) == 0){
		recvMsgAndSendPkt2Midware(conn_fd);//receive message from cwmp
		exit(0);
	}

	signal(SIGALRM,registerFirmware);
	alarm(10);	
	sleep(10); /*wait register frimware*/

	/*receive packet from midware and sent to cwmp for processing*/
	while(1)
	{
		ret = recvMidwarePkt(conn_fd,recvBuf);
		if(ret <= 0){
			perror("receive midware packet error!\n");
			continue;
		}else if(ret > 0){
			if(midware_debug == 1){
				printf("***receive a packet from midware:\n");
				printPacket(recvBuf);
			}
			ret = processMidwarePkt(conn_fd,recvBuf);
			switch(ret){
			case SENDPKT2CWMP:
				sendMsg2Cwmp(&sendMsg);
				break;
			case PROCESS_OK:
				break;
			case PROCESS_ERROR:
				perror("receive a unknown packet!\n");
				break;
			default:
				break;
			}
		}
	}

	exit(0);
}


