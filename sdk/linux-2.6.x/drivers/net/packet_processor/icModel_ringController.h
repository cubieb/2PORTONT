
#ifndef __ICMODEL_RING_CONTROLLER__
#define __ICMODEL_RING_CONTROLLER__



//#define MAC_LLIP_SIM 1  /* enable Low-Level IP simulation. */
//#define SAR_LLIP_SIM 1  /* enable Low-Level IP simulation. */
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#else
#define EXT_LLIP_SIM 1  /* enable Low-Level IP simulation. */
#endif

#define SAR_INTFS	8
#define PTM_INTFS	8
#define MAC_RX_INTFS	2
#define MAC_TX_INTFS	5
#define EXT_INTFS	3

//#define BUFFER_SIZE 2048
struct mac_pTx
{
#ifdef __LITTLE_ENDIAN
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 data_length:17;
	uint32 rsv:6;
	uint32 crc:1;
	uint32 rsv2:1;
	uint32 tcpcs:1;
	uint32 udpcs:1;
	uint32 ipcs:1;
#else
	uint32 data_length:12;
	uint32 rsv:11;
	uint32 crc:1;
	uint32 rsv2:4;
#endif
	uint32 ls:1;
	uint32 fs:1;
	uint32 eor:1;
	uint32 own:1;
#else //big endian
	uint32 own:1;
	uint32 eor:1;
	uint32 fs:1;	
	uint32 ls:1;	
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ipcs:1;
	uint32 udpcs:1;
	uint32 tcpcs:1;
	uint32 rsv2:1;
	uint32 crc:1;
	uint32 rsv:6;
	uint32 data_length:17;
#else
	uint32 rsv2:4;	
	uint32 crc:1;	
	uint32 rsv:11;	
	uint32 data_length:12;	
#endif
#endif

	uint32 tx_buffer_addr;


#ifdef __LITTLE_ENDIAN
	uint32 vlan_vidh:4;
	uint32 vlan_cfi:1;
	uint32 vlan_prio:3;
	uint32 vlan_vidl:8;
	uint32 tagc:1;
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ctagmsk:10;
	uint32 cpri:3;
	uint32 aspri:1;
	uint32 ctag:1;
#else
	uint32 rsv3:15;
#endif
#else //big endian
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ctag:1;
	uint32 aspri:1;
	uint32 cpri:3;
	uint32 ctagmsk:10;
#else
	uint32 rsv3:15;
#endif
	uint32 tagc:1;
	uint32 vlan_vidl:8;
	uint32 vlan_prio:3;
	uint32 vlan_cfi:1;	
	uint32 vlan_vidh:4;	
#endif

#ifdef __LITTLE_ENDIAN
	uint32 fcpu:1;
	uint32 oeor:1;
	uint32 orgAddr:30;	
#else
	uint32 orgAddr:30;
	uint32 oeor:1;
	uint32 fcpu:1;
#endif

#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
#ifdef __LITTLE_ENDIAN
	uint32 rsv6:20;
	uint32 lsmtu:11;
	uint32 lgsen:1;
#else
	uint32 lgsen:1;
	uint32 lsmtu:11;
	uint32 rsv6:20;
#endif
#endif

#ifdef __LITTLE_ENDIAN
	uint32 sram_size:8;
	uint32 rsv4:8;
	uint32 sram_map_id:9;
	uint32 rsv5:7;
#else
	uint32 rsv5:7;
	uint32 sram_map_id:9;
	uint32 rsv4:8;	
	uint32 sram_size:8;
#endif
};

struct mac_vTx
{
	//-------------------------- 1st DW -----------------------//
#ifdef __LITTLE_ENDIAN
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 data_length:17;
	uint32 rsv:6;
	uint32 crc:1;
	uint32 rsv2:1;
	uint32 tcpcs:1;
	uint32 udpcs:1;
	uint32 ipcs:1;
#else
	uint32 data_length:12;
	uint32 rsv:11;
	uint32 crc:1;
	uint32 rsv2:4;
#endif
	uint32 ls:1;
	uint32 fs:1;
	uint32 eor:1;
	uint32 own:1;
#else //big endian
	uint32 own:1;
	uint32 eor:1;
	uint32 fs:1;
	uint32 ls:1;
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ipcs:1;
	uint32 udpcs:1;
	uint32 tcpcs:1;
	uint32 rsv2:1;
	uint32 crc:1;
	uint32 rsv:6;
	uint32 data_length:17;
#else
	uint32 rsv2:4;	
	uint32 crc:1;	
	uint32 rsv:11;	
	uint32 data_length:12;	
#endif
#endif

	//-------------------------- 2nd DW -----------------------//
	uint32 tx_buffer_addr;

	//-------------------------- 3rd DW -----------------------//
#ifdef __LITTLE_ENDIAN
	uint32 vlan_vidh:4;
	uint32 vlan_cfi:1;
	uint32 vlan_prio:3;
	uint32 vlan_vidl:8;
	uint32 tagc:1;
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ctagmsk:10;
	uint32 cpri:3;
	uint32 aspri:1;
	uint32 ctag:1;
#else
	uint32 rsv3:15;
#endif
#else //big endian
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ctag:1;
	uint32 aspri:1;
	uint32 cpri:3;
	uint32 ctagmsk:10;
#else
	uint32 rsv3:15;
#endif
	uint32 tagc:1;
	uint32 vlan_vidl:8;	
	uint32 vlan_prio:3;	
	uint32 vlan_cfi:1;	
	uint32 vlan_vidh:4;	
#endif

	//-------------------------- 4th DW -----------------------//
#ifdef __LITTLE_ENDIAN
	uint32 fcpu:1;
	uint32 rsv4:1;
	uint32 orgAddr:30;	
#else
	uint32 orgAddr:30;
	uint32 rsv4:1;
	uint32 fcpu:1;
#endif

	//-------------------------- 5th DW -----------------------//
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
#ifdef __LITTLE_ENDIAN
	uint32 rsv6:20;
	uint32 lsmtu:11;
	uint32 lgsen:1;
#else
	uint32 lgsen:1;
	uint32 lsmtu:11;
	uint32 rsv6:20;
#endif
#endif

};

struct sar_pTx
{
#ifdef __LITTLE_ENDIAN
	uint32 data_length:12;
	uint32 rsv:4;
	uint32 clp:1;
	uint32 rsv2:1;
	uint32 pti:2;
	uint32 ethnt_offset:6;
	uint32 trlren:1;
	uint32 atmport:1;
	uint32 ls:1;
	uint32 fs:1;
	uint32 eor:1;
	uint32 own:1;
#else
	uint32 own:1;
	uint32 eor:1;
	uint32 fs:1;
	uint32 ls:1;	
	uint32 atmport:1;	
	uint32 trlren:1;	
	uint32 ethnt_offset:6;	
	uint32 pti:2;	
	uint32 rsv2:1;	
	uint32 clp:1;	
	uint32 rsv:4;	
	uint32 data_length:12;
#endif
	
	uint32 tx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 rsv3:16;
	uint32 trlr:16;
#else
	uint32 trlr:16;
	uint32 rsv3:16;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 fcpu:1;
	uint32 oeor:1;
	uint32 orgAddr:30;	
#else
	uint32 orgAddr:30;
	uint32 oeor:1;
	uint32 fcpu:1;
#endif

#ifdef __LITTLE_ENDIAN	
	uint32 sram_size:8;
	uint32 rsv4:8;
	uint32 sram_map_id:9;
	uint32 rsv5:7;	
#else
	uint32 rsv5:7;	
	uint32 sram_map_id:9;
	uint32 rsv4:8;	
	uint32 sram_size:8;	
#endif

};

struct sar_vTx
{

#ifdef __LITTLE_ENDIAN
	uint32 data_length:12;
	uint32 rsv:4;
	uint32 clp:1;
	uint32 rsv2:1;
	uint32 pti:2;
	uint32 ethnt_offset:6;
	uint32 trlren:1;
	uint32 atmport:1;
	uint32 ls:1;
	uint32 fs:1;
	uint32 eor:1;
	uint32 own:1;
#else
	uint32 own:1;
	uint32 eor:1;
	uint32 fs:1;	
	uint32 ls:1;
	uint32 atmport:1;	
	uint32 trlren:1;	
	uint32 ethnt_offset:6;
	uint32 pti:2;
	uint32 rsv2:1;
	uint32 clp:1;	
	uint32 rsv:4;
	uint32 data_length:12;	
#endif

	
	uint32 tx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 rsv3:16;
	uint32 trlr:16;
#else
	uint32 trlr:16;
	uint32 rsv3:16;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 fcpu:1;
	uint32 rsv4:1;
	uint32 orgAddr:30;	
#else
	uint32 orgAddr:30;
	uint32 rsv4:1;
	uint32 fcpu:1;
#endif
	

};


struct ext_Tx
{
#ifdef __LITTLE_ENDIAN
	uint32 data_length:12;
	uint32 rsv:18;
	uint32 eor:1;	
	uint32 own:1;	
#else
	uint32 own:1;	
	uint32 eor:1;	
	uint32 rsv:18;	
	uint32 data_length:12;	
#endif

	uint32 tx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 linkid:7;
	uint32 rsv2:25;
#else
	uint32 rsv2:25;
	uint32 linkid:7;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 fcpu:1;
	uint32 oeor:1;
	uint32 orgAddr:30;		
#else
	uint32 orgAddr:30;		
	uint32 oeor:1;
	uint32 fcpu:1;
#endif

	uint32 skb_header_addr;
};


struct mac_pRx
{
#ifdef __LITTLE_ENDIAN
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 data_length:11;
	uint32 ipv6:1;
#else
	uint32 data_length:12;
#endif
	uint32 ipseg:1;
	uint32 tcpf:1;
	uint32 udpf:1;
	uint32 ipf:1;
	uint32 pid0:1;
	uint32 pid1:1;
	uint32 crc:1;
	uint32 runt:1;
	uint32 res:1;
	uint32 lpkt:1;
	uint32 e8023:1;
	uint32 pppoe:1;
	uint32 bar:1;
	uint32 pam:1;
	uint32 mar:1;	
	uint32 fae:1;		
	uint32 ls:1;	
	uint32 fs:1;	
	uint32 eor:1;		
	uint32 own:1;	
#else	
	uint32 own:1;	
	uint32 eor:1;		
	uint32 fs:1;	
	uint32 ls:1;	
	uint32 fae:1;		
	uint32 mar:1;	
	uint32 pam:1;
	uint32 bar:1;
	uint32 pppoe:1;
	uint32 e8023:1;
	uint32 lpkt:1;
	uint32 res:1;
	uint32 runt:1;
	uint32 crc:1;
	uint32 pid1:1;
	uint32 pid0:1;
	uint32 ipf:1;
	uint32 udpf:1;
	uint32 tcpf:1;
	uint32 ipseg:1;
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ipv6:1;
	uint32 data_length:11;
#else
	uint32 data_length:12;
#endif

#endif
	uint32 rx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 vlan_vidh:4;
	uint32 vlan_cfi:1;
	uint32 vlan_prio:3;
	uint32 vlan_vidl:8;
	uint32 ava:1;
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 rsv:1;
	uint32 ACLHIT:1;
	uint32 CTAG:1;
	uint32 CTagSPA:4;
	uint32 ACLIdx:8;
#else
	uint32 rsv:15;
#endif
#else  //big indian
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ACLIdx:8;
	uint32 CTagSPA:4;
	uint32 CTAG:1;
	uint32 ACLHIT:1;
	uint32 rsv:1;
#else
	uint32 rsv:15;
#endif
	uint32 ava:1;
	uint32 vlan_vidl:8;
	uint32 vlan_prio:3;
	uint32 vlan_cfi:1;
	uint32 vlan_vidh:4;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 partial_checksum:16;
	uint32 reason:5;
	uint32 rsv1:3;
	uint32 rx_shift:8;
#else
	uint32 rx_shift:8;
	uint32 rsv1:3;
	uint32 reason:5;
	uint32 partial_checksum:16;
#endif

	uint32 skb_header_addr;

#ifdef __LITTLE_ENDIAN
	uint32 sram_map_of:8;
	uint32 rsv2:8;
	uint32 sram_map_id:9;
	uint32 rsv3:6;
	uint32 sram_en:1;
#else
	uint32 sram_en:1;
	uint32 rsv3:6;
	uint32 sram_map_id:9;
	uint32 rsv2:8;
	uint32 sram_map_of:8;
#endif

};


struct mac_vRx
{

#ifdef __LITTLE_ENDIAN
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 data_length:11;
	uint32 ipv6:1;
#else
	uint32 data_length:12;
#endif
	uint32 ipsec:1;
	uint32 tcpf:1;
	uint32 udpf:1;
	uint32 ipf:1;
	uint32 pid0:1;
	uint32 pid1:1;
	uint32 crc:1;
	uint32 runt:1;
	uint32 res:1;
	uint32 lpkt:1;
	uint32 e8023:1;
	uint32 pppoe:1;
	uint32 bar:1;
	uint32 pam:1;
	uint32 mar:1;	
	uint32 fae:1;		
	uint32 ls:1;	
	uint32 fs:1;	
	uint32 eor:1;		
	uint32 own:1;	
#else	
	uint32 own:1;	
	uint32 eor:1;	
	uint32 fs:1;		
	uint32 ls:1;		
	uint32 fae:1;			
	uint32 mar:1;		
	uint32 pam:1;	
	uint32 bar:1;	
	uint32 pppoe:1;	
	uint32 e8023:1;	
	uint32 lpkt:1;	
	uint32 res:1;	
	uint32 runt:1;
	uint32 crc:1;	
	uint32 pid1:1;
	uint32 pid0:1;
	uint32 ipf:1;	
	uint32 udpf:1;	
	uint32 tcpf:1;	
	uint32 ipsec:1;	
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ipv6:1;
	uint32 data_length:11;
#else
	uint32 data_length:12;
#endif
#endif	


	uint32 rx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 vlan_vidh:4;
	uint32 vlan_cfi:1;
	uint32 vlan_prio:3;
	uint32 vlan_vidl:8;
	uint32 ava:1;
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 rsv:1;
	uint32 ACLHIT:1;
	uint32 CTAG:1;
	uint32 CTagSPA:4;
	uint32 ACLIdx:8;
#else
	uint32 rsv:15;
#endif
#else //big endian
#if defined(CONFIG_RLE0437) || defined(CONFIG_RTL8681)
	uint32 ACLIdx:8;
	uint32 CTagSPA:4;
	uint32 CTAG:1;
	uint32 ACLHIT:1;
	uint32 rsv:1;
#else
	uint32 rsv:15;
#endif
	uint32 ava:1;
	uint32 vlan_vidl:8;	
	uint32 vlan_prio:3;	
	uint32 vlan_cfi:1;	
	uint32 vlan_vidh:4;	
#endif


#ifdef __LITTLE_ENDIAN
	uint32 partial_checksum:16;
	uint32 reason:5;
	uint32 rsv1:3;
	uint32 rx_shift:8;
#else
	uint32 rx_shift:8;
	uint32 rsv1:3;
	uint32 reason:5;	
	uint32 partial_checksum:16;	
#endif


	uint32 skb_header_addr;

};


struct sar_pRx
{
#ifdef __LITTLE_ENDIAN
	uint32 data_length:12;
	uint32 ipv6:1;
	uint32 l4csmf:1;
	uint32 mar:1;
	uint32 bar:1;
	uint32 clp:1;
	uint32 pti:3;
	uint32 wii:1;
	uint32 frgi:1;
	uint32 ppi:2;
	uint32 ipcerr:1;
	uint32 lenerr:1;
	uint32 crc32err:1;
	uint32 atmport:1;
	uint32 ls:1;
	uint32 fs:1;
	uint32 eor:1;		
	uint32 own:1;	
#else
	uint32 own:1;	
	uint32 eor:1;	
	uint32 fs:1;	
	uint32 ls:1;	
	uint32 atmport:1;	
	uint32 crc32err:1;	
	uint32 lenerr:1;	
	uint32 ipcerr:1;	
	uint32 ppi:2;	
	uint32 frgi:1;	
	uint32 wii:1;	
	uint32 pti:3;	
	uint32 clp:1;	
	uint32 bar:1;	
	uint32 mar:1;	
	uint32 l4csmf:1;	
	uint32 ipv6:1;	
	uint32 data_length:12;	
#endif

	uint32 rx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 bpc:16;
	uint32 trlr:16;
#else
	uint32 trlr:16;
	uint32 bpc:16;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 tucacc:16;
	uint32 reason:5;
	uint32 rsv:3;
	uint32 rx_shift:8;
#else
	uint32 rx_shift:8;
	uint32 rsv:3;
	uint32 reason:5;	
	uint32 tucacc:16;	
#endif

	uint32 skb_header_addr;

#ifdef __LITTLE_ENDIAN
	uint32 sram_map_of:8;
	uint32 rsv1:8;
	uint32 sram_map_id:9;
	uint32 rsv2:6;	
	uint32 sram_en:1;
#else
	uint32 sram_en:1;
	uint32 rsv2:6;		
	uint32 sram_map_id:9;	
	uint32 rsv1:8;	
	uint32 sram_map_of:8;	
#endif


};


struct sar_vRx
{
#ifdef __LITTLE_ENDIAN
	uint32 data_length:12;
	uint32 ipv6:1;
	uint32 l4csmf:1;
	uint32 mar:1;
	uint32 bar:1;
	uint32 clp:1;
	uint32 pti:3;
	uint32 wii:1;
	uint32 frgi:1;
	uint32 ppi:2;
	uint32 ipcerr:1;
	uint32 lenerr:1;
	uint32 crc32err:1;
	uint32 atmport:1;
	uint32 ls:1;
	uint32 fs:1;
	uint32 eor:1;		
	uint32 own:1;	
#else
	uint32 own:1;	
	uint32 eor:1;	
	uint32 fs:1;	
	uint32 ls:1;	
	uint32 atmport:1;	
	uint32 crc32err:1;	
	uint32 lenerr:1;	
	uint32 ipcerr:1;	
	uint32 ppi:2;	
	uint32 frgi:1;	
	uint32 wii:1;	
	uint32 pti:3;	
	uint32 clp:1;	
	uint32 bar:1;	
	uint32 mar:1;	
	uint32 l4csmf:1;	
	uint32 ipv6:1;	
	uint32 data_length:12;	
#endif

	uint32 rx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 bpc:16;
	uint32 trlr:16;
#else
	uint32 trlr:16;
	uint32 bpc:16;
#endif

#ifdef __LITTLE_ENDIAN
	uint32 tucacc:16;
	uint32 reason:5;
	uint32 rsv:3;
	uint32 rx_shift:8;
#else
	uint32 rx_shift:8;
	uint32 rsv:3;
	uint32 reason:5;	
	uint32 tucacc:16;	
#endif

	uint32 skb_header_addr;

};


struct ext_Rx
{
#ifdef __LITTLE_ENDIAN
	uint32 data_length:12;
	uint32 rsv:18;
	uint32 eor:1;	
	uint32 own:1;	
#else
	uint32 own:1;	
	uint32 eor:1;	
	uint32 rsv:18;	
	uint32 data_length:12;	
#endif

	uint32 rx_buffer_addr;

#ifdef __LITTLE_ENDIAN
	uint32 linkid:7;
	uint32 rsv2:9;
	uint32 reason:5;
	uint32 rsv3:3;	
	uint32 rx_shift:8;	
#else
	uint32 rx_shift:8;	
	uint32 rsv3:3;	
	uint32 reason:5;	
	uint32 rsv2:9;	
	uint32 linkid:7;	
#endif

	uint32 skb_header_addr;
};


struct sp_pRx
{
#ifdef __LITTLE_ENDIAN
//	uint32 data_length:12;
	uint32 rsv4:12;
	uint32 rsv:18;
	uint32 eor:1;		
	uint32 own:1;	
#else
	uint32 own:1;	
	uint32 eor:1;		
	uint32 rsv:18;
	uint32 rsv4:12;	
//	uint32 data_length:12;	
#endif

	uint32 rx_buffer_addr;

#if 0
#ifdef __LITTLE_ENDIAN
	uint32 linkid:7;
	uint32 rsv2:17;
//	uint32 rx_shift:8;
	uint32 rsv5:8;	
#else
	uint32 rsv5:8;	
//	uint32 rx_shift:8;
	uint32 rsv2:17;
	uint32 linkid:7;
#endif
#endif

	uint32 skb_header_addr;

#ifdef __LITTLE_ENDIAN
	uint32 sram_map_of:8;
	uint32 rsv1:8;
	uint32 sram_map_id:9;
	uint32 rsv3:6;	
	uint32 sram_en:1;
#else
	uint32 sram_en:1;
	uint32 rsv3:6;	
	uint32 sram_map_id:9;
	uint32 rsv1:8;
	uint32 sram_map_of:8;			
#endif

};

#ifdef RTL867X_MODEL_USER
#else
struct sk_buff2
{
	unsigned char *head;
	unsigned char *data;
	unsigned char *tail;	
	unsigned char *end;	
	unsigned int data_len;
};
#endif

// packet processor first desc pointer 
extern struct mac_pRx *first_mac_prx[MAC_RX_INTFS];
extern struct sar_pRx *first_sar_prx[SAR_INTFS];
extern struct ext_Rx *first_ext_prx[EXT_INTFS];

extern struct mac_pTx *first_mac_ptx[MAC_TX_INTFS];
extern struct sar_pTx*first_sar_ptx[SAR_INTFS];
extern struct ext_Tx *first_ext_ptx[EXT_INTFS];

extern struct mac_vRx *first_mac_vrx[MAC_RX_INTFS];
extern struct sar_vRx *first_sar_vrx[SAR_INTFS];
extern struct ext_Rx *first_ext_vrx[EXT_INTFS];

extern struct mac_vTx *first_mac_vtx[MAC_TX_INTFS];
extern struct sar_vTx*first_sar_vtx[SAR_INTFS];
extern struct ext_Tx *first_ext_vtx[EXT_INTFS];

extern struct sp_pRx *first_sp_to_mac[MAC_TX_INTFS];
extern struct sp_pRx *first_sp_to_sar[SAR_INTFS];
extern struct sp_pRx *first_sp_to_ext[EXT_INTFS];

// asic current pointer
extern struct mac_pRx *current_mac_prx[MAC_RX_INTFS];
extern struct sar_pRx *current_sar_prx[SAR_INTFS];
extern struct ext_Rx *current_ext_prx[EXT_INTFS];

extern struct mac_pTx *current_mac_ptx[MAC_TX_INTFS];
extern struct sar_pTx*current_sar_ptx[SAR_INTFS];
extern struct ext_Tx *current_ext_ptx[EXT_INTFS];

//struct mac_vRx *current_mac_vrx[MAC_RX_INTFS];
//struct sar_vRx *current_sar_vrx[SAR_INTFS];
//struct ext_Rx *current_ext_vrx[EXT_INTFS];

//struct mac_vTx *current_mac_vtx[MAC_TX_INTFS];
//struct sar_vTx*current_sar_vtx[SAR_INTFS];
//struct ext_Tx *current_ext_vtx[EXT_INTFS];

extern struct sp_pRx *current_sp_to_mac[MAC_TX_INTFS];
extern struct sp_pRx *current_sp_to_sar[SAR_INTFS];
extern struct sp_pRx *current_sp_to_ext[EXT_INTFS];


// packet processor pp pointer 
extern struct mac_pRx *pp_mac_prx[MAC_RX_INTFS];
extern struct sar_pRx *pp_sar_prx[SAR_INTFS];
extern struct ext_Rx *pp_ext_prx[EXT_INTFS];

extern struct mac_pTx *pp_mac_ptx[MAC_TX_INTFS];
extern struct sar_pTx*pp_sar_ptx[SAR_INTFS];
extern struct ext_Tx *pp_ext_ptx[EXT_INTFS];

extern struct mac_vRx *pp_mac_vrx[MAC_RX_INTFS];
extern struct sar_vRx *pp_sar_vrx[SAR_INTFS];
extern struct ext_Rx *pp_ext_vrx[EXT_INTFS];

extern struct mac_vTx *pp_mac_vtx[MAC_TX_INTFS];
extern struct sar_vTx*pp_sar_vtx[SAR_INTFS];
extern struct ext_Tx *pp_ext_vtx[EXT_INTFS];

extern struct sp_pRx *pp_sp_to_mac[MAC_TX_INTFS];
extern struct sp_pRx *pp_sp_to_sar[SAR_INTFS];
extern struct sp_pRx *pp_sp_to_ext[EXT_INTFS];

extern int	 mac_prx_ring_used[MAC_RX_INTFS];  // if used == 0 means the ring is empty
extern int	mac_ptx_ring_used[MAC_TX_INTFS];
extern int	sar_prx_ring_used[SAR_INTFS];
extern int	sar_ptx_ring_used[SAR_INTFS];
extern int	ext_prx_ring_used[EXT_INTFS];
extern int	ext_ptx_ring_used[EXT_INTFS];

/* ring size */
extern int mac_prx_ring_size[MAC_RX_INTFS];
extern int	mac_ptx_ring_size[MAC_TX_INTFS];
extern int	sar_prx_ring_size[SAR_INTFS];
extern int	sar_ptx_ring_size[SAR_INTFS];
extern int	ext_prx_ring_size[EXT_INTFS];
extern int	ext_ptx_ring_size[EXT_INTFS];
extern int	mac_vrx_ring_size[MAC_RX_INTFS];
extern int	mac_vtx_ring_size[MAC_TX_INTFS];
extern int	sar_vrx_ring_size[SAR_INTFS];
extern int	sar_vtx_ring_size[SAR_INTFS];
extern int	ext_vrx_ring_size[EXT_INTFS];
extern int	ext_vtx_ring_size[EXT_INTFS];

/* model code only */
extern int	mac_prx_ring_empty[MAC_RX_INTFS];
extern int	mac_ptx_ring_full[MAC_TX_INTFS];
extern int	sar_prx_ring_empty[SAR_INTFS];
extern int	sar_ptx_ring_full[SAR_INTFS];


/* counter */
extern int	mac_prx_drop[MAC_RX_INTFS];
extern int	mac_vrx_recv[MAC_RX_INTFS];
extern int	mac_vtx_fwd[MAC_TX_INTFS];
extern int	sar_prx_drop[SAR_INTFS];
extern int	sar_vrx_recv[SAR_INTFS];
extern int	sar_vtx_fwd[SAR_INTFS];
extern int	ext_prx_drop[EXT_INTFS];
extern int	ext_vrx_recv[EXT_INTFS];
extern int	ext_vtx_fwd[EXT_INTFS];
extern int	sp_to_sar_fwd[SAR_INTFS];
extern int	sp_to_mac_fwd[MAC_TX_INTFS];
extern int	sp_to_ext_fwd[MAC_TX_INTFS];

/* weight */
extern uint8 sar_prx_ring_weight[SAR_INTFS];
extern uint8 mac_prx_ring_weight[MAC_RX_INTFS];
extern uint8 ext_prx_ring_weight[EXT_INTFS];
extern uint8 sar_vtx_ring_weight[SAR_INTFS];
extern uint8 mac_vtx_ring_weight[MAC_TX_INTFS];
extern uint8 ext_vtx_ring_weight[EXT_INTFS];

extern unsigned char *sram_buf; /* for global use: SRAM mapping base address */
extern int8 ethnt_offset_map[2][5];

void model_ring_controller_init(void);

int model_mac_rx(int intf,unsigned char *pkt,int length);
int model_sar_rx(int intf,unsigned char *pkt,int length);
void model_mac_tx(void);
void model_sar_tx(void);


struct sk_buff *model_ext_rx(int intf,int length,struct sk_buff *rxskb,uint32 linkid);
void model_pp(void);
void model_interrupt(int intfidx, uint32 isr);
//int init_ring(void);



#endif

