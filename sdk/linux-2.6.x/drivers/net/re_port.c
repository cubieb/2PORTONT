#include "re830x.h"

int enable_port_mapping=0;
int enable_vlan_grouping=0;

struct r8305_struc	rtl8305_info;
int get_vid(int pvid)
{
	if (pvid>=SW_PORT_NUM+1 && pvid<=(SW_PORT_NUM+1+4))
		return rtl8305_info.vlan[pvid].vid;
	return 0;
}
int get_pvid(int vid)
{
	int i;
	
	for (i=SW_PORT_NUM+1; i<=(SW_PORT_NUM+1+4); i++) {
		if (rtl8305_info.vlan[i].vid == vid)
			return i;
	}
	return 0;
}

int bitmap_virt2phy(int mbr)
{
	return mbr;
}
EXPORT_SYMBOL(bitmap_virt2phy);

