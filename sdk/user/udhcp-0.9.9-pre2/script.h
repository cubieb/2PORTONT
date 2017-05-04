#ifndef _SCRIPT_H
#define _SCRIPT_H

void run_script(struct dhcpMessage *packet, const char *name);

#ifdef CONFIG_USER_DHCP_OPT_33_249
void dealStaticRTOption(unsigned char *data, int code);
#endif

#endif
