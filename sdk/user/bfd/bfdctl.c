#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <ctype.h>
#if 0
#include <rtk/utility.h>
#else
#define BUFFER_SIZE	256

struct data_to_pass_st {
	int	id;
	char data[BUFFER_SIZE];
};
#endif

#define BFD_SERVER_FIFO_NAME "/tmp/bfd_serv_fifo"
//#define BFD_CLIENT_FIFO_NAME "/tmp/bfd_cli_fifo"

int main(int argc, char **argv)
{
	int server_fifo_fd;
	struct data_to_pass_st msg;
	int i;
	int read_res;
	char *buf;
	
	server_fifo_fd = open(BFD_SERVER_FIFO_NAME, O_WRONLY|O_NONBLOCK);
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

