#include "netlogger.h"
#include "rtk/utility.h"
#include "rtk/options.h"

static struct  sockaddr_in data_source;
char *netlog_recv_buf;
static int netlog_fd;
fd_set netlog_in_fds;		/* set of fds that wait_input waits for */
int netlog_max_in_fd;		/* highest fd set in in_fds */
static int netlogPollflag;
static int enable_netlogger;
static struct netlogIO ackIO;
static struct netlogIO cmdIO;
#define		RUNFILE "/var/run/netlogger.pid"
 
 
#ifdef _USE_RSDK_WRAPPER_
static void netlog_ack(int acktype,int fd);
static void netlog_bitReverse(char *buf,int buflen);
static void   netlog_stopLog(void);
#endif //_USE_RSDK_WRAPPER_


static int netlog_parseInputCmd(const char* buf, int len)
{     
       int ret;
	   
#if NETLOG_DEBUG
     	printf("netlog_parseInputCmd---\n");
#endif
	const char *bufp=buf;

	if(!memcmp(bufp, "CMD:START", 9))
		ret=NETLOGSTART;
	
	else if(!memcmp(bufp, "CMD:STOP", 8))
		ret=NETLOGSTOP;
	
	else if(!memcmp(bufp, "INPUT:", 6))
		ret=NETLOGINPUT;
	
	else
		if(!memcmp(bufp,"SIG:POL",7))
		 ret=NETLOGPOLL;
		
	else
		ret=0;
	
	#if NETLOG_DEBUG
	printf("return value %d\n",ret);
	#endif
	
	return ret;	
}   

static int netlog_handleInput(const char* buf, int buflen)
{
   
#if NETLOG_DEBUG
      printf("netlog_handleInput---\n");
#endif

     
      int fd ; //serial port open
      int cmd;
      int i ;  
      if((fd = open ("/dev/ttyS0",O_RDWR|O_NONBLOCK))<0){
    	    exit(1) ;
      }

	 memset((void *)&cmdIO, 0, sizeof(cmdIO));  

      netlog_bitReverse(buf,buflen);
	  

      switch(netlog_parseInputCmd(buf,buflen))
     {
        case NETLOGSTART:

			 cmd =1;
			 
                      if( ioctl(fd,NLGSETNETLOGGER,&cmd)<0){

                         printf("ioctl NLGSENTLOGGER error\n");
		           return 0;
				   
    	               }
					  
			netlog_ack(NETLOGSTART,fd);
			
			break;
			
	  case NETLOGSTOP:

                      
		      netlog_ack(NETLOGSTOP,fd);
	  	      break;
			  
         case NETLOGINPUT:/*INPUT:+DATA*/
#if NETLOG_DEBUG	
			 printf("NETLOGINPUT\n");
#endif

               for( i = 0;i <buflen;i++)
			 {
			    cmdIO.netlog_recv_buf[i]=buf[i];
			 }

			 cmdIO.buflen=buflen;
			 
	   	     if( ioctl(fd,NLGSENTDATA,&cmdIO)<0){
                         usleep(60);
               if(ioctl(fd,NLGSENTDATA,&cmdIO)<0){  
			  printf("ioctl NLGSENDDATA error\n");
		         return 0;
			  }
		        
				 
    	              }

			 break;
						
	 case NETLOGPOLL:
	 	       netlogPollflag=1;
	 	  
			  if(ioctl(fd,NLGSENDPOLL,&netlogPollflag)<0)
    	            {
    	                printf("ioctl NLGSENDPOLL error\n");
		         return 0;
    	            }
			  
			break;
			
	  default:
		      break;	
    }

	close(fd);
	
       return 1;
}


static void netlog_ack(int acktype,int fd)
{

     char buf[10];	
     int buflen=0;
  
     char BufStart[9]="ACK:START";
     char BufStop[8]="ACK:STOP";
#if NETLOG_DEBUG
       printf("netlog_ack---\n");
#endif

      switch(acktype)
     {			
	    case NETLOGSTART:

//			memcpy(buf,"ACK:START",9);
			buflen=9;
			netlog_handleOutput(BufStart,buflen,fd);
			break;
			
	    case NETLOGSTOP:

//			memcpy(buf,"ACK:STOP",8);
			buflen=8;
            netlog_handleOutput(BufStop,buflen,fd);
                     break;
					 
	    default:
			break;
    }
   
//	netlog_handleOutput(buf,buflen,fd);	
}

static void netlog_bitReverse(char *buf,int buflen)
{
      char *tempbuf;
      tempbuf=(char *)buf;
	  
      while(buflen-->0){
		*tempbuf=(*tempbuf) ^0xFF;
	        tempbuf++;
       };
}

static int  netlog_handleOutput(const char* buf,int buflen,int fd)
{
     int length;
 
     int retry = 0;
	 
#if NETLOG_DEBUG
     printf("netlog_handleOutput---\n");
#endif
     memset((void *)&ackIO, 0, sizeof(ackIO));
     memcpy(ackIO.netlog_recv_buf,buf,buflen);
     ackIO.buflen=buflen;
   //open serial port
 
    if( ioctl(fd,NLGSENDCMD,&ackIO)<0){

         usleep(40);
         if(ioctl(fd,NLGSENDCMD,&ackIO)<0)
      {	 
    	     printf("ioctl NLGSENDCMD error\n");
		  }
     return 0;
		 
    	}
 
    return 1;
}

 void hup(int signum)
{  
    enable_netlogger=0;
	
    netlog_stopLog();
	
    exit(EXIT_SUCCESS);
}
 
static int netlog_init()
{
       int retry = 0;
       int netlog_retval;
       struct ifreq ifr;
       struct sockaddr_in *addr;
	int fd;
	 //read src mac
	 unsigned char value[6];
	 //read ip address 
	 struct in_addr origIp;
	  int cmd ;
	
      if((fd = open ("/dev/ttyS0",O_RDWR|O_NONBLOCK))<0){
    	    exit(1) ;
      }
	   
      netlog_recv_buf = malloc(NETLOG_RECV_BUF_SIZE);
	  
      if(netlog_fd>0){
	  	printf("has been open netlog_fd\n");
		exit(1);
      	}
	  	
      if((netlog_fd= socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP))==1){
 		printf("socket error\n");
	       exit(1);
 	}
	  
      bzero(&data_source, sizeof(struct sockaddr_in));
	  
      if (ioctl(netlog_fd, SIOCGIFADDR, &ifr) == 0) {
		addr = ((struct  sockaddr_in*)(&ifr.ifr_addr));
                data_source.sin_addr = *((struct in_addr *)&addr->sin_addr);		
	}
	
        data_source.sin_family= AF_INET;
  //      data_source.sin_addr.s_addr=htonl(0xc0a80101);
        
	 data_source.sin_port=htons(NETLOGPORT);
	   
	if((bind (netlog_fd, (struct sockaddr *)&data_source, sizeof(struct sockaddr)))== -1){

		printf("bind error\n");
		exit(1);
		
	}
	
	 mib_get(MIB_ADSL_LAN_IP, (void *)&origIp);
	
	 unsigned int cmdip =  origIp.s_addr;
	  printf("netlogIp = %x \n",cmdip);
	  
	 if( ioctl(fd,NLGSETIP,&cmdip)<0){
    	      printf("ioctl NLGSETNETLOGGER error\n");
	      return 0;
    	  }

	
      if (mib_get(MIB_ELAN_MAC_ADDR, (void *)value) != 0)
      	{
      	   if( ioctl(fd,NLGSETMAC,value)<0){
    	      printf("ioctl NLGSETNETLOGGER error\n");
	      return 0;
    	  }
      	}
	 
	  cmd =1;
  
        if( ioctl(fd,NLGSETNETLOGGER,&cmd)<0){
    	      printf("ioctl NLGSETNETLOGGER error\n");
	      return 0;
    	   }
		
	 close (fd);
	 enable_netlogger =1 ;
        return(1);
}

static void netlog_startLog(void)
{
        /*set flag 1 for True*/
        int cmd =1;
	 int fd;
        if((fd = open ("/dev/ttyS0",O_RDWR|O_NONBLOCK))<0){
    	    exit(1) ;
        }
        
       if( ioctl(fd,NLGSETNETLOGGER,&cmd)<0){
	   	
    	  printf("ioctl NLGSENDCMD error\n");
		 return ;
		 
    	 }
	   
       close(fd);

}
static void   netlog_stopLog(void)
{
    /*set flag 0 for false*/
 //   start_netlog_tx(0);
       int cmd =0;
	
	int fd;
	
       fd = open("/dev/ttyS0",O_RDWR|O_NONBLOCK);
	   
       ioctl(fd,NLGSETNETLOGGER,&cmd);
	   
       close (fd);
	   
       close(netlog_fd);
	   
       free(netlog_recv_buf);
	   
       netlog_fd=0;
}




/*

 * add_fd - add an fd to the set that wait_input waits for.

 */

static void add_fd(int fd)

{

    FD_SET(fd, &netlog_in_fds);

    if (fd > netlog_max_in_fd)

	netlog_max_in_fd = fd;

}


int main(int argc, char **argv)
{

	int pid ;
	
	char **parse;
	FILE *pidfile;


       if(argc>1){

          argv++;
		  
	   parse = argv;
	   
	   if (!strcmp(*parse, "disable")) {

		enable_netlogger=0;
              netlog_stopLog();
		return 0 ;
	   }

	   return 0;
      	}
  
       if( pid = fork() ) 
            exit(0);
            
       else if( pid < 0 )
	     exit(1); 
		
       if( setsid() < 0)
	     exit(1);
     
	   
       netlog_init();
  //    netlog_startLog();
	

       signal(SIGHUP, hup);

	signal(SIGTERM, hup);


	add_fd(netlog_fd);

	
	{
	 int flags;

	 flags = fcntl(netlog_fd, F_GETFL);

	 if (flags == -1 || fcntl(netlog_fd, F_SETFL, flags | O_NONBLOCK) == -1)

	   	printf("Couldn't set netlog_fd to nonblock\n");

	}

	/* write pidfile */
	if ((pidfile = fopen(RUNFILE, "w"))) {
		fprintf(pidfile, "%d\n", (int) getpid());
		fclose(pidfile);
	}

	/* process loop */

	while(enable_netlogger)

	{

		fd_set in;

		struct timeval tv;

		int ret;

		int recvlen;

		tv.tv_sec = 0;

		tv.tv_usec = 200000;//200 ms

		in = netlog_in_fds;

		ret = select(netlog_max_in_fd+1, &in, NULL, NULL, &tv);



		if( ret <= 0 ){

			continue;

		} 	

		if(FD_ISSET(netlog_fd, &netlog_in_fds)) {

			recvlen = recvfrom(netlog_fd, netlog_recv_buf, NETLOG_RECV_BUF_SIZE,

				   0, NULL, &recvlen);

			if (recvlen < 0) {
				
                     printf("error");

		    	continue;
			}

                    netlog_handleInput(netlog_recv_buf,recvlen);
		
	    }
		

	}

	return 0;


}


