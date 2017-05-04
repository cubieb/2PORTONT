/*
 *      Web server handler routines for TCP/IP stuffs
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "fmdefs.h"
#include "utility.h"
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif

#define	CONFIG_DIR	"/var/config"
#define CA_FNAME	CONFIG_DIR"/cacert.pem"
#define CERT_FNAME	CONFIG_DIR"/client.pem"

#define RECONNECT_MSG(url) { \
	boaHeader(wp); \
	boaWrite(wp, "<head><META http-equiv=content-type content=\"text/html; charset=gbk\"></head>");\
	boaWrite(wp, "<body><blockquote><h4>设定成功! " \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", url);\
	boaFooter(wp); \
	boaDone(wp, 200); \
}


#define UPLOAD_MSG(url) { \
	boaHeader(wp); \
	boaWrite(wp, "<head><META http-equiv=content-type content=\"text/html; charset=gbk\"></head>");\
	boaWrite(wp, "<body><blockquote><h4>上传成功! " \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", url);\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

#define DEL_MSG(url) { \
	boaHeader(wp); \
	boaWrite(wp, "<head><META http-equiv=content-type content=\"text/html; charset=gbk\"></head>");\
	boaWrite(wp, "<body><blockquote><h4>删除成功! " \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", url);\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

//copy from fmmgmt.c
//find the start and end of the upload file.
FILE * uploadGetCert(request *wp, unsigned int *startPos, unsigned int *endPos)
{
	FILE *fp=NULL;
	struct stat statbuf;
	unsigned char c, *buf;
	char boundary[80];


	if (wp->method == M_POST)
	{
		int i;

		fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);

		printf("file size=%d\n",statbuf.st_size);
		fp=fopen(wp->post_file_name,"rb");
		if(fp==NULL) goto error;

		memset( boundary, 0, sizeof( boundary ) );
		if( fgets( boundary,80,fp )==NULL ) goto error;
		if( boundary[0]!='-' || boundary[1]!='-') goto error;

		i= strlen( boundary ) - 1;
		while( boundary[i]=='\r' || boundary[i]=='\n' )
		{
			boundary[i]='\0';
			i--;
		}
		printf( "boundary=%s\n", boundary );
	}
	else goto error;


   	//printf("_uploadGet\n");
   	do
   	{
		if(feof(fp))
		{
			printf("Cannot find start of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		break;
	}while(1);
	(*startPos)=ftell(fp);

   	if(fseek(fp,statbuf.st_size-0x200,SEEK_SET)<0)
      		goto error;

	do
	{
		if(feof(fp))
		{
			printf("Cannot find end of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;

		{
			int i, blen;

			blen= strlen( boundary );
			for( i=0; i<blen; i++)
			{
				c= fgetc(fp);
				//printf("%c(%u)", c, c);
				if (c!=boundary[i])
				{
					ungetc( c, fp );
					break;
				}
			}
			//printf("\r\n");
			if( i!=blen ) continue;
		}

		break;
	}while(1);
	(*endPos)=ftell(fp)-strlen(boundary)-2;

   	return fp;
error:
   	return NULL;
}

///////////////////////////////////////////////////////////////////
void formTR069Config(request *wp, char *path, char *query)
{
	char *strData;
	char tmpStr[256 + 1];
	char tmpBuf[100];
	unsigned char vChar;
	unsigned char cwmp_flag = 1;
	unsigned char informEnble;
	unsigned int informInterval;
	unsigned char tr069URLChangeFlag = 0;
#ifdef CONFIG_MIDDLEWARE
	unsigned char tr069Enable;
	unsigned char lastValue,validValue;
	char *midwareAddr;
	unsigned int midwarePort,vInt;
#endif	//end of CONFIG_MIDDLEWARE
	// Mason Yu
	char changeflag = 1;
	char waitMwExit = 0;
	char hotSetMidware=0;

	strData = boaGetVar(wp,"applyTr069Config","");
	if (strData[0]){

		unsigned char configurable = 0;

		mib_get(CWMP_CONFIGURABLE, &configurable);
		if(configurable == 0)
			return 0;

		strData = boaGetVar(wp,"inform","");
		if (strData[0]) {
			informEnble = (strData[0] == '0') ? 0 : 1;

			if (!mib_set(CWMP_INFORM_ENABLE, &informEnble)) {
				strcpy(tmpBuf,strSetInformEnableerror);
				goto setErr_tr069;
			}
		}

		if (informEnble) {
			strData = boaGetVar(wp,"informInterval","");
			if (strData[0]) {
				informInterval = strtoul(strData, NULL, 0);

				if (!mib_set(CWMP_INFORM_INTERVAL, &informInterval)) {
					strcpy(tmpBuf,strSetInformIntererror);
					goto setErr_tr069;
				}
			}
		}

		strData = boaGetVar(wp,"acsURL","");
		if (strData[0]) {
			if (strlen(strData) == 0) {
				strcpy(tmpBuf,strACSURLWrong);
				goto setErr_tr069;
			}
	#ifndef _CWMP_WITH_SSL_
			if (strstr(strData, "https://")) {
				strcpy(tmpBuf,strSSLWrong);
				goto setErr_tr069;
			}
	#endif
			mib_get(CWMP_ACS_URL, tmpStr);
			printf("acs=%s | %s\n", tmpStr, strData);
			if (strcmp(tmpStr, strData))
			{
				mib_set(CWMP_ACS_URL_OLD, (void *)tmpStr);
			}

/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
			storeOldACS();
/*star:20100305 END*/

			DelTR069WANInterface(Old_ACS_URL);  // Magician: Del pre-defined routing of TR-069 WAN interface, if ACS URL is changed.

			if (!mib_set(CWMP_ACS_URL, strData)) {
				tr069URLChangeFlag = 1;
				strcpy(tmpBuf,strSetACSURLerror);
				goto setErr_tr069;
			}
			cmd_set_dns_config(NULL);
			restart_dnsrelay();
		}

		strData = boaGetVar(wp,"acsUser","");
		//if (strData[0])
		{
			if (!mib_set(CWMP_ACS_USERNAME, strData)) {
				strcpy(tmpBuf,strSetUserNameerror);
				goto setErr_tr069;
			}
		}

		strData = boaGetVar(wp,"acsPwd","");
		//if (strData[0])
		{
			if (!mib_set(CWMP_ACS_PASSWORD, strData)) {
				strcpy(tmpBuf,strSetPasserror);
				goto setErr_tr069;
			}
		}

		strData = boaGetVar(wp,"connReqUser","");
		//if (strData[0])
		{
			if (!mib_set( CWMP_CONREQ_USERNAME, strData)) {
				strcpy(tmpBuf,strSetConReqUsererror);
				goto setErr_tr069;
			}
		}

		strData = boaGetVar(wp,"connReqPwd","");
		//if (strData[0])
		{
			if (!mib_set(CWMP_CONREQ_PASSWORD, (void *)strData)) {
				strcpy(tmpBuf,strSetConReqPasserror);
				goto setErr_tr069;
			}
		}

		strData = boaGetVar(wp,"certauth","");
		if (strData[0]) {
			if (mib_get(CWMP_FLAG, &cwmp_flag)) {
				if (strData[0] == '0')
					cwmp_flag &= ~CWMP_FLAG_CERT_AUTH;
				else
					cwmp_flag |= CWMP_FLAG_CERT_AUTH;

				if (!mib_set(CWMP_FLAG, &cwmp_flag)) {
					strcpy(tmpBuf,strSetCWMPFlagerror);
					goto setErr_tr069;
				}
			} else {
				strcpy(tmpBuf,strGetCWMPFlagerror);
				goto setErr_tr069;
			}
		}

#if 0  // Magician 20110512: Always use default value for connection request url and port.
	strData = boaGetVar(wp,"conreqpath","");
	//if ( strData[0] )
	{
		mib_get( CWMP_CONREQ_PATH, (void *)tmpStr);
		if (strcmp(tmpStr,strData)!=0){
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PATH, (void *)strData)) {
				strcpy(tmpBuf,"Set Connection Request Path error!");
				goto setErr_tr069;
			}
		}
	}

	strData = boaGetVar(wp,"conreqport","");
	if ( strData[0] ) {
		cur_port = atoi(strData);
		mib_get( CWMP_CONREQ_PORT, (void *)&vInt);
		if ( vInt != cur_port ) {
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PORT, (void *)&cur_port)) {
				strcpy(tmpBuf,"Set Connection Request Port error!");
				goto setErr_tr069;
			}
		}
	}
#endif
	}	//end if applyTr069Config

#ifdef CONFIG_MIDDLEWARE
	strData = boaGetVar(wp,"applyMidwareConfig","");
	if (strData[0]){
		strData = boaGetVar(wp,"ctMDW","");
		tr069Enable = strData[0]-'0';

		strData = boaGetVar(wp,"mwsURLIP","");
		midwareAddr = strData;

		strData = boaGetVar(wp,"mwsURLPort","");
		if (strData[0]) {
			midwarePort = atoi(strData);
		}
		mib_get( CWMP_TR069_ENABLE, (void*)&lastValue);
		mib_get( CWMP_TR069_ENABLE_OLD, (void*)&validValue);
		if(lastValue != tr069Enable){
			/* ensure switch true */
			if( ((validValue == 0)&&(tr069Enable == 2)) || ((validValue == 2)&&(tr069Enable == 0)) )
			{
				strcpy(tmpBuf, strSwitchTR069EnableError);
				goto setErr_tr069;
			}

			if((validValue == 2 && tr069Enable == 1) ||(validValue == 1 && tr069Enable == 2)){	/*need close ct_midware*/
				hotSetMidware = 1;
				if(tr069Enable == 1){
					waitMwExit = 1;
				}
			}else{
				changeflag = 0;		//need system reboot for takeaction
			}
			if ( !mib_set( CWMP_TR069_ENABLE, (void *)&tr069Enable)) {
				strcpy(tmpBuf, strSetTR069EnableError);
				goto setErr_tr069;
			}
		}

		mib_get( CWMP_MIDWARE_SERVER_ADDR, (void *)tmpStr);
		if(strcmp(tmpStr,midwareAddr)!=0){
			if ( !mib_set( CWMP_MIDWARE_SERVER_ADDR_OLD, (void *)tmpStr))
			{
				strcpy(tmpBuf, strSetMidwareAddrError);
				goto setErr_tr069;
			}

			if ( !mib_set( CWMP_MIDWARE_SERVER_ADDR, (void *)midwareAddr))
			{
				strcpy(tmpBuf, strSetMidwareAddrError);
				goto setErr_tr069;
			}
		}

		mib_get( CWMP_MIDWARE_SERVER_PORT, (void*)&vInt);
		if(vInt != midwarePort){
			if ( !mib_set( CWMP_MIDWARE_SERVER_PORT, (void *)&midwarePort)) {
				strcpy(tmpBuf, strSetMidwarePortError);
				goto setErr_tr069;
			}
		}
	} //end if applyMidwareConfig
#endif //CONFIG_MIDDLEWARE

#ifdef _CWMP_WITH_SSL_
end_tr069:
#endif
	// Mason Yu
#ifdef APPLY_CHANGE
#ifdef CONFIG_MIDDLEWARE
		if(changeflag == 1){
			int midintf_pid = 0;
			midintf_pid = read_pid((char*)MWINTF_MAIN_RUNFILE);
			if(midintf_pid > 0){	//midware is running
				sendMWExit2Midproc();
				waitMwExit = 1;
			}
		}
		/*waiting for ct_midware exit*/
		if(waitMwExit){
			int loopcount = MW_EXIT_WAIT_TIME;
			int midware_intf_pid = 0;
			while(loopcount){
				midware_intf_pid = read_pid((char*)MWINTF_MAIN_RUNFILE);
				if(midware_intf_pid > 0)
					loopcount--;
				else
					break;
				sleep(1);
			}
		}
#endif	//end of CONFIG_MIDDLEWARE

	if(changeflag == 1){
		if(cwmp_flag == 0) {  // disable TR069
			off_tr069();
		} else {
			off_tr069();
			sleep(3);

			if (-1 == startCWMP()) {
				strcpy(tmpBuf,"Start tr069 Fail *****");
				printf("Start tr069 Fail *****\n");
				goto setErr_tr069;
			}
		}
	}
#if 0
#ifdef CONFIG_MIDDLEWARE
	else{//only tr069Enable:0->1 or 1->0
		if(mwServerChangeFlag == 1){
			setMidwareRouteFW(1);
		}
	}
#endif	//end of CONFIG_MIDDLEWARE
#endif
#endif	//end of APPLY_CHANGE

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	SetTR069WANInterface();

	strData = boaGetVar(wp,"submit-url","");
	RECONNECT_MSG(strData);// display reconnect msg to remote

	return;

setErr_tr069:
	ERR_MSG(tmpBuf);
}

void formTR069CPECert(request *wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	FILE	*fp=NULL,*fp_input;
	unsigned char *buf;
	unsigned int startPos,endPos,nLen,nRead;
	if ((fp = uploadGetCert(wp, &startPos, &endPos)) == NULL)
	{
		strcpy(tmpBuf,strUploaderror);
 		goto setErr_tr069cpe;
 	}

	nLen = endPos - startPos;
	//printf("filesize is %d\n", nLen);
	buf = malloc(nLen+1);
	if (!buf)
	{
		strcpy(tmpBuf,strMallocFail);
 		goto setErr_tr069cpe;
 	}

	fseek(fp, startPos, SEEK_SET);
	nRead = fread((void *)buf, 1, nLen, fp);
	buf[nRead]=0;
	if (nRead != nLen)
 		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	//printf("write to %d bytes from %08x\n", nLen, buf);

	fp_input=fopen(CERT_FNAME,"w");
	if (!fp_input)
		printf("create %s file fail!\n", CERT_FNAME);
	fprintf(fp_input,buf);
	printf("create file %s\n", CERT_FNAME);
	free(buf);
	fclose(fp_input);

#ifdef CONFIG_USER_FLATFSD_XXX
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif

	off_tr069();

	if (startCWMP() == -1)
	{
		strcpy(tmpBuf,"Start tr069 Fail *****");
		printf("Start tr069 Fail *****\n");
		goto setErr_tr069cpe;
	}

	strData = boaGetVar(wp,"submit-url","/net_tr069.asp");
	UPLOAD_MSG(strData);// display reconnect msg to remote
	return;

setErr_tr069cpe:
	ERR_MSG(tmpBuf);
}

void formTR069CACert(request *wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	FILE	*fp=NULL,*fp_input;
	unsigned char *buf;
	unsigned int startPos,endPos,nLen,nRead;
	if ((fp = uploadGetCert(wp, &startPos, &endPos)) == NULL)
	{
		strcpy(tmpBuf,strUploaderror);
 		goto setErr_tr069ca;
 	}

	nLen = endPos - startPos;
	//printf("filesize is %d\n", nLen);
	buf = malloc(nLen+1);
	if (!buf)
	{
		strcpy(tmpBuf,strMallocFail);
 		goto setErr_tr069ca;
 	}

	fseek(fp, startPos, SEEK_SET);
	nRead = fread((void *)buf, 1, nLen, fp);
	buf[nRead]=0;
	if (nRead != nLen)
 		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	//printf("write to %d bytes from %08x\n", nLen, buf);

	fp_input=fopen(CA_FNAME,"w");
	if (!fp_input)
		printf("create %s file fail!\n", CA_FNAME );
	fprintf(fp_input,buf);
	printf("create file %s\n",CA_FNAME);
	free(buf);
	fclose(fp_input);

#ifdef CONFIG_USER_FLATFSD_XXX
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif

	off_tr069();

	if (startCWMP() == -1)
	{
		strcpy(tmpBuf,"Start tr069 Fail *****");
		printf("Start tr069 Fail *****\n");
		goto setErr_tr069ca;
	}

	strData = boaGetVar(wp,"submit-url","/net_certca.asp");
	UPLOAD_MSG(strData);// display reconnect msg to remote
	return;

setErr_tr069ca:
	ERR_MSG(tmpBuf);
}


void formTR069CACertDel(request *wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	FILE	*fp=NULL,*fp_input;
	unsigned char *buf;
	unsigned int startPos,endPos,nLen,nRead;

	unlink(CA_FNAME);

#ifdef CONFIG_USER_FLATFSD_XXX
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif

	off_tr069();

	if (startCWMP() == -1)
	{
		strcpy(tmpBuf,"Start tr069 Fail *****");
		printf("Start tr069 Fail *****\n");
		goto setErr_tr069ca;
	}

	strData = boaGetVar(wp,"submit-url","/net_certca.asp");
	DEL_MSG(strData);// display reconnect msg to remote
	return;

setErr_tr069ca:
	ERR_MSG(tmpBuf);
}

void formMidwareConfig(request *wp, char *path, char *query)
{
	//We have no midleware currently.
	_COND_REDIRECT;
}

/*******************************************************/
/*show extra fileds at net_tr069.asp*/
/*******************************************************/
#ifdef _CWMP_WITH_SSL_
int ShowACSCertCPE(request *wp)
{
	int nBytesSent=0;
	unsigned char vChar=0;
	int isEnable=0;

	if ( mib_get( CWMP_FLAG, (void *)&vChar) )
		if ( (vChar & CWMP_FLAG_CERT_AUTH)!=0 )
			isEnable=1;

	nBytesSent += boaWrite(wp,"  <tr>\n");
	nBytesSent += boaWrite(wp,"      <td width=\"30%%\"><font size=2><b>ACS Certificates CPE:</b></td>\n");
	nBytesSent += boaWrite(wp,"      <td width=\"70%%\"><font size=2>\n");
	nBytesSent += boaWrite(wp,"      <input type=\"radio\" name=certauth value=0 %s >No&nbsp;&nbsp;\n", isEnable==0?"checked":"" );
	nBytesSent += boaWrite(wp,"      <input type=\"radio\" name=certauth value=1 %s >Yes\n", isEnable==1?"checked":"" );
	nBytesSent += boaWrite(wp,"      </td>\n");
	nBytesSent += boaWrite(wp,"  </tr>\n");

//		"\n"), isEnable==0?"checked":"", isEnable==1?"checked":"" );

	return nBytesSent;
}

int ShowMNGCertTable(request *wp)
{
	int nBytesSent=0;
	char buffer[256]="";

	getMIB2Str(CWMP_CERT_PASSWORD,buffer);

	nBytesSent += boaWrite(wp, "\n"
		"<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"
		"  <tr><hr size=1 noshade align=top></tr>\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>Certificat Management:</b></td>\n"
		"      <td width=\"70%%\"><b></b></td>\n"
		"  </tr>\n"
		"\n");


	nBytesSent += boaWrite(wp, "\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>CPE Certificat Password:</b></td>\n"
		"      <td width=\"70%%\">\n"
		"		<form action=/boaform/admin/formTR069Config method=POST name=\"cpe_passwd\">\n"
		"		<input type=\"text\" name=\"certpw\" size=\"24\" maxlength=\"64\" value=\"%s\">\n"
		"		<input type=\"submit\" value=\"Apply\" name=\"CPE_Cert\">\n"
		"		<input type=\"reset\" value=\"Undo\" name=\"reset\">\n"
		"		<input type=\"hidden\" value=\"/net_tr069_sc.asp\" name=\"submit-url\">\n"
		"		</form>\n"
		"      </td>\n"
		"  </tr>\n"
		"\n", buffer);

	nBytesSent += boaWrite(wp, "\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>CPE Certificat:</b></td>\n"
		"      <td width=\"70%%\"><font size=2>\n"
		"           <form action=/boaform/admin/formTR069CPECert method=POST enctype=\"multipart/form-data\" name=\"cpe_cert\">\n"
		"           <input type=\"file\" name=\"binary\" size=24>&nbsp;&nbsp;\n"
		"           <input type=\"submit\" value=\"Upload\" name=\"load\">\n"
		"           </form>\n"
		"      </td>\n"
		"  </tr>\n"
		"\n");

	nBytesSent += boaWrite(wp, "\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>CA Certificat:</b></td>\n"
		"      <td width=\"70%%\"><font size=2>\n"
		"           <form action=/boaform/admin/formTR069CACert method=POST enctype=\"multipart/form-data\" name=\"ca_cert\">\n"
		"           <input type=\"file\" name=\"binary\" size=24>&nbsp;&nbsp;\n"
		"           <input type=\"submit\" value=\"Upload\" name=\"load\">\n"
		"           </form>\n"
		"      </td>\n"
		"  </tr>\n"
		"\n");

	nBytesSent += boaWrite(wp, "\n"
		"</table>\n"
		"\n");


	return nBytesSent;
}
#endif

#ifdef _INFORM_EXT_FOR_X_CT_
int ShowCTInformExt(request *wp)
{
	int nBytesSent=0;
	unsigned char vChar=0;
	int isEnable=0;

	if ( mib_get( CWMP_FLAG, (void *)&vChar) )
		if ( (vChar & CWMP_FLAG_CTINFORMEXT)!=0 )
			isEnable=1;

	nBytesSent += boaWrite(wp,"  <tr>\n");
	nBytesSent += boaWrite(wp,"      <td width=\"30%%\"><font size=2><b>CT Inform Extension:</b></td>\n");
	nBytesSent += boaWrite(wp,"      <td width=\"70%%\"><font size=2>\n");
	nBytesSent += boaWrite(wp,"      <input type=\"radio\" name=ctinformext value=0 %s >Disabled&nbsp;&nbsp;\n", isEnable==0?"checked":"" );
	nBytesSent += boaWrite(wp,"      <input type=\"radio\" name=ctinformext value=1 %s >Enabled\n", isEnable==1?"checked":"" );
	nBytesSent += boaWrite(wp,"      </td>\n");
	nBytesSent += boaWrite(wp,"  </tr>\n");

	return nBytesSent;
}
#endif

int TR069ConPageShow(int eid, request *wp, int argc, char **argv)
{
	int nBytesSent=0;
	char *name;

	if (boaArgs(argc, argv,"%s", &name) < 1) {
		boaError(wp, 400,strArgerror);
		return -1;
	}

#ifdef _CWMP_WITH_SSL_
	if ( !strcmp(name,"ShowACSCertCPE") )
		return ShowACSCertCPE( wp );
	else if ( !strcmp(name,"ShowMNGCertTable") )
		return ShowMNGCertTable( wp );
#endif
#ifdef _INFORM_EXT_FOR_X_CT_
	if ( !strcmp(name,"ShowCTInformExt") )
		return ShowCTInformExt( wp );
#endif

	if(!strcmp(name, "cwmp-configurable"))
	{
		unsigned char configurable = 0;

		mib_get(CWMP_CONFIGURABLE, &configurable);
		if(configurable)
			nBytesSent += boaWrite(wp,"1");
		else
			nBytesSent += boaWrite(wp,"0");
	}

	return nBytesSent;
}
