#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>

#define SOLAR_FIFO	"/var/run/solar_watchdog.fifo"
#define SOLAR_NAME	"solar"

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
	
	if (access(SOLAR_FIFO, F_OK) == -1)
	{
		if (mkfifo(SOLAR_FIFO, 0755) == -1)
		{
			fprintf(stderr, "access %s failed: %s\n", SOLAR_FIFO, strerror(errno));
			exit(0);
		}
	}
	
	time(&t);
	fprintf(stderr, "waiting %s start...: %s\n", SOLAR_NAME, ctime(&t));
	h_pipe = open(SOLAR_FIFO, O_RDONLY);
	if (h_pipe == -1)
	{
		fprintf(stderr, "open %s failed: %s\n", SOLAR_FIFO, strerror(errno));
		exit(0);
	}

	if (argc == 2)
	{
		timeout_in = atoi(argv[1]);
	}

	while (1)
	{
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

		if (ret == 0) // timeout
		{
			// stop solar, and restart it on EOF event
			fprintf(stderr, "timeout! stop %s...\n", SOLAR_NAME);
			sprintf(command, "killall %s", SOLAR_NAME);
			if (system(command) == -1)
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
					fprintf(stderr, "%s is stopped\n", SOLAR_NAME);
					close(h_pipe);

					if (timeout_in == -1)
					{
						printf("not restart version.\n");
						exit(0);
					}

					// restart solar
					sprintf(command, "%s &", SOLAR_NAME);
					if (system(command) == -1)
					{
						fprintf(stderr, "exec %s failed: %s\n", command, strerror(errno));
						exit(0);
					}

					time(&t);
					fprintf(stderr, "waiting %s start...: %s\n", SOLAR_NAME, ctime(&t));
					h_pipe = open(SOLAR_FIFO, O_RDONLY);
					if (h_pipe == -1)
					{
						fprintf(stderr, "open %s failed: %s\n", SOLAR_FIFO, strerror(errno));
						exit(0);
					}					
				}
				else if (ret == -1)
				{
					fprintf(stderr, "read %s failed: %s\n", SOLAR_FIFO, strerror(errno));
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
