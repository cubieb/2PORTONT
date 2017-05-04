#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#if defined(CONFIG_DEFAULTS_KERNEL_3_18)
#else
#include <linux/config.h>
#endif



#include <rtk_rg_struct.h>
#include <rtk_rg_internal.h>
#include <rtk_rg_debug.h>
#include <rtk_rg_callback.h>
#include <rtk_rg_liteRomeDriver.h>

#ifdef CONFIG_RG_WMUX_SUPPORT
extern void wmux_cleanup_devices(void);
#endif

//static char ifconfig[]="/bin/ifconfig";
static char ipUtility[]="ip";
static char SNAT_INFO_FILE[]="/tmp/SNAT_INFO";
static char IGMPPROXY_PID[]="/var/run/igmp_pid";

mm_segment_t rg_callback_oldfs;

struct file *openFile(char *path,int flag,int mode)
{
    struct file *fp;

    fp=filp_open(path, flag, 0);
 
   	if(IS_ERR(fp)) 
   		return NULL;
	else
		return fp;
}

int readFile(struct file *fp,char *buf,int readlen)
{
    if (fp->f_op && fp->f_op->read)
        return fp->f_op->read(fp,buf,readlen, &fp->f_pos);
    else
        return -1;
}

int closeFile(struct file *fp)
{
    filp_close(fp,NULL);
    return 0;
}

void initKernelEnv(void)
{
    rg_callback_oldfs = get_fs();
    set_fs(KERNEL_DS);
}

int read_pid(char *filename)
{
	struct file *fp;
	char pidBuf[32];
	int ret, pid=0;

	initKernelEnv();
	fp = openFile(filename,O_RDWR,0);
	if (fp!=NULL)
	{
		memset(pidBuf,0,32);
		if ((ret=readFile(fp,pidBuf,31))>0){
			//rtlglue_printf("buff:%s\n",snat_file_buff);
		}
		else{ 
			rtlglue_printf("[rg callback]openFile %s error, ret=%d\n",pidBuf,ret);
		}
		closeFile(fp);
		set_fs(rg_callback_oldfs);
		sscanf(pidBuf,"%d",&pid);
	}
	return pid;

}


#if defined(CONFIG_APOLLO)
/*internal utils*/
#define inet_ntoa_r(x,y) _inet_ntoa_r(x,y)
#define inet_ntoa(x) _inet_ntoa(x)
#endif

static inline int8 *_ui8tod( uint8 n, int8 *p )
{
	if( n > 99 ) *p++ = (n/100) + '0';
	if( n >  9 ) *p++ = ((n/10)%10) + '0';
	*p++ = (n%10) + '0';
	return p;
}


static int8 *_inet_ntoa(rtk_ip_addr_t ina)
{
	static int8 buf[4*sizeof "123"];
	int8 *p = buf;
	uint8 *ucp = (unsigned char *)&ina;

	p = _ui8tod( ucp[0] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[3] & 0xFF, p);
	*p++ = '\0';

	return (buf);
}



/*IPv6 address to string*/
int8 *_rtk_rg_inet_n6toa(const uint8 *ipv6)
{
#define RG_IPV6_TMP_BUFFER_LENGTH 8

	static int8 buf[8*sizeof "FFFF:"];
    uint32  i;
    uint16  ipv6_ptr[RG_IPV6_TMP_BUFFER_LENGTH] = {0};

    for (i = 0; i < RG_IPV6_TMP_BUFFER_LENGTH ;i++)
    {
        ipv6_ptr[i] = ipv6[i*2+1];
        ipv6_ptr[i] |=  ipv6[i*2] << 8;
    }

    sprintf(buf, "%x:%x:%x:%x:%x:%x:%x:%x", ipv6_ptr[0], ipv6_ptr[1], ipv6_ptr[2], ipv6_ptr[3]
    , ipv6_ptr[4], ipv6_ptr[5], ipv6_ptr[6], ipv6_ptr[7]);
	return (buf);
}


/* convert IPv6 address from string to number. Length of ipv6_addr must more than 16 characters */
int32
_rg_str2ipv6(uint8 *ipv6, const uint8 *str)
{
#define RG_IN6ADDRSZ 16
#define RG_INT16SZ     2
    static const uint8 xdigits_l[] = "0123456789abcdef",
              xdigits_u[] = "0123456789ABCDEF";
    uint8 tmp[RG_IN6ADDRSZ], *tp, *endp, *colonp;
    const uint8 *xdigits, *curtok;
    int ch, saw_xdigit;
    int val;

    if ((NULL == str) || (NULL == ipv6))
    {
        return RT_ERR_FAILED;
    }

    memset((tp = tmp), '\0', RG_IN6ADDRSZ);
    endp = tp + RG_IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*str == ':')
        if (*++str != ':')
            return (RT_ERR_FAILED);
    curtok = str;
    saw_xdigit = 0;
    val = 0;
    while ((ch = *str++) != '\0') {
        const uint8 *pch;

        if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
            pch = strchr((xdigits = xdigits_u), ch);
        if (pch != NULL) {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return (RT_ERR_FAILED);
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':') {
            curtok = str;
            if (!saw_xdigit) {
                if (colonp)
                    return (RT_ERR_FAILED);
                colonp = tp;
                continue;
            }
            if (tp + RG_INT16SZ > endp)
                return (RT_ERR_FAILED);
            *tp++ = (uint8) (val >> 8) & 0xff;
            *tp++ = (uint8) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }

        return (RT_ERR_FAILED);
    }
    if (saw_xdigit) {
        if (tp + RG_INT16SZ > endp)
            return (RT_ERR_FAILED);
        *tp++ = (uint8) (val >> 8) & 0xff;
        *tp++ = (uint8) val & 0xff;
    }
    if (colonp != NULL) {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        for (i = 1; i <= n; i++) {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return (RT_ERR_FAILED);
    memcpy(ipv6, tmp, RG_IN6ADDRSZ);
    return (RT_ERR_OK);
}/* end of diag_util_str2Ipv6 */

/* Check if the MAC address is a broadcast address or not */
#define RG_CALLBACK_DHCPD_MINIMUM_IP_NUM	8
int _rg_cacluate_dhcp_range(ipaddr_t ip, ipaddr_t mask, ipaddr_t *start, ipaddr_t *end)
{
	uint32 notMask=(~mask)&0xffffffff;
	uint32 region_length=notMask>>1;
	//ipaddr_t first_region_start,first_region_end;
	//ipaddr_t second_region_start,second_region_end;

	if(mask==0)
		return (RT_ERR_FAILED);
	if(region_length < RG_CALLBACK_DHCPD_MINIMUM_IP_NUM)
		return (RT_ERR_FAILED);

	//first_region_start = ip&mask;
	//first_region_end = first_region_start + region_length;
	//second_region_start = first_region_end++;
	//second_region_end = second_region_start + region_length;
#if 0
	if(ip-(ip&mask)>=region_length)
	{
		//server ip located in second region, so we choose FIRST region for dhcpd to distribute
		*start=ip&mask;//first_region_start;
		*end=(*start)+region_length;//first_region_end;
	}
	else
	{
		//server ip located in first region, so we choose SECOND region for dhcpd to distribute
		*start=ip&mask+region_length+1;//second_region_start;
		*end=(*start)+region_length;//second_region_end;
	}
#else
	if((ip-(ip&mask))>=region_length)
		*start=ip&mask;	//server ip located in second region, so we choose FIRST region for dhcpd to distribute
	else
		*start=(ip&mask)+region_length+1;	//server ip located in first region, so we choose SECOND region for dhcpd to distribute
	*end=(*start)+region_length;
#endif
	//Check for boundary
	if(((*start)&0xff)==0)(*start)++;
	else if(((*end)&0xff)==0xff)(*end)--;

	return (RT_ERR_OK);
}
void _rg_delete_dhcpc(char *dev_name, int forceAll)
{
	if(forceAll)
	{
		//clear old udhcpd daemon and conf file
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[u]dhcpc' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`");
		rtk_rg_callback_pipe_cmd("`for FILE in $(ls /var/udhcpc/udhcpc*); do rm -rf $FILE>/dev/null 2>&1; done`");
	}
	else if(dev_name)
	{
		//clear old udhcpd daemon and conf file for specific interface
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[u]dhcpc -i %s' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`",dev_name);
		rtk_rg_callback_pipe_cmd("`for FILE in $(ls /var/udhcpc/udhcpc.%d); do rm -rf $FILE>/dev/null 2>&1; done`",dev_name);
	}
}
void _rg_delete_dhcpd(int vlan_id, int forceAll)
{
	if(forceAll)
	{
		//clear old udhcpd daemon and conf file
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[u]dhcpd' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`");
		rtk_rg_callback_pipe_cmd("`for FILE in $(ls /var/udhcpd/udhcpd*); do rm -rf $FILE>/dev/null 2>&1; done`");
	}
	else if(vlan_id>0)
	{
		//clear old udhcpd daemon and conf file
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[u]dhcpd_br0.%d.conf' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`",vlan_id);
		rtk_rg_callback_pipe_cmd("`for FILE in $(ls /var/udhcpd/udhcpd_br0.%d*); do rm -rf $FILE>/dev/null 2>&1; done`",vlan_id);
	}
	else
	{
		//clear old udhcpd daemon and conf file
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[u]dhcpd_br0.conf' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`");
		rtk_rg_callback_pipe_cmd("rm -rf /var/udhcpd/udhcpd_br0.conf > /dev/null 2>&1");
	}
}
void _rg_setup_dhcpd(ipaddr_t ip, ipaddr_t mask, int vlan_id)
{
	ipaddr_t dhcpIPstart, dhcpIPend;

	_rg_delete_dhcpd(vlan_id,0);
	
	if(_rg_cacluate_dhcp_range(ip,mask,&dhcpIPstart,&dhcpIPend)==RT_ERR_OK)
	{
		unsigned char dhcp_start[16], dhcp_end[16];
		memcpy(dhcp_start,(void*)_inet_ntoa(dhcpIPstart),16);
		memcpy(dhcp_end,(void*)_inet_ntoa(dhcpIPend),16);
		if(vlan_id>0)
		{
			//create conf file
			rtk_rg_callback_pipe_cmd("mkdir -p /var/udhcpd/");
			rtk_rg_callback_pipe_cmd("echo > /var/udhcpd/udhcpd_br0.%d.conf",vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"poolname br0.%d\" >> /var/udhcpd/udhcpd_br0.%d.conf",vlan_id,vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"interface br0.%d\" >> /var/udhcpd/udhcpd_br0.%d.conf",vlan_id,vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"server %s\" >> /var/udhcpd/udhcpd_br0.%d.conf",(char*)_inet_ntoa(ip),vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"start %s\" >> /var/udhcpd/udhcpd_br0.%d.conf",dhcp_start,vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"end %s\" >> /var/udhcpd/udhcpd_br0.%d.conf",dhcp_end,vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"opt subnet %s\" >> /var/udhcpd/udhcpd_br0.%d.conf",(char*)_inet_ntoa(mask),vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"opt router %s\" >> /var/udhcpd/udhcpd_br0.%d.conf",(char*)_inet_ntoa(ip),vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"opt dns %s\" >> /var/udhcpd/udhcpd_br0.%d.conf",(char*)_inet_ntoa(ip),vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"opt dns 8.8.8.8\" >> /var/udhcpd/udhcpd_br0.%d.conf",vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"opt lease 86400\" >> /var/udhcpd/udhcpd_br0.%d.conf",vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"opt domain domain.name\" >> /var/udhcpd/udhcpd_br0.%d.conf",vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"opt venspec 3561 4 00E04C 5 000000000002 6 IGD\" >> /var/udhcpd/udhcpd_br0.%d.conf",vlan_id);
			rtk_rg_callback_pipe_cmd("echo \"poolend end\" >> /var/udhcpd/udhcpd_br0.%d.conf",vlan_id);

			//execute dhcpd daemon
			rtk_rg_callback_pipe_cmd("udhcpd -S /var/udhcpd/udhcpd_br0.%d.conf > /dev/null 2>&1 &",vlan_id);
		}
		else
		{
			//create conf file
			rtk_rg_callback_pipe_cmd("mkdir -p /var/udhcpd/");
			rtk_rg_callback_pipe_cmd("echo > /var/udhcpd/udhcpd_br0.conf");
			rtk_rg_callback_pipe_cmd("echo \"poolname br0\" >> /var/udhcpd/udhcpd_br0.conf");
			rtk_rg_callback_pipe_cmd("echo \"interface br0\" >> /var/udhcpd/udhcpd_br0.conf");
			rtk_rg_callback_pipe_cmd("echo \"server %s\" >> /var/udhcpd/udhcpd_br0.conf",(char*)_inet_ntoa(ip));
			rtk_rg_callback_pipe_cmd("echo \"start %s\" >> /var/udhcpd/udhcpd_br0.conf",dhcp_start);
			rtk_rg_callback_pipe_cmd("echo \"end %s\" >> /var/udhcpd/udhcpd_br0.conf",dhcp_end);
			rtk_rg_callback_pipe_cmd("echo \"opt subnet %s\" >> /var/udhcpd/udhcpd_br0.conf",(char*)_inet_ntoa(mask));
			rtk_rg_callback_pipe_cmd("echo \"opt router %s\" >> /var/udhcpd/udhcpd_br0.conf",(char*)_inet_ntoa(ip));
			rtk_rg_callback_pipe_cmd("echo \"opt dns %s\" >> /var/udhcpd/udhcpd_br0.conf",(char*)_inet_ntoa(ip));
			rtk_rg_callback_pipe_cmd("echo \"opt dns 8.8.8.8\" >> /var/udhcpd/udhcpd_br0.conf");
			rtk_rg_callback_pipe_cmd("echo \"opt lease 86400\" >> /var/udhcpd/udhcpd_br0.conf");
			rtk_rg_callback_pipe_cmd("echo \"opt domain domain.name\" >> /var/udhcpd/udhcpd_br0.conf");
			rtk_rg_callback_pipe_cmd("echo \"opt venspec 3561 4 00E04C 5 000000000002 6 IGD\" >> /var/udhcpd/udhcpd_br0.conf");
			rtk_rg_callback_pipe_cmd("echo \"poolend end\" >> /var/udhcpd/udhcpd_br0.conf");

			//execute dhcpd daemon
			rtk_rg_callback_pipe_cmd("udhcpd -S /var/udhcpd/udhcpd_br0.conf > /dev/null 2>&1 &");
		}
	}
}

void _rg_delete_radvd(int vlan_id, int forceAll)
{
	if(forceAll)
	{
		//clear old udhcpd daemon and conf file
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[r]advd' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`",vlan_id);
		rtk_rg_callback_pipe_cmd("`for FILE in $(ls /var/radvd*); do rm -rf $FILE>/dev/null 2>&1; done`",vlan_id);
	}
	else if(vlan_id>0)
	{
		//clear old udhcpd daemon and conf file
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[r]advd_br0.%d.conf' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`",vlan_id);
		rtk_rg_callback_pipe_cmd("`for FILE in $(ls /var/radvd_br0.%d*); do rm -rf $FILE>/dev/null 2>&1; done`",vlan_id);

		rtk_rg_callback_pipe_cmd("ip -6 addr delete fe80::1/64 dev br0.%d",vlan_id);
	}
	else
	{
		//clear old udhcpd daemon and conf file
		rtk_rg_callback_pipe_cmd("`for FILE in $(ps | grep '[r]advd_br0.conf' | sed 's/^[ \t]*//g;s/[ \t][ \t]*.*//g'); do kill $FILE > /dev/null 2>&1; done`");
		rtk_rg_callback_pipe_cmd("rm -rf /var/radvd_br0.conf > /dev/null 2>&1");

		rtk_rg_callback_pipe_cmd("ip -6 addr delete fe80::1/64 dev br0");
	}
	rtk_rg_callback_pipe_cmd("echo 0 > /proc/sys/net/ipv6/conf/all/forwarding");
}
void _rg_setup_radvd(unsigned char *ipv6, int maskLen, int vlan_id)
{
	int prefix;
	unsigned short idx,bitMask;
	rtk_ipv6_addr_t  ipv6MaskedAddr={{0}};
		
	if(maskLen==128)
		memcpy(ipv6MaskedAddr.ipv6_addr,ipv6,IPV6_ADDR_LEN);
	else
	{
		prefix=maskLen;
		idx=(prefix>>3)&0xff;
			
		if((prefix&0x7)==0)
			bitMask=0;
		else
			bitMask=(0xff<<(8-(prefix&0x7)))&0xff;
		//DEBUG("prefix = %d, idx = %d, bitMask =%02x",prefix,idx,bitMask);
	
		memcpy(ipv6MaskedAddr.ipv6_addr,ipv6,idx);
		ipv6MaskedAddr.ipv6_addr[idx]=ipv6[idx]&bitMask;
		DEBUG("v6 mask addr is %s",_rtk_rg_inet_n6toa(ipv6MaskedAddr.ipv6_addr));
	}

	_rg_delete_radvd(vlan_id,0);

	rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");

	if(vlan_id>0)
	{
		rtk_rg_callback_pipe_cmd("echo \"interface br0.%d\" > /var/radvd_br0.%d.conf",vlan_id,vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"{\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvSendAdvert on;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       MaxRtrAdvInterval 600;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       MinRtrAdvInterval 198;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvCurHopLimit 64;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvDefaultLifetime 1800;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvReachableTime 0;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvRetransTimer 0;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvLinkMTU 0;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvManagedFlag off;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       AdvOtherConfigFlag off;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       prefix %s/%d\" >> /var/radvd_br0.%d.conf",_rtk_rg_inet_n6toa(ipv6MaskedAddr.ipv6_addr),maskLen,vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       {\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"         AdvOnLink on;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"         AdvAutonomous on;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"         AdvRouterAddr on;\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"       };\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo \"};\" >> /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("echo "" >> /var/radvd_br0.%d.conf",vlan_id);

		rtk_rg_callback_pipe_cmd("/bin/radvd -s -C /var/radvd_br0.%d.conf",vlan_id);
		rtk_rg_callback_pipe_cmd("ip -6 addr add fe80::1/64 dev br0.%d",vlan_id);
	}
	else
	{
		rtk_rg_callback_pipe_cmd("echo \"interface br0\" > /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"{\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvSendAdvert on;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		MaxRtrAdvInterval 600;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		MinRtrAdvInterval 198;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvCurHopLimit 64;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvDefaultLifetime 1800;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvReachableTime 0;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvRetransTimer 0;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvLinkMTU 0;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvManagedFlag off;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		AdvOtherConfigFlag off;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		prefix %s/%d\" >> /var/radvd_br0.conf",_rtk_rg_inet_n6toa(ipv6MaskedAddr.ipv6_addr),maskLen);
		rtk_rg_callback_pipe_cmd("echo \"		{\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		  AdvOnLink on;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		  AdvAutonomous on;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		  AdvRouterAddr on;\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"		};\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo \"};\" >> /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("echo "" >> /var/radvd_br0.conf");

		rtk_rg_callback_pipe_cmd("/bin/radvd -s -C /var/radvd_br0.conf");
		rtk_rg_callback_pipe_cmd("ip -6 addr add fe80::1/64 dev br0");
	}
}


void _rg_configure_protocol_stack(int intfIdx, char *dev)
{
	rtk_rg_intfInfo_t *intfInfo;
	intfInfo=&rg_db.systemGlobal.interfaceInfo[intfIdx].storedInfo;
	
	//ps cmd for bring up nas0 & nas0_x for dhcp
	//do cmd
#if defined(CONFIG_APOLLO)
    rtk_rg_callback_pipe_cmd("echo %d %s > /proc/rtl8686gmac/dev_port_mapping",intfInfo->wan_intf.wan_intf_conf.wan_port_idx,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME);
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) //end defined(CONFIG_APOLLO) elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 
	
		FIXME("FIXME XDSL callback\n");
	
#endif //end defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 

	if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WIRED)
	{
		rtk_rg_callback_pipe_cmd("%s link set %s up",ipUtility,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME);
		rtk_rg_callback_pipe_cmd("%s link set %s down",ipUtility,dev);

#ifdef CONFIG_RG_WMUX_SUPPORT
		rtk_rg_callback_pipe_cmd("echo %s %d > /proc/rg/wmux_add",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,intfIdx);
#else
		rtk_rg_callback_pipe_cmd("ethctl remsmux bridge %s %s",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev);

		//smux: ignor napt cmd, just care vlan
		if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on){
			//have vlan tag
			rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s nonapt vlan %d",intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE?"pppoe":"ipoe",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev,intfInfo->wan_intf.wan_intf_conf.egress_vlan_id); 
		}else{
			//no vlan tag
			rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s ",intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE?"pppoe":"ipoe",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev);
		}
#endif
	}
	
	rtk_rg_callback_pipe_cmd("%s link set %s address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,dev,
			   intfInfo->wan_intf.wan_intf_conf.gmac.octet[0],
			   intfInfo->wan_intf.wan_intf_conf.gmac.octet[1],
			   intfInfo->wan_intf.wan_intf_conf.gmac.octet[2],
			   intfInfo->wan_intf.wan_intf_conf.gmac.octet[3],
			   intfInfo->wan_intf.wan_intf_conf.gmac.octet[4],
			   intfInfo->wan_intf.wan_intf_conf.gmac.octet[5]);

	rtk_rg_callback_pipe_cmd("%s link set %s txqueuelen 10",ipUtility,dev);

	rtk_rg_callback_pipe_cmd("%s link set %s up",ipUtility,dev);
}

#ifdef CONFIG_RG_CALLBACK
char snat_file_buff[CB_MAX_FILE_SIZE];
int _delete_iptables_snat(void){

	struct file *fp;
	char* buff_line[CB_MAX_FILE_LINE]={NULL};
	char* token, *ptr_head, *ptr_tail;
	int ret,len,i;
	char intf[IFNAMSIZ];
	char ip[16];
	
	/*cat ipatbles SNAT info into /tmp/SNAT_INFO */
	rtk_rg_callback_pipe_cmd("iptables -t nat -nvL POSTROUTING > /tmp/SNAT_INFO");



	/*read /tmp/SNAT_INFO */
    initKernelEnv();
    fp=openFile(SNAT_INFO_FILE,O_RDWR,0);
    if (fp!=NULL)
    {
        memset(snat_file_buff,0,CB_MAX_FILE_SIZE);
        if ((ret=readFile(fp,snat_file_buff,(CB_MAX_FILE_SIZE-1)))>0){
            //rtlglue_printf("buff:%s\n",snat_file_buff);
        }
        else{ 
			rtlglue_printf("[rg callback]openFile %s error, ret=%d\n",SNAT_INFO_FILE,ret);
        }
		closeFile(fp);

    }
    set_fs(rg_callback_oldfs);
	
	/*parse /tmp/SNAT_INFO */	
	len = strlen(snat_file_buff);
	token = snat_file_buff; //buff is static, cannot use as strsep parameter!
	while (strsep(&token, "\n"));//parse each line to buff_line[i]
	for (i=0, token=snat_file_buff; token<(snat_file_buff+len) && i<CB_MAX_FILE_LINE ;i++) { 
		buff_line[i] = token; 
		token=token+strlen(token)+1; 
	} 


	//start from buff_line[2] to parse intfname & ip, adn delete such iptables rule
	for(i=2; buff_line[i]!=NULL && i<5;i++){ //first two line are not rules in /tmp/SNAT_INFO
		/*parse intf name*/
		ptr_head = strstr(buff_line[i], "nas");
		if(ptr_head==NULL) continue;//string is not for SNAT rule 		
		ptr_tail = strchr(ptr_head, ' ');	
		if(ptr_tail==NULL) continue;//string is not for SNAT rule
		
		len = ptr_tail - ptr_head;
		strncpy(intf,ptr_head,len);
		intf[len]='\0';

		/*parse ip*/
		ptr_head = strstr(ptr_tail+1, "to:");
		if(ptr_head==NULL) continue;//string is not for SNAT rule 	
		ptr_tail = strchr(ptr_head, ' ');
		if(ptr_tail==NULL) continue;//string is not for SNAT rule
		
		len = ptr_tail - (ptr_head+3); //+3 to skip "to:"
		strncpy(ip,ptr_head+3,len);
		ip[len]='\0';
		
		rtk_rg_callback_pipe_cmd("iptables -t nat -D POSTROUTING -o %s -j SNAT --to-source %s",intf,ip);
	}

	/*remove  /tmp/SNAT_INFO */
	rtk_rg_callback_pipe_cmd("rm %s",SNAT_INFO_FILE);

	return SUCCESS;
}



/*Default callback functions*/

#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK

//callback for rg init
int _rtk_rg_initParameterSetByHwCallBack(void)
{
	int i;
	char intf_name[20];

	rtk_rg_callback_pipe_cmd("%s br0 down",ifconfig);

	for(i=0;i<8;i++){
		sprintf(intf_name,"eth%d",i);
		rtk_rg_callback_pipe_cmd("%s %s down",ifconfig,intf_name);

		//1 FIXME: PPPoE disconnet issue
		/*
		if(rg_kernel.ppp_diaged[i]==ENABLED){
			rtk_rg_callback_pipe_cmd("/bin/spppctl down ppp%d",i);
			rg_kernel.ppp_diaged[i]=DISABLED;
		}
		*/
		//rtk_rg_callback_pipe_cmd("/bin/spppctl down ppp%d",i);
		//rtk_rg_callback_pipe_cmd("/bin/spppd");
	}

	//patch for ipatbles
	_delete_iptables_snat();
	
    return SUCCESS;

}


//rg_db.systemGlobal.initParam.interfaceAddByHwCallBack;
int _rtk_rg_interfaceAddByHwCallBack(rtk_rg_intfInfo_t* intfInfo, int* intfIdx)
{

	unsigned char ip[16];
	unsigned char gw_ip[16];
    unsigned char mask[16];
	unsigned char brcast_ip[16];
	int i;
    char dev[IFNAMSIZ]; //device name, ex:nas0_0
    char* wan_type; //wan type: ex:PPPoE
    int vid=0;
	rtk_ip_addr_t bc_ip; //broadCast ip for wan intf
	rtk_rg_ipStaticInfo_t static_info;

	
    if(intfInfo->is_wan)
    {
        //add wan interface        
		memset(&static_info,0,sizeof(rtk_rg_ipStaticInfo_t));

		//get static_info by different type
		if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_STATIC){
			 memcpy(&static_info,&intfInfo->wan_intf.static_info,sizeof(rtk_rg_ipStaticInfo_t));
		}else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP){
			 memcpy(&static_info,&intfInfo->wan_intf.dhcp_client_info.hw_info,sizeof(rtk_rg_ipStaticInfo_t));
		}else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE){
			 memcpy(&static_info,&intfInfo->wan_intf.pppoe_info.after_dial.hw_info,sizeof(rtk_rg_ipStaticInfo_t));
		}
		
        //sprintf(dev,"nas0_%d",(*intfIdx-1));
        sprintf(dev,"eth%d",(*intfIdx));
        memcpy(ip,(void*)_inet_ntoa(static_info.ip_addr),16);
		memcpy(gw_ip,(void*)_inet_ntoa(static_info.gateway_ipv4_addr),16);
        memcpy(mask,(void*)_inet_ntoa(static_info.ip_network_mask),16);

		//caculate broacast ip
		bc_ip = static_info.gateway_ipv4_addr & static_info.ip_network_mask;

		for(i=0;i<32;i++){
			if(static_info.ip_network_mask & (1<<i)){
				//wan domain (mask)
			}else{
				//wan broacast ip
				bc_ip |= (1<<i);
			}
		}
		memcpy(brcast_ip,(void*)_inet_ntoa(bc_ip),16);

        switch(intfInfo->wan_intf.wan_intf_conf.wan_type)
        {
	        case RTK_RG_STATIC:
	            wan_type="ipoe";
	            break;
	        case RTK_RG_DHCP:
	            wan_type="dhcp";
	            break;
	        case RTK_RG_PPPoE:
	            wan_type="pppoe";
	            break;
	        case RTK_RG_BRIDGE:
	            wan_type="bridge";
	            break;
	        default:
				wan_type=" ";
	            break;
        }

        if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
            vid = intfInfo->wan_intf.wan_intf_conf.egress_vlan_id;

        //do cmd
        rtk_rg_callback_pipe_cmd("echo %d wan0 > /proc/rtl8686gmac/dev_port_mapping",intfInfo->wan_intf.wan_intf_conf.wan_port_idx);

		//create nas0_X
		if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP){
			//DHCP protocal-stack setting is set by _rtk_rg_dhcpRequestByHwCallBack
		}else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE){
			//PPPoE protocal-stack setting is set by _rtk_rg_pppoeBeforeDiagByHwCallBack
		}else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE){
			/*rtk_rg_callback_pipe_cmd("%s nas0 up",ifconfig);
			rtk_rg_callback_pipe_cmd("/bin/ethctl remsmux bridge nas0 %s",dev);
			
			if(static_info.napt_enable && intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
			{	
				//NAPT + VALN TAG
				rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s napt vlan %d",wan_type,dev,vid); 
			}
			else if(static_info.napt_enable)
			{
				//NAPT
				rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s napt",wan_type,dev);
			}
			else if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on){
				//VALN TAG
				rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s nonapt vlan %d",wan_type,dev,vid); 
			}
			else{
				rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s",wan_type,dev);
			}*/
			
			rtk_rg_callback_pipe_cmd("%s %s hw ether %02x%02x%02x%02x%02x%02x",ifconfig,dev,
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[0],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[1],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[2],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[3],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[4],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[5]);

			rtk_rg_callback_pipe_cmd("%s %s txqueuelen 10",ifconfig,dev);
			rtk_rg_callback_pipe_cmd("%s %s up",ifconfig,dev);
			//rtk_rg_callback_pipe_cmd("/bin/brctl addif br0 %s",dev);

		}
		else{
			//set STATIC & BRIDGE protocal-stack
	        /*rtk_rg_callback_pipe_cmd("%s nas0 up",ifconfig);
	        rtk_rg_callback_pipe_cmd("/bin/ethctl remsmux bridge nas0 %s",dev);

			if(static_info.napt_enable && intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
	        {	
	        	//NAPT + VALN TAG
	            rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s napt vlan %d",wan_type,dev,vid); 
	        }
	        else if(static_info.napt_enable)
	        {
	        	//NAPT
	            rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s napt",wan_type,dev);
	        }
			else if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on){
				//VALN TAG
				rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s nonapt vlan %d",wan_type,dev,vid); 
			}
			else{
				rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux %s nas0 %s",wan_type,dev);
			}*/
			
	        rtk_rg_callback_pipe_cmd("%s %s hw ether %02x%02x%02x%02x%02x%02x",ifconfig,dev,
	                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[0],
	                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[1],
	                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[2],
	                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[3],
	                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[4],
	                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[5]);
			
	        rtk_rg_callback_pipe_cmd("%s %s up",ifconfig,dev);
			rtk_rg_callback_pipe_cmd("%s %s mtu %d",ifconfig,dev,static_info.mtu);       
    	}


		/***IPv4 Setting***/
		if(static_info.ip_version==IPVER_V4ONLY || static_info.ip_version==IPVER_V4V6){

			/*if(static_info.ipv4_default_gateway_on){
				rtk_rg_callback_pipe_cmd("/bin/route add default gw %s",gw_ip);
			}
			
			//enable forwarding state
			rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv4/ip_forward");*/

			if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP){
				//NAT set by _rtk_rg_dhcpRequestByHwCallBack
				rtk_rg_callback_pipe_cmd("%s %s %s netmask %s broadcast %s",ifconfig,dev,ip,mask,brcast_ip);
			}
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE){
				//NAT set by _rtk_rg_pppoeBeforeDiagByHwCallBack
			}
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE){
			
			}
			else{		
				rtk_rg_callback_pipe_cmd("%s %s %s netmask %s broadcast %s",ifconfig,dev,ip,mask,brcast_ip);
				/*if(static_info.napt_enable){//add NAPT in iptables
					rtk_rg_callback_pipe_cmd("/bin/iptables -t nat -A POSTROUTING -o %s -j SNAT --to-source %s",dev,ip);
				}else{
					//pure routing
				}*/
			}
			
			//bring message to DDNS
			rtk_rg_callback_pipe_cmd("/bin/updateddctrl %s",dev);

			//do_sys_cmd("/bin/ip route add default via %s table 32",gw_ip);
			//do_sys_cmd("/bin/ip route add 192.168.150.0/24 dev nas0_0 table 32");
		}

		/***IPv6 Setting***/
		if(static_info.ip_version==IPVER_V6ONLY || static_info.ip_version==IPVER_V4V6){
			if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP){
				//NAT set by _rtk_rg_dhcpRequestByHwCallBack
				rtk_rg_callback_pipe_cmd("%s %s %s netmask %s broadcast %s",ifconfig,dev,ip,mask,brcast_ip);
			}
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE){
				//NAT set by _rtk_rg_pppoeBeforeDiagByHwCallBack
			}
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE){
			
			}
			else{		
				//current support ipv6_static only!!!
				rtk_rg_callback_pipe_cmd("%s %s add %s/%d",ifconfig,dev,_rtk_rg_inet_n6toa(&static_info.ipv6_addr.ipv6_addr[0]),static_info.ipv6_mask_length);
				//rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
				/*if(static_info.ipv6_default_gateway_on){
					rtk_rg_callback_pipe_cmd("/bin/route -A inet6 add ::/0 gw %s %s",_rtk_rg_inet_n6toa(&static_info.gateway_ipv6_addr.ipv6_addr[0]),dev);
				}*/			
			}
		}
		
    }
    else
    {
    	//add lan interface
        memcpy(ip,(void*)_inet_ntoa(intfInfo->lan_intf.ip_addr),16);
        memcpy(mask,(void*)_inet_ntoa(intfInfo->lan_intf.ip_network_mask),16);

		rtk_rg_callback_pipe_cmd("brctl addbr br0");
		rtk_rg_callback_pipe_cmd("brctl addif eth0");
#ifdef CONFIG_DUALBAND_CONCURRENT
		rtk_rg_callback_pipe_cmd("/bin/brctl addif br0 wlan1");
		rtk_rg_callback_pipe_cmd("%s wlan1 down",ifconfig);
#endif		
        rtk_rg_callback_pipe_cmd("%s eth0 down",ifconfig);
        rtk_rg_callback_pipe_cmd("%s br0 down",ifconfig);

		rtk_rg_callback_pipe_cmd("%s br0 hw ether %02x%02x%02x%02x%02x%02x",ifconfig,
				   intfInfo->lan_intf.gmac.octet[0],
				   intfInfo->lan_intf.gmac.octet[1],
				   intfInfo->lan_intf.gmac.octet[2],
				   intfInfo->lan_intf.gmac.octet[3],
				   intfInfo->lan_intf.gmac.octet[4],
				   intfInfo->lan_intf.gmac.octet[5]);


		/***IPv4 Setting***/
		if(intfInfo->lan_intf.ip_version==IPVER_V4ONLY || intfInfo->lan_intf.ip_version==IPVER_V4V6){
	        rtk_rg_callback_pipe_cmd("%s br0 %s netmask %s mtu %d",ifconfig,ip,mask,intfInfo->lan_intf.mtu);
		}

		/***IPv6 Setting***/
		if(intfInfo->lan_intf.ip_version==IPVER_V6ONLY || intfInfo->lan_intf.ip_version==IPVER_V4V6){
			rtk_rg_callback_pipe_cmd("%s br0 add %s/%d",ifconfig,_rtk_rg_inet_n6toa(&intfInfo->lan_intf.ipv6_addr.ipv6_addr[0]),intfInfo->lan_intf.ipv6_network_mask_length);
		}

		rtk_rg_callback_pipe_cmd("%s br0 up",ifconfig);
        rtk_rg_callback_pipe_cmd("%s eth0 up",ifconfig);


#ifdef CONFIG_DUALBAND_CONCURRENT
		rtk_rg_callback_pipe_cmd("%s wlan1 up",ifconfig);
#endif

        //patch for iptables
        rtk_rg_callback_pipe_cmd("/bin/iptables -A INPUT -s %s -d %s -j ACCEPT",ip,ip);
#if 0
		//restart udhcpd: this will make signal hang
		do_sys_cmd("/bin/udhcpd -B /var/udhcpd/udhcpd.conf");
#endif
    }

	//rtlglue_printf("intf[0] valid==%d\n",rg_db.systemGlobal.interfaceInfo[0].valid);
	//rtlglue_printf("intfIdx=%d	wan_type=%d@@@@@@n",*intfIdx,rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type);

    return SUCCESS;
}

//rg_db.systemGlobal.initParam.interfaceDelByHwCallBack;
int _rtk_rg_interfaceDelByHwCallBack(rtk_rg_intfInfo_t* intfInfo, int* intfIdx)
{
	int ret,dhcpc_pid;
	struct task_struct *t;
	
	if(intfInfo->is_wan==0){
		rtk_rg_callback_pipe_cmd("%s eth0 down",ifconfig);
		//rtk_rg_callback_pipe_cmd("%s br0 down",ifconfig);
	}else{
		char dev[IFNAMSIZ]; //device name, ex:nas0_0
		//sprintf(dev,"nas0_%d",(*intfIdx-1));
		sprintf(dev,"eth%d",(*intfIdx));

		if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP)
		{
			//1 FIXME:if there is two or more WAN use DHCP, this may not working
			dhcpc_pid = read_pid((char*)DHCPC_PID);
			CBACK("the dhcpc_pid is %d",dhcpc_pid);
			if (dhcpc_pid > 0)
			{
				rcu_read_lock();
				t = find_task_by_vpid(dhcpc_pid);
				if(t == NULL){
					CBACK("no such pid");
					rcu_read_unlock();
				}
				else
				{
					rcu_read_unlock();
					ret = send_sig(SIGTERM,t,0);//send_sig_info(SIG_TEST, &info, t);    //send the signal
					if (ret < 0) {
						CBACK("error sending signal\n");
					}
				}
				
				//kill(dhcpc_pid, SIGTERM);
				//rtk_rg_callback_pipe_cmd("kill -%d %d",SIGTERM,dhcpc_pid);//kill(dhcpc_pid, SIGTERM);
			}
		}
		else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
		{
			if(rg_db.systemGlobal.not_disconnect_ppp==0){
				rtk_rg_callback_pipe_cmd("/bin/spppctl down %d",*intfIdx);
				//let spppd to down the eth wan for us, otherwise PADT won't send out
				return SUCCESS;
			}else{
				//don't call spppctl to disconnet interface pppX, because this case is "Server disconnet problem".
				//so, do nothing!
				rg_db.systemGlobal.not_disconnect_ppp=0;
			}
		}
		
		//wan-intf
		rtk_rg_callback_pipe_cmd("%s %s down",ifconfig,dev);
	}

    return SUCCESS;
}

//rg_db.systemGlobal.initParam.dhcpRequestByHwCallBack;
int _rtk_rg_dhcpRequestByHwCallBack(int* intfIdx){

	char dev[IFNAMSIZ]; //device name, ex:nas0_0
	int dev_idx;
	//int vid;
	rtk_rg_intfInfo_t intfInfo;
	CBACK("%s is called!!!",__func__);

	memcpy(&intfInfo,&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo,sizeof(rtk_rg_intfInfo_t));

	////check wan_intf info & set dev name
	if((*intfIdx>7) || (*intfIdx<0)){
		CBACK("invalid wan_intf_idx\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else{
		//dev_idx = (*intfIdx-1);
		dev_idx = (*intfIdx);
		sprintf(dev,"eth%d",dev_idx);
	}
	
	if(rg_db.systemGlobal.interfaceInfo[*intfIdx].valid==0){
		CBACK("Assigned wan_intf_idx is not valid.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(!intfInfo.is_wan){
		CBACK("Assigned wan_intf_idx is not wan.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(intfInfo.wan_intf.wan_intf_conf.wan_type!=RTK_RG_DHCP){
		CBACK("Assigned wan_intf_idx is not DHCP.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}
	
	//ps cmd for bring up nas0 & nas0_x for dhcp

	//do cmd
    rtk_rg_callback_pipe_cmd("echo %d wan0 > /proc/rtl8686gmac/dev_port_mapping",intfInfo.wan_intf.wan_intf_conf.wan_port_idx);
	
	//rtk_rg_callback_pipe_cmd("%s nas0 up",ifconfig);
	rtk_rg_callback_pipe_cmd("%s %s down",ifconfig,dev);
	//rtk_rg_callback_pipe_cmd("/bin/ethctl remsmux bridge nas0 %s",dev);

	//smux: ignor napt cmd in ppp, just care vlan
	/*if(intfInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on){
		//have vlan tag
		vid = intfInfo.wan_intf.wan_intf_conf.egress_vlan_id;
		rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux ipoe nas0 %s nonapt vlan %d",dev,vid); 
	}else{
		//no vlan tag
		rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux ipoe nas0 %s ",dev);
	}*/
	
	rtk_rg_callback_pipe_cmd("%s %s hw ether %02x%02x%02x%02x%02x%02x",ifconfig,dev,
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[0],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[1],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[2],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[3],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[4],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[5]);

	rtk_rg_callback_pipe_cmd("/bin/ifconfig %s txqueuelen 10",dev);
	rtk_rg_callback_pipe_cmd("/bin/ifconfig %s mtu 1500",dev);
	rtk_rg_callback_pipe_cmd("/bin/iptables -A INPUT -i %s -p UDP --dport 69 -d 255.255.255.255 -m state --state NEW -j ACCEPT",dev);

	rtk_rg_callback_pipe_cmd("%s %s up",ifconfig,dev);

	rtk_rg_callback_pipe_cmd("/bin/udhcpc -i %s -W &",dev);
	//rtk_rg_callback_pipe_cmd("/bin/iptables -t nat -A POSTROUTING -o %s -j MASQUERADE",dev);
	
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.pppoeBeforeDiagByHwCallBack;
int _rtk_rg_pppoeBeforeDiagByHwCallBack(rtk_rg_pppoeClientInfoBeforeDial_t* before_diag, int* intfIdx){


	char dev[IFNAMSIZ]; //device name, ex:nas0_0
	int dev_idx;
	//int vid;
	rtk_rg_intfInfo_t intfInfo;
	memcpy(&intfInfo,&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo,sizeof(rtk_rg_intfInfo_t));

	////check wan_intf info & set dev name
	if((*intfIdx>7) || (*intfIdx<0)){
		CBACK("invalid wan_intf_idx\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else{
		//dev_idx = (*intfIdx-1);
		dev_idx = (*intfIdx);	
		sprintf(dev,"eth%d",dev_idx);
	}
	
	if(rg_db.systemGlobal.interfaceInfo[*intfIdx].valid==0){
		CBACK("Assigned wan_intf_idx is not valid.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(!intfInfo.is_wan){
		CBACK("Assigned wan_intf_idx is not wan.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(intfInfo.wan_intf.wan_intf_conf.wan_type!=RTK_RG_PPPoE){
		CBACK("Assigned wan_intf_idx is not PPPoE.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}
	
	//ps cmd for bring up nas0 & nas0_x for ppp	

#if defined(CONFIG_APOLLO)
	//do cmd
    rtk_rg_callback_pipe_cmd("echo %d wan0 > /proc/rtl8686gmac/dev_port_mapping",intfInfo.wan_intf.wan_intf_conf.wan_port_idx);
#endif
	
	//rtk_rg_callback_pipe_cmd("%s nas0 up",ifconfig);
	//rtk_rg_callback_pipe_cmd("/bin/ethctl remsmux bridge nas0 %s",dev);

	//smux: ignor napt cmd in ppp, just care vlan
	/*if(intfInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on){
		//have vlan tag
		vid = intfInfo.wan_intf.wan_intf_conf.egress_vlan_id;
		rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux pppoe nas0 %s nonapt vlan %d",dev,vid); 
	}else{
		//no vlan tag
		rtk_rg_callback_pipe_cmd("/bin/ethctl addsmux pppoe nas0 %s ",dev);
	}*/
	
	rtk_rg_callback_pipe_cmd("%s %s hw ether %02x%02x%02x%02x%02x%02x",ifconfig,dev,
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[0],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[1],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[2],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[3],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[4],
			   intfInfo.wan_intf.wan_intf_conf.gmac.octet[5]);
	

	rtk_rg_callback_pipe_cmd("%s %s txqueuelen 10",ifconfig,dev);
	rtk_rg_callback_pipe_cmd("%s %s up",ifconfig,dev);


	rtk_rg_callback_pipe_cmd("/bin/ifconfig ppp%d txqueuelen 0",dev_idx);


	/*set proc for Radvd create global v6 ip*/
	rtk_rg_callback_pipe_cmd("/bin/echo 1 > /proc/sys/net/ipv6/conf/ppp%d/autoconf",dev_idx);
	rtk_rg_callback_pipe_cmd("/bin/echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
	rtk_rg_callback_pipe_cmd("/bin/echo 0 > /proc/sys/net/ipv6/conf/ppp%d/forwarding",dev_idx);
	//enable,disable ipv6 ppp diag
	//rtk_rg_callback_pipe_cmd("/bin/echo 1 > proc/sys/net/ipv6/conf/ppp%d/disable_ipv6",dev_idx);

	
	rtk_rg_callback_pipe_cmd("/bin/spppctl add %d pppoe %s username %s password %s gw 1 mru 1492 nohwnat ippt 0 debug 0 ipt 2 ",dev_idx,dev,before_diag->username,before_diag->password);
	rtk_rg_callback_pipe_cmd("/bin/spppctl katimer 100");

	//1 FIX ME: PPPoE disconnet issue
	/*Need to judge if PPPoE diag failed!!!*/
	//rg_kernel.ppp_diaged[dev_idx]=ENABLED;

	rtk_rg_callback_pipe_cmd("/bin/iptables -t nat -A POSTROUTING -o ppp%d -j MASQUERADE",dev_idx);
	
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.arpAddByHwCallBack;
int _rtk_rg_arpAddByHwCallBack(rtk_rg_arpInfo_t* arpInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.arpDelByHwCallBack;
int _rtk_rg_arpDelByHwCallBack(rtk_rg_arpInfo_t* arpInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.macAddByHwCallBack=NULL;
int _rtk_rg_macAddByHwCallBack(rtk_rg_macEntry_t* macInfo){
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.macDelByHwCallBack=NULL;
int _rtk_rg_macDelByHwCallBack(rtk_rg_macEntry_t* macInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.naptAddByHwCallBack=NULL;
int _rtk_rg_naptAddByHwCallBack(rtk_rg_naptInfo_t* naptInfo){
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.naptDelByHwCallBack=NULL;
int _rtk_rg_naptDelByHwCallBack(rtk_rg_naptInfo_t* naptInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.routingAddByHwCallBack=NULL;
int _rtk_rg_routingAddByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.routingDelByHwCallBack=NULL;
int _rtk_rg_routingDelByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.bindingAddByHwCallBack=NULL;
int _rtk_rg_bindingAddByHwCallBack(rtk_rg_bindingEntry_t* bindingInfo){
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.bindingDelByHwCallBack=NULL;
int _rtk_rg_bindingDelByHwCallBack(rtk_rg_bindingEntry_t* bindingInfo){
	return SUCCESS;
}


int _rtk_rg_neighborAddByHwCallBack(rtk_rg_neighborInfo_t* neighborInfo){
	return SUCCESS;
}

int _rtk_rg_neighborDelByHwCallBack(rtk_rg_neighborInfo_t* neighborInfo){
	return SUCCESS;
}

int _rtk_rg_v6RoutingAddByHwCallBack(rtk_rg_ipv6RoutingEntry_t* cb_routv6Et){
	return SUCCESS;
}

int _rtk_rg_v6RoutingDelByHwCallBack(rtk_rg_ipv6RoutingEntry_t* cb_routv6Et){
	return SUCCESS;
}

#else		//normal multi eth wan

//callback for rg init
int _rtk_rg_initParameterSetByHwCallBack(void)
{
	int i,process_pid,ret;
	struct task_struct *t;
	
	//Leo: The /var/tmp will be created after boad had been executed
	//     So, we check the path /var/tmp instead of boa pid
	if (0!=rtk_rg_callback_pipe_cmd("cd /var/tmp")) {
		rtk_rg_callback_pipe_cmd("/etc/init.d/rc2");
	}
	
	rtk_rg_callback_pipe_cmd("%s link set br0 down",ipUtility);
	rtk_rg_callback_pipe_cmd("brctl delbr br0");
	rtk_rg_callback_pipe_cmd("brctl addbr br0");
	rtk_rg_callback_pipe_cmd("brctl setfd br0 0");	
	rtk_rg_callback_pipe_cmd("%s addr flush dev br0",ipUtility);		//reset br0's address information
	
#ifdef CONFIG_RG_WMUX_SUPPORT
	wmux_cleanup_devices();
#endif

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
	for(i=0;i<RTK_RG_PORT_PON;i++)
	{
		rtk_rg_callback_pipe_cmd("%s link set eth0.%d down",ipUtility,i+2);
		rtk_rg_callback_pipe_cmd("brctl addif br0 eth0.%d",i+2);				
	}
//#else
#endif
	rtk_rg_callback_pipe_cmd("brctl addif br0 eth0");

	rtk_rg_callback_pipe_cmd("%s link set eth0 down",ipUtility);
//#endif
	
	rtk_rg_callback_pipe_cmd("%s link set %s down",ipUtility,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME);

	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++)
	{
		//rtk_rg_callback_pipe_cmd("brctl delbr br_%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,i);
		rtk_rg_callback_pipe_cmd("%s link set br_%s_%d down",ipUtility,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,i);
		rtk_rg_callback_pipe_cmd("ethctl remsmux bridge %s %s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,i);

#ifdef CONFIG_GPON_FEATURE
		//20150312LUKE: trigger OMCI to remove related CF rules
		rtk_rg_callback_pipe_cmd("echo %d -1 -1 -1 0 > /proc/omci/wanInfo",i);
#endif
		//1 FIX ME: PPPoE disconnet issue
		/*
		if(rg_kernel.ppp_diaged[i]==ENABLED){
			rtk_rg_callback_pipe_cmd("/bin/spppctl down ppp%d",i);
			rg_kernel.ppp_diaged[i]=DISABLED;
		}
		*/
		//rtk_rg_callback_pipe_cmd("/bin/spppctl down ppp%d",i);
		//rtk_rg_callback_pipe_cmd("/bin/spppd");
	}

#if defined(CONFIG_APOLLO)
	for(i=0;i<RTK_RG_MAC_PORT_MAX;i++)
	{
		//reset all port-mapping to eth0
		rtk_rg_callback_pipe_cmd("echo %d eth0 > /proc/rtl8686gmac/dev_port_mapping",i);
	}
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
	for(i=0;i<RTK_RG_PORT_PON;i++)
	{
		//reset all port-mapping to multi-LAN dev
		rtk_rg_callback_pipe_cmd("echo %d eth0.%d > /proc/rtl8686gmac/dev_port_mapping",i,i+2);
	}
#endif
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) //end defined(CONFIG_APOLLO) elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 

	CBACK("FIXME XDSL callback\n");

#endif //end defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 

	//Clear all udhcpc daemon
	_rg_delete_dhcpc(NULL,1);

	//Clear all udhcpd daemon
	_rg_delete_dhcpd(0,1);

	//Clear all radvd daemon
	_rg_delete_radvd(0,1);

	//Clear all vconfig settings
	rtk_rg_callback_pipe_cmd("`for FILE in $(ifconfig -a | grep 'br0.' | sed 's/[ \\t].*//;/^$/d'); do vconfig rem $FILE > /dev/null 2>&1; done`");
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
	rtk_rg_callback_pipe_cmd("`for FILE in $(ifconfig -a | grep 'eth0.2.' | sed 's/[ \\t].*//;/^$/d'); do vconfig rem $FILE > /dev/null 2>&1; done`");
	rtk_rg_callback_pipe_cmd("`for FILE in $(ifconfig -a | grep 'eth0.3.' | sed 's/[ \\t].*//;/^$/d'); do vconfig rem $FILE > /dev/null 2>&1; done`");
	rtk_rg_callback_pipe_cmd("`for FILE in $(ifconfig -a | grep 'eth0.4.' | sed 's/[ \\t].*//;/^$/d'); do vconfig rem $FILE > /dev/null 2>&1; done`");
	rtk_rg_callback_pipe_cmd("`for FILE in $(ifconfig -a | grep 'eth0.5.' | sed 's/[ \\t].*//;/^$/d'); do vconfig rem $FILE > /dev/null 2>&1; done`");
//#else
#endif
	rtk_rg_callback_pipe_cmd("`for FILE in $(ifconfig -a | grep 'eth0.' | sed 's/[ \\t].*//;/^$/d'); do vconfig rem $FILE > /dev/null 2>&1; done`");
//#endif

	//Clear all un-terminated PPPoE connections
	rtk_rg_callback_pipe_cmd("`for FILE in $(ifconfig -a | grep 'ppp' | sed 's/[ \\t].*//;s/ppp//;/^$/d'); do spppctl del $FILE > /dev/null 2>&1; done`");

	//Clear un-terminated IGMP proxy client
	//1 FIXME:if there is two or more WAN use IGMP Proxy, this may not working
	process_pid = read_pid((char*)IGMPPROXY_PID);
	CBACK("the igmp_proxy_pid is %d",process_pid);
	if (process_pid > 0)
	{
		rcu_read_lock();
		t = find_task_by_vpid(process_pid);
		if(t == NULL){
			CBACK("no such pid");
			rcu_read_unlock();
		}
		else
		{
			rcu_read_unlock();
			ret = send_sig(SIGTERM,t,0);//send_sig_info(SIG_TEST, &info, t);    //send the signal
			if (ret < 0) {
				CBACK("error sending signal\n");
			}
		}
	}

	//Clear all Firewall rules
	rtk_rg_callback_pipe_cmd("iptables -F");
	rtk_rg_callback_pipe_cmd("iptables -X");
	rtk_rg_callback_pipe_cmd("iptables -P INPUT ACCEPT");
	rtk_rg_callback_pipe_cmd("iptables -P FORWARD DROP");	//disable protocol stack forwarding IPv4
	rtk_rg_callback_pipe_cmd("iptables -P OUTPUT ACCEPT");
	//20140627LUKE:fix rg init didn't clear SNAT rules problem
	rtk_rg_callback_pipe_cmd("iptables -t nat -F");
	rtk_rg_callback_pipe_cmd("iptables -t nat -X");
	rtk_rg_callback_pipe_cmd("iptables -t nat -P PREROUTING ACCEPT");
	rtk_rg_callback_pipe_cmd("iptables -t nat -P POSTROUTING ACCEPT");
	rtk_rg_callback_pipe_cmd("iptables -t nat -P OUTPUT ACCEPT");
	
	rtk_rg_callback_pipe_cmd("ip6tables -F");
	rtk_rg_callback_pipe_cmd("ip6tables -X");
	rtk_rg_callback_pipe_cmd("ip6tables -P INPUT ACCEPT");
	rtk_rg_callback_pipe_cmd("ip6tables -P FORWARD DROP");	//disable protocol stack forwarding IPv6
	rtk_rg_callback_pipe_cmd("ip6tables -P OUTPUT ACCEPT");
	rtk_rg_callback_pipe_cmd("ebtables -X");
	rtk_rg_callback_pipe_cmd("ebtables -F");
	rtk_rg_callback_pipe_cmd("ebtables -t broute -X");
	rtk_rg_callback_pipe_cmd("ebtables -t broute -F");
	rtk_rg_callback_pipe_cmd("ebtables -P INPUT ACCEPT");
	rtk_rg_callback_pipe_cmd("ebtables -P FORWARD DROP");
	rtk_rg_callback_pipe_cmd("ebtables -P OUTPUT ACCEPT");

	//patch for ipatbles
	_delete_iptables_snat();

	//20140617LUKE:we do not let protocol stack forward ANY packet, so just change default policy above.
	//fix ptorocol stack send duplicated broadcast and multicast packets problem
	/*rtk_rg_callback_pipe_cmd("ebtables -D FORWARD -j disBCMC");
	rtk_rg_callback_pipe_cmd("ebtables -X disBCMC");
	
	rtk_rg_callback_pipe_cmd("ebtables -N disBCMC");
	rtk_rg_callback_pipe_cmd("ebtables -A disBCMC -d Broadcast -j DROP");
	rtk_rg_callback_pipe_cmd("ebtables -A disBCMC -d Multicast -j DROP");
	rtk_rg_callback_pipe_cmd("ebtables -I FORWARD 1 -j disBCMC");*/

	//enable IGMP query if IGMP snooping is turn on in RG
	if(rg_db.systemGlobal.initParam.igmpSnoopingEnable)
	{
		rtk_rg_callback_pipe_cmd("echo 1 > /proc/br_igmpquery");
	}
	else
	{
		rtk_rg_callback_pipe_cmd("echo 0 > /proc/br_igmpquery");
	}

	//flush all route table
	rtk_rg_callback_pipe_cmd("%s route flush table all",ipUtility);

#ifdef CONFIG_DUALBAND_CONCURRENT	
	//reset route for IPC
	rtk_rg_callback_pipe_cmd("%s link set vwlan down",ipUtility);
	rtk_rg_callback_pipe_cmd("%s link set vwlan up",ipUtility);
#endif
	//reset route for lo
	rtk_rg_callback_pipe_cmd("%s route add 127.0.0.0/24 dev lo",ipUtility);

	//20141021 Leo Force system to release memory
	//rtk_rg_callback_pipe_cmd("sync");
	rtk_rg_callback_pipe_cmd("echo 3 > /proc/sys/vm/drop_caches");
	rtk_rg_callback_pipe_cmd("echo 0 > /proc/sys/vm/drop_caches");

	//20141205LUKE: turn on eth0 in case packet won't be received by NIC
	rtk_rg_callback_pipe_cmd("%s link set eth0 up",ipUtility);

    return SUCCESS;

}

void _rtk_rg_checkVlanBindingAndAddVconfig(rtk_rg_port_idx_t portIdx, int vlanId)
{
	int i;
	
	for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
	{
		if(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->port_mask.portmask&(0x1<<portIdx))
		{
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
			if(portIdx>=RTK_RG_PORT_CPU)
			{
				//add virtual ether device for VLAN
				rtk_rg_callback_pipe_cmd("vconfig add eth0 %d",vlanId);
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,vlanId,
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[0],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[1],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[2],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[3],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[4],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[5]);
				
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d up",ipUtility,vlanId);
				rtk_rg_callback_pipe_cmd("brctl addif br0 eth0.%d",vlanId);
				rtk_rg_callback_pipe_cmd("ebtables -t broute -A BROUTING -i eth0 -p 802_1Q -j DROP");
			}
			else
			{
				//add virtual ether device for VLAN
				rtk_rg_callback_pipe_cmd("vconfig add eth0.%d %d",2+portIdx,vlanId);
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d.%d address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,2+portIdx,vlanId,
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[0],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[1],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[2],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[3],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[4],
					rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[5]);
				
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d.%d up",ipUtility,2+portIdx,vlanId);
				rtk_rg_callback_pipe_cmd("brctl addif br0 eth0.%d.%d",2+portIdx,vlanId);
				rtk_rg_callback_pipe_cmd("ebtables -t broute -A BROUTING -i eth0.%d -p 802_1Q -j DROP",2+portIdx);
			}
#else
			//add virtual ether device for VLAN
			rtk_rg_callback_pipe_cmd("vconfig add eth0 %d",vlanId);
			rtk_rg_callback_pipe_cmd("%s link set eth0.%d address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,vlanId,
				rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[0],
				rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[1],
				rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[2],
				rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[3],
				rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[4],
				rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet[5]);
			
			rtk_rg_callback_pipe_cmd("%s link set eth0.%d up",ipUtility,vlanId);
			rtk_rg_callback_pipe_cmd("brctl addif br0 eth0.%d",vlanId);
			rtk_rg_callback_pipe_cmd("ebtables -t broute -A BROUTING -i eth0 -p 802_1Q -j DROP");
#endif
			break;
		}
	}
}

//rg_db.systemGlobal.initParam.interfaceAddByHwCallBack;
int _rtk_rg_interfaceAddByHwCallBack(rtk_rg_intfInfo_t* intfInfo, int* intfIdx)
{
	int i,taggingCPU;
	unsigned char ip[16];
	unsigned char gw_ip[16];
    unsigned char mask[16];
	unsigned char brcast_ip[16];
	
    if(intfInfo->is_wan)
    {
        //add wan interface
        int i;
        char dev[IFNAMSIZ]; //device name, ex:nas0_0
        char* wan_type; //wan type: ex:PPPoE
        int vid=0;
		rtk_ip_addr_t bc_ip; //broadCast ip for wan intf

        switch(intfInfo->wan_intf.wan_intf_conf.wan_type)
        {
	        case RTK_RG_STATIC:
	            wan_type="ipoe";
	            break;
	        case RTK_RG_DHCP:
	            wan_type="dhcp";
	            break;
	        case RTK_RG_PPPoE:
	            wan_type="pppoe";
	            break;
	        case RTK_RG_BRIDGE:
	            wan_type="bridge";
	            break;
			case RTK_RG_PPTP:
	            wan_type="pptp";
	            break;
			case RTK_RG_L2TP:
	            wan_type="l2tp";
	            break;
			case RTK_RG_DSLITE:
	            wan_type="dslite";
	            break;
			case RTK_RG_PPPoE_DSLITE:
	            wan_type="pppoe_dslite";
	            break;
	        default:
				wan_type=" ";
	            break;
        }

        if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
            vid = intfInfo->wan_intf.wan_intf_conf.egress_vlan_id;

		if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD)
			sprintf(dev,"wlan0-vxd");
		else if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD)
			sprintf(dev,"wlan1-vxd");
		else
			sprintf(dev,"%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,(*intfIdx));

#if defined(CONFIG_APOLLO)
        //do cmd
        rtk_rg_callback_pipe_cmd("echo %d %s > /proc/rtl8686gmac/dev_port_mapping",intfInfo->wan_intf.wan_intf_conf.wan_port_idx,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME);

#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) //end defined(CONFIG_APOLLO) elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 

		CBACK("FIXME XDSL callback\n");

#endif //end defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 

		//create nas0_X
		if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE)
		{
			if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WIRED)
			{
				rtk_rg_callback_pipe_cmd("%s link set %s up",ipUtility,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME);
#ifdef CONFIG_RG_WMUX_SUPPORT
				rtk_rg_callback_pipe_cmd("echo %s %d > /proc/rg/wmux_add",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,(*intfIdx));
#else
				rtk_rg_callback_pipe_cmd("ethctl remsmux bridge %s %s",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev);
				
				if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on){
					//VALN TAG
					rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s napt brpppoe vlan %d",wan_type,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev,vid); 
				}
				else{
					rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s napt brpppoe",wan_type,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev);
				}
#endif
			}
			
			rtk_rg_callback_pipe_cmd("%s link set %s address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,dev,
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[0],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[1],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[2],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[3],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[4],
					   intfInfo->wan_intf.wan_intf_conf.gmac.octet[5]);

			rtk_rg_callback_pipe_cmd("%s link set %s txqueuelen 10",ipUtility,dev);
			rtk_rg_callback_pipe_cmd("%s link set %s up",ipUtility,dev);
			rtk_rg_callback_pipe_cmd("brctl addif br0 %s",dev);

			//20140620LUKE:if add bridge tagged WAN, we should add virtual ether device for it's VLAN
			if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
			{
				rtk_rg_callback_pipe_cmd("vconfig add br0 %d",intfInfo->wan_intf.wan_intf_conf.egress_vlan_id);
				rtk_rg_callback_pipe_cmd("%s link set br0.%d up",ipUtility,intfInfo->wan_intf.wan_intf_conf.egress_vlan_id);
			}
		}
		else if(strlen(intfInfo->intf_name))	//not called by rtk_rg_apollo_wanInterface_add, but by _rtk_rg_internal_wanSet
		{	
			rtk_rg_ipStaticInfo_t *static_info;
			if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP)
			{
				static_info=&intfInfo->wan_intf.dhcp_client_info.hw_info;
				memcpy(ip,(void*)_inet_ntoa(static_info->ip_addr),16);
				memcpy(gw_ip,(void*)_inet_ntoa(static_info->gateway_ipv4_addr),16);
		        memcpy(mask,(void*)_inet_ntoa(static_info->ip_network_mask),16);

				//configure wan interface in protocol stack
				_rg_configure_protocol_stack(*intfIdx,dev);
			}
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
			{
				static_info=&intfInfo->wan_intf.pppoe_info.after_dial.hw_info;
				memcpy(ip,(void*)_inet_ntoa(static_info->ip_addr),16);
				memcpy(gw_ip,(void*)_inet_ntoa(static_info->gateway_ipv4_addr),16);
		        memcpy(mask,(void*)_inet_ntoa(static_info->ip_network_mask),16);
			}
			else
			{
				if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP)
					static_info=&intfInfo->wan_intf.pptp_info.after_dial.hw_info;
				else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP)
					static_info=&intfInfo->wan_intf.l2tp_info.after_dial.hw_info;
				else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE)
					static_info=&intfInfo->wan_intf.dslite_info.static_info;
				else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
					static_info=&intfInfo->wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info;
				else	
					static_info=&intfInfo->wan_intf.static_info;
		        memcpy(ip,(void*)_inet_ntoa(static_info->ip_addr),16);
				memcpy(gw_ip,(void*)_inet_ntoa(static_info->gateway_ipv4_addr),16);
		        memcpy(mask,(void*)_inet_ntoa(static_info->ip_network_mask),16);

				//caculate broacast ip
				bc_ip = static_info->gateway_ipv4_addr & static_info->ip_network_mask;

				for(i=0;i<32;i++){
					if(static_info->ip_network_mask & (1<<i)){
						//wan domain (mask)
					}else{
						//wan broacast ip
						bc_ip |= (1<<i);
					}
				}
				memcpy(brcast_ip,(void*)_inet_ntoa(bc_ip),16);
				
				//set STATIC protocal-stack
				if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WIRED)
				{
					rtk_rg_callback_pipe_cmd("%s link set %s up",ipUtility,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME);
#ifdef CONFIG_RG_WMUX_SUPPORT
					rtk_rg_callback_pipe_cmd("echo %s %d > /proc/rg/wmux_add",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,(*intfIdx));
#else
			        rtk_rg_callback_pipe_cmd("ethctl remsmux bridge %s %s",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev);

					if(static_info->napt_enable && intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
			        {	
			        	//NAPT + VALN TAG
			            rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s napt vlan %d",wan_type,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev,vid); 
			        }
			        else if(static_info->napt_enable)
			        {
			        	//NAPT
			            rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s napt",wan_type,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev);
			        }
					else if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on){
						//VALN TAG
						rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s nonapt vlan %d",wan_type,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev,vid); 
					}
					else{
						rtk_rg_callback_pipe_cmd("ethctl addsmux %s %s %s",wan_type,CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev);
					}
#endif
				}
				rtk_rg_callback_pipe_cmd("%s link set %s address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,dev,
		                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[0],
		                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[1],
		                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[2],
		                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[3],
		                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[4],
		                   intfInfo->wan_intf.wan_intf_conf.gmac.octet[5]);

				if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WIRED)
		        	rtk_rg_callback_pipe_cmd("%s link set %s up",ipUtility,dev);

				//setup MTU
				rtk_rg_callback_pipe_cmd("%s link set %s mtu %d",ipUtility,dev,static_info->mtu);
			}

			/***IPv4 Setting***/
			if(static_info->ip_version==IPVER_V4ONLY || static_info->ip_version==IPVER_V4V6)
			{
				//enable forwarding state
				rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv4/ip_forward");

				//20140616LUKE:before add address, forced delete once!(for change IP at same interface)
				rtk_rg_callback_pipe_cmd("%s -4 addr flush dev %s",ipUtility,dev);
				rtk_rg_callback_pipe_cmd("%s -4 route flush dev %s",ipUtility,dev);
				if(static_info->ip_version==IPVER_V4ONLY)
				{
					rtk_rg_callback_pipe_cmd("%s -6 addr flush dev %s",ipUtility,dev);
					rtk_rg_callback_pipe_cmd("%s -6 route flush dev %s",ipUtility,dev);
				}

				//STATIC or DHCP
				switch(intfInfo->wan_intf.wan_intf_conf.wan_type)
				{
					case RTK_RG_STATIC:
					case RTK_RG_DHCP:
					case RTK_RG_DSLITE:
					case RTK_RG_PPPoE_DSLITE:
						rtk_rg_callback_pipe_cmd("%s addr add %s/%s broadcast + dev %s",ipUtility,ip,mask,dev);
						if(static_info->ipv4_default_gateway_on)
						{
							rtk_rg_callback_pipe_cmd("%s route add default via %s dev %s mtu %d",ipUtility,gw_ip,dev,static_info->mtu);
						}
						
						if((intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_STATIC)&&(static_info->napt_enable && static_info->ip_addr!=0))//add NAPT in iptables, if routing no need this
						{
							//20140625LUKE:delete our IP once before add, in case IPv4 didn't change in re-set staticInfo
							rtk_rg_callback_pipe_cmd("iptables -t nat -D POSTROUTING -o %s -j SNAT --to-source %s",dev,ip);
							rtk_rg_callback_pipe_cmd("iptables -t nat -A POSTROUTING -o %s -j SNAT --to-source %s",dev,ip);
						}
						break;
					case RTK_RG_PPTP:
					case RTK_RG_L2TP:
						if(static_info->ipv4_default_gateway_on)
						{
							rtk_rg_callback_pipe_cmd("%s route add default via %s dev ppp%d mtu %d",ipUtility,gw_ip,(*intfIdx),static_info->mtu);
						}	
						break;
					default:
						break;
				}
				
				//bring message to DDNS
				//rtk_rg_callback_pipe_cmd("/bin/updateddctrl %s",dev);

				//20150225LUKE: for ARP request, we should check IP for incoming interface				
#if 0				
				rtk_rg_callback_pipe_cmd("brctl addbr br_%s",dev);
				rtk_rg_callback_pipe_cmd("brctl addif br_%s %s",dev,dev);
				rtk_rg_callback_pipe_cmd("brctl setfd br_%s 0",dev);	//set forward delay
				rtk_rg_callback_pipe_cmd("%s link set br_%s up",ipUtility,dev);
				rtk_rg_callback_pipe_cmd("ebtables -A INPUT -i %s -p arp --arp-ip-dst ! %s -j DROP",dev,ip);
#endif				
			}

			/***IPv6 Setting***/
			if(static_info->ip_version==IPVER_V6ONLY || static_info->ip_version==IPVER_V4V6)
			{
				//20140630LUKE:for setup IPv6 in protocol stack, the MTU need to be bigger than 1280!!
				if(static_info->mtu<1280)
					WARNING("for IPv6, the smallest MTU is 1280, otherwise IPv6 address and route could not be added.");
				
				//20140616LUKE:before add address, forced delete once!(for change IP at same interface)
				rtk_rg_callback_pipe_cmd("%s -6 addr flush dev %s",ipUtility,dev);
				rtk_rg_callback_pipe_cmd("%s -6 route flush dev %s",ipUtility,dev);
				if(static_info->ip_version==IPVER_V6ONLY)
				{
					rtk_rg_callback_pipe_cmd("%s -4 addr flush dev %s",ipUtility,dev);
					rtk_rg_callback_pipe_cmd("%s -4 route flush dev %s",ipUtility,dev);
				}
				
				if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP)
				{
					//NAT set by _rtk_rg_dhcpRequestByHwCallBack
					rtk_rg_callback_pipe_cmd("%s addr add %s/%d dev %s",ipUtility,_rtk_rg_inet_n6toa(static_info->ipv6_addr.ipv6_addr),static_info->ipv6_mask_length,dev);
				}
				else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
				{
					//NAT set by _rtk_rg_pppoeBeforeDiagByHwCallBack
				}
				else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE)
				{
				}
				else
				{		
					//current support ipv6_static only!!!
					rtk_rg_callback_pipe_cmd("%s addr add %s/%d dev %s",ipUtility,_rtk_rg_inet_n6toa(static_info->ipv6_addr.ipv6_addr),static_info->ipv6_mask_length,dev);
					rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
					if(static_info->ipv6_default_gateway_on)
					{
						rtk_rg_callback_pipe_cmd("%s route add default via %s dev %s mtu %d",ipUtility,_rtk_rg_inet_n6toa(static_info->gateway_ipv6_addr.ipv6_addr),dev,static_info->mtu);
					}

					if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE)
						rtk_rg_callback_pipe_cmd("%s addr add %s/%d dev %s",ipUtility,_rtk_rg_inet_n6toa(intfInfo->wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr),static_info->ipv6_mask_length,dev);
					else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
						rtk_rg_callback_pipe_cmd("%s addr add %s/%d dev %s",ipUtility,_rtk_rg_inet_n6toa(intfInfo->wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr),static_info->ipv6_mask_length,dev);
				}
			}
		}
#ifdef CONFIG_GPON_FEATURE
		//20150312LUKE: trigger OMCI to create related CF rules
		if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WIRED)
		{
			if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
			{
				if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
					rtk_rg_callback_pipe_cmd("echo %d %d %d 0 1 > /proc/omci/wanInfo",*intfIdx,intfInfo->wan_intf.wan_intf_conf.egress_vlan_id,
						intfInfo->wan_intf.wan_intf_conf.vlan_based_pri_enable?intfInfo->wan_intf.wan_intf_conf.vlan_based_pri:-1);
				else
					rtk_rg_callback_pipe_cmd("echo %d -1 -1 0 1 > /proc/omci/wanInfo",*intfIdx);
			}
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE)
			{
				if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
					rtk_rg_callback_pipe_cmd("echo %d %d %d 2 1 > /proc/omci/wanInfo",*intfIdx,intfInfo->wan_intf.wan_intf_conf.egress_vlan_id,
						intfInfo->wan_intf.wan_intf_conf.vlan_based_pri_enable?intfInfo->wan_intf.wan_intf_conf.vlan_based_pri:-1);
				else
					rtk_rg_callback_pipe_cmd("echo %d -1 -1 2 1 > /proc/omci/wanInfo",*intfIdx);
			}
			else if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
				rtk_rg_callback_pipe_cmd("echo %d %d %d 1 1 > /proc/omci/wanInfo",*intfIdx,intfInfo->wan_intf.wan_intf_conf.egress_vlan_id,
					intfInfo->wan_intf.wan_intf_conf.vlan_based_pri_enable?intfInfo->wan_intf.wan_intf_conf.vlan_based_pri:-1);
			else
				rtk_rg_callback_pipe_cmd("echo %d -1 -1 1 1 > /proc/omci/wanInfo",*intfIdx);
		}
#endif
    }
    else
    {
		taggingCPU=(intfInfo->lan_intf.untag_mask.portmask&(0x1<<RTK_RG_MAC_PORT_CPU))?0:1;
	
    	//add lan interface
        memcpy(ip,(void*)_inet_ntoa(intfInfo->lan_intf.ip_addr),16);
        memcpy(mask,(void*)_inet_ntoa(intfInfo->lan_intf.ip_network_mask),16);
		
        //rtk_rg_callback_pipe_cmd("%s eth0 down",ifconfig);
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
		for(i=0;i<RTK_RG_PORT_PON;i++)
		{
			if(intfInfo->lan_intf.port_mask.portmask&(0x1<<i))
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d down",ipUtility,i+2);
		}
//#else
#endif
        rtk_rg_callback_pipe_cmd("%s link set eth0 down",ipUtility);
//#endif
		rtk_rg_callback_pipe_cmd("brctl addif br0 wlan0");
#ifdef CONFIG_DUALBAND_CONCURRENT
		rtk_rg_callback_pipe_cmd("brctl addif br0 wlan1");
		rtk_rg_callback_pipe_cmd("echo 3 > /proc/vwlan");	//echo 3 means down
#elif defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
		rtk_rg_callback_pipe_cmd("brctl addif br0 wlan1");
#endif

		//1 FIXME:br0.vid should match br0's ether address, but it's not definitely required
		if(!taggingCPU)
			rtk_rg_callback_pipe_cmd("%s link set br0 address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,
				   intfInfo->lan_intf.gmac.octet[0],intfInfo->lan_intf.gmac.octet[1],
				   intfInfo->lan_intf.gmac.octet[2],intfInfo->lan_intf.gmac.octet[3],
				   intfInfo->lan_intf.gmac.octet[4],intfInfo->lan_intf.gmac.octet[5]);

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
		for(i=0;i<RTK_RG_PORT_PON;i++)
		{
			if(intfInfo->lan_intf.port_mask.portmask&(0x1<<i))
			{
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,i+2,
				   intfInfo->lan_intf.gmac.octet[0],
				   intfInfo->lan_intf.gmac.octet[1],
				   intfInfo->lan_intf.gmac.octet[2],
				   intfInfo->lan_intf.gmac.octet[3],
				   intfInfo->lan_intf.gmac.octet[4],
				   intfInfo->lan_intf.gmac.octet[5]);
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d up",ipUtility,i+2);
			}
		}
		
		if(intfInfo->lan_intf.port_mask.portmask&(0x1<<RTK_RG_EXT_PORT0))	//sync eth0 mac with wlan mac when multiple LAN is on.
#endif
			rtk_rg_callback_pipe_cmd("%s link set eth0 address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,
					   intfInfo->lan_intf.gmac.octet[0],
					   intfInfo->lan_intf.gmac.octet[1],
					   intfInfo->lan_intf.gmac.octet[2],
					   intfInfo->lan_intf.gmac.octet[3],
					   intfInfo->lan_intf.gmac.octet[4],
					   intfInfo->lan_intf.gmac.octet[5]);
		rtk_rg_callback_pipe_cmd("%s link set eth0 up",ipUtility);


		rtk_rg_callback_pipe_cmd("%s link set br0 up",ipUtility);

		//add virtual ether device for VLAN
		if(taggingCPU)
			rtk_rg_callback_pipe_cmd("vconfig add br0 %d",intfInfo->lan_intf.intf_vlan_id);
		
		/***IPv4 Setting***/
		if(intfInfo->lan_intf.ip_version==IPVER_V4ONLY || intfInfo->lan_intf.ip_version==IPVER_V4V6)
		{	
			if(taggingCPU)
			{
				//20140702LUKE:before add address, forced delete once!(for change IP at same interface)
				if(intfInfo->lan_intf.replace_subnet)
				{
					//when replace, the "more subnet" added before will be flushed also
					rtk_rg_callback_pipe_cmd("%s -4 addr flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
					if(intfInfo->lan_intf.ip_version==IPVER_V4ONLY)
					{
						rtk_rg_callback_pipe_cmd("%s -6 addr flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
						rtk_rg_callback_pipe_cmd("%s -6 route flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
					}
				}
			
				//CPU tagged, set br0.vid
				//remove br0's ipv4 address, may be set by boa or other app.
				rtk_rg_callback_pipe_cmd("%s link set br0.%d mtu %d",ipUtility,intfInfo->lan_intf.intf_vlan_id,intfInfo->lan_intf.mtu);
				if(intfInfo->lan_intf.ip_network_mask != 0)
				{
					rtk_rg_callback_pipe_cmd("%s addr add %s/%s broadcast + dev br0.%d",ipUtility,ip,mask,intfInfo->lan_intf.intf_vlan_id);
				}
				//Setup dhcpd for the adding interface
				_rg_setup_dhcpd(intfInfo->lan_intf.ip_addr,intfInfo->lan_intf.ip_network_mask,intfInfo->lan_intf.intf_vlan_id);
			}
			else
			{
				//20140702LUKE:before add address, forced delete once!(for change IP at same interface)
				if(intfInfo->lan_intf.replace_subnet)
				{
					//when replace, the "more subnet" added before will be flushed also
					rtk_rg_callback_pipe_cmd("%s -4 addr flush dev br0",ipUtility);
					if(intfInfo->lan_intf.ip_version==IPVER_V4ONLY)
					{
						rtk_rg_callback_pipe_cmd("%s -6 addr flush dev br0",ipUtility);
						rtk_rg_callback_pipe_cmd("%s -6 route flush dev br0",ipUtility);
					}
				}
				
				//CPU untag, set br0 directly
				rtk_rg_callback_pipe_cmd("%s link set br0 mtu %d",ipUtility,intfInfo->lan_intf.mtu);
				if(intfInfo->lan_intf.ip_network_mask != 0)
				{
					rtk_rg_callback_pipe_cmd("%s addr add %s/%s broadcast + dev br0",ipUtility,ip,mask);
				}

				//Setup dhcpd for the adding interface
				_rg_setup_dhcpd(intfInfo->lan_intf.ip_addr,intfInfo->lan_intf.ip_network_mask,-1);
			}
		}

		/***IPv6 Setting***/
		if(intfInfo->lan_intf.ip_version==IPVER_V6ONLY || intfInfo->lan_intf.ip_version==IPVER_V4V6)
		{
			if(intfInfo->lan_intf.ipv6_network_mask_length != 0 && 
				!(*(unsigned int *)intfInfo->lan_intf.ipv6_addr.ipv6_addr == 0 &&
				*(unsigned int *)(intfInfo->lan_intf.ipv6_addr.ipv6_addr+4) == 0 &&
				*(unsigned int *)(intfInfo->lan_intf.ipv6_addr.ipv6_addr+8) == 0 &&
				*(unsigned int *)(intfInfo->lan_intf.ipv6_addr.ipv6_addr+12) == 0))
			{
				if(taggingCPU)
				{
					//20140702LUKE:before add address, forced delete once!(for change IP at same interface)
					if(intfInfo->lan_intf.replace_subnet)
					{
						//FIXME:since we don't support IPv6 more subnet right now, here we just flush all(one) IPv6 address.
						rtk_rg_callback_pipe_cmd("%s -6 addr flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
						rtk_rg_callback_pipe_cmd("%s -6 route flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
						if(intfInfo->lan_intf.ip_version==IPVER_V6ONLY)
						{
							rtk_rg_callback_pipe_cmd("%s -4 addr flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
							rtk_rg_callback_pipe_cmd("%s -4 route flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
						}
					}
				
					//CPU tagged, set br0.vid
					rtk_rg_callback_pipe_cmd("%s link set br0.%d mtu %d",ipUtility,intfInfo->lan_intf.intf_vlan_id,intfInfo->lan_intf.mtu);
					//remove br0's ipv6 address, may be set by boa or other app.
					rtk_rg_callback_pipe_cmd("%s addr add %s/%d dev br0.%d",ipUtility,_rtk_rg_inet_n6toa(intfInfo->lan_intf.ipv6_addr.ipv6_addr),intfInfo->lan_intf.ipv6_network_mask_length,intfInfo->lan_intf.intf_vlan_id);

					//Setup radvd for the adding interface
					_rg_setup_radvd(intfInfo->lan_intf.ipv6_addr.ipv6_addr,intfInfo->lan_intf.ipv6_network_mask_length,intfInfo->lan_intf.intf_vlan_id);
				}
				else
				{
					//20140702LUKE:before add address, forced delete once!(for change IP at same interface)
					if(intfInfo->lan_intf.replace_subnet)
					{
						//FIXME:since we don't support IPv6 more subnet right now, here we just flush all(one) IPv6 address.
						rtk_rg_callback_pipe_cmd("%s -6 addr flush dev br0",ipUtility);
						rtk_rg_callback_pipe_cmd("%s -6 route flush dev br0",ipUtility);
						if(intfInfo->lan_intf.ip_version==IPVER_V6ONLY)
						{
							rtk_rg_callback_pipe_cmd("%s -4 addr flush dev br0",ipUtility);
							rtk_rg_callback_pipe_cmd("%s -4 route flush dev br0",ipUtility);
						}
					}
					
					//CPU untag, set br0 directly
					rtk_rg_callback_pipe_cmd("%s link set br0 mtu %d",ipUtility,intfInfo->lan_intf.mtu);
					rtk_rg_callback_pipe_cmd("%s addr add %s/%d dev br0",ipUtility,_rtk_rg_inet_n6toa(intfInfo->lan_intf.ipv6_addr.ipv6_addr),intfInfo->lan_intf.ipv6_network_mask_length);

					//Setup radvd for the adding interface
					_rg_setup_radvd(intfInfo->lan_intf.ipv6_addr.ipv6_addr,intfInfo->lan_intf.ipv6_network_mask_length,-1);
				}	
			}
		}

		if(taggingCPU)
		{
			rtk_rg_callback_pipe_cmd("%s link set br0.%d address %02x:%02x:%02x:%02x:%02x:%02x",ipUtility,intfInfo->lan_intf.intf_vlan_id,
				   intfInfo->lan_intf.gmac.octet[0],intfInfo->lan_intf.gmac.octet[1],
				   intfInfo->lan_intf.gmac.octet[2],intfInfo->lan_intf.gmac.octet[3],
				   intfInfo->lan_intf.gmac.octet[4],intfInfo->lan_intf.gmac.octet[5]);
			rtk_rg_callback_pipe_cmd("%s link set br0.%d up",ipUtility,intfInfo->lan_intf.intf_vlan_id);
		}

		//20140702LUKE:when change LAN's ip setting, the vlan-binding should be remove and readd for new IP address!!
		//re-check all Vlan-binding rule for re-create vconfig 
		for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
		{
			if(rg_db.bind[i].valid && rg_db.bind[i].rtk_bind.vidLan!=0)
			{	
				if(rg_db.bind[i].rtk_bind.portMask.bits[0]>0)
				{
					if(rg_db.bind[i].rtk_bind.portMask.bits[0]&intfInfo->lan_intf.port_mask.portmask)
					{
						//delete it first, readd later
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
						if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT0))
						{
							rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d.%d",2+RTK_RG_PORT0,rg_db.bind[i].rtk_bind.vidLan);
							rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0.%d -p 802_1Q -j DROP",2+RTK_RG_PORT0);
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT0,rg_db.bind[i].rtk_bind.vidLan);
						}
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT1))
						{
							rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d.%d",2+RTK_RG_PORT1,rg_db.bind[i].rtk_bind.vidLan);
							rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0.%d -p 802_1Q -j DROP",2+RTK_RG_PORT1);
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT1,rg_db.bind[i].rtk_bind.vidLan);
						}
#if !defined(CONFIG_RTL9602C_SERIES)

						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT2))
						{
							rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d.%d",2+RTK_RG_PORT2,rg_db.bind[i].rtk_bind.vidLan);
							rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0.%d -p 802_1Q -j DROP",2+RTK_RG_PORT2);
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT2,rg_db.bind[i].rtk_bind.vidLan);
						}
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT3))
						{
							rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d.%d",2+RTK_RG_PORT3,rg_db.bind[i].rtk_bind.vidLan);
							rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0.%d -p 802_1Q -j DROP",2+RTK_RG_PORT3);
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT3,rg_db.bind[i].rtk_bind.vidLan);
						}
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT_RGMII))
						{
							rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d.%d",2+RTK_RG_PORT_RGMII,rg_db.bind[i].rtk_bind.vidLan);
							rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0.%d -p 802_1Q -j DROP",2+RTK_RG_PORT_RGMII);
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT_RGMII,rg_db.bind[i].rtk_bind.vidLan);
						}
#endif						
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT_PON))
						{
							rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d.%d",2+RTK_RG_PORT_PON,rg_db.bind[i].rtk_bind.vidLan);
							rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0.%d -p 802_1Q -j DROP",2+RTK_RG_PORT_PON);
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT_PON,rg_db.bind[i].rtk_bind.vidLan);
						}

						else
							CBACK("vlan-binding[%d] portMask wrong...%x",i,rg_db.bind[i].rtk_bind.portMask.bits[0]);
#else
						rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d",rg_db.bind[i].rtk_bind.vidLan);
						rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0 -p 802_1Q -j DROP");

						if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT0))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT0,rg_db.bind[i].rtk_bind.vidLan);
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT1))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT1,rg_db.bind[i].rtk_bind.vidLan);
#if !defined(CONFIG_RTL9602C_SERIES)					
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT2))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT2,rg_db.bind[i].rtk_bind.vidLan);
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT3))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT3,rg_db.bind[i].rtk_bind.vidLan);
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT_RGMII))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT_RGMII,rg_db.bind[i].rtk_bind.vidLan);
#endif						
						else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT_PON))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT_PON,rg_db.bind[i].rtk_bind.vidLan);						
						else
							CBACK("vlan-binding[%d] portMask wrong...%x",i,rg_db.bind[i].rtk_bind.portMask.bits[0]);
#endif						
					}
				}
				else
				{
					if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(intfInfo->lan_intf.port_mask.portmask>>RTK_RG_EXT_PORT0))
					{
						//delete it first, readd later
						rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d",rg_db.bind[i].rtk_bind.vidLan);
						rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0 -p 802_1Q -j DROP");
						
						if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT0-RTK_RG_EXT_PORT0)))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT0,rg_db.bind[i].rtk_bind.vidLan);
						else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT1-RTK_RG_EXT_PORT0)))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT1,rg_db.bind[i].rtk_bind.vidLan);
						else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT2-RTK_RG_EXT_PORT0)))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT2,rg_db.bind[i].rtk_bind.vidLan);
						else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT3-RTK_RG_EXT_PORT0)))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT3,rg_db.bind[i].rtk_bind.vidLan);
						else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT4-RTK_RG_EXT_PORT0)))
							_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT4,rg_db.bind[i].rtk_bind.vidLan);
						else
							CBACK("vlan-binding[%d] ext-portMask wrong...%x",i,rg_db.bind[i].rtk_bind.extPortMask.bits[0]);
					}
				}
			}
		}

#ifdef CONFIG_DUALBAND_CONCURRENT
		rtk_rg_callback_pipe_cmd("echo 2 > /proc/vwlan");	//echo 2 means up
#endif
		

#if defined(CONFIG_APOLLO)
		//do cmd
		for(i=0;i<RTK_RG_MAC_PORT_MAX;i++){
			if(intfInfo->lan_intf.port_mask.portmask & (1<<i)){
				rtk_rg_callback_pipe_cmd("echo %d eth0 > /proc/rtl8686gmac/dev_port_mapping",i);
			}
		}
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
		for(i=0;i<RTK_RG_PORT_PON;i++)
		{	
			if(intfInfo->lan_intf.port_mask.portmask & (1<<i))
				rtk_rg_callback_pipe_cmd("echo %d eth0.%d > /proc/rtl8686gmac/dev_port_mapping",i,i+2);
		}
#endif
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) //end defined(CONFIG_APOLLO) elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 

	FIXME("FIXME XDSL callback\n");

#endif //end defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 


        //patch for iptables
        rtk_rg_callback_pipe_cmd("iptables -A INPUT -s %s -d %s -j ACCEPT",ip,ip);
#if 0
		//restart udhcpd: this will make signal hang
		do_sys_cmd("/bin/udhcpd -B /var/udhcpd/udhcpd.conf");
#endif
    }

    return SUCCESS;
}

//rg_db.systemGlobal.initParam.interfaceDelByHwCallBack;
int _rtk_rg_interfaceDelByHwCallBack(rtk_rg_intfInfo_t* intfInfo, int* intfIdx)
{
	int ret;
	
	if(intfInfo->is_wan==0)
	{
		//clear dhcpd settings
		_rg_delete_dhcpd((intfInfo->lan_intf.untag_mask.portmask&(0x1<<RTK_RG_MAC_PORT_CPU))?-1:intfInfo->lan_intf.intf_vlan_id,0);

		//clear radvd settings
		_rg_delete_radvd((intfInfo->lan_intf.untag_mask.portmask&(0x1<<RTK_RG_MAC_PORT_CPU))?-1:intfInfo->lan_intf.intf_vlan_id,0);

		if(intfInfo->lan_intf.untag_mask.portmask&(0x1<<RTK_RG_MAC_PORT_CPU))
		{
			rtk_rg_callback_pipe_cmd("%s addr flush dev br0",ipUtility);
			rtk_rg_callback_pipe_cmd("%s route flush dev br0",ipUtility);
		}
		else
		{
			rtk_rg_callback_pipe_cmd("%s addr flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
			rtk_rg_callback_pipe_cmd("%s route flush dev br0.%d",ipUtility,intfInfo->lan_intf.intf_vlan_id);
			//removing virtual ethernet device for VLAN
			rtk_rg_callback_pipe_cmd("vconfig rem br0.%d",intfInfo->lan_intf.intf_vlan_id);
		}

		if(rg_db.systemGlobal.lanIntfTotalNum==0)
		{
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
			int i;
			for(i=0;i<RTK_RG_PORT_PON;i++)
				rtk_rg_callback_pipe_cmd("%s link set eth0.%d down",ipUtility,i+2);
#else
			rtk_rg_callback_pipe_cmd("%s link set eth0 down",ipUtility);
#endif
			rtk_rg_callback_pipe_cmd("%s link set br0 down",ipUtility);
		}
	}
	else
	{
		char dev[IFNAMSIZ]; //device name, ex:nas0_0
		//sprintf(dev,"nas0_%d",(*intfIdx-1));
		if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD)
			sprintf(dev,"wlan0-vxd");
		else if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD)
			sprintf(dev,"wlan1-vxd");
		else
			sprintf(dev,"%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,(*intfIdx));
		if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP)
		{
			_rg_delete_dhcpc(dev, 0);
		}
		else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
		{
			if(rg_db.systemGlobal.not_disconnect_ppp==0){
				rtk_rg_callback_pipe_cmd("spppctl del %d",*intfIdx);
			}else{
				//don't call spppctl to disconnet interface pppX, because this case is "Server disconnet problem".
				//so, do nothing!
				rg_db.systemGlobal.not_disconnect_ppp=0;
			}
		}
		else if((intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP)||
			(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP))
		{
			rtk_rg_callback_pipe_cmd("spppctl del %d",*intfIdx);
		}
		else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE)
		{
			if(intfInfo->wan_intf.wan_intf_conf.egress_vlan_tag_on)
			//removing virtual ethernet device for VLAN
			rtk_rg_callback_pipe_cmd("vconfig rem br0.%d",intfInfo->wan_intf.wan_intf_conf.egress_vlan_id);
		}

		//20150226LUKE: for ARP request, we should check IP for incoming interface	
#if 0		
		if(intfInfo->wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE)
		{
			rtk_rg_ipStaticInfo_t *static_info;
			unsigned char ip[16];
			if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DHCP)
				static_info=&intfInfo->wan_intf.dhcp_client_info.hw_info;
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
				static_info=&intfInfo->wan_intf.pppoe_info.after_dial.hw_info;
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP)
				static_info=&intfInfo->wan_intf.pptp_info.after_dial.hw_info;
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP)
				static_info=&intfInfo->wan_intf.l2tp_info.after_dial.hw_info;
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE)
				static_info=&intfInfo->wan_intf.dslite_info.static_info;
			else if(intfInfo->wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
				static_info=&intfInfo->wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info;
			else	
				static_info=&intfInfo->wan_intf.static_info;
			memcpy(ip,(void*)_inet_ntoa(static_info->ip_addr),16);
			rtk_rg_callback_pipe_cmd("%s link set br_%s down",ipUtility,dev);
			rtk_rg_callback_pipe_cmd("brctl delbr br_%s",dev);
			rtk_rg_callback_pipe_cmd("ebtables -D INPUT -i %s -p arp --arp-ip-dst ! %s -j DROP",dev,ip);
		}
#endif		
		//wan-intf
		rtk_rg_callback_pipe_cmd("%s link set %s down",ipUtility,dev);
#ifdef CONFIG_RG_WMUX_SUPPORT
		rtk_rg_callback_pipe_cmd("echo %s > /proc/rg/wmux_del",dev);
#endif

		//reset to eth0 if no other WAN use the same wan port
		for(ret=0;ret<rg_db.systemGlobal.wanIntfTotalNum;ret++)
		{
			if(rg_db.systemGlobal.wanIntfGroup[ret].p_wanIntfConf->wan_port_idx==intfInfo->wan_intf.wan_intf_conf.wan_port_idx)
				break;
		}
#if defined(CONFIG_APOLLO)
		if(ret==rg_db.systemGlobal.wanIntfTotalNum)
			rtk_rg_callback_pipe_cmd("echo %d eth0 > /proc/rtl8686gmac/dev_port_mapping",intfInfo->wan_intf.wan_intf_conf.wan_port_idx);
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) //end defined(CONFIG_APOLLO) elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 
		
		FIXME("FIXME XDSL callback\n");
		
#endif //end defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) 

#ifdef CONFIG_GPON_FEATURE
		//20150312LUKE: trigger OMCI to remove related CF rules
		if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WIRED)
			rtk_rg_callback_pipe_cmd("echo %d -1 -1 -1 0 > /proc/omci/wanInfo",*intfIdx);
#endif
	}

    return SUCCESS;
}


//rg_db.systemGlobal.initParam.dhcpRequestByHwCallBack;
int _rtk_rg_dhcpRequestByHwCallBack(int* intfIdx){

	char dev[IFNAMSIZ]; //device name, ex:nas0_0
	int dev_idx;
	rtk_rg_intfInfo_t *intfInfo;
	CBACK("%s is called!!!",__func__);

	//memcpy(&intfInfo,&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo,sizeof(rtk_rg_intfInfo_t));
	intfInfo=&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo;

	////check wan_intf info & set dev name
	if((*intfIdx>7) || (*intfIdx<0)){
		CBACK("invalid wan_intf_idx\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else{
		//dev_idx = (*intfIdx-1);
		dev_idx = (*intfIdx);
		if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD)
			sprintf(dev,"wlan0-vxd");
		else if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD)
			sprintf(dev,"wlan1-vxd");
		else
			sprintf(dev,"%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev_idx);
	}
	
	if(rg_db.systemGlobal.interfaceInfo[*intfIdx].valid==0){
		CBACK("Assigned wan_intf_idx is not valid.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(!intfInfo->is_wan){
		CBACK("Assigned wan_intf_idx is not wan.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(intfInfo->wan_intf.wan_intf_conf.wan_type!=RTK_RG_DHCP){
		CBACK("Assigned wan_intf_idx is not DHCP.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}

	//configure wan interface in protocol stack
	_rg_configure_protocol_stack(*intfIdx,dev);
	
	rtk_rg_callback_pipe_cmd("iptables -A INPUT -i %s -p UDP --dport 69 -d 255.255.255.255 -m state --state NEW -j ACCEPT",dev);

	//Delete the file to avoid accessing the last one.
	rtk_rg_callback_pipe_cmd("rm -f /var/udhcpc/udhcpc.info");
	_rg_delete_dhcpc(dev,0);
	
	rtk_rg_callback_pipe_cmd("udhcpc -i %s -W -s /etc/scripts/udhcpc_rgcb.sh &",dev);
	rtk_rg_callback_pipe_cmd("iptables -t nat -A POSTROUTING -o %s -j MASQUERADE",dev);

	rtlglue_printf("Please cat /var/udhcpc/udhcpc.info to read DHCP request information.\n");

	return SUCCESS;
}


//rg_db.systemGlobal.initParam.pppoeBeforeDiagByHwCallBack;
int _rtk_rg_pppoeBeforeDiagByHwCallBack(rtk_rg_pppoeClientInfoBeforeDial_t* before_diag, int* intfIdx)
{
	char dev[IFNAMSIZ]; //device name, ex:nas0_0
	int dev_idx;
	rtk_rg_intfInfo_t *intfInfo;
	//memcpy(&intfInfo,&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo,sizeof(rtk_rg_intfInfo_t));
	intfInfo=&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo;

	////check wan_intf info & set dev name
	if((*intfIdx>7) || (*intfIdx<0)){
		CBACK("invalid wan_intf_idx\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else{
		//dev_idx = (*intfIdx-1);
		dev_idx = (*intfIdx);	
		if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN0_VXD)
			sprintf(dev,"wlan0-vxd");
		else if(intfInfo->wan_intf.wirelessWan==RG_WWAN_WLAN1_VXD)
			sprintf(dev,"wlan1-vxd");
		else
			sprintf(dev,"%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev_idx);
	}
	
	if(rg_db.systemGlobal.interfaceInfo[*intfIdx].valid==0){
		CBACK("Assigned wan_intf_idx is not valid.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(!intfInfo->is_wan){
		CBACK("Assigned wan_intf_idx is not wan.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(intfInfo->wan_intf.wan_intf_conf.wan_type!=RTK_RG_PPPoE){
		CBACK("Assigned wan_intf_idx is not PPPoE.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}

	//configure wan interface in protocol stack
	_rg_configure_protocol_stack(*intfIdx,dev);
	
	rtk_rg_callback_pipe_cmd("spppctl add %d pppoe %s username %s password %s gw 1 mru 1492 nohwnat ippt 0 debug 0 ipt 2",dev_idx,dev,before_diag->username,before_diag->password);
	rtk_rg_callback_pipe_cmd("spppctl katimer 100");

	rtk_rg_callback_pipe_cmd("%s link set ppp%d txqueuelen 0",ipUtility,dev_idx);

	/*set proc for Radvd create global v6 ip*/
	rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/ppp%d/autoconf",dev_idx);
	rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
	rtk_rg_callback_pipe_cmd("echo 0 > /proc/sys/net/ipv6/conf/ppp%d/forwarding",dev_idx);
	//enable,disable ipv6 ppp diag
	//rtk_rg_callback_pipe_cmd("/bin/echo 1 > proc/sys/net/ipv6/conf/ppp%d/disable_ipv6",dev_idx);

	//1 FIX ME: PPPoE disconnet issue
	/*Need to judge if PPPoE diag failed!!!*/
	//rg_kernel.ppp_diaged[dev_idx]=ENABLED;

	rtk_rg_callback_pipe_cmd("iptables -t nat -A POSTROUTING -o ppp%d -j MASQUERADE",dev_idx);
	
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.pptpBeforeDialByHwCallBack;
int _rtk_rg_pptpBeforeDialByHwCallBack(rtk_rg_pptpClientInfoBeforeDial_t* before_dial, int* intfIdx)
{
	char dev[IFNAMSIZ]; //device name, ex:nas0_0
	int dev_idx;
	rtk_rg_intfInfo_t *intfInfo;
	intfInfo=&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo;

	////check wan_intf info & set dev name
	if((*intfIdx>7) || (*intfIdx<0)){
		CBACK("invalid wan_intf_idx\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else{
		dev_idx = (*intfIdx);	
		sprintf(dev,"%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev_idx);
	}
	
	if(rg_db.systemGlobal.interfaceInfo[*intfIdx].valid==0){
		CBACK("Assigned wan_intf_idx is not valid.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(!intfInfo->is_wan){
		CBACK("Assigned wan_intf_idx is not wan.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(intfInfo->wan_intf.wan_intf_conf.wan_type!=RTK_RG_PPTP){
		CBACK("Assigned wan_intf_idx is not PPTP.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}

	rtk_rg_callback_pipe_cmd("spppctl add %d pptp auth auto username %s password %s server %s gw 0 enctype none",dev_idx,before_dial->username,before_dial->password,_inet_ntoa(before_dial->pptp_ipv4_addr));

	return SUCCESS;
}


//rg_db.systemGlobal.initParam.l2tpBeforeDialByHwCallBack;
int _rtk_rg_l2tpBeforeDialByHwCallBack(rtk_rg_l2tpClientInfoBeforeDial_t* before_dial, int* intfIdx)
{
	char dev[IFNAMSIZ]; //device name, ex:nas0_0
	int dev_idx;
	rtk_rg_intfInfo_t *intfInfo;
	intfInfo=&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo;

	////check wan_intf info & set dev name
	if((*intfIdx>7) || (*intfIdx<0)){
		CBACK("invalid wan_intf_idx\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else{
		dev_idx = (*intfIdx);	
		sprintf(dev,"%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev_idx);
	}
	
	if(rg_db.systemGlobal.interfaceInfo[*intfIdx].valid==0){
		CBACK("Assigned wan_intf_idx is not valid.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(!intfInfo->is_wan){
		CBACK("Assigned wan_intf_idx is not wan.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(intfInfo->wan_intf.wan_intf_conf.wan_type!=RTK_RG_L2TP){
		CBACK("Assigned wan_intf_idx is not L2TP.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}

	//FIXME: should we configure the MRU value here?
	rtk_rg_callback_pipe_cmd("spppctl add %d l2tp username %s password %s gw 0 mru 1458 auth auto enctype none server %s tunnel_auth none",dev_idx,before_dial->username,before_dial->password,_inet_ntoa(before_dial->l2tp_ipv4_addr));

	return SUCCESS;
}

//rg_db.systemGlobal.initParam.pppoeDsliteBeforeDialByHwCallBack
int _rtk_rg_pppoeDsliteBeforeDialByHwCallBack(rtk_rg_pppoeClientInfoBeforeDial_t *before_diag, int *intfIdx)
{
	char dev[IFNAMSIZ]; //device name, ex:nas0_0
	int dev_idx;
	rtk_rg_intfInfo_t *intfInfo;
	//memcpy(&intfInfo,&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo,sizeof(rtk_rg_intfInfo_t));
	intfInfo=&rg_db.systemGlobal.interfaceInfo[*intfIdx].storedInfo;

	////check wan_intf info & set dev name
	if((*intfIdx>7) || (*intfIdx<0)){
		CBACK("invalid wan_intf_idx\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else{
		//dev_idx = (*intfIdx-1);
		dev_idx = (*intfIdx);	
		sprintf(dev,"%s_%d",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,dev_idx);
	}
	
	if(rg_db.systemGlobal.interfaceInfo[*intfIdx].valid==0){
		CBACK("Assigned wan_intf_idx is not valid.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(!intfInfo->is_wan){
		CBACK("Assigned wan_intf_idx is not wan.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}else if(intfInfo->wan_intf.wan_intf_conf.wan_type!=RTK_RG_PPPoE_DSLITE){
		CBACK("Assigned wan_intf_idx is not PPPoE_Dslite.\n");
		return RT_ERR_RG_INVALID_PARAM;
	}

	//configure wan interface in protocol stack
	_rg_configure_protocol_stack(*intfIdx,dev);
	
	rtk_rg_callback_pipe_cmd("spppctl add %d pppoe %s username %s password %s gw 1 mru 1492 nohwnat ippt 0 debug 0 ipt 2",dev_idx,dev,before_diag->username,before_diag->password);
	rtk_rg_callback_pipe_cmd("spppctl katimer 100");

	rtk_rg_callback_pipe_cmd("%s link set ppp%d txqueuelen 0",ipUtility,dev_idx);

	/*set proc for Radvd create global v6 ip*/
	rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/ppp%d/autoconf",dev_idx);
	rtk_rg_callback_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
	rtk_rg_callback_pipe_cmd("echo 0 > /proc/sys/net/ipv6/conf/ppp%d/forwarding",dev_idx);
	//enable,disable ipv6 ppp diag
	//rtk_rg_callback_pipe_cmd("/bin/echo 1 > proc/sys/net/ipv6/conf/ppp%d/disable_ipv6",dev_idx);

	//1 FIX ME: PPPoE disconnet issue
	/*Need to judge if PPPoE diag failed!!!*/
	//rg_kernel.ppp_diaged[dev_idx]=ENABLED;

	rtk_rg_callback_pipe_cmd("iptables -t nat -A POSTROUTING -o ppp%d -j MASQUERADE",dev_idx);
	
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.arpAddByHwCallBack;
int _rtk_rg_arpAddByHwCallBack(rtk_rg_arpInfo_t* arpInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.arpDelByHwCallBack;
int _rtk_rg_arpDelByHwCallBack(rtk_rg_arpInfo_t* arpInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.macAddByHwCallBack=NULL;
int _rtk_rg_macAddByHwCallBack(rtk_rg_macEntry_t* macInfo){
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.macDelByHwCallBack=NULL;
int _rtk_rg_macDelByHwCallBack(rtk_rg_macEntry_t* macInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.naptAddByHwCallBack=NULL;
int _rtk_rg_naptAddByHwCallBack(rtk_rg_naptInfo_t* naptInfo){
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.naptDelByHwCallBack=NULL;
int _rtk_rg_naptDelByHwCallBack(rtk_rg_naptInfo_t* naptInfo){
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.routingAddByHwCallBack=NULL;
int _rtk_rg_routingAddByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo){
	unsigned char ip[16],mask[16];
	int taggingCPU,vlanId;

	if(rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].valid &&
		rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].storedInfo.is_wan==0 &&
		rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].p_lanIntfConf->ip_addr!=routingInfo->dest_ip)
	{
		//add one more subnet with same interface!! sync to protocol stack
		taggingCPU=(rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].p_lanIntfConf->untag_mask.portmask&(0x1<<RTK_RG_MAC_PORT_CPU))?0:1;
	
    	//add lan interface
        memcpy(ip,(void*)_inet_ntoa(routingInfo->dest_ip),16);
        memcpy(mask,(void*)_inet_ntoa(routingInfo->ip_mask),16);
		vlanId=rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].p_lanIntfConf->intf_vlan_id;
		
		if(taggingCPU)
		{		
			//CPU tagged, set br0.vid
			if(routingInfo->ip_mask != 0)
			{
				rtk_rg_callback_pipe_cmd("%s addr add %s/%s broadcast + dev br0.%d",ipUtility,ip,mask,vlanId);
			}
		}
		else
		{		
			//CPU untag, set br0 directly
			if(routingInfo->ip_mask != 0)
			{
				rtk_rg_callback_pipe_cmd("%s addr add %s/%s broadcast + dev br0",ipUtility,ip,mask);
			}
		}
	}
	
	return SUCCESS;
}


//rg_db.systemGlobal.initParam.routingDelByHwCallBack=NULL;
int _rtk_rg_routingDelByHwCallBack(rtk_rg_ipv4RoutingEntry_t* routingInfo){
	unsigned char ip[16];

	//20140616LUKE: when we deleting IPv4 routing entry from NAPT WAN, deleting iptables's SNAT rule
	if(rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].valid &&
		rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].storedInfo.is_wan && 
		(rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_STATIC) &&
		rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].p_wanStaticInfo->ip_version!=IPVER_V6ONLY &&
		rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].p_wanStaticInfo->napt_enable)
	{	
		memcpy(ip,(void*)_inet_ntoa(rg_db.systemGlobal.interfaceInfo[routingInfo->wan_intf_idx].p_wanStaticInfo->ip_addr),16);
		rtk_rg_callback_pipe_cmd("iptables -t nat -D POSTROUTING -o %s_%d -j SNAT --to-source %s",CONFIG_RG_CALLBACK_MULTI_WAN_DEVICE_NAME,routingInfo->wan_intf_idx,ip);
	}
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.bindingAddByHwCallBack=NULL;
int _rtk_rg_bindingAddByHwCallBack(rtk_rg_bindingEntry_t* bindingInfo){	
	if(bindingInfo->type==BIND_TYPE_VLAN)
	{	
		//WARNING("vlan binding: vid is %d, port is %d",bindingInfo->vlan.vlan_bind_vlan_id,bindingInfo->vlan.vlan_bind_port_idx);
		_rtk_rg_checkVlanBindingAndAddVconfig(bindingInfo->vlan.vlan_bind_port_idx,bindingInfo->vlan.vlan_bind_vlan_id);
	}
	
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.bindingDelByHwCallBack=NULL;
int _rtk_rg_bindingDelByHwCallBack(rtk_rg_bindingEntry_t* bindingInfo){
	int i;
	
	if(bindingInfo->type==BIND_TYPE_VLAN)
	{
		//WARNING("vlan binding: vid is %d, port is %d",bindingInfo->vlan.vlan_bind_vlan_id,bindingInfo->vlan.vlan_bind_port_idx);
		
		//del virtual ether device for VLAN
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
		if(bindingInfo->vlan.vlan_bind_port_idx>=RTK_RG_PORT_CPU)
		{
			rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d",bindingInfo->vlan.vlan_bind_vlan_id);
			rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0 -p 802_1Q -j DROP");
		}
		else
		{
			rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d.%d",2+bindingInfo->vlan.vlan_bind_port_idx,bindingInfo->vlan.vlan_bind_vlan_id);
			rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0.%d -p 802_1Q -j DROP",2+bindingInfo->vlan.vlan_bind_port_idx);
		}
#else
		rtk_rg_callback_pipe_cmd("vconfig rem eth0.%d",bindingInfo->vlan.vlan_bind_vlan_id);
		rtk_rg_callback_pipe_cmd("ebtables -t broute -D BROUTING -i eth0 -p 802_1Q -j DROP");
#endif

		//re-check all Vlan-binding rule for re-create vconfig 
		for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
		{
			if(rg_db.bind[i].valid && rg_db.bind[i].rtk_bind.vidLan!=0)
			{	
				if(rg_db.bind[i].rtk_bind.portMask.bits[0]>0)
				{
					if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT0))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT0,rg_db.bind[i].rtk_bind.vidLan);
					else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT1))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT1,rg_db.bind[i].rtk_bind.vidLan);
#if !defined(CONFIG_RTL9602C_SERIES)					
					else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT2))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT2,rg_db.bind[i].rtk_bind.vidLan);
					else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT3))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT3,rg_db.bind[i].rtk_bind.vidLan);
					else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT_RGMII))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT_RGMII,rg_db.bind[i].rtk_bind.vidLan);
#endif					
					else if(rg_db.bind[i].rtk_bind.portMask.bits[0]&(0x1<<RTK_RG_PORT_PON))						
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_PORT_PON,rg_db.bind[i].rtk_bind.vidLan);
					else
						CBACK("vlan-binding[%d] portMask wrong...%x",i,rg_db.bind[i].rtk_bind.portMask.bits[0]);
				}
				else
				{
					if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT0-RTK_RG_EXT_PORT0)))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT0,rg_db.bind[i].rtk_bind.vidLan);
					else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT1-RTK_RG_EXT_PORT0)))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT1,rg_db.bind[i].rtk_bind.vidLan);
					else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT2-RTK_RG_EXT_PORT0)))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT2,rg_db.bind[i].rtk_bind.vidLan);
					else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT3-RTK_RG_EXT_PORT0)))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT3,rg_db.bind[i].rtk_bind.vidLan);
					else if(rg_db.bind[i].rtk_bind.extPortMask.bits[0]&(0x1<<(RTK_RG_EXT_PORT4-RTK_RG_EXT_PORT0)))
						_rtk_rg_checkVlanBindingAndAddVconfig(RTK_RG_EXT_PORT4,rg_db.bind[i].rtk_bind.vidLan);
					else
						CBACK("vlan-binding[%d] ext-portMask wrong...%x",i,rg_db.bind[i].rtk_bind.extPortMask.bits[0]);
				}
			}
		}
	}
	return SUCCESS;
}


int _rtk_rg_neighborAddByHwCallBack(rtk_rg_neighborInfo_t* neighborInfo){
	return SUCCESS;
}

int _rtk_rg_neighborDelByHwCallBack(rtk_rg_neighborInfo_t* neighborInfo){
	return SUCCESS;
}

int _rtk_rg_v6RoutingAddByHwCallBack(rtk_rg_ipv6RoutingEntry_t* cb_routv6Et){
	return SUCCESS;
}

int _rtk_rg_v6RoutingDelByHwCallBack(rtk_rg_ipv6RoutingEntry_t* cb_routv6Et){
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.softwareNaptInfoAddCallBack=NULL;
int _rtk_rg_softwareNaptInfoAddCallBack(rtk_rg_naptInfo_t* naptInfo){
	CBACK("enter %s!!! ",__FUNCTION__);
	CBACK("naptTuples.is_tcp = %s",naptInfo->naptTuples.is_tcp?"TCP":"UDP");
	CBACK("naptTuples.local_ip = %s",_inet_ntoa(naptInfo->naptTuples.local_ip));
	CBACK("naptTuples.local_port = %d",naptInfo->naptTuples.local_port);
	CBACK("naptTuples.remote_ip = %s",_inet_ntoa(naptInfo->naptTuples.remote_ip));
	CBACK("naptTuples.remote_port = %d",naptInfo->naptTuples.remote_port);
	CBACK("naptTuples.wan_intf_idx = %d",naptInfo->naptTuples.wan_intf_idx);
	CBACK("naptTuples.inbound_pri_valid = %d",naptInfo->naptTuples.inbound_pri_valid);
	CBACK("naptTuples.inbound_priority = %d",naptInfo->naptTuples.inbound_priority);
	CBACK("naptTuples.external_port = %d",naptInfo->naptTuples.external_port);
	CBACK("naptTuples.outbound_pri_valid = %d",naptInfo->naptTuples.outbound_pri_valid);
	CBACK("naptTuples.outbound_priority = %d",naptInfo->naptTuples.outbound_priority);
	CBACK("idleSecs = %d",naptInfo->idleSecs);
	switch(naptInfo->state){
		case INVALID:
			CBACK("state = INVALID");
			break;
		case SYN_RECV:
			CBACK("state = SYN_RECV");
			break;
		case UDP_FIRST:
			CBACK("state = UDP_FIRST");
			break;
		case SYN_ACK_RECV:
			CBACK("state = SYN_ACK_RECV");
			break;
		case UDP_SECOND:
			CBACK("state = UDP_SECOND");
			break;
		case TCP_CONNECTED:
			CBACK("state = TCP_CONNECTED");
			break;
		case UDP_CONNECTED:
			CBACK("state = UDP_CONNECTED");
			break;
		case FIRST_FIN:
			CBACK("state = FIRST_FIN");
			break;
		case RST_RECV:
			CBACK("state = RST_RECV");
			break;
		case FIN_SEND_AND_RECV:
			CBACK("state = FIN_SEND_AND_RECV");
			break;
		case LAST_ACK:
			CBACK("state = LAST_ACK");
			break;
		default:
			break;
	}
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.softwareNaptInfoDeleteCallBack=NULL;
int _rtk_rg_softwareNaptInfoDeleteCallBack(rtk_rg_naptInfo_t* naptInfo){
	CBACK("enter %s!!! ",__FUNCTION__);
	CBACK("naptTuples.is_tcp = %s",naptInfo->naptTuples.is_tcp?"TCP":"UDP");
	CBACK("naptTuples.local_ip = %s",_inet_ntoa(naptInfo->naptTuples.local_ip));
	CBACK("naptTuples.local_port = %d",naptInfo->naptTuples.local_port);
	CBACK("naptTuples.remote_ip = %s",_inet_ntoa(naptInfo->naptTuples.remote_ip));
	CBACK("naptTuples.remote_port = %d",naptInfo->naptTuples.remote_port);
	CBACK("naptTuples.wan_intf_idx = %d",naptInfo->naptTuples.wan_intf_idx);
	CBACK("naptTuples.inbound_pri_valid = %d",naptInfo->naptTuples.inbound_pri_valid);
	CBACK("naptTuples.inbound_priority = %d",naptInfo->naptTuples.inbound_priority);
	CBACK("naptTuples.external_port = %d",naptInfo->naptTuples.external_port);
	CBACK("naptTuples.outbound_pri_valid = %d",naptInfo->naptTuples.outbound_pri_valid);
	CBACK("naptTuples.outbound_priority = %d",naptInfo->naptTuples.outbound_priority);
	CBACK("idleSecs = %d",naptInfo->idleSecs);
	switch(naptInfo->state){
		case INVALID:
			CBACK("state = INVALID");
			break;
		case SYN_RECV:
			CBACK("state = SYN_RECV");
			break;
		case UDP_FIRST:
			CBACK("state = UDP_FIRST");
			break;
		case SYN_ACK_RECV:
			CBACK("state = SYN_ACK_RECV");
			break;
		case UDP_SECOND:
			CBACK("state = UDP_SECOND");
			break;
		case TCP_CONNECTED:
			CBACK("state = TCP_CONNECTED");
			break;
		case UDP_CONNECTED:
			CBACK("state = UDP_CONNECTED");
			break;
		case FIRST_FIN:
			CBACK("state = FIRST_FIN");
			break;
		case RST_RECV:
			CBACK("state = RST_RECV");
			break;
		case FIN_SEND_AND_RECV:
			CBACK("state = FIN_SEND_AND_RECV");
			break;
		case LAST_ACK:
			CBACK("state = LAST_ACK");
			break;
		default:
			break;
	}
	return SUCCESS;
}

//rg_db.systemGlobal.initParam.naptPreRouteDPICallBack=NULL;
int _rtk_rg_naptPreRouteDPICallBack(void *data, rtk_rg_naptDirection_t direct){
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;
	int8 sip_buf[4*sizeof "123"], dip_buf[4*sizeof "123"];

	//only for display string
	memcpy(sip_buf,_inet_ntoa(pPktHdr->ipv4Sip),sizeof(sip_buf));
	memcpy(dip_buf,_inet_ntoa(pPktHdr->ipv4Dip),sizeof(dip_buf));

	CBACK("Callback %s enter!! ppkthdr=%p",__FUNCTION__,pPktHdr);

	CBACK("direction = %s",direct==NAPT_DIRECTION_OUTBOUND?"NAPT":"NAPTR");
	CBACK("protocol = 0x%x",pPktHdr->ipProtocol);
	CBACK("ipv4Sip: %s -> %s",sip_buf,_inet_ntoa(*pPktHdr->pIpv4Sip));
	CBACK("ipv4Dip: %s -> %s",dip_buf,_inet_ntoa(*pPktHdr->pIpv4Dip));
	if((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF)){
		CBACK("sport: %d -> %d",pPktHdr->sport,*pPktHdr->pSport);
		CBACK("dport: %d -> %d",pPktHdr->dport,*pPktHdr->pDport);
	}

	//default return value: RG_FWDENGINE_PREROUTECB_CONTINUE
	return rg_db.systemGlobal.demo_dpiPreRouteCallback_retValue;
}

//rg_db.systemGlobal.initParam.naptForwardDPICallBack=NULL;
int _rtk_rg_naptForwardDPICallBack(void *data, rtk_rg_naptDirection_t direct){
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;
	int8 sip_buf[4*sizeof "123"], dip_buf[4*sizeof "123"];

	//only for display string
	memcpy(sip_buf,_inet_ntoa(pPktHdr->ipv4Sip),sizeof(sip_buf));
	memcpy(dip_buf,_inet_ntoa(pPktHdr->ipv4Dip),sizeof(dip_buf));

	CBACK("Callback %s enter!! ppkthdr=%p",__FUNCTION__,pPktHdr);
	
	CBACK("direction = %s",direct==NAPT_DIRECTION_OUTBOUND?"NAPT":"NAPTR");
	CBACK("protocol = 0x%x",pPktHdr->ipProtocol);
	CBACK("ipv4Sip: %s -> %s",sip_buf,_inet_ntoa(*pPktHdr->pIpv4Sip));
	CBACK("ipv4Dip: %s -> %s",dip_buf,_inet_ntoa(*pPktHdr->pIpv4Dip));
	if((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF)){
		CBACK("sport: %d -> %d",pPktHdr->sport,*pPktHdr->pSport);
		CBACK("dport: %d -> %d",pPktHdr->dport,*pPktHdr->pDport);
	}

	//default return value:RG_FWDENGINE_FORWARDCB_FINISH_DPI
	return rg_db.systemGlobal.demo_dpiFwdCallback_retValue;
}
#endif

#endif

