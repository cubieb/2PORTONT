#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>

#define RCM_WDT_FIFO	"/var/run/solar_watchdog.fifo"
#define RCM_WDT_LOG	"/var/log/maserati_wdt.log"
#define RCM_NAME	"solar"
#define RCM_PID		"/var/run/solar.pid"


#define RCM_WDT_LOG_MAX_SIZE (16*1024) //16KB
static void rotate_log_file(int inc_size)
{
	static unsigned int total_size = 0;

	total_size += inc_size;

	if(total_size > RCM_WDT_LOG_MAX_SIZE)
	{
		rename(RCM_WDT_LOG, RCM_WDT_LOG ".old");
		total_size = 0;
	}
}

int main(int argc, char *argv[])
{
	int i, ret;
	fd_set fdset;
	int h_pipe;
	struct timeval timeout;
	char command[256];
	static time_t t;
	int restart_cnt = 0;
	int timeout_in = 0; // default: don't check timeout 
	
	if (access(RCM_WDT_FIFO, F_OK) == -1)
	{
		if (mkfifo(RCM_WDT_FIFO, 0755) == -1)
		{
			fprintf(stderr, "access %s failed: %s\n", RCM_WDT_FIFO, strerror(errno));
			exit(0);
		}
	}
	
	time(&t);
	fprintf(stderr, "waiting %s start...: %s\n", RCM_NAME, ctime(&t));
	h_pipe = open(RCM_WDT_FIFO, O_RDONLY);
	if (h_pipe == -1)
	{
		fprintf(stderr, "open %s failed: %s\n", RCM_WDT_FIFO, strerror(errno));
		exit(0);
	}

	if (argc == 2)
	{
		timeout_in = atoi(argv[1]);
	}

	while (1)
	{
		char time_info_buf[26] = {0}; //ctime returns a 26-char array

		FD_ZERO(&fdset);
		FD_SET(h_pipe, &fdset);

		if (timeout_in <= 0)
		{
			ret = select(h_pipe + 1, &fdset, NULL, NULL, NULL);
		}
		else
		{	
			timeout.tv_sec = timeout_in;
			timeout.tv_usec = 0;
			ret = select(h_pipe + 1, &fdset, NULL, NULL, &timeout);
		}

		time(&t);
		//strip the trailing newline and null characters
		memcpy(time_info_buf, ctime(&t), 24);

		if (ret == 0) // timeout
		{
			snprintf(command, 256, "echo %s: timeout! stop " RCM_NAME " >> " RCM_WDT_LOG, time_info_buf);
			fprintf(stderr, "%s\n", command);
			rotate_log_file(strlen(command));
			if (system(command) == -1)
			{
				fprintf(stderr, "exec %s failed: %s\n", command, strerror(errno));
				exit(0);
			}
			
			// stop solar, and restart it on EOF event
			fprintf(stderr, "timeout! stop %s...\n", RCM_NAME);

			//sprintf(command, "killall %s", RCM_NAME);
			//sprintf(command, "kill `cat %s`", RCM_PID);
			//if (system(command) == -1)
			if (system("kill `cat " RCM_PID "`") == -1)
			{
				fprintf(stderr, "exec %s failed: %s\n", command, strerror(errno));
				exit(0);
			}
		}
		else if (ret > 0)
		{
			// data in
			if (FD_ISSET(h_pipe, &fdset))
			{
				char buffer[100];
				int ret;

				// NOTE: If buffer is smaller than fifo, then select will trigger again
				ret = read(h_pipe, buffer, sizeof(buffer));
				if (ret == 0)
				{
					fprintf(stderr, "%s is stopped\n", RCM_NAME);
					close(h_pipe);

					if (timeout_in == -1)
					{
						printf("not restart version.\n");
						exit(0);
					}

					snprintf(command, 256, "echo %s: " RCM_NAME " is stopped, restart it >> " RCM_WDT_LOG, time_info_buf);
					fprintf(stderr, "%s\n", command);
					rotate_log_file(strlen(command));
					if (system(command) == -1)
					{
						fprintf(stderr, "exec %s failed: %s\n", command, strerror(errno));
						exit(0);
					}

					// restart solar
					sprintf(command, "%s &", RCM_NAME);
					if (system(command) == -1)
					{
						fprintf(stderr, "exec %s failed: %s\n", command, strerror(errno));
						exit(0);
					}

					time(&t);
					fprintf(stderr, "waiting %s start...: %s\n", RCM_NAME, ctime(&t));
					h_pipe = open(RCM_WDT_FIFO, O_RDONLY);
					if (h_pipe == -1)
					{
						fprintf(stderr, "open %s failed: %s\n", RCM_WDT_FIFO, strerror(errno));
						exit(0);
					}					
				}
				else if (ret == -1)
				{
					fprintf(stderr, "read %s failed: %s\n", RCM_WDT_FIFO, strerror(errno));
					exit(0);
				}
			}
		}
		else 
		{
			fprintf(stderr, "select failed: %s\n", strerror(errno));
			exit(0);
		}
	}
	
	return 0;
}
