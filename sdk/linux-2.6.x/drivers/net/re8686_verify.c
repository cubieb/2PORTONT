static struct tx_info local_tx_info;

static unsigned int test_mode = 0;
unsigned int pendding_test_pkt = 0;

int inVerifyMode(void){
	if(test_mode){
		return 1;
	}
	return 0;
}

#define INVERIFYMODE inVerifyMode()

#define PTP_TRAP 0x7e
#define PTP_MIRROR 0x7f

#ifdef RTL0371
	//no FPGA testing code
#else//RTL0371
	#ifdef GMAC_FPGA
		#define RL6266_FPGA
	#endif
#endif//RTL0371

struct ApolloCPUTAG_RX_TYPE4{
	u16 etherType;
	u16 protocol:8;
	u16 reason:8;
	u16 priority:3;
	u16 ttl:5;
	u16 l3r:1;
	u16 org:1;
	u16 rev:3;
	u16 spa:3;
	u16 rev1:10;
	u16 extPortMask:6;
};
struct ApolloCPUTAG_RX_TYPE2{
	u16 etherType;
	u16 protocol:8;
	u16 reason:8;
	u16 priority:3;
	u16 ttl:5;
	u16 l3r:1;
	u16 org:1;
	u16 rev:3;
	u16 spa:3;
	u16 rev1:6;
	u16 dsl_vc:4;
	u16 extPortMask:6;
};
struct ApolloCPUTAG_RX_TYPE3{
	u16 etherType;
	u16 protocol:8;
	u16 reason:8;
	u16 priority:3;
	u16 ttl:5;
	u16 l3r:1;
	u16 org:1;
	u16 rev:3;
	u16 spa:3;
	u16 rev1:2;
	u16 pctrl:1;
	u16 pon_stream_id:7;
	u16 extPortMask:6;
};
struct ApolloCPUTAG_RX_TYPE5{
	u16 etherType;
	u16 protocol:8;
	u16 reason:8;
	u16 priority:3;
	u16 rev:10;
	u16 spa:3;
	u16 second_ptp_1;
	u16 second_ptp_2;
	u16 second_ptp_3;
	u16 nano_ptp_1;
	u16 nano_ptp_2;
};
struct ApolloCPUTAG_RX_TYPE6{
	u16 etherType;
	u16 protocol:8;
	u16 reason:8;
	u16 priority:3;
	u16 rev:3;
	u16 ptp_sequenceId:7;
	u16 spa:3;
	u16 second_ptp_1;
	u16 second_ptp_2;
	u16 second_ptp_3;
	u16 nano_ptp_1;
	u16 nano_ptp_2;
};

struct ApolloCPUTAG_TX_TYPE2{
	u16 etherType;
	u16 protocol:8;
	u16 l3cs:1;
	u16 l4cs:1;
	u16 tx_portmask:6;
	u16 efid:1;
	u16 enhanced_fid:3;
	u16 priority_select:1;
	u16 priority:3;
	u16 keep:1;
	u16 vsel:1;
	u16 dislrn:1;
	u16 psel:1;
	#ifdef RTL0371
	u16 resvd:4;
	#else
	u16 rsv1:1;
	u16 rsv0:1;
	u16 L34_keep:1;
	u16 PTP:1;
	#endif
	u16 extspa:3;
	u16 pppoeact:2;
	u16 pppoeidx:3;
	u16 l2br:1;
	u16 pon_stream_id:7;
	#ifdef RTL0371
	u16 dsl_vc_forward_mask:16;
	#else
	u16 resvd:16;
	#endif
	u16 resvd1:16;
};

enum{
	NO_TEST,
	SELF_CPUTAG_RX,
	SELF_CPUTAG_TX,
	SELF_VLAN_RX,
	SELF_VLAN_TX,
	SELF_CPUTAG_VLAN_RX,
	SELF_CPUTAG_VLAN_TX,
	SELF_MRING_RX,
};

#define CHECK_IF_EQUAL(x,y,z) if((x) != (y)){ \
						printk("[!]%s\n", (z)); \
						goto error;                 \
						}

void tx_additional_setting_verify_version(struct sk_buff *skb, struct net_device *dev, struct tx_info *pTxInfo){
	#ifndef RL6266_FPGA
	if(!test_mode){
		_tx_additional_setting(skb, dev, pTxInfo);
	}
	#endif
}

void self_cputag_tx_test_process(struct sk_buff *skb, struct rx_info* pRxInfo){
	struct ApolloCPUTAG_TX_TYPE2* pTxInfo;
	pTxInfo = (struct ApolloCPUTAG_TX_TYPE2*)&skb->data[12];
	CHECK_IF_EQUAL(pTxInfo->l3cs , local_tx_info.opts1.bit.cputag_ipcs, "l3cs")
	CHECK_IF_EQUAL(pTxInfo->l4cs , local_tx_info.opts1.bit.cputag_l4cs, "l4cs")
	CHECK_IF_EQUAL(pTxInfo->tx_portmask , local_tx_info.opts3.bit.tx_portmask, "tx portmask")
	CHECK_IF_EQUAL(pTxInfo->efid , local_tx_info.opts2.bit.efid, "efid")
	CHECK_IF_EQUAL(pTxInfo->enhanced_fid, local_tx_info.opts2.bit.enhance_fid, "enhanced fid")
	CHECK_IF_EQUAL(pTxInfo->priority_select, local_tx_info.opts2.bit.aspri, "aspri")
	CHECK_IF_EQUAL(pTxInfo->priority, local_tx_info.opts2.bit.cputag_pri, "priority")
	CHECK_IF_EQUAL(pTxInfo->keep, local_tx_info.opts1.bit.keep, "keep")
	CHECK_IF_EQUAL(pTxInfo->vsel , local_tx_info.opts1.bit.vsel, "vsel")
	CHECK_IF_EQUAL(pTxInfo->dislrn , local_tx_info.opts1.bit.dislrn, "dislrn")
	CHECK_IF_EQUAL(pTxInfo->psel , local_tx_info.opts1.bit.cputag_psel, "psel")
	CHECK_IF_EQUAL(pTxInfo->extspa , local_tx_info.opts3.bit.extspa, "extspa")
	CHECK_IF_EQUAL(pTxInfo->pppoeact , local_tx_info.opts2.bit.tx_pppoe_action, "pppoeaction")
	CHECK_IF_EQUAL(pTxInfo->pppoeidx , local_tx_info.opts2.bit.tx_pppoe_idx, "pppoeidx")
	CHECK_IF_EQUAL(pTxInfo->l2br , local_tx_info.opts1.bit.blu, "l2br")
	CHECK_IF_EQUAL(pTxInfo->pon_stream_id , local_tx_info.opts3.bit.tx_dst_stream_id, "pon_stream_id")
	#ifdef RTL0371
	CHECK_IF_EQUAL(pTxInfo->dsl_vc_forward_mask , local_tx_info.opts3.bit.tx_dst_vc_mask, "dsl_vc_forward_mask")
	#else
	CHECK_IF_EQUAL(pTxInfo->rsv1 , local_tx_info.opts3.bit.rsv1, "rsv1")
	CHECK_IF_EQUAL(pTxInfo->rsv0 , local_tx_info.opts3.bit.rsv0, "rsv0")
	CHECK_IF_EQUAL(pTxInfo->L34_keep , local_tx_info.opts3.bit.l34_keep, "L34_keep")
	CHECK_IF_EQUAL(pTxInfo->PTP , local_tx_info.opts3.bit.ptp, "PTP")
	#endif
	return ;
error:
	memDump(skb->data, skb->len, "skb data");
	memDump(&local_tx_info,sizeof(local_tx_info),"local_tx_info");
}

unsigned int insert_double_vlantag(char* pkt, char* vlan1, char* vlan2, char* pattern, unsigned int pattern_len, unsigned int vtag_start_offset){
	int i;
	int vlan_len;

	if(vlan1 == NULL && vlan2 == NULL){
		memcpy(pkt, pattern, pattern_len);
		return pattern_len;
	}
	else if(vlan1 == NULL && vlan2 != NULL){
		vlan_len = 4;
	}
	else if(vlan1 != NULL && vlan2 == NULL){
		vlan_len = 4;
	}
	else{
		vlan_len = 8;
	}
	
	for(i = 0 ; i<vtag_start_offset ; i++)
	{
		pkt[i] = pattern[i];
	}
	if(vlan1){
		pkt[i++] = vlan1[0];
		pkt[i++] = vlan1[1];
		pkt[i++] = vlan1[2];
		pkt[i++] = vlan1[3];
	}
	if(vlan2){
		pkt[i++] = vlan2[0];
		pkt[i++] = vlan2[1];
		pkt[i++] = vlan2[2];
		pkt[i++] = vlan2[3];
	}
	for(; i<(pattern_len+vlan_len) ; i++)
	{
		pkt[i] = pattern[i - vlan_len];
	}
	return pattern_len + vlan_len;
}


unsigned char test_arp_packet[60] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x08, 0x06, 0x00, 0x01,
	0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xc0, 0xa8, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char test_arp_rxcputag_packet[68] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 
	0x88, 0x99, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, //rx cpu tag
	0x08, 0x06, 0x00, 0x01,
	0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xc0, 0xa8, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
unsigned char test_arp_rxcputag_ptp_packet[76] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 
	0x88, 0x99, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//rx cpu tag
	0x08, 0x06, 0x00, 0x01,
	0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xc0, 0xa8, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x01, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define RXVLANTAGTEST_TXPKT_SIZE 128
unsigned char rxvlantagtest_txpkt[RXVLANTAGTEST_TXPKT_SIZE] = {0};
#define TXVLANTAGTEST_TXPKT_SIZE 128
unsigned char txvlantagtest_txpkt[TXVLANTAGTEST_TXPKT_SIZE] = {0};
struct tx_info gtxvlantagtest_txInfo;

void self_cputag_rx_test_process(struct sk_buff *skb, struct rx_info* pRxInfo){
	struct ApolloCPUTAG_RX_TYPE4* pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
	//because ptp will not in rxInfo, so we need to check this first....any better idea....?
	if(pRxInfo->opts2.bit.ptp_in_cpu_tag_exist){
		struct ApolloCPUTAG_RX_TYPE5* pCpuTagType5 = (struct ApolloCPUTAG_RX_TYPE5*)&(skb->data[12]);
		struct ApolloCPUTAG_RX_TYPE5* pPattern = (struct ApolloCPUTAG_RX_TYPE5*)&test_arp_rxcputag_ptp_packet[12];		
		if(memcmp(&pCpuTagType5->second_ptp_1, &pPattern->second_ptp_1, 10)){
			printk("[!] PTP is wrong!\n");
			memDump(pCpuTagType5, 22, "pCpuTagType5");
			memDump(pPattern, 22, "pPattern");
			return;
		}
		#ifndef RTL0371
		{
			//when PTP exist, pon_stream_id will become ptp_sequenceId
			struct ApolloCPUTAG_RX_TYPE6* pPattern = (struct ApolloCPUTAG_RX_TYPE6*)&test_arp_rxcputag_ptp_packet[12];		
			CHECK_IF_EQUAL(pRxInfo->opts2.bit.pon_stream_id , pPattern->ptp_sequenceId, "ptp_sequenceId")
		}
		#endif
	}
	else{
		if(pRxInfo->opts3.bit.reason != pCpuTagType4->reason){
			printk("[!] reason %x, %x\n", pRxInfo->opts3.bit.reason, pCpuTagType4->reason);
			goto error;
		}
		if(pRxInfo->opts3.bit.internal_priority != pCpuTagType4->priority){
			printk("[!] priority %x, %x\n", pRxInfo->opts3.bit.internal_priority, pCpuTagType4->priority);
			goto error;
		}
		if(pRxInfo->opts3.bit.ext_port_ttl_1 != pCpuTagType4->ttl){
			printk("[!] ttl %x, %x\n", pRxInfo->opts3.bit.ext_port_ttl_1, pCpuTagType4->ttl);
			goto error;
		}
		if(pRxInfo->opts1.bit.l3routing != pCpuTagType4->l3r){
			printk("[!] l3r %x, %x\n", pRxInfo->opts1.bit.l3routing, pCpuTagType4->l3r);
			goto error;
		}
		if(pRxInfo->opts1.bit.origformat != pCpuTagType4->org){
			printk("[!] org %x, %x\n", pRxInfo->opts1.bit.origformat, pCpuTagType4->org);
			goto error;
		}
		if(pRxInfo->opts3.bit.dst_port_mask != pCpuTagType4->extPortMask){
			printk("[!] dst_port_mask %x, %x\n", pRxInfo->opts3.bit.dst_port_mask, pCpuTagType4->extPortMask);
			goto error;
		}
		if(pCpuTagType4->spa == VDSL_PORT){
			struct ApolloCPUTAG_RX_TYPE2* pCpuTagType2 = (struct ApolloCPUTAG_RX_TYPE2*)&test_arp_rxcputag_packet[12];
			if((pRxInfo->opts3.bit.src_port_num & 0x10) == 0){
				printk("[!] src_port_num %x, %x\n", pRxInfo->opts3.bit.src_port_num, pCpuTagType2->spa);
				goto error;
			}
			if((pRxInfo->opts3.bit.src_port_num & 0x0f) != pCpuTagType2->dsl_vc){
				printk("[!] src_port_num %x, %x\n", pRxInfo->opts3.bit.src_port_num, pCpuTagType2->spa);
				goto error;
			}
		}
		else if(pCpuTagType4->spa == PON_PORT){
			struct ApolloCPUTAG_RX_TYPE3* pCpuTagType3 = (struct ApolloCPUTAG_RX_TYPE3*)&test_arp_rxcputag_packet[12];
			if(pRxInfo->opts3.bit.src_port_num != pCpuTagType3->spa){
				printk("[!] src_port_num %x, %x\n", pRxInfo->opts3.bit.src_port_num, pCpuTagType3->spa);
				goto error;
			}
			if(pRxInfo->opts1.bit.pctrl != pCpuTagType3->pctrl){
				printk("[!] pctrl %x, %x\n", pRxInfo->opts1.bit.pctrl, pCpuTagType3->pctrl);
				goto error;
			}
			if(pRxInfo->opts2.bit.pon_stream_id != pCpuTagType3->pon_stream_id){
				printk("[!] pon_stream_id %x, %x\n", pRxInfo->opts2.bit.pon_stream_id, pCpuTagType3->pon_stream_id);
				goto error;
			}
		}
		else{
			if(pRxInfo->opts3.bit.src_port_num != pCpuTagType4->spa){
				printk("[!] src_port_num %x, %x\n", pRxInfo->opts3.bit.src_port_num, pCpuTagType4->spa);
				goto error;
			}
		}
	}

	return;
	
error:
	printk("rxInfo:\n");
	printk("opts1\t= 0x%08x\n", pRxInfo->opts1.dw);
	printk("addr\t= 0x%08x\n", pRxInfo->addr);
	printk("opts2\t= 0x%08x\n", pRxInfo->opts2.dw);
	printk("opts3\t= 0x%08x\n", pRxInfo->opts3.dw);

	memDump(pCpuTagType4, 8, "pCpuTagType4");
}

static inline unsigned int is_stag(unsigned int vlan_reg){
	return ((vlan_reg >> 15) & 0x01);
}

static inline unsigned int tag(unsigned int vlan_reg){
	return is_stag(vlan_reg)?((vlan_reg >> 16) & 0x0ffff) : 0x8100;
}

unsigned int generate_s_offset(unsigned int vlan_reg, unsigned int vlan_start_offset){
	unsigned int result = 0;
	
	if((vlan_reg >> 16) == 0x8100){
		if(txvlantagtest_txpkt[vlan_start_offset] == 0x81 &&
		txvlantagtest_txpkt[vlan_start_offset + 1] == 0x00 && 
		txvlantagtest_txpkt[vlan_start_offset + 4] == 0x81 &&
		txvlantagtest_txpkt[vlan_start_offset + 5] == 0x00){
			result = vlan_start_offset;
		}
		else if(txvlantagtest_txpkt[vlan_start_offset] == 0x81 && txvlantagtest_txpkt[vlan_start_offset + 1] == 0x00 &&
			gtxvlantagtest_txInfo.opts2.bit.tx_vlan_action != TXD_VLAN_INSERT){
			if(is_stag(vlan_reg)){
				result = vlan_start_offset;
			}
		}
		else if(txvlantagtest_txpkt[vlan_start_offset] == 0x81 && txvlantagtest_txpkt[vlan_start_offset + 1] == 0x00 &&
			gtxvlantagtest_txInfo.opts2.bit.tx_vlan_action == TXD_VLAN_INSERT){
			if(!is_stag(vlan_reg)){
				result = vlan_start_offset;
			}
		}
	}
	else{
		if(*(unsigned short*)&txvlantagtest_txpkt[vlan_start_offset] == ((vlan_reg >> 16) & 0x0ffff)){
			result = vlan_start_offset;
		}
	}
	return result;
}

unsigned int generate_c_offset(unsigned int vlan_reg, unsigned int vlan_start_offset, unsigned int s_offset){
	unsigned int result = 0;
	
	if((vlan_reg >> 16) == 0x8100){
		if(txvlantagtest_txpkt[vlan_start_offset] == 0x81 &&
		txvlantagtest_txpkt[vlan_start_offset + 1] == 0x00 && 
		txvlantagtest_txpkt[vlan_start_offset + 4] == 0x81 &&
		txvlantagtest_txpkt[vlan_start_offset + 5] == 0x00){
			result = vlan_start_offset + 4;
		}
		else if(txvlantagtest_txpkt[vlan_start_offset] == 0x81 && txvlantagtest_txpkt[vlan_start_offset + 1] == 0x00 &&
			gtxvlantagtest_txInfo.opts2.bit.tx_vlan_action != TXD_VLAN_INSERT){
			if(!is_stag(vlan_reg)){
				result = vlan_start_offset;
			}
		}
		else if(txvlantagtest_txpkt[vlan_start_offset] == 0x81 && txvlantagtest_txpkt[vlan_start_offset + 1] == 0x00 &&
			gtxvlantagtest_txInfo.opts2.bit.tx_vlan_action == TXD_VLAN_INSERT){
			if(is_stag(vlan_reg)){
				result = vlan_start_offset;
			}
		}
	}
	else{
		if(s_offset && *(unsigned short*)&txvlantagtest_txpkt[vlan_start_offset + 4] == 0x8100){
			result = vlan_start_offset + 4;
		}
		else if(!s_offset && *(unsigned short*)&txvlantagtest_txpkt[vlan_start_offset] == 0x8100){
			result = vlan_start_offset;
		}
	}
	return result;
}

int generate_predict_pkt(unsigned int vlan_reg, unsigned int vlan_start_offset, unsigned char* predict_pkt, struct sk_buff *skb){

#define TAG_ID (gtxvlantagtest_txInfo.opts2.bit.prio << 13) | (gtxvlantagtest_txInfo.opts2.bit.cfi << 12) | \
		(gtxvlantagtest_txInfo.opts2.bit.vidh << 8) | (gtxvlantagtest_txInfo.opts2.bit.vidl);
	
	unsigned int insert_offset = 0;
	unsigned int remove_offset = 0;
	unsigned int remark_offset = vlan_start_offset;
	unsigned int s_offset = generate_s_offset(vlan_reg, vlan_start_offset);
	unsigned int c_offset = generate_c_offset(vlan_reg, vlan_start_offset, s_offset);
	
	switch(gtxvlantagtest_txInfo.opts2.bit.tx_vlan_action){
		case TXD_VLAN_INTACT:
			memcpy(predict_pkt, txvlantagtest_txpkt, skb->len);
			break;
		case TXD_VLAN_INSERT:
			if(is_stag(vlan_reg) && !s_offset){
				insert_offset = vlan_start_offset;
			}
			else if(!is_stag(vlan_reg) && !c_offset){
				insert_offset = (s_offset)?s_offset+4:vlan_start_offset;
			}
			if(insert_offset){
				memcpy(predict_pkt, txvlantagtest_txpkt, insert_offset);
				*(unsigned short*)(predict_pkt + insert_offset) = tag(vlan_reg);
				*(unsigned short*)(predict_pkt + insert_offset + 2) = TAG_ID;
				memcpy(predict_pkt + insert_offset + 4
					, txvlantagtest_txpkt + insert_offset, skb->len - insert_offset);
			}
			else{
				memcpy(predict_pkt, txvlantagtest_txpkt, skb->len);
			}
			break;
		case TXD_VLAN_REMOVE:
			if(is_stag(vlan_reg) && s_offset){
				remove_offset = s_offset;
			}
			else if(!is_stag(vlan_reg) && c_offset){
				remove_offset = c_offset;
			}
			if(remove_offset){
				memcpy(predict_pkt, txvlantagtest_txpkt, remove_offset);
				memcpy(predict_pkt + remove_offset 
					, txvlantagtest_txpkt + remove_offset + 4, skb->len - remove_offset);
			}
			else{
				memcpy(predict_pkt, txvlantagtest_txpkt, skb->len);
			}
			break;
		case TXD_VLAN_REMARKING:
			if(s_offset || c_offset){
				if(is_stag(vlan_reg)){
					remark_offset = (s_offset)?s_offset:c_offset;
				}
				else{
					remark_offset = c_offset?c_offset:(s_offset+4);
				}
			}
			if((is_stag(vlan_reg) && !s_offset) || (!is_stag(vlan_reg) && !c_offset)){//insert
				memcpy(predict_pkt, txvlantagtest_txpkt, remark_offset);
				*(unsigned short*)(predict_pkt + remark_offset) = tag(vlan_reg);
				*(unsigned short*)(predict_pkt + remark_offset + 2) = TAG_ID;
				memcpy(predict_pkt + remark_offset + 4
					, txvlantagtest_txpkt + remark_offset, skb->len - remark_offset);
			}
			else{//remark
				memcpy(predict_pkt, txvlantagtest_txpkt, remark_offset);
				*(unsigned short*)(predict_pkt + remark_offset) = tag(vlan_reg);
				*(unsigned short*)(predict_pkt + remark_offset + 2) = TAG_ID;
				memcpy(predict_pkt + remark_offset + 4
					, txvlantagtest_txpkt + remark_offset + 4, skb->len - remark_offset -4);
			}
			break;
		default:
			printk("strange tx_vlan_action %x\n", gtxvlantagtest_txInfo.opts2.bit.tx_vlan_action);
			return -1;
	}
	return 0;
}

void _self_vlantag_tx_test_process(struct sk_buff *skb, struct rx_info* pRxInfo, unsigned int skb_vlan_start_offset){
	unsigned int vlan_reg;
	unsigned char predict_pkt[TXVLANTAGTEST_TXPKT_SIZE];
	vlan_reg = RTL_R32(VLAN_REG);

	if(generate_predict_pkt(vlan_reg, 12, predict_pkt, skb)){
		goto error;
	}
	if(memcmp(predict_pkt, skb->data, 12) || memcmp(predict_pkt + 12, skb->data + skb_vlan_start_offset, skb->len - skb_vlan_start_offset)){
		goto error;
	}
	
	return;
	
error:
	printk("vlan_reg %x\n", vlan_reg);
	printk("txInfo:\n");
	printk("opts1\t= 0x%08x\n", gtxvlantagtest_txInfo.opts1.dw);
	printk("addr\t= 0x%08x\n", gtxvlantagtest_txInfo.addr);
	printk("opts2\t= 0x%08x\n", gtxvlantagtest_txInfo.opts2.dw);
	printk("opts3\t= 0x%08x\n", gtxvlantagtest_txInfo.opts3.dw);
	memDump(skb->data, skb->len, "skb data");
	memDump(predict_pkt, skb->len, "predict_pkt");
	memDump(txvlantagtest_txpkt, TXVLANTAGTEST_TXPKT_SIZE, "txvlantagtest_txpkt");
}

void self_vlantag_tx_test_process(struct sk_buff *skb, struct rx_info* pRxInfo){
	_self_vlantag_tx_test_process(skb, pRxInfo, 12);
}

void self_vlantag_rx_test_process(struct sk_buff *skb, struct rx_info* pRxInfo){
	unsigned int vlan_reg;
	unsigned int stag_len = 0;
	vlan_reg = RTL_R32(VLAN_REG);
	//strange logic.................
	//two case hw will think there is a stag:
	// 1. vlan_reg != 0x8100 and txpkt[12 13] == vlan_reg
	// 2. vlan_reg == 0x8100 and txpkt[12 13] == txpkt[16 17] == 0x8100
	if(((vlan_reg >> 16) != 0x8100 && !memcmp(rxvlantagtest_txpkt + 12, &vlan_reg, 2)) //condition 1....
		||
		((vlan_reg >> 16) == 0x8100 && 
		rxvlantagtest_txpkt[12] == 0x81 &&
		rxvlantagtest_txpkt[13] == 0x00 &&
		rxvlantagtest_txpkt[16] == 0x81 &&
		rxvlantagtest_txpkt[17] == 0x00 )//condition 2....
		){//process sid
		//printk("%s %d\n", __func__, __LINE__);
		if(pRxInfo->opts2.bit.svlan_tag_exist != 1){
			printk("[!] svlan wrong!\n");
			goto error;
		}
		if(memcmp(&skb->data[12], rxvlantagtest_txpkt + 12, 4)){
			printk("[!] svlan in rx pkt wrong!\n");
			goto error;
		}
		stag_len = 4;
	}
	//printk("%s %d stag_len %d\n", __func__, __LINE__, stag_len);
	if(rxvlantagtest_txpkt[12 + stag_len] == 0x81 && rxvlantagtest_txpkt[13 + stag_len] == 0x00){
		//printk("%s %d\n", __func__, __LINE__);
		if(pRxInfo->opts2.bit.ctagva != 1){
			printk("[!] ctagva wrong!\n");
			goto error;
		}
		if(rxvlantagtest_txpkt[14 + stag_len] != (pRxInfo->opts2.bit.cvlan_tag&0x00ff) ||
			rxvlantagtest_txpkt[15 + stag_len] != ((pRxInfo->opts2.bit.cvlan_tag&0xff00) >> 8)){
			printk("[!] cvlan_tag wrong!\n");
			goto error;
		}
		if(memcmp(&skb->data[12+stag_len], &rxvlantagtest_txpkt[12+stag_len+4], 4)){
		//if(skb->data[12+stag_len] == 0x81 && skb->data[13+stag_len] == 0x00){
			printk("[!] ctag not skip!!\n");
			goto error;
		}
	}
	
	return;
	
error:
	printk("vlan_reg %x\n", vlan_reg);
	printk("rxInfo:\n");
	printk("opts1\t= 0x%08x\n", pRxInfo->opts1.dw);
	printk("addr\t= 0x%08x\n", pRxInfo->addr);
	printk("opts2\t= 0x%08x\n", pRxInfo->opts2.dw);
	printk("opts3\t= 0x%08x\n", pRxInfo->opts3.dw);
	memDump(skb->data, skb->len, "skb data");
	memDump(rxvlantagtest_txpkt, RXVLANTAGTEST_TXPKT_SIZE, "rxvlantagtest_txpkt");
}

int send_test_packet(char* pkt, unsigned int len, struct tx_info* ptxInfo){
	struct sk_buff *skb = NULL;
	unsigned int cnt = 1000;
	skb = re8670_getAlloc(SKB_BUF_SIZE);
	if(!skb){
		printk("%s %d\n", __func__, __LINE__);
		return -1;
	}
	memcpy(skb->data, pkt, len);
	skb_put(skb, len);
	pendding_test_pkt = 1;
	//printk("RXOK %x\n", RTL_R16(RXOKCNT));
	re8686_send_with_txInfo(skb, ptxInfo,0);/*use ring 0 to send*/
	while(pendding_test_pkt && cnt--){
		//printk("RXOK %x\n", RTL_R16(RXOKCNT));
		mdelay(1);
	}
	
	if(pendding_test_pkt){
		printk("%s %d: %x\n", __func__, __LINE__, cnt);
		return -1;
	}
		
	return 0;
}

void self_cputag_rx_test(void){
	struct ApolloCPUTAG_RX_TYPE4* pCpuTagType4;
	struct ApolloCPUTAG_RX_TYPE3* pCpuTagType3;
	struct ApolloCPUTAG_RX_TYPE2* pCpuTagType2;
	struct ApolloCPUTAG_RX_TYPE5* pCpuTagType5;
	unsigned int i;
	struct tx_info txInfo;
	memset(&txInfo, 0, sizeof(txInfo));
#if 0 /* 2012-3-26 krammer add */
	//mytest
	printk("my test\n");
	pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
	pCpuTagType4->reason = 0x7e;
	pCpuTagType4->priority = 0x07;
	pCpuTagType4->ttl = 0x1f;
	pCpuTagType4->l3r = 1;
	pCpuTagType4->org = 1;
	if(send_test_arp_rxcputag_packet()){
		goto something_wrong;
	}
	printk("my test2\n");
	pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
	pCpuTagType4->reason = 0x5a;
	pCpuTagType4->priority = 0x07;
	pCpuTagType4->ttl = 0x1f;
	pCpuTagType4->l3r = 1;
	pCpuTagType4->org = 1;
	if(send_test_arp_rxcputag_packet()){
		goto something_wrong;
	}
#else
	//reason
	printk("reason test\n");
	for(i=0;i<(1<<8);i++){
		if(i == PTP_MIRROR || i == PTP_TRAP){
			continue;//skip now
		}
		pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		pCpuTagType4->reason = i;
		if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
			goto something_wrong;
		}
	}
	//priority
	printk("priority test\n");
	for(i=0;i<(1<<3);i++){
		pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		pCpuTagType4->priority = i;
		if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
			goto something_wrong;
		}
	}
	//ttl
	printk("ttl test\n");
	for(i=0;i<(1<<5);i++){
		pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		pCpuTagType4->ttl = i;
		if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
			goto something_wrong;
		}
	}
	//l3r
	printk("l3r test\n");
	for(i=0;i<(1<<1);i++){
		pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		pCpuTagType4->l3r = i;
		if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
			goto something_wrong;
		}
	}
	//org
	printk("org test\n");
	for(i=0;i<(1<<1);i++){
		pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		pCpuTagType4->org = i;
		if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
			goto something_wrong;
		}
	}
	//extPortMask
	printk("extPortMask test\n");
	for(i=0;i<(1<<6);i++){
		pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		pCpuTagType4->extPortMask= i;
		if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
			goto something_wrong;
		}
	}
	//spa
	printk("spa test\n");
	for(i=0;i<(1<<3);i++){
		int j;
		pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		pCpuTagType4->spa = i;
		if(i == VDSL_PORT){
			//dsl_vc
			printk("dsl_vc test\n");
			for(j=0;j<(1<<4);j++){
				pCpuTagType2 = (struct ApolloCPUTAG_RX_TYPE2*)&test_arp_rxcputag_packet[12];
				pCpuTagType2->dsl_vc = j;
				if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
					goto something_wrong;
				}
			}
		}
		else if(i == PON_PORT){
			//pon_stream_id
			printk("pon_stream_id test\n");
			for(j=0;j<(1<<7);j++){
				pCpuTagType3 = (struct ApolloCPUTAG_RX_TYPE3*)&test_arp_rxcputag_packet[12];
				pCpuTagType3->pon_stream_id= j;
				if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
					goto something_wrong;
				}
			}
			//pctrl
			printk("pctrl test\n");
			for(j=0;j<(1<<1);j++){
				pCpuTagType3 = (struct ApolloCPUTAG_RX_TYPE3*)&test_arp_rxcputag_packet[12];
				pCpuTagType3->pctrl= j;
				if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
					goto something_wrong;
				}
			}
		}
		else{
			if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
				goto something_wrong;
			}
		}
	}
	//ptp
	printk("ptp test\n");
	{
		pCpuTagType5 = (struct ApolloCPUTAG_RX_TYPE5*)&test_arp_rxcputag_ptp_packet[12];
		pCpuTagType5->spa = 0;//spa should be others
		//PTP_TRAP
		pCpuTagType5->reason = PTP_TRAP;
		pCpuTagType5->second_ptp_1 = 0x5a5a;
		pCpuTagType5->second_ptp_2 = 0x5a5a;
		pCpuTagType5->second_ptp_3 = 0x5a5a;
		pCpuTagType5->nano_ptp_1 = 0x5a5a;
		pCpuTagType5->nano_ptp_2 = 0x5a5a;
		if(send_test_packet(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), &txInfo)){
			goto something_wrong;
		}
		pCpuTagType5->reason = PTP_TRAP;
		pCpuTagType5->second_ptp_1 = 0xa5a5;
		pCpuTagType5->second_ptp_2 = 0xa5a5;
		pCpuTagType5->second_ptp_3 = 0xa5a5;
		pCpuTagType5->nano_ptp_1 = 0xa5a5;
		pCpuTagType5->nano_ptp_2 = 0xa5a5;
		if(send_test_packet(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), &txInfo)){
			goto something_wrong;
		}
		//PTP_MIRROR
		pCpuTagType5->reason = PTP_MIRROR;
		pCpuTagType5->second_ptp_1 = 0x5a5a;
		pCpuTagType5->second_ptp_2 = 0x5a5a;
		pCpuTagType5->second_ptp_3 = 0x5a5a;
		pCpuTagType5->nano_ptp_1 = 0x5a5a;
		pCpuTagType5->nano_ptp_2 = 0x5a5a;
		if(send_test_packet(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), &txInfo)){
			goto something_wrong;
		}
		pCpuTagType5->reason = PTP_MIRROR;
		pCpuTagType5->second_ptp_1 = 0xa5a5;
		pCpuTagType5->second_ptp_2 = 0xa5a5;
		pCpuTagType5->second_ptp_3 = 0xa5a5;
		pCpuTagType5->nano_ptp_1 = 0xa5a5;
		pCpuTagType5->nano_ptp_2 = 0xa5a5;
		if(send_test_packet(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), &txInfo)){
			goto something_wrong;
		}
		#ifndef RTL0371
		{
			struct ApolloCPUTAG_RX_TYPE6* pCpuTagType6;
			printk("ptp sequencedID test\n");
			for(i=0;i<(1<<7);i++){
				pCpuTagType6 = (struct ApolloCPUTAG_RX_TYPE6*)&test_arp_rxcputag_ptp_packet[12];
				pCpuTagType6->ptp_sequenceId = i;
				if(send_test_packet(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), &txInfo)){
					goto something_wrong;
				}
			}
		}
		#endif
	}
#endif /* 2012-3-26 krammer add */

	return;
		
something_wrong:
	printk("there are something wrong\n");
}

void self_cputag_tx_test(void){
	unsigned int i;
	//l3cs
	printk("l3cs\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.cputag_ipcs = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//l4cs
	printk("l4cs\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.cputag_l4cs = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//tx portmask
	printk("tx portmask\n");
	for(i=0;i<(1<<6);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.tx_portmask = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//efid
	printk("efid\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.efid = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//enhance_fid
	printk("enhance_fid\n");
	for(i=0;i<(1<<3);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.enhance_fid = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//priority select
	printk("priority select\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.aspri = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//cputag pri
	printk("cputag pri\n");
	for(i=0;i<(1<<3);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.cputag_pri = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//keep
	printk("keep\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.keep = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//vsel
	printk("vsel\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.vsel = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//dislrn
	printk("dislrn\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.dislrn = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//psel
	printk("psel\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.cputag_psel = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//extra spa
	printk("extra spa\n");
	for(i=0;i<(1<<3);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.extspa = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//pppoe act
	printk("pppoe act\n");
	for(i=0;i<(1<<2);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.tx_pppoe_action = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//pppoe idx
	printk("pppoe idx\n");
	for(i=0;i<(1<<3);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.tx_pppoe_idx = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//l2br
	printk("l2br\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.blu = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	//pon stream id
	printk("pon stream id\n");
	for(i=0;i<(1<<7);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.tx_dst_stream_id = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	#ifdef RTL0371
	//dsl vc forward mask
	printk("dsl vc forward mask\n");
	for(i=0;i<(1<<16);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.tx_dst_vc_mask = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	#else
	printk("rsv1\n");
	for(i=0;i<(1<<0);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.rsv1 = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	printk("rsv0\n");
	for(i=0;i<(1<<0);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.rsv0 = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	printk("L34_keep\n");
	for(i=0;i<(1<<0);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.l34_keep = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	printk("PTP\n");
	for(i=0;i<(1<<0);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.ptp = i;
		local_tx_info.opts2.bit.cputag = 1;
		if(send_test_packet(test_arp_packet, sizeof(test_arp_packet), &local_tx_info)){
			goto something_wrong;
		}
	}
	#endif

	return;
		
something_wrong:
	printk("there are something wrong\n");
}

void self_vtag_tx_test(struct tx_info* pInTxInfo){
	int i, j, k, l, m;
	unsigned int tdsc_vlan_type[2] = {0, (1<<15)};
	unsigned int txd_vlan_action[4] = {TXD_VLAN_INTACT, TXD_VLAN_INSERT, TXD_VLAN_REMOVE, TXD_VLAN_REMARKING};
	unsigned int stag_pid[2] = {0x88a80000, 0x81000000};
	unsigned char vtag8100[4] = {0x81, 0x00, 0x01, 0x23};
	unsigned char vtag8100_1[4] = {0x81, 0x00, 0x45, 0x67};
	unsigned char vtag88a8[4] = {0x88, 0xa8, 0x89, 0xab};
	unsigned char vtag88a8_1[4] = {0x88, 0xa8, 0xcd, 0xef};
	unsigned char *vtagNoTag = NULL;
	unsigned char *pVtag_pattern[3] = {vtagNoTag, vtag8100, vtag88a8};
	unsigned char *pVtag_pattern_1[3] = {vtagNoTag, vtag8100_1, vtag88a8_1};

	for(i=0;i<sizeof(tdsc_vlan_type)/sizeof(unsigned int);i++){
		for(j=0;j<sizeof(txd_vlan_action)/sizeof(unsigned int);j++){
			for(k=0;k<sizeof(stag_pid)/sizeof(unsigned int);k++){
				for(l=0;l<sizeof(pVtag_pattern)/sizeof(unsigned char *);l++){
					for(m=0;m<sizeof(pVtag_pattern_1)/sizeof(unsigned char *);m++){
						struct tx_info txInfo;
						unsigned int pkt_len;
						unsigned char pkt[TXVLANTAGTEST_TXPKT_SIZE];
						if(pInTxInfo){
							memcpy(&txInfo, pInTxInfo, sizeof(struct tx_info));
						}
						else{
							memset(&txInfo, 0, sizeof(struct tx_info));
						}
						RTL_W32(VLAN_REG, stag_pid[k] | tdsc_vlan_type[i]);
						pkt_len = insert_double_vlantag(pkt, pVtag_pattern[l], pVtag_pattern_1[m], test_arp_packet, sizeof(test_arp_packet), 12);
						memcpy(txvlantagtest_txpkt, pkt, pkt_len);
						txInfo.opts2.bit.vidl = 0x5a;
						txInfo.opts2.bit.prio = 0x5;
						txInfo.opts2.bit.cfi = 0x1;
						txInfo.opts2.bit.vidh = 0xa;
						txInfo.opts2.bit.tx_vlan_action = j;
						memcpy(&gtxvlantagtest_txInfo, &txInfo, sizeof(txInfo));
						if(send_test_packet(pkt, pkt_len, &txInfo)){
							goto something_wrong;
						}
					}
				}
			}
		}
	}

	return;

something_wrong:
	printk("there are something wrong, %d %d %d %d %d\n", i, j, k, l, m);
}

void all_rxvtag_combination(unsigned char* orig_pkt, unsigned int orig_pkt_len, unsigned int vtag_start_offset){
	int i, j, k;
	unsigned int stag_pid[2] = {0x88a80000, 0x81000000};
	unsigned char vtag8100[4] = {0x81, 0x00, 0x5a, 0xa5};
	unsigned char vtag8100_2[4] = {0x81, 0x00, 0xa5, 0x5a};
	unsigned char vtag88a8[4] = {0x88, 0xa8, 0xa5, 0x5a};
	unsigned char vtag88a8_2[4] = {0x88, 0xa8, 0x5a, 0xa5};
	unsigned char* vtagNoTag = NULL;
	unsigned char *pVtag_pattern[3] = {vtagNoTag, vtag8100, vtag88a8};
	unsigned char *pVtag_pattern_2[3] = {vtagNoTag, vtag8100_2, vtag88a8_2};
	for(i=0;i<sizeof(stag_pid)/sizeof(unsigned int);i++){
		for(j=0;j<sizeof(pVtag_pattern)/sizeof(unsigned char *);j++){
			for(k=0;k<sizeof(pVtag_pattern_2)/sizeof(unsigned char *);k++){
				struct tx_info txInfo;
				unsigned int pkt_len;
				unsigned char pkt[RXVLANTAGTEST_TXPKT_SIZE];
				memset(&txInfo, 0, sizeof(struct tx_info));
				RTL_W32(VLAN_REG, stag_pid[i]);
				pkt_len = insert_double_vlantag(pkt, pVtag_pattern[j], pVtag_pattern_2[k], orig_pkt, orig_pkt_len, vtag_start_offset);
				memcpy(rxvlantagtest_txpkt, pkt, pkt_len);
				if(send_test_packet(pkt, pkt_len, &txInfo)){
					goto something_wrong;
				}
			}
		}
	}
	return;

something_wrong:
	printk("something wrong!\n");
}

void self_vtag_rx_test(void){
	all_rxvtag_combination(test_arp_packet, sizeof(test_arp_packet), 12);
}

void self_ctag_vtag_rx_test(void){
	struct ApolloCPUTAG_RX_TYPE4* pCpuTagType4;
	struct ApolloCPUTAG_RX_TYPE5* pCpuTagType5;
	struct tx_info txInfo;
	memset(&txInfo, 0, sizeof(txInfo));
	//reason
	printk("reason test\n");
	pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
	pCpuTagType4->reason = 0xa5;
	all_rxvtag_combination(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), 12 + sizeof(struct ApolloCPUTAG_RX_TYPE4));
	//ptp
	printk("ptp test\n");
	{
		pCpuTagType5 = (struct ApolloCPUTAG_RX_TYPE5*)&test_arp_rxcputag_ptp_packet[12];
		pCpuTagType5->spa = 0;//spa should be others
		//PTP_TRAP
		pCpuTagType5->reason = PTP_TRAP;
		pCpuTagType5->second_ptp_1 = 0x5a5a;
		pCpuTagType5->second_ptp_2 = 0x5a5a;
		pCpuTagType5->second_ptp_3 = 0x5a5a;
		pCpuTagType5->nano_ptp_1 = 0x5a5a;
		pCpuTagType5->nano_ptp_2 = 0x5a5a;
		all_rxvtag_combination(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), 12 + sizeof(struct ApolloCPUTAG_RX_TYPE5));
		pCpuTagType5->reason = PTP_MIRROR;
		pCpuTagType5->second_ptp_1 = 0xa5a5;
		pCpuTagType5->second_ptp_2 = 0xa5a5;
		pCpuTagType5->second_ptp_3 = 0xa5a5;
		pCpuTagType5->nano_ptp_1 = 0xa5a5;
		pCpuTagType5->nano_ptp_2 = 0xa5a5;
		all_rxvtag_combination(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), 12 + sizeof(struct ApolloCPUTAG_RX_TYPE5));
		#ifndef RTL0371
		{
			struct ApolloCPUTAG_RX_TYPE6* pCpuTagType6;
			printk("ptp sequencedID test\n");
			pCpuTagType6 = (struct ApolloCPUTAG_RX_TYPE6*)&test_arp_rxcputag_ptp_packet[12];
			pCpuTagType6->ptp_sequenceId = 0x5a;
			all_rxvtag_combination(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), 12 + sizeof(struct ApolloCPUTAG_RX_TYPE6));
		}
		#endif
	}
}

void self_ctag_vtag_tx_test(void){
	unsigned int i;
	//l3cs
	printk("l3cs\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.cputag_ipcs = i;
		local_tx_info.opts2.bit.cputag = 1;
		self_vtag_tx_test(&local_tx_info);
	}
#if 1 /* 2012-6-18 krammer add */
	//tx portmask
	printk("tx portmask\n");
	for(i=0;i<(1<<6);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts3.bit.tx_portmask = i;
		local_tx_info.opts2.bit.cputag = 1;
		self_vtag_tx_test(&local_tx_info);
	}
	//priority select
	printk("priority select\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.aspri = i;
		local_tx_info.opts2.bit.cputag = 1;
		self_vtag_tx_test(&local_tx_info);
	}
	//cputag pri
	printk("cputag pri\n");
	for(i=0;i<(1<<3);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts2.bit.cputag_pri = i;
		local_tx_info.opts2.bit.cputag = 1;
		self_vtag_tx_test(&local_tx_info);
	}
	//dislrn
	printk("dislrn\n");
	for(i=0;i<(1<<1);i++){
		memset(&local_tx_info, 0, sizeof(local_tx_info));
		local_tx_info.opts1.bit.dislrn = i;
		local_tx_info.opts2.bit.cputag = 1;
		self_vtag_tx_test(&local_tx_info);
	}
#else
#endif /* 2012-6-18 krammer add */

	return;
}

void self_mring_rx_test(void){
	int i=0, j=0;
	unsigned int tmp = 0;
	unsigned char pri[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
	unsigned int rring_routing_addr[6] = {RRING_ROUTING1, RRING_ROUTING2, RRING_ROUTING3,
		RRING_ROUTING4, RRING_ROUTING5, RRING_ROUTING6};
	struct ApolloCPUTAG_RX_TYPE4* pCpuTagType4;
	struct ApolloCPUTAG_RX_TYPE5* pCpuTagType5;
	struct tx_info txInfo;
	memset(&txInfo, 0, sizeof(txInfo));

	if(RX_MULTIRING_BITMAP != ((1<<MAX_RXRING_NUM)-1)){
		printk("RX_MULTIRING_BITMAP 0x%08x, plz open to 6 ring.....\n", RX_MULTIRING_BITMAP);
		goto something_wrong;
	}

#if 1 /* 2012-6-29 krammer add */
	for(i=0;i<6;i++){
#else
	for(i=2;i<8;i++){
#endif /* 2012-6-29 krammer add */
		tmp = 0;
		for(j=0;j<8;j++){
			if(i==0 && j==0){
				tmp |= 0x03;//special case( for compatibility issue test)
			}
			else{
				tmp |= (pri[(i + j)%8])<<(j*4);
			}
		}
		RTL_W32(rring_routing_addr[i%6], tmp);
	}

	printk("cpu tag type4 test\n");
	pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
	pCpuTagType4->reason = 0xa5;
	for(i=0;i<(1<<6);i++){
		for(j=0;j<(1<<3);j++){
			pCpuTagType4->extPortMask = i;
			pCpuTagType4->priority = j;
			//printk("send extPortMask %d priority %d\n", i, j);
			//mdelay(10);
			if(send_test_packet(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), &txInfo)){
				goto something_wrong;
			}
		}
	}
	printk("cpu tag type5 test\n");
	pCpuTagType5 = (struct ApolloCPUTAG_RX_TYPE5*)&test_arp_rxcputag_ptp_packet[12];
	pCpuTagType5->spa = 0;//spa should be others
	//PTP_TRAP
	pCpuTagType5->reason = PTP_TRAP;
	for(j=0;j<(1<<3);j++){
		pCpuTagType5->priority = j;
		//printk("priority %d\n", j);
		if(send_test_packet(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), &txInfo)){
			goto something_wrong;
		}
	}

	goto out;

something_wrong:
	printk("%s %d, something wrong, test fail....i %d, j %d\n", __func__, __LINE__, i, j);
out:
	//reset routing_reg
	for(i=0;i<6;i++){
		RTL_W32(rring_routing_addr[i], 0);
	}
}

static void re8670_rx (struct re_private *cp);
static inline unsigned char getRxRingBitMap(struct re_private *cp);
static inline void retriveRxInfo(DMA_RX_DESC *desc, struct rx_info *pRxInfo);
static inline void updateGmacFlowControl(unsigned rx_tail,int ring_num);
void check_ring_and_data(int ring_num, struct rx_info rxInfo, struct sk_buff *skb){
	unsigned int routing_reg_offset = 0;
	unsigned int ring_idx = 0;
	unsigned int extPortMask_is_0 = 0;
	unsigned int idx_to_ring[8] = {0,0,1,2,3,4,5,0};
	int pri = 0;
	
	if(rxInfo.opts2.bit.ptp_in_cpu_tag_exist){
		struct ApolloCPUTAG_RX_TYPE5* pCpuTagType5 = (struct ApolloCPUTAG_RX_TYPE5*)&test_arp_rxcputag_ptp_packet[12];
		extPortMask_is_0 = 1;
		routing_reg_offset = RRING_ROUTING1;
		pri = pCpuTagType5->priority;
		if(memcmp(skb->data, test_arp_rxcputag_ptp_packet, skb->len))
			goto error;
	}
	else{
		struct ApolloCPUTAG_RX_TYPE4* pCpuTagType4 = (struct ApolloCPUTAG_RX_TYPE4*)&test_arp_rxcputag_packet[12];
		switch(pCpuTagType4->extPortMask){
			case 0x02:
				routing_reg_offset = RRING_ROUTING2;
				break;
			case 0x04:
				routing_reg_offset = RRING_ROUTING3;
				break;
			case 0x08:
				routing_reg_offset = RRING_ROUTING4;
				break;
			case 0x10:
				routing_reg_offset = RRING_ROUTING5;
				break;
			case 0x20:
				routing_reg_offset = RRING_ROUTING6;
				break;
			case 0x0:
				extPortMask_is_0 = 1;
				/*pass through!!*/
			case 0x1:
			default:
				routing_reg_offset = RRING_ROUTING1;
				break;
				
		}
		pri = pCpuTagType4->priority;
		if(memcmp(skb->data, test_arp_rxcputag_packet, 12))
			goto error;
		if(memcmp(skb->data + 12, 
			test_arp_rxcputag_packet + 12 + sizeof(struct ApolloCPUTAG_RX_TYPE4), 
			skb->len - 12))
			goto error;
	}

	ring_idx = (RTL_R32(routing_reg_offset) & (0x07<<(pri*4)))>>(pri*4);

	if(extPortMask_is_0 && pri == 0){/*compact issue*/
		if(ring_num != 0)
			goto error;
	}
	else if(idx_to_ring[ring_idx] != ring_num){
		goto error;
	}

	return;
error:
	printk("ring_num %d idx_to_ring[ring_idx] %d\n", ring_num, idx_to_ring[ring_idx]);
	memDump(skb->data, skb->len, "skb data");
	if(rxInfo.opts2.bit.ptp_in_cpu_tag_exist){
		memDump(test_arp_rxcputag_ptp_packet, sizeof(test_arp_rxcputag_ptp_packet), "test_arp_rxcputag_ptp_packet");
	}
	else{
		memDump(test_arp_rxcputag_packet, sizeof(test_arp_rxcputag_packet), "test_arp_rxcputag_packet");
	}
	printk("RRING_ROUTING1 0x%08x\n", RTL_R32(RRING_ROUTING1));
	printk("RRING_ROUTING2 0x%08x\n", RTL_R32(RRING_ROUTING2));
	printk("RRING_ROUTING3 0x%08x\n", RTL_R32(RRING_ROUTING3));
	printk("RRING_ROUTING4 0x%08x\n", RTL_R32(RRING_ROUTING4));
	printk("RRING_ROUTING5 0x%08x\n", RTL_R32(RRING_ROUTING5));
	printk("RRING_ROUTING6 0x%08x\n", RTL_R32(RRING_ROUTING6));
}

void re8670_mringtest_rx (struct re_private *cp)
{
	unsigned long flags;  
	int ring_num=0;
	unsigned rx_Mtail;	  
	unsigned char rx_ring_bitmap_current=getRxRingBitMap(cp);
	unsigned char pri_map[MAX_RXRING_NUM] = {0,1,2,3,4,5};
	int i = 0;

	spin_lock_irqsave(&cp->lock,flags);   
	for(i=0;rx_ring_bitmap_current && i<MAX_RXRING_NUM;i++){
		ring_num = pri_map[i];
		
		if(NOT_IN_BITMAP(rx_ring_bitmap_current, ring_num))
			continue;

		rx_ring_bitmap_current&=~(1<<ring_num);
		
		rx_Mtail = cp->rx_Mtail[ring_num];	 

		while (1)
		{
			u32 len;
			struct sk_buff *skb, *new_skb;
			DMA_RX_DESC *desc;
			unsigned buflen;
			struct rx_info rxInfo;

			cp->cp_stats.rx_hw_num++;						
			skb = cp->rx_skb[ring_num][rx_Mtail].skb;

			if (unlikely(!skb))
				BUG();	   
			desc = &cp->rx_Mring[ring_num][rx_Mtail];	
			retriveRxInfo(desc, &rxInfo);

			if (rxInfo.opts1.bit.own)
				break;
			RXINFO_DBG(&rxInfo, ring_num);
			len = (rxInfo.opts1.bit.data_length & 0x07ff) - 4;
			buflen = cp->rx_buf_sz + RX_OFFSET;
			new_skb = re8670_getAlloc(SKB_BUF_SIZE);
			if (unlikely(!new_skb)) {
				printk("%s %d!!!!!!!!!!!!!!!!!!!!!!!!\n", __func__, __LINE__);
				dma_cache_wback_inv((unsigned long)skb->data,buflen);
				goto rx_next;
			}
			skb_reserve(skb, RX_OFFSET);
			skb_put(skb, len);
			SKB_DBG(skb, debug_enable, RTL8686_SKB_RX); 	
			cp->rx_Mring[ring_num][rx_Mtail].addr = (u32)new_skb->data;
			cp->rx_skb[ring_num][rx_Mtail].skb = new_skb;
			dma_cache_wback_inv((unsigned long)new_skb->data,buflen);
			
			check_ring_and_data(ring_num, rxInfo, skb);
			dev_kfree_skb_any(skb);
rx_next:
			desc->opts1 = (DescOwn | cp->rx_buf_sz) | ((rx_Mtail == (RE8670_RX_MRING_SIZE[ring_num] - 1))?RingEnd:0);
			updateGmacFlowControl(rx_Mtail,ring_num);
			rx_Mtail = NEXT_RX(rx_Mtail,RE8670_RX_MRING_SIZE[ring_num]);

		}
		cp->rx_Mtail[ring_num] = rx_Mtail;
	}
	RTL_R16(IMR)|=(u16)(RX_ALL);//we still open imr when rx_work==0 for a quickly schedule	
	UNMASK_IMR0_RXALL();
	spin_unlock_irqrestore (&cp->lock, flags); 
	pendding_test_pkt = 0;
}


//void re8686_verify_rx(struct re_private *cp, struct sk_buff *skb, struct rx_info* pRxInfo){
int re8686_verify_rx(struct re_private *cp, struct sk_buff *skb, struct rx_info* pRxInfo){
	switch(test_mode){
		case SELF_CPUTAG_RX:
			self_cputag_rx_test_process(skb, pRxInfo);
			pendding_test_pkt = 0;
			break;
		case SELF_CPUTAG_TX:
			self_cputag_tx_test_process(skb, pRxInfo);
			pendding_test_pkt = 0;
			break;	
		case SELF_VLAN_RX:
			self_vlantag_rx_test_process(skb, pRxInfo);
			pendding_test_pkt = 0;
			break;
		case SELF_VLAN_TX:
			self_vlantag_tx_test_process(skb, pRxInfo);
			pendding_test_pkt = 0;
			break;
		case SELF_CPUTAG_VLAN_RX:
			self_cputag_rx_test_process(skb, pRxInfo);
			self_vlantag_rx_test_process(skb, pRxInfo);
			pendding_test_pkt = 0;
			break;
		case SELF_CPUTAG_VLAN_TX:
			self_cputag_tx_test_process(skb, pRxInfo);
			_self_vlantag_tx_test_process(skb, pRxInfo, 12 + sizeof(struct ApolloCPUTAG_TX_TYPE2));
			pendding_test_pkt = 0;
			break;
		default:
			printk("why go to %s %d? something wrong\n", __func__, __LINE__);
			break;
	}

	dev_kfree_skb_any(skb);
	return 0;
}

void turnoff_sw_cputag_rx(void){
	WRITE_MEM32(0xbb023068, 0);
}
void turnoff_sw_cputag_tx(void){
	WRITE_MEM32(0xbb023064, 0);
}
void turnon_sw_cputag_rx(void){
	WRITE_MEM32(0xbb023068, 0x40);
}
void turnon_sw_cputag_tx(void){
	WRITE_MEM32(0xbb023064, 0x100);
}

void re8686_set_all_rxfunc_to_verify(void){
	unsigned int i;
	for(i=0;i<SW_PORT_NUM;i++){
		re8686_register_rxfunc_by_port(i, re8686_verify_rx);
	}
}

void vlan_detag(int onoff);

void _start_test(void){
	re8686_set_all_rxfunc_to_verify();
#ifndef RL6266_FPGA
	turnoff_sw_cputag_rx();
	turnoff_sw_cputag_tx();
	switch_dumb();
#endif
	vlan_detag(OFF);
}

void _end_test(void){
	vlan_detag(ON);
#ifndef RL6266_FPGA
	switch_cpu();
	turnon_sw_cputag_rx();
	turnon_sw_cputag_tx();
#endif
	re8686_reset_rxfunc_to_default();
}

void start_self_ctag_rx(void){
	test_mode = SELF_CPUTAG_RX;
	_start_test();
	self_cputag_rx_test();
	_end_test();
	test_mode = NO_TEST;
}

void start_self_ctag_tx(void){
	test_mode = SELF_CPUTAG_TX;
	_start_test();
	RTL_W32(CPUtagCR, RTL_R32(CPUtagCR)&~CTEN_RX);
	self_cputag_tx_test();
	RTL_W32(CPUtagCR, RTL_R32(CPUtagCR)|CTEN_RX);
	_end_test();
	test_mode = NO_TEST;
}

void start_self_vlan_rx(void){
	test_mode = SELF_VLAN_RX;
	_start_test();
	RTL_W8(CMD, RTL_R8(CMD)|RxVLAN_Detag);
	self_vtag_rx_test();
	RTL_W8(CMD, RTL_R8(CMD)&~RxVLAN_Detag);
	_end_test();
	test_mode = NO_TEST;
}
void start_self_vlan_tx(void){
	test_mode = SELF_VLAN_TX;
	_start_test();
	self_vtag_tx_test(NULL);
	_end_test();
	test_mode = NO_TEST;
}
void start_self_ctag_vlan_rx(void){
	test_mode = SELF_CPUTAG_VLAN_RX;
	_start_test();
	RTL_W8(CMD, RTL_R8(CMD)|RxVLAN_Detag);
	self_ctag_vtag_rx_test();
	RTL_W8(CMD, RTL_R8(CMD)&~RxVLAN_Detag);
	_end_test();
	test_mode = NO_TEST;
}
void start_self_ctag_vlan_tx(void){
	test_mode = SELF_CPUTAG_VLAN_TX;
	_start_test();	
	RTL_W32(CPUtagCR, RTL_R32(CPUtagCR)&~CTEN_RX);
	self_ctag_vtag_tx_test();	
	RTL_W32(CPUtagCR, RTL_R32(CPUtagCR)|CTEN_RX);
	_end_test();
	test_mode = NO_TEST;
}

void start_self_mring_rx(void){
	test_mode = SELF_MRING_RX;
	_start_test();	
	re_private_data.rx_tasklets.func=(void (*)(unsigned long))re8670_mringtest_rx;
	self_mring_rx_test();	
	re_private_data.rx_tasklets.func=(void (*)(unsigned long))re8670_rx;
	_end_test();
	test_mode = NO_TEST;
}

static int verify_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "self_rxctag\n");
	len += sprintf(page + len, "self_rxvlan\n");
	len += sprintf(page + len, "self_txctag\n");
	len += sprintf(page + len, "self_txvlan\n");
	len += sprintf(page + len, "self_rxctvlan\n");
	len += sprintf(page + len, "self_txctvlan\n");
	len += sprintf(page + len, "self_mring_rx\n");
	len += sprintf(page + len, "all\n");

	return len;
}
static int verify_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char		tmpbuf[64];
	char		*strptr, *cmd;

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd = strsep(&strptr," ");
		if (cmd==NULL)
		{
			goto errout;
		}
		printk("cmd %s\n", cmd);
		
		if(!memcmp(cmd, "hw", strlen("hw"))){
			printk("do hw spicify patch\n");
			#ifdef RL6266_FPGA
			RTL_W32(MIIAR, 0x84090000);//phy patch
			mdelay(50);
			RTL_W32(MIIAR, 0x84005140);
			mdelay(50);
			RTL_W32(MSR, 0xe03b8000);
			mdelay(50);
			#endif
		}
		else if(!memcmp(cmd, "self_rxctag", strlen("self_rxctag"))){
			printk("start to self cputag rx test, plz use a loopback connector!\n");
			start_self_ctag_rx();
		}
		else if(!memcmp(cmd, "self_rxvlan", strlen("self_rxvlan"))){
			printk("start to self vlan rx test, plz use a loopback connector!\n");
			start_self_vlan_rx();
		}
		else if(!memcmp(cmd, "self_txctag", strlen("self_txctag"))){
			printk("start to self cputag tx test, plz use a loopback connector!\n");
			start_self_ctag_tx();
		}
		else if(!memcmp(cmd, "self_txvlan", strlen("self_txvlan"))){
			printk("start to self vlan tx test, plz use a loopback connector!\n");
			start_self_vlan_tx();
		}
		else if(!memcmp(cmd, "self_rxctvlan", strlen("self_rxctvlan"))){
			printk("start to self ctag+vlan rx test, plz use a loopback connector!\n");
			start_self_ctag_vlan_rx();
		}
		else if(!memcmp(cmd, "self_txctvlan", strlen("self_txctvlan"))){
			printk("start to self ctag+vlan tx test, plz use a loopback connector!\n");
			start_self_ctag_vlan_tx();
		}
		else if(!memcmp(cmd, "self_mring_rx", strlen("self_mring_rx"))){
			printk("start to self mring rx test, plz use a loopback connector!\n");
			start_self_mring_rx();
		}
		else if(!memcmp(cmd, "all", strlen("all"))){
			printk("start to self test, plz use a loopback connector!\n");
			printk("========start_self_ctag_rx\n");
			start_self_ctag_rx();
			printk("========start_self_vlan_rx\n");
			start_self_vlan_rx();
			printk("========start_self_ctag_tx\n");
			start_self_ctag_tx();
			printk("========start_self_vlan_tx\n");
			start_self_vlan_tx();
			printk("========start_self_ctag_vlan_rx\n");
			start_self_ctag_vlan_rx();
			printk("========start_self_ctag_vlan_tx\n");
			start_self_ctag_vlan_tx();
			printk("========start_self_mring_rx\n");
			start_self_mring_rx();
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("self_rxctag\n");
		printk("self_rxvlan\n");
		printk("self_txctag\n");
		printk("self_txvlan\n");
		printk("self_rxctvlan\n");
		printk("self_txctvlan\n");
		printk("self_mring_rx\n");
		printk("all\n");
	}

	return len;
}

