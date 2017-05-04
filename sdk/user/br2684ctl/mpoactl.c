#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <ctype.h>

#define SERVER_FIFO_NAME "/tmp/serv_fifo"
#define CLIENT_FIFO_NAME "/tmp/cli_fifo"
#define BUFFER_SIZE	64

struct data_to_pass_st {
	int	id;
	char data[BUFFER_SIZE];
};


int main(int argc, char **argv)
{
	int server_fifo_fd;
	struct data_to_pass_st msg;
	int i;
	char 	client_fifo[256];
	char *buf;
	server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
	if(server_fifo_fd == -1) {
		fprintf(stderr, "Sorry, no server\n");
		exit(EXIT_FAILURE);
	}
	
	msg.id = 2684;
	buf = msg.data;
	for(i=0; i<argc; i++) {
		sprintf(buf, "%s", argv[i]);
		buf[strlen(argv[i])] = 0x20;
		buf += (strlen(argv[i])+1);
	}
	buf--;
	*buf = '\0';
	write(server_fifo_fd, &msg, sizeof(msg));
	close(server_fifo_fd);
	exit(EXIT_SUCCESS);
}

		

