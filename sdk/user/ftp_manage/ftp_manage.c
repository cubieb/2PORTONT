#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "debug.h"

char bftpd_pid[42];
char bftpd_enable[42];
char bftpd_sel[42];

char *get_config_opt(FILE *fp_config, char *buf, const char *opt_name)
{
	char *ret;
	ret =NULL;
	if (fp_config == NULL)
	{
		DEBUG_PRINT("fp_config=NULL\n");
		return ret;
	}

	fseek(fp_config, 0, SEEK_SET);

	while ( fgets(buf, 40, fp_config) )
	{
		DEBUG_PRINT("fgets:%s\n", buf);
		if ( (strncmp(buf, opt_name, strlen(opt_name)) == 0) && (buf[strlen(opt_name)] == '=') )
		{
			ret = &(buf[strlen(opt_name)+1]);
			if ( (*ret) == 0x0a )
			{
				ret = NULL;
			}
			else
			{
				DEBUG_PRINT("get the opt: %s=%s\n", opt_name, ret);
			}
			break;
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	FILE *fp_config=NULL;
	FILE *fp_map=NULL;
	char *bftpd_pid_value;
	char *bftpd_enable_value;
	char *bftpd_sel_value;
	unsigned int usb_mask;
	unsigned int usb_sel;
	int ipid;
	long pos;
	char command[64];
	//char *myopt[3] = {NULL, "-d", NULL};
	char *myopt[3] = {NULL, "-D", NULL};
	char *myprog = "/bin/bftpd";
	char *myenv[2] = {"PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin",NULL};
	unsigned int i;
	int retval;
	int mode=0;

	//cdh_debug_init();

	/* xl_yue:20080728 add options for kill or start bftpd*/
	while ((retval = getopt(argc, argv, "sk")) > -1) {
		switch (retval) {
			case 'k': 
				mode = 0; 
				DEBUG_PRINT("mode=0,kill bftpd\n");
				break;	//kill bftpd process
			case 's': 
				mode = 1; 
				DEBUG_PRINT("mode=1,start bftpd\n");
				break;	//start bftpd process
		}
	}
	/* xl_yue:20080728*/
	
	fp_config = fopen("/var/run/bftpd/ftpdconfig", "r+");
	if (fp_config == NULL)
	{
		DEBUG_PRINT("Fail to open \"/var/run/bftpd/ftpdpassword\"!\n");
		cdh_debug_cleanup();

		// additional operation *****
		DEBUG_PRINT("exit ftp_manage for fp_config==NULL\n");
		return -1;
	}
	DEBUG_PRINT("%s\n", "main...\n");
	// check the bftpd is exist?
	bftpd_pid_value = get_config_opt(fp_config, bftpd_pid, "bftpd_pid");
	DEBUG_PRINT("bftpd_pid_value=%s\n", bftpd_pid_value);

	// check the bftpd is enable?
	bftpd_enable_value = get_config_opt(fp_config, bftpd_enable, "ftp_enable");
	DEBUG_PRINT("bftpd_enable_value=%s\n", bftpd_enable_value);

	// get the usb selection?
	bftpd_sel_value = get_config_opt(fp_config, bftpd_sel, "ftp_root");
	DEBUG_PRINT("bftpd_sel_value=%s\n", bftpd_sel_value);

	if (sscanf(bftpd_sel_value, "%x", &usb_sel) < 0)
	{
		DEBUG_PRINT("Fail to convert string to hex integer\n");
		fclose(fp_config);
		fp_config=NULL;
		cdh_debug_cleanup();
		// additional operation *****
			DEBUG_PRINT("exit ftp_manage for sscanf bftpd_sel_value error\n");
		return -1;
	}

	DEBUG_PRINT("usb_sel=%x\n", usb_sel);

    /* lijian: 20080716 bug-id 3649 START: when usb changed, kill bftpd first */
	if ( (bftpd_pid_value) && (bftpd_pid_value[0] != 0) )
	{
		if (sscanf(bftpd_pid_value, "%d", &ipid) < 0)
		{
			DEBUG_PRINT("Fail to convert ipid to integer\n");
		}
		else
		{
			char strtemp[64];
			DEBUG_PRINT("kill ... %d...\n", ipid);
			/* kill(ipid, SIGKILL); */
			kill(ipid, SIGTERM);
			
			DEBUG_PRINT("kill bftpd process,pid=%d\n",ipid);
			
			fseek(fp_config, 0, SEEK_SET);
			pos = ftell(fp_config);
			while ( fgets(strtemp, 62, fp_config) )
			{
				if ( ! strncmp(strtemp, "bftpd_pid=", 10) )
				{
					fseek(fp_config, pos, SEEK_SET);
					fprintf(fp_config, "bftpd_pid=\n\n");
					fflush(fp_config);
					DEBUG_PRINT("change bftpd_pid=\n");
					break;
				}
				
				pos = ftell(fp_config);
			}
		}

	    	// close the file 
	    	fclose(fp_config);
	    	fp_config=NULL;        
	        
	        /* wait usb mount/unmout, 2 seconds */
		/* xl_yue:20080728 donot wait*/
	        //usleep(2000000);
	        /* xl_yue:20080728*/
	}
    /* lijian: 20080716 bug-id 3649 END */

	/* xl_yue:20080728 start bftpd if mnt_map is valid*/
	if(mode == 1){
		unsigned int retry=0;
    		if ((bftpd_enable_value != NULL) && ( strncmp(bftpd_enable_value, "0", 1) )){
			/*xl_yue: 20080730 start/kill bftpd is independant of usb mount*/
			#if 0
			while(1){
				fp_map = fopen("/tmp/usb/mnt_map", "r");
				if(fp_map == NULL){
					if(++retry > 5) break;
					usleep(1000000);
					continue;
				}else{
					fscanf(fp_map, "%x", &usb_mask);
					DEBUG_PRINT("%x", usb_mask);
					fclose(fp_map);
					fp_map = NULL;
					if(usb_mask == 0){
						if(++retry > 5) break;
						usleep(1000000);
						continue;
					}
			#endif
			/*xl_yue: 20080730 */	
				
					DEBUG_PRINT("execv bftpd process\n");
					// start the bftpd
					sprintf(command, "/bin/ln -s -f /mnt /var/bftpd_root");
					system(command);
					myopt[0] = myprog;
					if(fork() == 0){
						execv(myprog, myopt, myenv);
						exit(0);
					}
			/*xl_yue: 20080730 mark this code for start/kill bftpd is independant of usb mount*/
			#if 0
					else{
						break;
					}
				}
			}
			#endif
			/*xl_yue:20080730*/
		}
	}
	/*xl_yue:20080728*/

/* xl_yue:20080728 mart the following code*/	
#if 0	
	// check the usb1 is enable?
	// check the usb2 is enable?
	// check the usb... is enable?
	fp_map = fopen("/tmp/usb/mnt_map", "r");
	if (fp_map == NULL)
	{
	/*
		DEBUG_PRINT("Fail to open \"/tmp/usb/mnt_map\"!\n");
		fclose(fp_config);
		fp_config=NULL;
		cdh_debug_cleanup();
		return -1;
	*/
		usb_mask = 0;
	}
	else
	{
		fscanf(fp_map, "%x", &usb_mask);
		DEBUG_PRINT("%x", usb_mask);
		fclose(fp_map);
		fp_map = NULL;
	}

    if ( (bftpd_enable_value != NULL)
		   && ( strncmp(bftpd_enable_value, "0", 1) )
	    )
	{
        if (!(usb_mask & (1 << usb_sel)))
        {
    		/* search 'usb_sel' */
    		usb_sel = 32;
    		for (i=0; i<32; i++)
    		{
    			if ( usb_mask & (1 << i) )
    			{
    				usb_sel = i;
    				break;
    			}
    		}
        }

		if (usb_sel >= 32)
		{
			/* Nothing to do */
		}
		else
		{
			sprintf(command, "/bin/ln -s -f /mnt /var/bftpd_root");
			system(command);

			myopt[0] = myprog;
			// start the bftpd
			execv(myprog, myopt);

			// start the bftpd
			// system("/bin/bftpd -d");
		}
	}
#endif
/* xl_yue:20080728 */

	cdh_debug_cleanup();

	DEBUG_PRINT("exit ftp_manage process\n");
	
	return 0;
}

