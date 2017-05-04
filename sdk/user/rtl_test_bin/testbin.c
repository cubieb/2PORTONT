#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define MAX_MSGSIZE 1024

//currently , we have nas0, eth0.2, ... eth0.5
#define DVE_NIC_PORT_MAX_NUM 5

#define AUG_PRT(fmt,args...)  printf("\033[1;33;46m<%s %d %s> \033[m"fmt, __FILE__, __LINE__, __func__ , ##args)

const char* dev[] = {"nas0", "eth0.2", "eth0.3", "eth0.4", "eth0.5", ""};

//copy my dumpdata func of 'tcpudptool' at here, it is a long time ago, missing...
void DumpData(const void* Buffer, int Length, int StartLabel)
{   
    //WINDOW *win = pwin;
    unsigned char *CurrentPointer = (unsigned char *)Buffer, LocalBuffer[17];
    int CurrentIndex;
    TODO:
    //I think I should add a timer here~
    //wprintw(win ,"Receive %d byte Data, Dump it in HEX and ASCII", Length);
    printf("Receive %d byte Data, Dump it in HEX and ASCII", Length);
    for (CurrentIndex = (StartLabel & ~0xF) - StartLabel;
        StartLabel + CurrentIndex < ((StartLabel + Length - 1) & ~0xF) + 16;
        CurrentIndex++)
    {
        int Offset = (StartLabel + CurrentIndex) & 0xF;
        if (Offset == 0)
            //wprintw(win, "\n0x%04X: ", StartLabel + CurrentIndex);
            printf("\n0x%04X: ", StartLabel + CurrentIndex);
        if (CurrentIndex < Length && CurrentIndex >= 0)
        {
            LocalBuffer[Offset] = *CurrentPointer++;
            //wprintw(win, "%02X ", LocalBuffer[Offset]);
            printf("%02X ", LocalBuffer[Offset]);
            if (LocalBuffer[Offset] < 0x20 || LocalBuffer[Offset] >= 0x7f)
                LocalBuffer[Offset] = '.';
        }
        else
        {
            //wprintw(win, "   ");
            printf("   ");
            LocalBuffer[Offset] = ' ';
        }
        if (Offset == 0xF)
        {
            LocalBuffer[16] = 0;
            //wprintw(win, "  | %s", LocalBuffer);
            printf("  | %s", LocalBuffer);
        }
    }
    //wprintw(win, "\n");
    printf("\n");
    //wrefresh(win);
}

int rtl_nl_bind(int socket, const RTL_NL_GRPS grps)
{
	struct sockaddr_nl saddr;

	memset(&saddr, 0, sizeof(saddr));

	saddr.nl_family = AF_NETLINK;
	saddr.nl_pid 	= getpid();
	saddr.nl_groups |= grps;

	return bind(socket, (struct sockaddr*)&saddr, sizeof(saddr));
}

int rtl_nl_recv(int socket, void* buff, const size_t length, int flag)
{
	int recvleng;

	struct nlmsghdr* tmp_buff;

	if(!buff || 0 == length)
		return -101;

	tmp_buff = (struct nlmsghdr*)malloc(NLMSG_SPACE(length));

	if(!tmp_buff)
		return -102;
	
	recvleng = recv(socket, tmp_buff, NLMSG_SPACE(length), flag);

	if(recvleng <= 0)
	{
		printf("rtl_nl_recv error!\n");
		free((void*)tmp_buff);
		return recvleng;
	}

	if(recvleng < NLMSG_SPACE(0))
	{
		printf("rtl_nl_recv unint message!\n");
		return -103;
	}
	
	memset(buff, 0, length);
	memcpy(buff, (NLMSG_DATA(tmp_buff)), length);

	free((void*)tmp_buff);
	
	return (recvleng - NLMSG_SPACE(0));
}

static void handle_link_change(void *msg)
{
	unsigned int old_ptmks;
	unsigned int new_ptmks;
	int i;

	RTL_NLMSG_LINKCHG *p_portst = (RTL_NLMSG_LINKCHG *)msg;

    old_ptmks = p_portst->extended.old_ptmks;
	new_ptmks = p_portst->extended.new_ptmks;

	for(i = 0; i < DVE_NIC_PORT_MAX_NUM; ++i)
	{
		//check if it is changed!
		if((1 << i) & (old_ptmks ^ new_ptmks))
		{
			//check it current stat
			if((1 << i) & new_ptmks)
				AUG_PRT("+++%s is connected+++\n", dev[i]);
			else
				AUG_PRT("---%s is disconnected---\n", dev[i]);
		}
	}
}

int rtl_nl_parsemsg(void *msg, unsigned int length)
{
	RTL_ST_NLMSG *pmsg = (RTL_ST_NLMSG *)msg;

	DumpData(msg, length, 0);

	//first, check if it is not noise 
	if(!(CHECK_RTL_NL_MSG_MAGIC(pmsg->rtl_magic)))
	{
		printf("magic not equal!!!\n");
		return -1;
	}
	
	if(RTL_NL_SRC_KENERL == (CHECK_RTL_NL_MSG_SRC(pmsg->rtl_magic)))
		printf("This is a msg from kernel!\n");
	else
		printf("This is a msg from user!\n");

	if(pmsg->length > (length - RTL_NLMSG_HEAD_SZ))
	{
		printf("recv a broken msg!\n");
		return -2;
	}
	
	switch(pmsg->type)
	{
		case RTL_NL_TYPE_LINKCHANGE:
			//add handle func;
			handle_link_change((void *)pmsg);
			break;

		default:
			printf("Unkown Message!!!\n");
			return -3;

	}
	return 0;
}

#if 1
int main(void)
{
		int sd, ret;
        int i;
        
        RTL_ST_NLMSG *p_portst;

        sd = socket(AF_NETLINK, SOCK_RAW, NETLINK_RTL_NLMSG);

		ret = rtl_nl_bind(sd, RTL_BASIC_GRP);
		if(ret)
		{
			printf("bind error\n");
			return ret;
		}

		p_portst = (RTL_ST_NLMSG *)malloc(MAX_MSGSIZE);

		while(1) 
        {	
			int recvlength;
        
			memset(p_portst, 0, MAX_MSGSIZE);
			
			if((recvlength = rtl_nl_recv(sd, p_portst, MAX_MSGSIZE, 0)) < 0)
			{
				printf("rtl_nl_recv error\n");
				continue;
			}
			rtl_nl_parsemsg(p_portst, recvlength);		
        }

 		free(p_portst);
        close(sd);
}

#else
int main(void)
{
	int sd;
	int ret, i;
	int recvlen;
	struct sockaddr_nl saddr;
	char buf[512];
	struct nlmsghdr *nlh;
	
        sd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	bzero(&saddr, sizeof(saddr));
	saddr.nl_family = AF_NETLINK;
	saddr.nl_pid 	= getpid();
	saddr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_NOTIFY | RTMGRP_LINK;

	ret = bind(sd, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret) {
		printf("netlink: open socket failed\n");
		return ret;
	}
	
	while(1) {
		memset(buf, 0x0, 512);
		recvlen = recv(sd, buf, sizeof(buf), 0);
		if(recvlen <= 0) {
			printf("netlink recv failed !\n");
			close(sd);
			return recvlen;
		}
		//printf("recvlen = %d\n", recvlen);
		nlh = (struct nlmsghdr *) buf;
		for (; NLMSG_OK(nlh, recvlen); nlh = NLMSG_NEXT(nlh, recvlen)) {
			//printf("type: 0x%x\n", nlh->nlmsg_type);
			switch (nlh->nlmsg_type)
			{
			case NLMSG_DONE:
			case NLMSG_ERROR:
				printf("netlink: Done or Error\n");
				break;
			case RTM_NEWADDR:
				printf("netlink: RTM_NEWADDR\n");
				break;
			case RTM_DELADDR:
				printf("netlink: RTM_DELADDR\n");
				break;
			case RTM_DELLINK:
				printf("netlink: DELLINK\n");
				break;
			case RTM_NEWLINK:
				printf("netlink: NEWLINK\n");
				break;
			default:
				printf("netlink: wrong msg\n");
				break;
			}
		}
	}
	
	close(sd);
	return 0;
}
#endif
