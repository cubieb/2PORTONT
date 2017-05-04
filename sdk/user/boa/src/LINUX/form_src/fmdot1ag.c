/*-- System inlcude files --*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "multilang.h"

int dot1ag_init(int eid, request * wp, int argc, char **argv)
{
	unsigned int i, total;
	MIB_CE_DOT1AG_T entry = {0};
	MIB_CE_ATM_VC_T vc_entry = {0};
	int bytesSent = 0;
	char ifname[IFNAMSIZ] = {0};

	arrange_dot1ag_table();

	total = mib_chain_total(MIB_DOT1AG_TBL);
	bytesSent += boaWrite(wp, "var dot1ag = new Array(%d);\n", total);

	for ( i = 0 ; i < total ; i++)
	{
		mib_chain_get(MIB_DOT1AG_TBL, i, (void *)&entry);

		ifGetName(entry.ifIndex, ifname, IFNAMSIZ);

		bytesSent += boaWrite(wp, "dot1ag[%d] = new Object();\n", i);
		bytesSent += boaWrite(wp, "dot1ag[%d].interface = \"%s\";\n", i, ifname);
		bytesSent += boaWrite(wp, "dot1ag[%d].md_name = \"%s\";\n", i, entry.md_name);
		bytesSent += boaWrite(wp, "dot1ag[%d].md_level = %d;\n", i, entry.md_level);
		bytesSent += boaWrite(wp, "dot1ag[%d].ma_name = \"%s\";\n", i, entry.ma_name);
		bytesSent += boaWrite(wp, "dot1ag[%d].mep_id = %d;\n", i, entry.mep_id);
		bytesSent += boaWrite(wp, "dot1ag[%d].ccm_enable = %d;\n", i, entry.ccm_enable);
		bytesSent += boaWrite(wp, "dot1ag[%d].ccm_interval = %d;\n", i, entry.ccm_interval);
	}

	total = mib_chain_total(MIB_ATM_VC_TBL);

	bytesSent += boaWrite(wp, "var interface = new Array();\n");
	for(i = 0 ; i < total ; i++)
	{

		mib_chain_get(MIB_ATM_VC_TBL, i, &vc_entry);
		ifGetName(entry.ifIndex, ifname, IFNAMSIZ);

		bytesSent += boaWrite(wp, "interface.push(\"%s\")\n", ifname);	
	}
	
}

void formDot1agConf(request * wp, char *path, char *query)
{
	MIB_CE_DOT1AG_T entry = {0};
	char *action = NULL;
	char *str_in = NULL;
	char *submitUrl = NULL;
	int select = -1;

	action = boaGetVar(wp, "action", "");

	if(strcmp(action, multilang(LANG_DELETE)) != 0)
	{
		// Add or Modify
		str_in = boaGetVar(wp, "interface", "");
		entry.ifIndex = strtoul(str_in, NULL, 0);

		str_in = boaGetVar(wp, "md_name", "");
		strncpy(entry.md_name, str_in, 44);

		str_in = boaGetVar(wp, "md_level", "");
		entry.md_level = atoi(str_in);
		
		str_in = boaGetVar(wp, "ma_name", "");
		strncpy(entry.ma_name, str_in, 44);

		str_in = boaGetVar(wp, "mep_id", "");
		entry.mep_id = atoi(str_in);

		str_in = boaGetVar(wp, "ccm_enable", "0");
		if(str_in[0] == '1')
		{
			entry.ccm_enable = 1;

			str_in = boaGetVar(wp, "ccm_interval", "1000");
			entry.ccm_interval = atoi(str_in);
		}

		if(strcmp(action, multilang(LANG_MODIFY)) == 0)
		{
			str_in = boaGetVar(wp, "select", "");
			select = atoi(str_in);
			mib_chain_update(MIB_DOT1AG_TBL, &entry, select);
		}
		else
			mib_chain_add(MIB_DOT1AG_TBL, &entry);
	}
	else
	{
		// Delete
		str_in = boaGetVar(wp, "select", "");
		select = atoi(str_in);
		mib_chain_delete(MIB_DOT1AG_TBL, select);
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	startDot1ag();
	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);

	return;
}

void formDot1agAction(request * wp, char *path, char *query)
{
	MIB_CE_DOT1AG_T entry = {0};
	char str_select[4] = {0};
	int select = -1;
	char ifname[IFNAMSIZ] = {0};
	char level[4] = {0};
	char action[4] = {0};
	char mac[32] = {0};
	char *argv[20] = {0};
	int idx = 1;
	char fname[128] = {0};

	getcgiparam(str_select, wp->query_string, "select", 4);
	select = atoi(str_select);
	mib_chain_get(MIB_DOT1AG_TBL, select, &entry);

	ifGetName(entry.ifIndex, ifname, IFNAMSIZ);
	argv[idx++] = "-i";
	argv[idx++] = ifname;

	sprintf(level, "%u", entry.md_level);
	argv[idx++] = "-l";
	argv[idx++] = level;
	
	getcgiparam(action, wp->query_string, "action", 4);
	getcgiparam(mac, wp->query_string, "mac", 32);

	sprintf(fname, "/tmp/dot1ag_action_result.%s", ifname);
	unlink(fname);

	if(strcmp(action, "lbm") == 0)
	{
		// Send LBM
		char count[16] = {0};

		getcgiparam(count, wp->query_string, "count", 32);

		argv[idx++] = "-c";
		argv[idx++] = count;
		argv[idx++] = mac;

		do_cmd_fout("/bin/ethping", argv, 0, fname);
	}
	else
	{
		// Send LTM
		argv[idx++] = mac;
		do_cmd_fout("/bin/ethtrace", argv, 0, fname);
	}

	//nothing to send to frontend
}

void dot1agActionRefresh(request * wp, char *path, char *query)
{
	char buf[256] = {0}, ifname[IFNAMSIZ] = {0};
	MIB_CE_DOT1AG_T entry = {0};
	int select;
	FILE *file = NULL;

	getcgiparam(buf, wp->query_string, "select", 256);
	select = atoi(buf);
	mib_chain_get(MIB_DOT1AG_TBL, select, &entry);

	ifGetName(entry.ifIndex, ifname, IFNAMSIZ);
	sprintf(buf, "/tmp/dot1ag_action_result.%s", ifname);
	file = fopen(buf, "r");

	if(file == NULL)
	{
		boaWrite(wp, "document.dot1agAction.result.innerHTML=\"Open %s failed\";\n", buf);
		return;
	}

	if(read_pid("/var/run/dot1ag_action.pid") <= 0)
		boaWrite(wp, "processing = false;\n");

	boaWrite(wp, "document.dot1agAction.result.innerHTML=\"\";\n");
	while(!feof(file))
	{
		if(fgets(buf, 256, file) != NULL)
		{
			// remove new line ('\n')
			buf[strlen(buf) - 1] = '\0';
			boaWrite(wp, "document.dot1agAction.result.innerHTML+=\"%s\\n\";\n", buf);
		}
	}

	fclose(file);
}

int dot1ag_status_init(int eid, request * wp, int argc, char **argv)
{
	unsigned int i, total;
	MIB_CE_DOT1AG_T entry = {0};
	MIB_CE_ATM_VC_T vc_entry = {0};
	int bytesSent = 0;
	char ifname[IFNAMSIZ] = {0};

	total = mib_chain_total(MIB_DOT1AG_TBL);
	bytesSent += boaWrite(wp, "var meps = new Array();\n");
	bytesSent += boaWrite(wp, "var mep;\n");

	for ( i = 0 ; i < total ; i++)
	{
		char fname[64] = {0};
		FILE *file = NULL;
		int pid = -1;
		char mac[32] = {0};
		int mep_id;
		int md_level;
		char md[44] = {0};
		char ma[44] = {0};
		char status[8] = {0};

		mib_chain_get(MIB_DOT1AG_TBL, i, (void *)&entry);

		if(entry.ccm_enable == 0)
			continue;

		ifGetName(entry.ifIndex, ifname, IFNAMSIZ);
		sprintf(fname, "/var/run/dot1ag_ccd.pid.%s", ifname);
		pid = read_pid(fname);

		if(pid <= 0)
			continue;

		kill(pid, SIGHUP);
		usleep(1);	//wait for daemon to write into file
		sprintf(fname, "/tmp/dot1ag_ccd_info.%s", ifname);
		file = fopen(fname, "r");
		if(file == NULL)
		{
			fprintf(stderr, "<%s:%d> boa: fopen error\n", __FUNCTION__, __LINE__);
			break;
		}

		while(!feof(file))
		{
			int ret = fscanf(file, "%[^','],%[^','],%d,%*d,%[^','],%[^','],%*[^','],%*[^',']\n",
                    mac, status, &mep_id, md, ma);
			
			if( ret != 5)
                    continue;

			bytesSent += boaWrite(wp, "mep = new Object();\n", i);
			bytesSent += boaWrite(wp, "mep.interface = \"%s\";\n", ifname);
			bytesSent += boaWrite(wp, "mep.status = \"%s\";\n", status);
			bytesSent += boaWrite(wp, "mep.md_name = \"%s\";\n", md);
			bytesSent += boaWrite(wp, "mep.ma_name = \"%s\";\n", ma);
			bytesSent += boaWrite(wp, "mep.mep_id = \"%d\";\n", mep_id);
			bytesSent += boaWrite(wp, "mep.mac = \"%s\";\n", mac);
			bytesSent += boaWrite(wp, "meps.push(mep);\n");
		}

		fclose(file);
	}
}


