#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#define SERV_PORT 		9999
#define SIG_PORT 		2313
#define MAXLENGTH 		1224


#define INADDR_SERVER ((unsigned long int)0x0AFDFD02)

//#define DEBUG_ICC
#ifdef DEBUG_ICC
#define dbg(fmt, args...) printf(fmt, ##args)
#else
#define dbg(fmt, args...) {}
#endif

int main(int argc, char *argv[])
{
	int fd;   //file description into transport
	char buf[MAXLENGTH];
	struct sockaddr_in myaddr; // address of this service
	struct sockaddr_in srv_addr; // address of server                            
	int i , size;
	char cmd_str[MAXLENGTH];
	//string cat to single string command
	if(argc <= 1)
	{
		printf("Error ?! please specific your command to execute ! \n");
		return -1;
	}

	memset(cmd_str , 0  , MAXLENGTH);
	strcpy(cmd_str , argv[1]); // copy first string , such as 'ls -al' -> ls
	cmd_str[strlen(cmd_str)] = '\0'; // append \0 end of string
	for(i = 2; i < argc ; i++)
	{
		cmd_str[strlen(cmd_str)] = ' '; // append \0 end of string
		strcpy(&cmd_str[strlen(cmd_str)] , argv[i]);
	}
	cmd_str[strlen(cmd_str)] = '\r'; // append \n end of string
	cmd_str[strlen(cmd_str)] = '\n'; // append \n end of string
	cmd_str[strlen(cmd_str)] = '\0'; 
	dbg("your command is : %s \n" , cmd_str);
	//Get a socket into UDP/IP
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
		perror ("socket failed");
		exit(1);
 	}

	
	//Set up our address
	bzero ((char *)&myaddr, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	bzero ((char *)&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_SERVER);
	srv_addr.sin_port = htons(SERV_PORT);


	//Bind to the address to which the service will be offered
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) <0) {
		printf ("bind failed\n");
		exit(1);
	}

	
	 /* send user command to slave cpu*/
	size = sizeof(srv_addr);
	if (sendto(fd, cmd_str, strlen(cmd_str), 0, &srv_addr, size) == -1) 
	{
		printf("Send command failed !");
		exit(1);
	}
	
	dbg("ICC:send %s (%d) \n" , cmd_str, strlen(cmd_str));

	
	/* receive slave cpu response */
	while(1)
	{
		if (recvfrom(fd, buf, MAXLENGTH, 0, &srv_addr, &size) < 0) 
		{
			printf ("Receive error !");
			break;
		}
		
		if(memcmp(buf, "COMPLETE" , 8) ==0)
		{
			break;
		}
	}
	
	close(fd);
	
	return 1;
}

