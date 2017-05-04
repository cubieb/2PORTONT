#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/config.h>

#include "fmdefs.h"
#include "mib.h"
#include "utility.h"


int initPageStorage(int eid, request * wp, int argc, char ** argv)
{
	//FTP服务器:
	unsigned char	ftpEnable = 0;
	int				errcode = 1, lineno = __LINE__;
	int cnt;
	MIB_CE_ACC_T accEntry={0};
	int acc_nums=mib_chain_total(MIB_ACC_TBL);

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	//mib_get(MIB_BFTPD_ENABLE, (void *) &ftpEnable);
	for(cnt=0; cnt < acc_nums; cnt++)
	{
		memset(&accEntry, 0, sizeof(accEntry));
		mib_chain_get(MIB_ACC_TBL, cnt, (void*)&accEntry);
		if(accEntry.ftp & 0x2){//lan ftp enabled
			ftpEnable = 1;
			break;
		}
	}

	/************Place your code here, do what you want to do! ************/

	_PUT_BOOL(ftpEnable);

check_err:
	_TRACE_LEAVEL;
	return 0;
}

//网页提交action函数:
void formStorage(request * wp, char *path, char *query)
{
	char *strData;
	char tmpBuf[256] = {0};

	//存放目录:
	char *			psaveDir = NULL;
	char				argPath[64];
	//用户名:
	char			user[32];
	char				arguser[64];
	//密码:
	char			passwd[32];
	char				argpasswd[64];
	//端口:
	unsigned short	port = 0;
	char portstr[32]={0};
	//远程URL:
	char *			prmtURL = NULL;
	char*			stemp = "";
	int				errcode = 1, lineno = __LINE__;
	char ftpurl[136];

	_TRACE_CALL;

	user[0] = passwd[0] = 0;
	strcpy(argPath, "path=/mnt/");
	strcpy(arguser, "user=");
	strcpy(argpasswd, "passwd=");

	_GET_PSTR(saveDir, _NEED);
	strcat(argPath, psaveDir);

	_GET_STR(user, _OPT);
	if(user[0])
	{
		strcat(arguser, user);
		_GET_STR(passwd, _OPT);
		if(passwd[0])strcat(argpasswd, passwd);
	}

	_GET_INT(port, _OPT);
	snprintf(portstr,sizeof(portstr),"port=%d",port);
	//if(port == 0){lineno = __LINE__; goto check_err;}

	_GET_PSTR(rmtURL, _NEED);

	if( strstr(prmtURL, "ftp://") != prmtURL )
		sprintf(ftpurl, "ftp://%s", prmtURL);
	else
		sprintf(ftpurl, "%s", prmtURL);

	/************Place your code here, do what you want to do! ************/
	//call_cmd("/bin/wget_manage", 4, 0, arguser, argpasswd, argPath, prmtURL);
	errcode = call_cmd("/bin/wget_manage", 5, 1, arguser, argpasswd, argPath, ftpurl, portstr);
	/************Place your code here, do what you want to do! ************/

	if(errcode == 0)
	{
		strData = boaGetVar(wp,"submit-url","");   // hidden page
		strcpy(tmpBuf,"下载成功！");
		OK_MSG1(tmpBuf, strData);
	}
	else
	{
		strcpy(tmpBuf,"下载失败！");
		ERR_MSG(tmpBuf);
	}
check_err:
	_TRACE_LEAVEL;
	return;
}

