#ifndef _ETHCTL_IMPLEMENT_H_
#define _ETHCTL_IMPLEMENT_H_


struct eth_arg{
	unsigned char cmd;
	unsigned int cmd2;
	unsigned int cmd3;
	unsigned int cmd4;
	unsigned int cmd5;
};

void eth_ctl(struct eth_arg * arg);
#endif

