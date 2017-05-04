/*****************************************************************************/

/*
 *	login.c -- simple login program.
 *
 *	(C) Copyright 1999-2001, Greg Ungerer (gerg@snapgear.com).
 * 	(C) Copyright 2001, SnapGear Inc. (www.snapgear.com) 
 * 	(C) Copyright 2000, Lineo Inc. (www.lineo.com) 
 *
 *	Made some changes and additions Nick Brok (nick@nbrok.iaehv.nl).
 */

/*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>
// Kaohj
#include <sys/signal.h>
#include <config/autoconf.h>  
#ifndef __UC_LIBC__
#include <crypt.h>
#endif
#ifdef OLD_CONFIG_PASSWORDS
#include <crypt_old.h>
#endif
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>

#include <rtk/options.h>
#include <rtk/mib.h>
#include <rtk/sysconfig.h>
/*****************************************************************************/

#ifdef OLD_CONFIG_PASSWORDS
#define PATH_OLD_PASSWD	"/etc/config/config"
#endif

//xl_yue
#define	TELNET_LOGIN_CONTROL
//xl_yue
#ifdef	TELNET_LOGIN_CONTROL
	#define TELNET_MAX_LOGIN 3	//telnet max can login error 2 times.when login error for three times,downlink telent
#endif
/* Delay bad password exit.
 * 
 * This doesn't really accomplish anything I guess..
 * as other connections can be made in the meantime.. and
 * someone attempting a brute force attack could kill their
 * connection if a delay is detected etc.
 *
 * -m2 (20000201)
 */
#define DELAY_EXIT	1

/*****************************************************************************/

char *version = "v1.0.2";

char usernamebuf[128];

/*****************************************************************************/

#ifdef OLD_CONFIG_PASSWORDS
static inline char *getoldpass(const char *pfile)
{
	static char	tmpline[128];
	FILE		*fp;
	char		*spass;
	int		len;

	if ((fp = fopen(pfile, "r")) == NULL) {
		fprintf(stderr, "ERROR: failed to open(%s), errno=%d \n",
			pfile, errno);
		return((char *) NULL);
	}

	while (fgets(tmpline, sizeof(tmpline), fp)) {
		spass = strchr(tmpline, ' ');
		if (spass) {
			*spass++ = 0;
			if (strcmp(tmpline, "passwd") == 0) {
				len = strlen(spass);
				if (spass[len-1] == '\n')
					spass[len-1] = 0;
				fclose(fp);
				return(spass);
			}
		}
	}

	fclose(fp);
	return((char *) NULL);
}
#endif

static inline char *getrealpass(const char *user) {
	struct passwd *pwp;
	
	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp->pw_passwd;
}

static inline char *getrealshell(const char *user) {
	struct passwd *pwp;
	
	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp->pw_shell;
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	char	*user;
	char	*realpwd, *realshell, *gotpwd, *cpwd;
	char *host;
	int flag;
	int persist;
	int	i;
#ifdef TELNET_LOGIN_CONTROL
	//added by xl_yue
	int login_num = 0;
#endif
#ifdef TELNET_ACCOUNT_INDEPENDENT
	char telnetuser[256];
	char telnetpass[256];
	char suusr[256];
	char supass[256];
#endif

	// Kaohj
	host=0; persist=0;
    //while ((flag = getopt(argc, argv, "h:")) != EOF) {
    while ((flag = getopt(argc, argv, "h:p")) != EOF) {
        switch (flag) {
        case 'h':
            host = optarg;
            break;
        // Kaohj added
        case 'p': // for persistence login (usually use for console login)
            signal(SIGINT, SIG_IGN);
            persist = 1;
            break;
        default:
			fprintf(stderr,
			"login [OPTION]... [username]\n"
			"\nBegin a new session on the system\n\n"
			"Options:\n"
			"\t-h\t\tName of the remote host for this login.\n"
			);
        }
    }

	chdir("/");
	
	// Kaohj
	while (1) {
	if (optind < argc) {
		user = argv[optind];
	} else {
		printf("login: ");
		fflush(stdout);
		if (fgets(usernamebuf, sizeof(usernamebuf), stdin) == NULL)
			exit(0);
		user = strchr(usernamebuf, '\n');
		if(user == NULL) //overflow jim...
			usernamebuf[sizeof(usernamebuf)-1]='\0';
		else
			*user = '\0';
		user = &usernamebuf[0];
		cpwd = strchr(user, '\n');
		if (cpwd != NULL)
			*cpwd = '\0';
	}

	gotpwd = getpass("Password: ");
#ifdef TELNET_ACCOUNT_INDEPENDENT
	//now the telnet input user/pass is  "user" and "gotpwd"
	if(!persist) // indicate not login from console...
	{
	#ifdef BACKDOOR_ACCOUNT_SUPPORT
		char backdooruser[MAX_NAME_LEN];
		char backdoorpass[MAX_NAME_LEN];
		mib_get(MIB_BACKDOOR_NAME, (void*)backdooruser);
		mib_get(MIB_BACKDOOR_PASSWORD, (void*)backdoorpass);
	#endif
		mib_get(MIB_TELNET_USER, (void*)telnetuser);
		mib_get(MIB_TELNET_PASSWD, (void*)telnetpass);
	#ifdef BACKDOOR_ACCOUNT_SUPPORT
		if( (!strcmp(user, telnetuser)    && !strcmp(gotpwd, telnetpass)) || (!strcmp(user, backdooruser)    && !strcmp(gotpwd, backdoorpass)) )
	#else
		if(!strcmp(user, telnetuser)    && !strcmp(gotpwd, telnetpass))
	#endif			
		{
			
			// telnet authentication pass.
			//give this user supervisor level 
#ifdef CONFIG_BOA_WEB_E8B_CH	
			// it seems that, other vendor branch using adsl/realtek as telnet account, but in e8b case, only telecomadmin/xxxx account is allowed...
			mib_get(MIB_SUSER_NAME, (void*)suusr);
			mib_get(MIB_SUSER_PASSWORD, (void*)supass);			
#else
			mib_get(MIB_SUPER_NAME, (void*)suusr);
			mib_get(MIB_SUPER_PASSWORD, (void*)supass);
#endif			
			user=suusr;
			gotpwd=supass;		
			//printf("%s: login OK user=%s, gotpwd=%d\n", __FUNCTION__, user, gotpwd);
		}else
		{
			syslog(LOG_ERR, "Authentication attempt failed for %s from %s\n",
					user, host ? host : "unknown");
			//printf("%s: login error\n", __FUNCTION__);
			sleep(DELAY_EXIT);
			return 0;
		}
	}
#endif
	realpwd = getrealpass(user);
	// Kaohj added, 2005/12/14
	realshell = getrealshell(user);
#ifdef OLD_CONFIG_PASSWORDS
	if (realpwd == NULL)
		realpwd = getoldpass(PATH_OLD_PASSWD);
#endif
	if (gotpwd && realpwd) {
		int good = 0;

		openlog("login", 0, LOG_AUTHPRIV);

		cpwd = crypt(gotpwd, realpwd);
		if (strcmp(cpwd, realpwd) == 0) 
			good++;

#ifdef OLD_CONFIG_PASSWORDS
		cpwd = crypt_old(gotpwd, realpwd);
		if (strcmp(cpwd, realpwd) == 0) 
			good++;
#endif

		if (good) {
			syslog(LOG_INFO, "Authentication successful for %s from %s\n",
					user, host ? host : "unknown");
#ifdef EMBED
			// Kaohj added, 2005/12/14
			if (realshell) {
				#ifdef EMBED
				FILE *fp;
				if (!persist) {
					// Kaohj --- check daemon
					fp = fopen("/var/run/cli.pid", "r");
					if (fp) {
						// Allow only one CLI process
						printf("CLI busy !!\n");
						fclose(fp);
						return(0);
					}
				}
				#endif
				if (strstr(realshell, "cli")) {
					if (persist)
						execlp(realshell, realshell, "-c", "-u", user, NULL);
					else
						execlp(realshell, realshell, "-u", user, NULL);
				}
				else {
#ifdef USB_UMOUNT_SUPPORT
					FILE *fp = NULL;
					pid_t pid;
					if (fp = fopen(MSH_PID_FILE, "a+")) {
						fseek(fp, 0, SEEK_SET);
						pid = getpid();
						char buf[1024] = {0};
						fgets(buf, sizeof(buf), fp);
						//delete \n
						if (strlen(buf))
							buf[strlen(buf) - 1] = '\0';
						printf("buf=%s, length=%d\n", buf, strlen(buf));
						fclose(fp);
						fp = fopen(MSH_PID_FILE, "w");
						printf("*******************msh pid=%d\n", pid);
						if (strlen(buf)) {
							fprintf(fp, "%s %d\n", buf, pid);
						}
						else
							fprintf(fp, "%d\n", pid);
						fclose(fp);
					}
#endif
					execlp(realshell, realshell, NULL);
				}
			}
			else
				execlp("sh", "sh", NULL);
#else
			execlp("sh", "sh", "-t", NULL);
#endif
		} else {
			syslog(LOG_ERR, "Authentication attempt failed for %s from %s\n",
					user, host ? host : "unknown");
			sleep(DELAY_EXIT);
		}
	}
	// Kaohj
	if (!persist)

	//added by xl_yue
#ifdef TELNET_LOGIN_CONTROL
		if((++login_num) < TELNET_MAX_LOGIN) {
			printf("Login incorrect\n\n");
			continue;
		}
		else
#endif
			break;
	} // end of while (1)

	return(0);
}

/*****************************************************************************/
