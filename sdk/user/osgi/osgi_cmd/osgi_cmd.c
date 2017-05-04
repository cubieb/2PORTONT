#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define TCP_SERVER_PORT 11111
#define TCP_SERVER_IPADDR 
#define OSGI_CMD_VERSION "1.0.0"

static int prepare_osgi_command(char sbuff[], char *arg[], int arg_len)
{
	int len = 1;
	/*
	printf("arg[0] : %s \n" , arg[0]);
	printf("arg[1] : %s \n" , arg[1]);
	printf("arg[2] : %s \n" , arg[2]);
	printf("arg[3] : %s \n" , arg[3]);
	*/
	do
	{
		sprintf(&sbuff[strlen(sbuff)] , "%s ", arg[len]);
	}while(len++ < arg_len);
	
	// trim last "space" char
	
	sbuff[strlen(sbuff)-1] = '\n';
	
	return strlen(sbuff);
}


int main(int argc, char *argv[])
{
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	char sendBuff[1024];
	struct sockaddr_in serv_addr; 
	int send_len = 0;
	if(argc < 3)
	{
		printf("\n OSGi Commander V%s Usage: %s <bundle/tr069> <cmd> <id/path> \n",OSGI_CMD_VERSION,argv[0]);
		return 1;
	} 
	

	memset(recvBuff, '\0',sizeof(recvBuff));
	memset(sendBuff, '\0', sizeof(sendBuff));

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 
	
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(TCP_SERVER_PORT); 
	
	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	} 

	// prepare osgi command 
	send_len = prepare_osgi_command(sendBuff, argv, argc-1);
	//printf("OSGi command : %s , len : %d\n" , sendBuff, send_len);
	
	if (write(sockfd, sendBuff, send_len) == -1) 
	{
		printf("write to server error !");
		return 1;
	}
	
	
	if((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
		recvBuff[n] = 0;
	
	if(n < 0)
	{
		printf("\n Read error \n");
	}
	else printf("resp : %s \n" , recvBuff);
	

	close(sockfd);
	return 0;
}
