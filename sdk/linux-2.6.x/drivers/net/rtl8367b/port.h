#ifndef RL6000_PORT_H
#define RL6000_PORT_H

#define RL6000_CPU_PORT                   6
#define RL6000_WAN_PORT                   0
#define RL6000_LAN_PORT_START             1
#define RL6000_LAN_PORT_NUM               4 /* LAN ports: port 1~ port4 */

#define RL6000_CPU_PORT_BITMAP (1<<RL6000_CPU_PORT)
#define RL6000_WAN_PORT_BITMAP (1<<RL6000_WAN_PORT)
#define RL6000_LAN_PORT_BITMAP (((1<<RL6000_LAN_PORT_NUM)-1)<<RL6000_LAN_PORT_START)
#define RL6000_ALL_PORT_BITMAP (RL6000_CPU_PORT_BITMAP|RL6000_WAN_PORT_BITMAP|RL6000_LAN_PORT_BITMAP)

int RL6000_vlan_set(void);
void RL6000_getPortStatus(void);
int RL6000_vlan_setAll(void);
void RL6000_port_isolation_setAll(void);
void RL6000_vlan_portIgrFilterEnable_setAll(void);
void RL6000_l2_limitLearningCnt_setAll(void);
void RL6000_cpu_tag(int enable);
void RL6000_RGMII(void);
void RL6000_LED(void);
unsigned int RL6000_set_phy(unsigned int port, unsigned int reg, unsigned int regData, unsigned int page);
unsigned int RL6000_get_phy(unsigned int port, unsigned int reg, unsigned int *pData, unsigned int page);
void reset_rtl8367b_PortStat(void);
void show_rtl8367b_PortStat(unsigned int port);
int RL6000_wanport_bandwidth_set(int bps);
int RL6000_wanport_bandwidth_flush(void);
extern int RL6000_read(unsigned int addr, unsigned int *data);
extern int RL6000_write(unsigned int addr, unsigned int data);

#endif
