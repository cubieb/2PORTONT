#include <stdio.h>
#include <stdint.h>
#include "web_voip.h"

#ifdef SUPPORT_CODEC_DESCRIPTOR
#include "codec_table.h"
#endif

extern char *supported_codec_string[SUPPORTED_CODEC_MAX];

#ifdef SUPPORT_CODEC_DESCRIPTOR
CT_ASSERT( ( sizeof( supported_codec_string ) / sizeof( supported_codec_string[ 0 ] ) ) == SUPPORTED_CODEC_MAX );
CT_ASSERT( SUPPORTED_CODEC_MAX == NUM_CODEC_TABLE );
#endif

//[SD6, bohungwu, e8c web
#define E8C_CODEC_NUM 4
static 	const char *e8c_codec_str[E8C_CODEC_NUM] = {"G711-ulaw", "G711-alaw", "G722", "G729"};
static 	int e8c_codec_idx[E8C_CODEC_NUM] = {0};
static	int rank[E8C_CODEC_NUM] = {0};


//e8c support
static char e8c_dscp[DSCP_MAX][25] = {"Class 0 (DSCP 0x00)",
			"Class 1 (DSCP 0x08)",
			"Class 2 (DSCP 0x10)",
			"Class 3 (DSCP 0x18)",
			"Class 4 (DSCP 0x20)",
			"Class 5 (DSCP 0x28)",
			"Class 6 (DSCP 0x30)",
			"Class 7 (DSCP 0x38)",
			"EF (DSCP 0x2e)"
			};

char e8c_dtmf[DTMF_MAX][12] = {"RFC2833", "SIP INFO", "Inband", "DTMF_delete"};
char e8c_cid[CID_MAX][25] = {"FSK_BELLCORE", "FSK_ETSI", "FSK_BT", "FSK_NTT", "DTMF"};
char server_type[SERVER_TYPE_MAX][20]={"IMS SIP", "Soft Switch SIP"};
void asp_e8c_codec_get(request * wp, voipCfgPortParam_t *pCfg)
{
	int i, j, k;
	int e8c_codec_preced[E8C_CODEC_NUM] = {0}; //For trivial sorting

	//Find out E8C codec index from the codec set
	for (i=0, j=0; i<SUPPORTED_CODEC_MAX; i++)
	{
		for(k=0; k<E8C_CODEC_NUM; k++)
		{
			if( (strcmp(supported_codec_string[i], e8c_codec_str[k]) == 0) )
			{
				e8c_codec_idx[j]=i;
				e8c_codec_preced[j++] = pCfg->precedence[i];
				break;
			}
		}
	}

	//Sorting
	for(i=0; i<E8C_CODEC_NUM; i++)
	{
		int smallest = INT32_MAX;
		int s_idx = -1;
		for(j=0; j<E8C_CODEC_NUM; j++)
		{
			if(smallest > e8c_codec_preced[j])
			{
				s_idx = j;
				smallest = e8c_codec_preced[j];
			}
		}
		if(s_idx != -1)
		{
			rank[i] = e8c_codec_idx[s_idx];
			e8c_codec_preced[s_idx] = INT32_MAX;
		}
	}
	
	for (i=0; i<E8C_CODEC_NUM; i++)
	{
#ifdef CONFIG_NEW_WEB_STYLE
		boaWrite(wp, "<tr><th>codec priority %d:</th>\n", i+1);
#else
		boaWrite(wp, "<tr><td width=200px>codec priority %d:</td>\n", i+1);
#endif

		boaWrite(wp, "<td><select name=preced%d>\n", i);
		for (j=0; j<E8C_CODEC_NUM; j++)
		{
			if(rank[i] == e8c_codec_idx[j])
			{
				boaWrite(wp, "<option value=%d selected>%s", e8c_codec_idx[j], supported_codec_string[e8c_codec_idx[j]]);
			}
			else
			{
				boaWrite(wp, "<option value=%d>%s", e8c_codec_idx[j], supported_codec_string[e8c_codec_idx[j]]);
			}
		}
		boaWrite(wp, "</select></td></tr>\n");
	}
}

void asp_e8c_codec_set(request * wp, voipCfgPortParam_t *pCfg)
{
	int i;
	char szPrecedence[12];
	int codec_index;
	int e8c_codec_rank[E8C_CODEC_NUM] = {0};

	for(i=0; i<E8C_CODEC_NUM; i++)
	{
		//printf("pCfg->precedence[rank[%d]]=%d\n", i, pCfg->precedence[rank[i]]);
		e8c_codec_rank[i] = pCfg->precedence[rank[i]];
	}
	
	for (i=0; i<E8C_CODEC_NUM; i++)
	{
		sprintf(szPrecedence, "preced%d", i);
		codec_index = atoi(boaGetVar(wp, szPrecedence, "-1"));
		//printf("%s=%d\n", szPrecedence, codec_index);
		if( (codec_index >= 0) && (codec_index < SUPPORTED_CODEC_MAX) )
			pCfg->precedence[codec_index] = e8c_codec_rank[i];
		else
			printf("Error: invalid/unknown codec_index=%d\n", codec_index);

	}
}
//]



int asp_voip_e8c_get(int ejid, request * wp, int argc, char **argv)
{
	voipCfgParam_t *pVoIPCfg;
	voipCfgPortParam_t *pCfg1;
	voipCfgPortParam_t *pCfg;
	int i;
	int voip_port;


	if (web_flash_get(&pVoIPCfg) != 0)
		return -1;


	pCfg1 = &pVoIPCfg->ports[0];



	if (strcmp(argv[0], "max_voip_ports")==0)
	{
		boaWrite(wp, "%d", VOIP_PORTS);
	}
	else if (strcmp(argv[0], "sip_number")==0)
	{
		for (voip_port=0; voip_port< VOIP_PORTS; voip_port++)
		{
			if(pVoIPCfg->ports[voip_port].proxies[0].number != NULL)
				boaWrite(wp, "<td>%s</td>", pVoIPCfg->ports[voip_port].proxies[0].number);	
		}

	}
	else if (strcmp(argv[0], "registerStatus")==0)
	{
		FILE *fh;
		char buf[MAX_VOIP_PORTS * MAX_PROXY];
		
		fh = fopen(_PATH_TMP_STATUS, "r");
		if (!fh) {
			printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
			printf("\nerrno=%d\n", errno);
		}

		memset(buf, 0, sizeof(buf));
		if (fread(buf, sizeof(buf), 1, fh) == 0) {
			printf("Web: The content of /tmp/status is NULL!!\n");
			printf("\nerrno=%d\n", errno);
			boaWrite(wp, "%s", "ERROR");
		}
		else {
			for (voip_port=0; voip_port< VOIP_PORTS; voip_port++)
			{
				/* SD6, bohungwu, fix incorrect registration status for the secondary proxy */
				char p0_reg_st, p1_reg_st, p_reg_st;
				if((pVoIPCfg->ports[voip_port].proxies[0].enable & PROXY_ENABLED)==0)
				{
					boaWrite(wp, "<td>%s</td>", "端口未激活");	
				}
				else
				{
				
					p0_reg_st = buf[voip_port * MAX_PROXY];
					p1_reg_st = buf[voip_port * MAX_PROXY + 1];
					if((p0_reg_st == '1')||(p0_reg_st == '2')||(p0_reg_st == '4')||(p0_reg_st == '5')){
						p_reg_st = p0_reg_st;
					}
					else{
						if((pVoIPCfg->ports[voip_port].proxies[1].enable & PROXY_ENABLED)&&(p1_reg_st != '0'))
						{
							p_reg_st = p1_reg_st;
						}
						else//sencond proxy disable, use main proxy status
						{
							p_reg_st = p0_reg_st;
						}
					}
					//fprintf(stderr, "buf is %s.\n", buf);
					switch (p_reg_st) {
					case '0':
						//boaWrite(wp, "<td>%s</td>", "Not Registered");
						boaWrite(wp, "<td>%s</td>", "端口未注册");
						break;
					case '1':
						//boaWrite(wp, "<td>%s</td>", "Registered");
						boaWrite(wp, "<td>%s</td>", "端口注册成功");
						break;
					case '2':
					case '5':
						//boaWrite(wp, "<td>%s</td>", "Registering");
						boaWrite(wp, "<td>%s</td>", "端口注册中");
						break;
					case '3':
						//boaWrite(wp, "<td>%s</td>", "Register Fail");
						boaWrite(wp, "<td>%s</td>", "端口注册失败");
						break;
					case '4':
						//boaWrite(wp, "<td>%s</td>", "VoIP Restart...");
						boaWrite(wp, "<td>%s</td>", "VoIP Restart...");
						break;
					default:
						boaWrite(wp, "<td>%s</td>", "ERROR");
						break;
					}
				}
			}
		}

		fclose(fh);
	}
	else if (strcmp(argv[0], "servertype")==0)
	{
		for (i=0; i<SERVER_TYPE_MAX; i++)
		{
			if (i == pVoIPCfg->X_CT_servertype)
				boaWrite(wp, "<option selected>%s</option>", server_type[i]);
			else
				boaWrite(wp, "<option>%s</option>", server_type[i]);
		}
	}
	//main proxy
	else if (strcmp(argv[0], "proxy0_addr")==0)
	{
		if(pCfg1->proxies[0].addr != NULL)
		{
			boaWrite(wp, "%s", pCfg1->proxies[0].addr);
		}
		else
			printf("Proxy 0 addr is null!!!\n");
	}
	else if (strcmp(argv[0], "proxy0_port")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[0].port);
	}
	else if (strcmp(argv[0], "proxy0_obEnable")==0)
	{
		boaWrite(wp, "%s", (pCfg1->proxies[0].outbound_enable) ? "checked" : "");
	}
	else if (strcmp(argv[0], "proxy0_obAddr")==0)
	{
			boaWrite(wp, "%s", pCfg1->proxies[0].outbound_addr);
	}	
	else if (strcmp(argv[0], "proxy0_obPort")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[0].outbound_port);
	}
	else if (strcmp(argv[0], "proxy0_domain_name")==0)
	{
		boaWrite(wp, "%s", pCfg1->proxies[0].domain_name);
	}
	else if (strcmp(argv[0], "proxy0_reg_expire")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[0].reg_expire);
	}
	else if (strcmp(argv[0], "proxy0_sessionEnable")==0)
	{
		boaWrite(wp, "%s", (pCfg1->proxies[0].SessionUpdateTimer) ? "checked" : "");
	}
	else if (strcmp(argv[0], "proxy0_sessionExpiry")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[0].SessionUpdateTimer);
	}
	
	//second proxy
	else if (strcmp(argv[0], "proxy1_enable")==0)
	{
		if (VOIP_PORTS==1)
		{
			boaWrite(wp, "%s", (pCfg1->proxies[1].enable & PROXY_ENABLED)? "checked" : "");
		}
		else
		{
			boaWrite(wp, "%s", ((pCfg1->proxies[1].enable & PROXY_ENABLED) ||(pVoIPCfg->ports[1].proxies[1].enable & PROXY_ENABLED))? "checked" : "");
		}
		
	}
	else if (strcmp(argv[0], "proxy1_addr")==0)
	{
		if(pCfg1->proxies[1].addr != NULL)
			boaWrite(wp, "%s", pCfg1->proxies[1].addr);
		else
			printf("Proxy 0 addr is null!!!\n");
	}
	else if (strcmp(argv[0], "proxy1_port")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[1].port);
	}
	else if (strcmp(argv[0], "proxy1_obEnable")==0)
	{
		boaWrite(wp, "%s", (pCfg1->proxies[1].outbound_enable) ? "checked" : "");
	}
	else if (strcmp(argv[0], "proxy1_obAddr")==0)
	{
		boaWrite(wp, "%s", pCfg1->proxies[1].outbound_addr);
	}
	else if (strcmp(argv[0], "proxy1_obPort")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[1].outbound_port);
	}	
	else if (strcmp(argv[0], "proxy1_domain_name")==0)
	{
		boaWrite(wp, "%s", pCfg1->proxies[1].domain_name);
	}
	else if (strcmp(argv[0], "proxy1_reg_expire")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[1].reg_expire);
	}
	else if (strcmp(argv[0], "proxy1_sessionEnable")==0)
	{
		boaWrite(wp, "%s", (pCfg1->proxies[1].SessionUpdateTimer) ? "checked" : "");
	}
	else if (strcmp(argv[0], "proxy1_sessionExpiry")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[1].SessionUpdateTimer);
	}
	else if (strcmp(argv[0], "init_port_account")==0)
	{
		boaWrite(wp,"portNum = %d;\n", VOIP_PORTS);
		for (voip_port=0; voip_port< VOIP_PORTS; voip_port++)
		{
			pCfg = &pVoIPCfg->ports[voip_port];
			boaWrite(wp,"port_line = new Array(%d, '%s','%s','%s');\n",(pCfg->proxies[0].enable & PROXY_ENABLED) ? 1 : 0,
						pCfg->proxies[0].number, pCfg->proxies[0].login_id, pCfg->proxies[0].password);
			boaWrite(wp,"portArray.push(port_line);\n");
		}
	}
	else if (strcmp(argv[0], "port_account")==0)
	{
		for (voip_port=0; voip_port< VOIP_PORTS; voip_port++)
		{

			pCfg = &pVoIPCfg->ports[voip_port];

#ifdef CONFIG_NEW_WEB_STYLE
			boaWrite(wp, "<div class=\"column\">" \
						   "<div class=\"column_title\">" \
						     "<div class=\"column_title_left\"></div>" \
						       "<p>Line %d Account</p>" \
						     "<div class=\"column_title_right\"></div>" \
						   "</div>" \
						   "<div class=\"data_common\">" \
						     "<table>" \
						       "<tr>\n" \
						         "<th width=\"200px\">Enable</th>" \
						         "<td><input name=port%d_account_enable type=checkbox value=enable %s ></td>" \
						       "</tr>\n", 
					  voip_port+1, voip_port+1, (pCfg->proxies[0].enable & PROXY_ENABLED) ? "checked" : "");

			boaWrite(wp, "<tr>\n" \
						   "<th>Subscriber Number:</th>" \
						   "<td><input name=port%d_number size=16 maxlength=32 style=width:140px value=%s></td>\n" \
						 "</tr>\n", 
					  voip_port+1, pCfg->proxies[0].number);

			boaWrite(wp, "<tr>\n" \
						   "<th>User Account:</th>" \
						   "<td><input name=port%d_login_id size=16 maxlength=32 style=width:140px value=%s></td>\n" \
						 "</tr>\n", 
					  voip_port+1, pCfg->proxies[0].login_id);

			boaWrite(wp, "<tr>\n" \
						   "<th>User Password:</th>" \
						   "<td><input name=port%d_password size=16 maxlength=32 type=password style=width:140px value=%s></td>\n" \
						 "</tr>\n", 
					  voip_port+1, pCfg->proxies[0].password);

			boaWrite(wp, "</table>\n" \
				         "</div>\n" \
				         "</div>");
#else
			boaWrite(wp, "<b>线路%d 账号</b>\n"  \
			"<table border=0 cellpadding=0 cellspacing=0 width=700>\n" \
			"<tr>\n" \
			"<td width=200px>启用</td><td><input name=port%d_account_enable type=checkbox value=enable %s ></td>\n" \
			"</tr>\n" \
			,voip_port+1, voip_port+1, (pCfg->proxies[0].enable & PROXY_ENABLED) ? "checked" : "");
			
			boaWrite(wp,"<tr>\n" \
			"<td>用户号码:</td><td><input name=port%d_number size=16 maxlength=32 style=width:140px value=%s></td>\n"
			"</tr>\n" \ 
			,voip_port+1, pCfg->proxies[0].number);
			
			boaWrite(wp,"<tr>\n" \
			"<td>用户账号:</td><td><input name=port%d_login_id size=16 maxlength=32 style=width:140px value=%s></td>\n"
			"</tr>\n" \ 
			,voip_port+1, pCfg->proxies[0].login_id);
			
			boaWrite(wp,"<tr>\n" \
			"<td>用户密码:</td><td><input name=port%d_password size=16 maxlength=32 type=password style=width:140px value=%s></td>\n"
			"</tr>\n" \ 
			
			,voip_port+1, pCfg->proxies[0].password);
			boaWrite(wp, "</table>\n");
#endif
		}

	}

	//sip config 2
	else if (strcmp(argv[0], "sipPort")==0)
		boaWrite(wp, "%d", pCfg1->sip_port);
	else if (strcmp(argv[0], "rtpPort")==0)
		boaWrite(wp, "%d", pCfg1->media_port);
	else if (strcmp(argv[0], "dtmfMode")==0)
	{
		for (i=0; i<DTMF_MAX; i++)
		{
			if (i == pCfg1->dtmf_mode)
				boaWrite(wp, "<option selected>%s</option>", e8c_dtmf[i]);
			else
				boaWrite(wp, "<option>%s</option>", e8c_dtmf[i]);
		}
	}
	else if (strcmp(argv[0], "useLec")==0)
		boaWrite(wp, "%s", (pCfg1->lec) ? "checked" : "");
	else if (strcmp(argv[0], "useVad")==0)
		boaWrite(wp, "%s", (pCfg1->vad) ? "checked" : "");
	else if(strcmp(argv[0], "useT38")==0)
		boaWrite(wp, "%s", (pCfg1->useT38) ? "checked" : "");
	else if (strcmp(argv[0], "caller_id")==0)
	{
		boaWrite(wp, "<select name=caller_id >");
			//!RTK_VOIP_IS_SLIC_CH( voip_port, g_VoIP_Feature ) ? "disabled" : "");

		for (i=0; i<CID_MAX ;i++)
		{
			if (i == (pCfg1->caller_id_mode & 7))
				boaWrite(wp, "<option selected>%s</option>", e8c_cid[i]);
			else
				boaWrite(wp, "<option>%s</option>", e8c_cid[i]);
		}

		boaWrite(wp, "</select>");
	}
	else if (strcmp(argv[0], "flash_hook_time")==0)
	{
		boaWrite(wp, "<input type=text name=flash_hook_time_min size=4 maxlength=5 value=%d >" \
			" <  Flash Time  < " \
			"<input type=text name=flash_hook_time size=4 maxlength=5 value=%d >",
			pCfg1->flash_hook_time_min,
			pCfg1->flash_hook_time);
	}
	else if (strcmp(argv[0], "e8c_codec")==0)
	{
		asp_e8c_codec_get(wp, pCfg1);
	}
	else if (strcmp(argv[0], "off_hook_alarm")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->off_hook_alarm);
	}
	else if (strcmp(argv[0], "auto_dial")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->auto_dial);
	}
	else if (strcmp(argv[0], "InterDigitTimerLong")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->InterDigitTimerLong);
	}
	else if (strcmp(argv[0], "BusyToneTimer")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->BusyToneTimer);
	}
	else if (strcmp(argv[0], "HangingReminderToneTimer")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->HangingReminderToneTimer);
	}
	else if (strcmp(argv[0], "RegisterRetryInterval")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[0].RegisterRetryInterval);
	}
	else if (strcmp(argv[0], "HeartbeatCycle")==0)
	{
		boaWrite(wp, "%d", pCfg1->proxies[0].HeartbeatCycle);
	}
	else if (strcmp(argv[0], "NoAnswerTimer")==0)
	{
		boaWrite(wp, "%d", pVoIPCfg->NoAnswerTimer);
	}

	else if (strcmp(argv[0], "rtpDscp")==0)
	{
		for (i=0; i<DSCP_MAX ;i++)
		{
			if (i == pVoIPCfg->rtpDscp)
				boaWrite(wp, "<option selected>%s</option>", e8c_dscp[i]);
			else
				boaWrite(wp, "<option>%s</option>", e8c_dscp[i]);
		}
	}

	else if (strcmp(argv[0], "sipDscp")==0)
	{
		for (i=0; i<DSCP_MAX ;i++)
		{
			if (i == pVoIPCfg->sipDscp)
				boaWrite(wp, "<option selected>%s</option>", e8c_dscp[i]);
			else
				boaWrite(wp, "<option>%s</option>", e8c_dscp[i]);
		}
	}
	else if (strcmp(argv[0], "dialplan")==0){
		boaWrite(wp, "%s", pCfg1->dialplan);
	}
	else if (strcmp(argv[0], "digitmap_enable")==0) {
    	boaWrite(wp, "<input type=\"radio\" name=\"digitmap_enable\" value=1 %s>on",
    				( pCfg1 ->digitmap_enable ? "checked" : "" ));
    	boaWrite(wp, "<input type=\"radio\" name=\"digitmap_enable\" value=0 %s>off",
    				( !pCfg1 ->digitmap_enable ? "checked" : "" ));
	}
	else
	{

		return -1;
	}

	return 0;
}

//[SD6, bohungwu, e8c web
//#define DBG_WEB_VAR(VAR_NAME) printf(#VAR_NAME "=%s\n", websGetVar(wp, T(#VAR_NAME), "0"))
#define DBG_WEB_VAR(VAR_NAME) do {} while(0)
//#define RCM_DBG printf("%s-%d\n", __FILE__, __LINE__)
#define RCM_DBG do {} while(0)
void asp_voip2_e8c_set(request * wp, char *path, char *query)
{
	voipCfgParam_t *pVoIPCfg;
	voipCfgPortParam_t *pCfg;
	int i;
	char szPrecedence[12];
	char redirect_url[50];
	int voip_port;
	char szName[20];
	char *ptr;
	
	printf("---Enter %s---\n", __FUNCTION__);
	if (web_flash_get(&pVoIPCfg) != 0)
		return;
	RCM_DBG;

	//E8C support SIP REDUNDANCY 
	pVoIPCfg->rfc_flags |=SIP_SERVER_REDUNDANCY;
		
	for (voip_port=0; voip_port< VOIP_PORTS; voip_port++)
	{
		pCfg = &pVoIPCfg->ports[voip_port];
		
		strcpy((char *) pCfg->dialplan, boaGetVar(wp, "dialplan", ""));
		pCfg ->digitmap_enable =  atoi( boaGetVar(wp, "digitmap_enable", "") );
		
		//sipPort
		pCfg->sip_port = atoi(boaGetVar(wp, "sipPort", "5060"));

		//rtp port
		if(voip_port==0)
			pCfg->media_port 	= atoi(boaGetVar(wp, "rtpPort", "9000"));
		else
			pCfg->media_port = atoi(boaGetVar(wp, "rtpPort", "9000"))+4;

		//dtmf relay mode
		ptr	 = boaGetVar(wp, "dtmfMode", "");
		for(i=0; i<DTMF_MAX; i++)
		{
			if (!gstrcmp(ptr, e8c_dtmf[i]))
				break;
		}
		if (i == DTMF_MAX)
			i = DTMF_INBAND;

		pCfg->dtmf_mode 		= i;
		
		//echo cancellation
		i = atoi(boaGetVar(wp, "echo_cancellation", ""));
		if(i == 0)
		{
			pCfg->lec = 0;
		}
		else if(i == 1)
		{
			pCfg->lec = 1;
		}
		else
		{
			fprintf(stderr, "Error, illega echo cancelation setting=%d\n", i);
		}

		//vad
		pCfg->vad = !gstrcmp(boaGetVar(wp, "useVad", ""), "on");
		
		//T38
		pCfg->useT38 = !gstrcmp(boaGetVar(wp, "useT38", ""), "on");
		
		//caller id
		ptr	 = boaGetVar(wp, "caller_id", "");
		for(i=0; i<CID_MAX; i++)
		{
			if (!gstrcmp(ptr, e8c_cid[i]))
				break;
		}
		if (i == CID_MAX)
			i = CID_DTMF;

		pCfg->caller_id_mode = i;

		//flash hook time

	        i = atoi(boaGetVar(wp, "flash_hook_time", ""));
		if ((i >= 100) && ( i <= 2000))
			pCfg->flash_hook_time = i;
		else
			pCfg->flash_hook_time = 300;

	        i = atoi(boaGetVar(wp, "flash_hook_time_min", ""));
		if ( i >= pCfg->flash_hook_time )
			pCfg->flash_hook_time_min = 0;
		else
			pCfg->flash_hook_time_min = i;

		
		//codec
		asp_e8c_codec_set(wp, pCfg);

		//heart beat cycle
		pCfg->proxies[0].HeartbeatCycle = atoi(boaGetVar(wp, "HeartbeatCycle", ""));
		pCfg->proxies[1].HeartbeatCycle = atoi(boaGetVar(wp, "HeartbeatCycle", ""));
		if(pCfg->proxies[0].HeartbeatCycle==0)//HeartbeatCycle=0:disable
		{
			pCfg->proxies[0].enable &= ~PROXY_OPTIONS_PING;
			pCfg->proxies[1].enable &= ~PROXY_OPTIONS_PING;
		}else{
			pCfg->proxies[0].enable |= PROXY_OPTIONS_PING;
			pCfg->proxies[1].enable |= PROXY_OPTIONS_PING;
		}
		//retry reg
		pCfg->proxies[0].RegisterRetryInterval = atoi(boaGetVar(wp, "RegisterRetryInterval", ""));
		pCfg->proxies[1].RegisterRetryInterval = atoi(boaGetVar(wp, "RegisterRetryInterval", ""));
	}
	
	pVoIPCfg->off_hook_alarm=atoi(boaGetVar(wp, "off_hook_alarm", ""));
	pVoIPCfg->auto_dial=atoi(boaGetVar(wp, "auto_dial", ""));
	pVoIPCfg->InterDigitTimerLong=atoi(boaGetVar(wp, "InterDigitTimerLong", ""));
	
	pVoIPCfg->BusyToneTimer=atoi(boaGetVar(wp, "BusyToneTimer", ""));
	pVoIPCfg->HangingReminderToneTimer=atoi(boaGetVar(wp, "HangingReminderToneTimer", ""));
	pVoIPCfg->NoAnswerTimer=atoi(boaGetVar(wp, "NoAnswerTimer", ""));

	ptr	 = boaGetVar(wp, "rtpDscp", "");
	for(i=0; i<DSCP_MAX; i++)
	{
		if (!gstrcmp(ptr, e8c_dscp[i]))
			break;
	}
	if (i == DSCP_MAX)
		i = DSCP_CS0;
	pVoIPCfg->rtpDscp = i;

	ptr	 = boaGetVar(wp, "sipDscp", "");
	for(i=0; i<DSCP_MAX; i++)
	{
		if (!gstrcmp(ptr, e8c_dscp[i]))
			break;
	}
	if (i == DSCP_MAX)
		i = DSCP_CS0;
	pVoIPCfg->sipDscp = i;


	
	RCM_DBG;

	web_flash_set(pVoIPCfg);


	sprintf(redirect_url, "/app_voip2.asp");
#ifdef REBOOT_CHECK
	OK_MSG(redirect_url);
#else
	web_restart_solar();
	boaRedirect(wp, redirect_url);
#endif
}

void asp_voip_e8c_set(request * wp, char *path, char *query)
{
	voipCfgParam_t *pVoIPCfg;
	voipCfgPortParam_t *pCfg;
	int i;
	char szPrecedence[12];
	char redirect_url[50];
	int voip_port;
	char szName[20];
	char *ptr;
	
	printf("---Enter %s---\n", __FUNCTION__);
	if (web_flash_get(&pVoIPCfg) != 0)
		return;

	RCM_DBG;
	
	//E8C support SIP REDUNDANCY 
	pVoIPCfg->rfc_flags |=SIP_SERVER_REDUNDANCY;
	ptr	 = boaGetVar(wp, "servertype", "");
	for(i=0; i<SERVER_TYPE_MAX; i++)
	{
		if (!gstrcmp(ptr, server_type[i]))
			break;
	}
	if (i == SERVER_TYPE_MAX)
		i = IMS_SIP;
	pVoIPCfg->X_CT_servertype = i;


	
	for (voip_port=0; voip_port< VOIP_PORTS; voip_port++)
	{
		pCfg = &pVoIPCfg->ports[voip_port];
		

		
		for (i=0; i< MAX_PROXY; i++)
		{

			//proxy addr
			sprintf(szName, "proxy%d_addr", i);
			DBG_WEB_VAR(szName);
			strcpy(pCfg->proxies[i].addr, boaGetVar(wp, szName, ""));
			RCM_DBG;
			
			//proxy port
			sprintf(szName, "proxy%d_port", i);
			DBG_WEB_VAR(szName);
			pCfg->proxies[i].port = atoi(boaGetVar(wp, szName, "5060"));
			RCM_DBG;

			//outbound proxy enable/disable
			sprintf(szName, "proxy%d_obEnable", i);
			DBG_WEB_VAR(szName);
			pCfg->proxies[i].outbound_enable = !gstrcmp(boaGetVar(wp, szName, ""), "enable");
			RCM_DBG;
			
			//outbound proxy addr
			sprintf(szName, "proxy%d_obAddr", i);
			DBG_WEB_VAR(szName);
			strcpy(pCfg->proxies[i].outbound_addr, boaGetVar(wp, szName, ""));
			RCM_DBG;
			
			//outbound proxy port
			sprintf(szName, "proxy%d_obPort", i);
			DBG_WEB_VAR(szName);
			pCfg->proxies[i].outbound_port = atoi(boaGetVar(wp, szName, "5060"));

			//domain_name
			sprintf(szName, "proxy%d_domain_name", i);
			strcpy(pCfg->proxies[i].domain_name, boaGetVar(wp, szName, ""));


			//reg_expire
			sprintf(szName, "proxy%d_reg_expire", i);
			pCfg->proxies[i].reg_expire = atoi(boaGetVar(wp, szName, "60"));
		
			//sessionEnable
			sprintf(szName, "proxy%d_sessionEnable", i);
			if( strcmp("enable", boaGetVar(wp, szName, "")) == 0){
				//sessionExpiry
				sprintf(szName, "proxy%d_sessionExpiry", i);
				pCfg->proxies[i].SessionUpdateTimer = atoi(boaGetVar(wp, szName, "90"));

			}else{
				pCfg->proxies[i].SessionUpdateTimer =0;
			}
			
		}


		//account_enable [main proxy enable]
		sprintf(szName, "port%d_account_enable", voip_port+1);
		pCfg->proxies[0].enable = 0;//Reset setting
		pCfg->proxies[1].enable = 0;//Reset setting
		if( strcmp("enable", boaGetVar(wp, szName, "")) == 0)
		{
			pCfg->proxies[0].enable |= PROXY_ENABLED;
			pCfg->proxies[0].enable |= PROXY_SUBSCRIBE; /* default enable subscribe*/
			if(pCfg->proxies[0].HeartbeatCycle!=0)
				pCfg->proxies[0].enable |= PROXY_OPTIONS_PING;
			
			//second proxy enable
			if( strcmp("enable", boaGetVar(wp, "proxy1_enable", "")) == 0)
			{
				pCfg->proxies[1].enable |= PROXY_ENABLED;
				pCfg->proxies[1].enable |= PROXY_SUBSCRIBE;
				if(pCfg->proxies[1].HeartbeatCycle!=0)
					pCfg->proxies[1].enable |= PROXY_OPTIONS_PING;
			}
		}

		//number
		sprintf(szName, "port%d_number", voip_port+1);
		strcpy(pCfg->proxies[0].number, boaGetVar(wp, szName, ""));
		strcpy(pCfg->proxies[1].number, boaGetVar(wp, szName, ""));
		
		//display_name:e8c display_name same as number
		strcpy(pCfg->proxies[0].display_name, boaGetVar(wp, szName, ""));
		strcpy(pCfg->proxies[1].display_name, boaGetVar(wp, szName, ""));
		
		//login_id
		sprintf(szName, "port%d_login_id", voip_port+1);
		strcpy(pCfg->proxies[0].login_id, boaGetVar(wp, szName, ""));
		strcpy(pCfg->proxies[1].login_id, boaGetVar(wp, szName, ""));
		

		
		//password
		sprintf(szName, "port%d_password", voip_port+1);
		strcpy(pCfg->proxies[0].password, boaGetVar(wp, szName, ""));
		strcpy(pCfg->proxies[1].password, boaGetVar(wp, szName, ""));


	}
	RCM_DBG;

	web_flash_set(pVoIPCfg);


	sprintf(redirect_url, "/app_voip.asp");
#ifdef REBOOT_CHECK
	OK_MSG(redirect_url);
#else
	web_restart_solar();
	boaRedirect(wp, redirect_url);
#endif
}
//]


