
#ifdef __KERNEL__
#include <linux/circ_buf.h>
#include <linux/sched.h>
#endif

#include "../8190n_cfg.h"
#include "../8190n.h"
#include "../8190n_util.h"
#include "../8190n_headers.h"
#include "../8190n_debug.h"

#include <linux/syscalls.h>

UINT8 dot11_rate_table[]={2,4,11,22,12,18,24,36,48,72,96,108,0}; // last element must be zero!!

int get_rate_index_from_ieee_value(UINT8 val)
{
	int i;
	for (i=0; dot11_rate_table[i]; i++) {
		if (val == dot11_rate_table[i]) {
			return i;
		}
	}
	_DEBUG_ERR("Local error, invalid input rate for get_rate_index_from_ieee_value() [%d]!!\n", val);
	return 0;
}

int get_bit_value_from_ieee_value(u32 val)
{
	int i=0;
	while(dot11_rate_table[i] != 0) {
		if (dot11_rate_table[i] == val)
			return BIT(i);
		i++;
	}
	return 0;
}

void free_skb_queue(struct mac80211_shared_priv *priv, struct sk_buff_head	*skb_que)
{
	struct sk_buff *skb;

	while (skb_que->qlen > 0) {
		skb = __skb_dequeue(skb_que);
		if (skb == NULL)
			break;
		dev_kfree_skb_any(skb);
	}
}

unsigned int set_fw_reg(struct mac80211_shared_priv *priv, unsigned int cmd, unsigned int val, unsigned int with_val)
{
	static unsigned long ioaddr;
	static unsigned int delay_count;

	ioaddr = priv->pshare_hw->ioaddr;
	delay_count = 10;

	do {
		if (!RTL_R32(0x2c0))
			break;
		udelay(5);
		delay_count--;
	} while (delay_count);
	delay_count = 10;

	if (with_val == 1)
		RTL_W32(0x2c4, val);

	RTL_W32(0x2c0, cmd);

	do {
		if (!RTL_R32(0x2c0))
			break;
		udelay(5);
		delay_count--;
	} while (delay_count);

	return 0;
}

int _atoi(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else {
		while (*s != '\0') {
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				_DEBUG_ERR("error hex format!\n");
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

unsigned char * get_da(unsigned char *pframe)
{
	unsigned char 	*da;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			da = GetAddr1Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			da = GetAddr1Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			da = GetAddr3Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			da = GetAddr3Ptr(pframe);
			break;
	}

	return da;
}


int rtl80211_isErpSta(struct ieee80211_sta *sta)
{
	/* check rate indexes besides of 2, 4, 11, 22 */
	/* dot11_rate_table => {2,4,11,22,12,18,24,36,48,72,96,108,0}*/
	if (sta->supp_rates[IEEE80211_BAND_2GHZ] & 0xfffffff0 ) 
		return 1;	// ERP sta existed
	return 0;
}


void rtl80211_addRATid(struct mac80211_shared_priv *priv, struct ieee80211_sta *sta)
{
	
	unsigned char limit=16;
	int i;	
#if defined(RTL8190)
	unsigned char action;
	unsigned int pstat_hwaddr = (pstat->rssi << 24);	// 11n ap AES debug
	pstat->rssi_tick = 0;	// enhance f/w Rate Adaptive
	unsigned long ioaddr = priv->pshare->ioaddr;
#endif
	int fw_aid = sta->aid + 1;
	bool is_cw40 = false;
	bool is_sgi40 = false; 
	bool is_sgi20 = false;
	
	struct priv_stainfo *stainfo = (struct priv_stainfo *)sta->drv_priv;
	
	stainfo->tx_ra_bitmap = sta->supp_rates[IEEE80211_BAND_2GHZ];
	printk("sta->supprates = 0x%08x\n", sta->supp_rates[IEEE80211_BAND_2GHZ]);


	if (sta->ht_cap.ht_supported) {
		u8 mimo_ps_mode;
		mimo_ps_mode = (sta->ht_cap.cap & IEEE80211_HT_CAP_SM_PS) >> 2;
		if ((mimo_ps_mode &  _HT_MIMO_PS_STATIC_) || 
			(get_rf_mimo_mode(priv)== MIMO_1T2R) ||
			(get_rf_mimo_mode(priv)== MIMO_1T1R))
			limit=8;
		
		for (i=0; i<limit; i++) {
			if (sta->ht_cap.mcs.rx_mask[i/8] & BIT(i%8))
				stainfo->tx_ra_bitmap |= BIT(i+12);
		}
		
			
		is_cw40 = sta->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40;
		is_sgi40 = sta->ht_cap.cap & IEEE80211_HT_CAP_SGI_40;
		is_sgi20 = sta->ht_cap.cap & IEEE80211_HT_CAP_SGI_20;
			
		if ((priv->pshare_hw->is_40m_bw && is_sgi40 && is_cw40 && 
				(priv->band.ht_cap.cap & IEEE80211_HT_CAP_SGI_40))|| 
			 (is_sgi20 && (priv->band.ht_cap.cap & IEEE80211_HT_CAP_SGI_20) &&
			 !is_cw40))
#if defined(RTL8192SE) || defined (RTL8192SU)
			stainfo->tx_ra_bitmap |= BIT(28);
#elif defined(RTL8190)
			stainfo->tx_ra_bitmap |= BIT(31);
#endif

	}

	if ((stainfo->rssi_level < 1) || (stainfo->rssi_level > 3)) {
		if (stainfo->rssi >= 50 /*priv->pshare_hw->rf_ft_var.raGoDownUpper*/)
			stainfo->rssi_level = 1;
		else if ((stainfo->rssi >= 18 /*priv->pshare_hw->rf_ft_var.raGoDown20MLower*/) ||
			((priv->pshare_hw->is_40m_bw) &&
			(stainfo->rssi >= 15 /*priv->pshare_hw->rf_ft_var.raGoDown40MLower*/) &&
			(sta->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40)))
			stainfo->rssi_level = 2;
		else
			stainfo->rssi_level = 3;
	}
			

	if (sta->ht_cap.ht_supported) {
		if((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)){
			switch (stainfo->rssi_level) {
			case 1:
#if defined(RTL8192SE) || defined (RTL8192SU)
				stainfo->tx_ra_bitmap &= 0x100f0000;
#elif defined(RTL8190)
				stainfo->tx_ra_bitmap &= 0x800f0000;
#endif
				break;
			case 2:
#if defined(RTL8192SE) || defined (RTL8192SU)
				stainfo->tx_ra_bitmap &= 0x100ff000;
#elif defined(RTL8190)
				stainfo->tx_ra_bitmap &= 0x800ff000;
#endif
				break;
			case 3:
				if (priv->pshare_hw->is_40m_bw)
#if defined(RTL8192SE) || defined (RTL8192SU)
					stainfo->tx_ra_bitmap &= 0x100ff005;
#elif defined(RTL8190)
					stainfo->tx_ra_bitmap &= 0x800ff005;
#endif
				else
#if defined(RTL8192SE) || defined (RTL8192SU)
					stainfo->tx_ra_bitmap &= 0x100ff001;
#elif defined(RTL8190)
					stainfo->tx_ra_bitmap &= 0x800ff001;
#endif
				break;
			}
		}else {
			switch (stainfo->rssi_level) {
			case 1:
#if defined(RTL8192SE) || defined (RTL8192SU)
				stainfo->tx_ra_bitmap &= 0x1f8f0000;
#elif defined(RTL8190)
				stainfo->tx_ra_bitmap &= 0x8f0f0000;
#endif
				break;
			case 2:
#if defined(RTL8192SE) || defined (RTL8192SU)
				stainfo->tx_ra_bitmap &= 0x1f8ff000;
#elif defined(RTL8190)
				stainfo->tx_ra_bitmap &= 0x8f0ff000;
#endif
				break;
			case 3:
				if (priv->pshare_hw->is_40m_bw && is_cw40)
#if defined(RTL8192SE) || defined (RTL8192SU)
					stainfo->tx_ra_bitmap &= 0x010ff005;
#elif defined(RTL8190)
					stainfo->tx_ra_bitmap &= 0x8f0ff005;
#endif
				else
#if defined(RTL8192SE) || defined (RTL8192SU)
					stainfo->tx_ra_bitmap &= 0x010ff001;
#elif defined(RTL8190)
					stainfo->tx_ra_bitmap &= 0x8f0ff001;
#endif
				break;
			}
		}
	}else if (rtl80211_isErpSta(sta)) {
		switch (stainfo->rssi_level) {
		case 1:
			stainfo->tx_ra_bitmap &= 0x00000f00;
			break;
		case 2:
			stainfo->tx_ra_bitmap &= 0x00000ff0;
			break;
		case 3:
			stainfo->tx_ra_bitmap &= 0x00000ff5;
			break;
		}
	}else {
		stainfo->tx_ra_bitmap &= 0x0000000d;
	}
	
	
	
	// disable tx short GI when station cannot rx MCS15(AP is 2T and station is 2R)
	// disable tx short GI when station cannot rx MCS7 (AP is 1T or station is 1R)
	if (sta->ht_cap.ht_supported) {
		if (((stainfo->tx_ra_bitmap & 0xff00000) && !(stainfo->tx_ra_bitmap & 0x8000000)) ||
			(!(stainfo->tx_ra_bitmap & 0xff00000) && (stainfo->tx_ra_bitmap & 0xff000) &&
			!(stainfo->tx_ra_bitmap & 0x80000)) 
			/* chris: temporarily comment 
			|| (
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			!IS_ROOT_INTERFACE(priv) &&
#endif
			!GET_ROOT(priv)->pmib->dot11StationConfigEntry.autoRate)
			*/
			)
			stainfo->tx_ra_bitmap &= ~BIT(28);
	}
	//stainfo->tx_ra_bitmap = 0x010ff0f0;
	
	printk("sta[%d] supp_rate = 0x%08x\n", (unsigned int)fw_aid, stainfo->tx_ra_bitmap);
	if (stainfo->tx_ra_bitmap & 0xff00000) {
		if (priv->pshare_hw->is_40m_bw)
			set_fw_reg(priv, (0xfd00002a | ((fw_aid & 0x1f)<<4 | ARFR_2T_40M)<<8), stainfo->tx_ra_bitmap, 1);
		else
			set_fw_reg(priv, (0xfd00002a | ((fw_aid & 0x1f)<<4 | ARFR_2T_20M)<<8), stainfo->tx_ra_bitmap, 1);
	}
	else if (stainfo->tx_ra_bitmap & 0xff000) {
		if (priv->pshare_hw->is_40m_bw)
			set_fw_reg(priv, (0xfd00002a | ((fw_aid & 0x1f)<<4 | ARFR_1T_40M)<<8), stainfo->tx_ra_bitmap, 1);
		else
			set_fw_reg(priv, (0xfd00002a | ((fw_aid & 0x1f)<<4 | ARFR_1T_20M)<<8), stainfo->tx_ra_bitmap, 1);
	}
	else if (stainfo->tx_ra_bitmap & 0xff0)
		set_fw_reg(priv, (0xfd00002a | ((fw_aid & 0x1f)<<4 | ARFR_BG_MIX)<<8), stainfo->tx_ra_bitmap, 1);
	else
		set_fw_reg(priv, (0xfd00002a | ((fw_aid & 0x1f)<<4 | ARFR_B_ONLY)<<8), stainfo->tx_ra_bitmap, 1);

	if((get_rf_mimo_mode(priv) == MIMO_2T2R)
#ifdef STA_EXT
		&& pstat->remapped_aid < FW_NUM_STAT-1
#endif
		){
		int bitmap;
#ifdef STA_EXT
		bitmap = pstat->remapped_aid;
#else
		bitmap = fw_aid;
#endif
		//is this a 2r STA?
		if((stainfo->tx_ra_bitmap & 0x0ff00000) && !(priv->pshare_hw->has_2r_sta & BIT(bitmap))){
			priv->pshare_hw->has_2r_sta |= BIT(bitmap);
		}
	}
}

void rtl80211_delRATid(struct mac80211_shared_priv *priv, struct ieee80211_sta *sta)
{
	struct priv_stainfo *stainfo = (struct priv_stainfo *)sta->drv_priv;
	int fw_aid = sta->aid + 1;
#if defined(RTL8190)
	unsigned char action;
	unsigned long ioaddr = priv->pshare->ioaddr;
#endif

#ifdef RTL8192SE
	if((stainfo->tx_ra_bitmap & 0x0ff00000) && (get_rf_mimo_mode(priv) == MIMO_2T2R))
#ifdef STA_EXT
		if (pstat->remapped_aid < (FW_NUM_STAT - 1))
			priv->pshare->has_2r_sta &= ~ BIT(pstat->remapped_aid);
#else
		priv->pshare_hw->has_2r_sta &= ~ BIT(fw_aid);
#endif
#ifdef STA_EXT
	if (pstat->remapped_aid < (FW_NUM_STAT - 1))
		set_fw_reg(priv, (0xfd0000a5 | pstat->remapped_aid<< 16), 0,0);
#else
	set_fw_reg(priv, (0xfd0000a5 | fw_aid << 16), 0,0);
#endif
#endif

#ifdef STA_EXT
		//update STA_map
		if(pstat->remapped_aid == FW_NUM_STAT-1/*priv->STA_map & BIT(pstat->aid) */){
	//		priv->STA_map &= ~(BIT(pstat->aid));
	//		pstat->remapped_aid = 0;
		}else{
			//release the remapped aid
			int i;
			for(i = 1; i < NUM_STAT; i++)
				if(remapped_aidarray[i] == pstat->aid){
					remapped_aidarray[i] = 0;
					priv->pshare->fw_free_space ++;
					break;
				}
		}
		pstat->sta_in_firmware = -1;
#endif
	
#ifdef RTL8190
		action = BIT(7);	// remove
		if (pstat->aid != MANAGEMENT_AID)	// AID 7 use RATid 0
			action |= pstat->aid;
		RTL_W8(_RATR1_, action);
	
		RTL_W8(_RATR_POLL_, 1);
#endif

	DEBUG_INFO("Remove id %d from ratr\n", fw_aid);
}

