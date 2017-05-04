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
#include <net/if.h>


/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "multilang.h"


#define OSGI_DEBUG 1
#define debug_print(...) \
            do { if (OSGI_DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)


// change this to non-volatile disk
#define BND_LOCATION "/tmp/"
#define JFFS2_PARTITION "/var/config_osgi/"
#define OSGI_RESULT_FILE "/tmp/OSGI_RESULT"
#define OSGI_UUID_FILE "/tmp/OSGI_UUID"


void deleteOsgiFlash(MIB_CE_OSGI_BUNDLE_T entry)
{
	int index;
	int bnd_cnt = mib_chain_total(MIB_OSGI_BUNDLE_TBL);
	MIB_CE_OSGI_BUNDLE_T bundle_entry;
	for (index = 0 ; index < bnd_cnt; index ++)
	{
		if (!mib_chain_get(MIB_OSGI_BUNDLE_TBL, index, (void *)&bundle_entry))
		{
	  		debug_print("%s(%d) get entry fail \n" , __func__, __LINE__);
			break;
		}
		if(strcmp(entry.bundle_name , bundle_entry.bundle_name) == 0 &&
			strcmp(entry.bundle_file , bundle_entry.bundle_file) == 0 &&
			entry.bundle_id == bundle_entry.bundle_id) // found it !
		{
			debug_print("%s(%d) delete the entry in flash \n", __func__, __LINE__);
			debug_print("action:%d,name:%s,file:%s,id:%d\n" , 
					bundle_entry.bundle_action,bundle_entry.bundle_name,
					bundle_entry.bundle_file, bundle_entry.bundle_id);
			mib_chain_delete(MIB_OSGI_BUNDLE_TBL , index);
			break;
			
		}
		
	}
}

int getOsgiFlash(MIB_CE_OSGI_BUNDLE_T entry, MIB_CE_OSGI_BUNDLE_Tp orig_entry)
{
	int index;
	int bnd_cnt = mib_chain_total(MIB_OSGI_BUNDLE_TBL);
	for (index = 0 ; index < bnd_cnt; index ++)
	{
		if (!mib_chain_get(MIB_OSGI_BUNDLE_TBL, index, (void *)orig_entry))
			{
	  			debug_print("%s(%d) get entry fail \n" , __func__, __LINE__);
				return 0; // fail
			}
		if(strcmp(entry.bundle_name , orig_entry->bundle_name) == 0) // found it !
		{
			debug_print("%s(%d) get the entry in flash \n", __func__, __LINE__);
			debug_print("action:%d,name:%s,file:%s,id:%d\n" , 
					orig_entry->bundle_action,orig_entry->bundle_name,
					orig_entry->bundle_file, orig_entry->bundle_id);
			return 1; 
		}
		
	}
	return 0;
}

// check the osgi entry exist in flash or not ?
int checkOsgiFlash(MIB_CE_OSGI_BUNDLE_T entry)
{
	int index;
	int bnd_cnt = mib_chain_total(MIB_OSGI_BUNDLE_TBL);
	MIB_CE_OSGI_BUNDLE_T bundle_entry;
	for (index = 0 ; index < bnd_cnt; index ++)
	{
		if (!mib_chain_get(MIB_OSGI_BUNDLE_TBL, index, (void *)&bundle_entry))
			{
	  			debug_print("%s(%d) get entry fail \n" , __func__, __LINE__);
				return 0; // fail
			}
		if(strcmp(entry.bundle_name , bundle_entry.bundle_name) == 0) // found it !
		{
			debug_print("%s(%d) check the entry in flash \n", __func__, __LINE__);
			debug_print("action:%d,name:%s,file:%s,id:%d\n" , 
					bundle_entry.bundle_action,bundle_entry.bundle_name,
					bundle_entry.bundle_file, bundle_entry.bundle_id);
			return 1; 
		}
	}

	return 0;// not found 
}

// update osgi entry in flash(check bundle name && bundle file name)
int updateOsgiFlash(MIB_CE_OSGI_BUNDLE_T entry)
{
	int index;
	int bnd_cnt = mib_chain_total(MIB_OSGI_BUNDLE_TBL);
	MIB_CE_OSGI_BUNDLE_T bundle_entry;
	for (index = 0 ; index < bnd_cnt; index ++)
	{
		if (!mib_chain_get(MIB_OSGI_BUNDLE_TBL, index, (void *)&bundle_entry))
			{
	  			printf("%s(%d) get entry fail \n" , __func__, __LINE__);
				return 0; // fail
			}
		if(strcmp(entry.bundle_name , bundle_entry.bundle_name) == 0 && 
			strcmp(entry.bundle_file , bundle_entry.bundle_file) == 0) // found it !
		{
			debug_print("%s(%d) update the entry in flash \n", __func__, __LINE__);
			debug_print("action:%d,name:%s,file:%s,id:%d\n" , 
					bundle_entry.bundle_action,bundle_entry.bundle_name,
					bundle_entry.bundle_file, bundle_entry.bundle_id);
			break;
		}
	}

	if(index == bnd_cnt) 
		return 0;
	else
		return mib_chain_update(MIB_OSGI_BUNDLE_TBL, (void *)&entry, index);
	
}

// add new osgi entry into flash
int addOsgiFlash(MIB_CE_OSGI_BUNDLE_T entry)
{
	return mib_chain_add(MIB_OSGI_BUNDLE_TBL, (unsigned char*)&entry);
}

int getOSGIInfo(int eid, request * wp, int argc, char **argv)
{
	char    *name;
	unsigned char buffer[256+1];
	int idx, ret;

	if (boaArgs(argc, argv, "%s", &name) < 1) 
	{
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	memset(buffer,0x00,64);
	
	if ( !strncmp(name, "fwname", 6) ) 
	{
		return boaWrite(wp, "%s", "RTK_OSGi_Framework");
	}
	else if(!strncmp(name, "fwver", 5))
	{
		return boaWrite(wp, "%s", "4.2.1");
	}
	else if(!strncmp(name, "fwstatus", 8))
	{
		FILE *fp = NULL;
		char *FILE_PATH = "/tmp/OSGI_UUID";

		if (!(fp=fopen(OSGI_UUID_FILE, "r"))) 
			return boaWrite(wp, "%s","Disabled");
		else
			return boaWrite(wp, "%s","Running");
	}
	else
		return boaWrite(wp, "%s", "WRONG PARAMETER");

}

char *ignore_bundle[] = 
{
	"System Bundle",
	"RealtekTCPSocketListener",
	"Apache Felix Bundle Repository",
	"Apache Felix Gogo Command",
	"Apache Felix Gogo Runtime",
	"Apache Felix Gogo Shell",
	"osgi.cmpn",
	"Apache Felix Declarative Services",
	"Apache Felix Http Jetty"
};

static int bundle_ignore(char *name)
{
	int i = 0 ;
	for(i = 0 ; i < sizeof(ignore_bundle) / sizeof(ignore_bundle[0]); i++)
	{
		if(strcmp(ignore_bundle[i] , name ) == 0 )
			return 1; 
	}

	return 0;
}

struct next_action_struct
{
	char *current_action;
	char *next_action;
};

struct next_action_struct nxt_act[] = 
{
		{"Active" , "Stop"},
		{"Resolved", "Start"},
		{"Installed", "Start"}
};

static char *next_action_func(char *cur_action)
{
	int i = 0 ; 
	for(i = 0 ; i < sizeof(nxt_act) / sizeof(struct next_action_struct) ; i ++)
	{
		if(strcmp(cur_action , nxt_act[i].current_action) == 0)
			return nxt_act[i].next_action;
	}

	// not found ??
	debug_print("not found this action : %s \n" , cur_action);
	return "Stop";
}

int getOSGIBundleList(int eid, request * wp, int argc, char **argv)
{
	
	FILE *fp;
	char stat[8];
	int bnd_cnt = 0;
	int i = 0;
	int show_mgt = 0;

	if (boaArgs(argc, argv, "%d", &show_mgt) < 1) 
	{
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}


	if (!(fp=fopen(OSGI_UUID_FILE, "r")))
	{
		debug_print("can't open UUID file\n");
		if(show_mgt)
			boaWrite(wp, "OSGi framework disabled\n");
		return 0;
	}
	fclose(fp);   // close UUID file
	
	system("/bin/osgi bundle lb");
	//struct bnd_desc *b_desc;
	if (!(fp=fopen(OSGI_RESULT_FILE, "r")))
	{
		debug_print("can't open file\n");
		return 0;
	}
	
	fscanf(fp, "%s\n",stat);
	debug_print("stat:%s\n", stat);

	if(strncmp(stat, "SUCC", 4) == 0)
	{
		
		fscanf(fp, "%d\n", &bnd_cnt);
		debug_print("total %d bundles \n" , bnd_cnt);
		//b_desc = (void*)malloc(sizeof(struct bnd_desc) * bnd_cnt);

		if(show_mgt)
		{
			boaWrite(wp, "<table><tr> <td width=100%% colspan=\"5\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>Bundle List</b></font></td> </tr> ");
			// header
			boaWrite(wp, "<tr><font size=1>"
			"<td align=center width=\"10%%\" bgcolor=\"#808080\">ID</td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#808080\">State</td>\n"
			"<td align=center width=\"35%%\" bgcolor=\"#808080\">Name</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#808080\">Version</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#808080\">Action</td></font></tr>\n");
		}
		else
		{
			boaWrite(wp, "<table><tr> <td width=100%% colspan=\"4\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>Bundle List</b></font></td> </tr> ");
			// header
			boaWrite(wp, "<tr><font size=1>"
			"<td align=center width=\"10%%\" bgcolor=\"#808080\">ID</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#808080\">State</td>\n"
			"<td align=center width=\"40%%\" bgcolor=\"#808080\">Name</td>\n"
			"<td align=center width=\"30%%\" bgcolor=\"#808080\">Version</td></font></tr>\n");			
		}

		for(i = 0 ; i < bnd_cnt ; i ++)
		{
				char line[1024];
				char *id,*status,*name,*version;
				char *saveptr1 = NULL;
		
				fgets(line,1024,fp);
				//printf("LINE : %s \n" , line);
				id =  strtok_r(line,"|", &saveptr1);
				status = strtok_r(NULL,"|", &saveptr1);
				name = strtok_r(NULL,"|", &saveptr1);
				version =  strtok_r(NULL,"|", &saveptr1);
				//debug_print("id:%s, st:%s,name:%s,ver:%s\n", id,status,name,version);
				if(show_mgt)
				{
					boaWrite(wp, "<tr><font size=1>"
					"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\">%s</td>\n"
					"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\">%s</td>\n"
					"<td width=\"35%%\" bgcolor=\"#C0C0C0\">%s</td>\n"
					"<td width=\"20%%\" bgcolor=\"#C0C0C0\">%s</td>\n"
					"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\">",
					id, status, name, version);
					// if this bundle is not ignore , add button to it
					if(bundle_ignore(name) == 0) // false
					{
						// add button item , button , hidden value for action & bundle id
						boaWrite(wp,"<input id=\"edit1\" type=\"submit\" name=\"action\""
						" value=\"%s\" onclick=\"BundleAction('%s','%s', '%s');\">\n",next_action_func(status),name, next_action_func(status), id);
						// uninstall
						boaWrite(wp,"<input id=\"edit1\" type=\"submit\" name=\"action\""
						" value=\"Uninstall\" onclick=\"BundleAction('%s','%s', '%s');\">\n",name,"uninstall", id);
					}

					boaWrite(wp, "</td></font></tr>\n");
				}
				else
				{
					boaWrite(wp, "<tr><font size=1>"
					"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\">%s</td>\n"
					"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\">%s</td>\n"
					"<td width=\"40%%\" bgcolor=\"#C0C0C0\">%s</td>\n"
					"<td width=\"30%%\" bgcolor=\"#C0C0C0\">%s</td></font></tr>\n",
					id, status, name, version);					
				}
				memset(line,0,sizeof(line));
				//printf("%d %d %s %s\n" , b_desc[i].b_id,b_desc[i].b_status,b_desc[i].b_name,b_desc[i].b_ver);
		}
		boaWrite(wp, "</table>");

	}
	else
		boaWrite(wp,"%s","Retrieve Bundle List Error\n");

	fclose(fp);  // close OSGI_RESULT file
}

void formOsgiMgt(request * wp, char *path, char *query)
{
	char *act2do;
	char *act2id;
	char *act2name;
	char exec_cmd[512];	
	char stat[8];
	FILE *fp;
	char tmpBuf[128];
	int bnd_tbl,idx;
	MIB_CE_OSGI_BUNDLE_T osgiEntry;

	act2name = boaGetVar(wp, "bundle_name", "");
	act2do = boaGetVar(wp, "bundle_action", "");
	act2id = boaGetVar(wp, "bundle_id", "");

	debug_print("name %s , action : %s , id : %s \n", act2name , act2do, act2id);
	memset(exec_cmd,0,512);
	sprintf(exec_cmd,"/bin/osgi bundle %s %s\n", act2do,act2id);
	system(exec_cmd);

	if (!(fp=fopen(OSGI_RESULT_FILE, "r")))
	{
		debug_print("can't open UUID file\n");
		return;
	}
	fscanf(fp, "%s\n",stat);
	debug_print("stat:%s\n", stat);


	if(strncmp(stat, "SUCC", 4) == 0)
	{
		snprintf(tmpBuf,128, multilang(LANG_S_S_SUCCEED), act2name , act2do);
		OK_MSG1(tmpBuf,"/osgi_bndmgt.asp");
		MIB_CE_OSGI_BUNDLE_T entry;
		MIB_CE_OSGI_BUNDLE_T orig_entry;
		strcpy(entry.bundle_name , act2name);

		if(checkOsgiFlash(entry))
		{
			if(getOsgiFlash(entry , &orig_entry) == 1)
			{
				if(strcmp(act2do, "uninstall") == 0) // delete flash item
				{
					deleteOsgiFlash(orig_entry);
					// delete file 
					snprintf(exec_cmd,512, "rm %s%s\n" , JFFS2_PARTITION , orig_entry.bundle_file);
					debug_print("exec_cmd %s", exec_cmd);
					system(exec_cmd);
				}
				else
				{
					if(strcmp(act2do, "start") == 0 || strcmp(act2do, "Start") == 0 )
						orig_entry.bundle_action = 3; // start
					else if(strcmp(act2do, "stop") == 0 || strcmp(act2do, "Stop") == 0 )
						orig_entry.bundle_action = 1; // resolve
					orig_entry.bundle_id = atoi(act2id);
					updateOsgiFlash(orig_entry);
				}
				mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash
			}
		}
		else debug_print("osgi bundle not found ???\n");
		
	}
	else
	{
		snprintf(tmpBuf,128, multilang(LANG_S_S_FAILED),act2name, act2do);
		OK_MSG1(tmpBuf,"/osgi_bndmgt.asp");
	}
}


// find the start and end of the upload file.
FILE * _OSGIuploadGet(request *wp, unsigned int *startPos, unsigned *endPos, char fname[]) {

	FILE *fp=NULL;
	struct stat statbuf;
	unsigned char c, *buf;
	unsigned char fbuf[128];

	if (wp->method == M_POST)
	{
		fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);

		debug_print("file size=%d\n",statbuf.st_size);
		fp=fopen(wp->post_file_name,"rb");
		if(fp==NULL) goto error;
	}
	else goto error;

	while(fgets(fbuf,128,fp) != NULL)
	{
		char *pch;
		if((pch = strstr(fbuf,"filename=\""))!=NULL)
		{
			pch += strlen("filenam=\"") + 1 ; // move to file name first char
			int index = 0;
			while(*pch != '"')
				fname[index++] = *pch++;
			fname[index] = '\0';		
		}		
	}
	debug_print("fname : %s \n" , fname);
	fseek(fp,0,SEEK_SET);
	
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
			printf("Cannot find the end of the file!\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		break;
	}while(1);
	(*endPos)=ftell(fp);
	*endPos -= 6;  // Magician

   return fp;
error:
   return NULL;
}


void formOsgiUpload(request * wp, char *path, char *query)
{
	FILE *fp_upload = NULL;
	FILE *fp_save = NULL;
	FILE *fp_lb = NULL;
	struct stat statbuf;
	char ch;
	int fize=0;
	char filename[512];
	char bnd_file[512];
	char tmpBuf[128];
	unsigned int startPos, endPos, nLen;
	MIB_CE_OSGI_BUNDLE_T osgiEntry;
	int index;
	
	if (wp->method == M_POST)
	{
		int i;
		int tlen=0;


		if ((fp_upload = _OSGIuploadGet(wp, &startPos, &endPos,filename)) == NULL) {
			goto error;
		}
		nLen = endPos - startPos;
		tlen=nLen;
		//printf("filesize is %d\n", nLen);
		sprintf(bnd_file, "%s%s", BND_LOCATION, filename);
		fp_save = fopen(bnd_file, "w");
		fseek(fp_upload, startPos, SEEK_SET);	// seek to the data star
		
		char tmpBuf[128];
		char cmd[128];
		FILE *fp;
		int nRead,nToRead;

		nToRead = nLen;
		do {
			nRead = nToRead > sizeof(tmpBuf) ? sizeof(tmpBuf) : nToRead;
		
			nRead = fread(tmpBuf, 1, nRead, fp_upload);
			fwrite(tmpBuf, 1, nRead, fp_save);
			nToRead -= nRead;
		} while (nRead > 0);

		fclose(fp_upload);
		fclose(fp_save);

		memset(bnd_file,0,512);
		sprintf(bnd_file,"/bin/osgi bundle install file:%s%s\n", BND_LOCATION,filename);
		debug_print("execute command : %s \n" , bnd_file);
		system(bnd_file);
		
		char stat[8];
		if (!(fp=fopen(OSGI_RESULT_FILE, "r")))
		{
			debug_print("can't open RESULT file\n");
			return;
		}
		fscanf(fp, "%s\n",stat);

		if(strncmp(stat, "SUCC", 4) == 0)
		{
			fclose(fp);   // close result file
			snprintf(tmpBuf,128, multilang(LANG_S_INSTALL_SUCCEED), filename);
			OK_MSG1(tmpBuf,"/osgi_bndins.asp");
			// copy the tmp/xxx.jar to non-volatile disk (JFFS2_PARTITION)
			memset(tmpBuf,'\0',128);
			snprintf(tmpBuf,128,"cp %s%s %s%s",BND_LOCATION,filename,JFFS2_PARTITION,filename);
			system(tmpBuf);

			// call osgi bundle lb to retrieve bundle id & name
			memset(tmpBuf,'\0',128);
			snprintf(tmpBuf,128,"osgi bundle lb");
			system(tmpBuf);
			if (!(fp_lb=fopen(OSGI_RESULT_FILE, "r")))
			{
				debug_print("can't open RESULT file, add bundle to flash failed\n");
				fclose(fp);
				return;
			}
			char lb_buf[256];
			int bnd_cnt;
			// ignore first lines
			fgets(lb_buf,256,fp_lb);
			// get bundle count
			fscanf(fp_lb, "%d\n", &bnd_cnt);
			char line[1024];
			char *id,*status,*name,*version;
			char *saveptr1 = NULL;
			for(index = 0 ; index < bnd_cnt; index++)
			{
				fgets(line,1024,fp_lb);
				//printf("LINE : %s \n" , line);
				id =  strtok_r(line,"|", &saveptr1);
				status = strtok_r(NULL,"|", &saveptr1);
				name = strtok_r(NULL,"|", &saveptr1);
				version =  strtok_r(NULL,"|", &saveptr1);
			}
			// add flash item
			osgiEntry.bundle_action = 2 ; // installed
			strcpy(osgiEntry.bundle_file,filename);
			strcpy(osgiEntry.bundle_name, name);
			osgiEntry.bundle_id = atoi(id);

			if(checkOsgiFlash(osgiEntry) == 1)
			{
				// exist ! , update only
				updateOsgiFlash(osgiEntry);
			}
			else 
			{
				// add new entry
				addOsgiFlash(osgiEntry);
			}
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);	// update to flash
			debug_print("ACT:%d,Name:%s,ID:%d,FILENAME:%s\n",osgiEntry.bundle_action,
				osgiEntry.bundle_name,osgiEntry.bundle_id,osgiEntry.bundle_file);
			fclose(fp_lb);
		}
		else
		{
			fclose(fp);   // close result file
			snprintf(tmpBuf,128, multilang(LANG_S_INSTALL_FAILED), filename);
			OK_MSG1(tmpBuf,"/osgi_bndins.asp");
		}
	}
	else goto error;
	
	return;
error:
	debug_print("Got Error !!!\n");
	boaWrite(wp,"Error !!!\n", filename);
}


