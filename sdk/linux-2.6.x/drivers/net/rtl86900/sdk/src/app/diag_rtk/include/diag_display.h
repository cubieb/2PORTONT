/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 * Purpose : Definition those table or hsba dispaly format.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) hsb/hsa display
 *
 */



#ifndef __DIAG_DISPLAY_H__
#define __DIAG_DISPLAY_H__


static void _diag_debug_hsb_display(rtk_hsb_t *hsbData)
{
    diag_util_printf("spa: %d pktLen: %d ponIdx: %d\n",
                                hsbData->spa,
                                hsbData->pkt_len,
                                hsbData->pon_idx);

    diag_util_printf("da: %s ",
                        diag_util_inet_mactoa(&hsbData->da.octet[0]));
    diag_util_printf("sa: %s etherType: 0x%4.4x\n",
                        diag_util_inet_mactoa(&hsbData->sa.octet[0]),
                        hsbData->ether_type);

    diag_util_printf("ctag: %d pri: %d cfi: %d vid: %d\n",
                        hsbData->ctag_if,
                        (hsbData->ctag&0xE000)>>13,
                        (hsbData->ctag&0x1000)>>12,
                        hsbData->ctag&0xFFF);

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
    		diag_util_printf("stag: %d pri: %d dei: %d vid: %d tpid: %d\n",
                        		hsbData->stag_if,
                        		(hsbData->stag&0xE000)>>13,
                        		(hsbData->stag&0x1000)>>12,
                        		hsbData->stag&0xFFF,
                        		hsbData->stag_tpid);
			break;
#endif
		default:
			diag_util_printf("stag: %d pri: %d dei: %d vid: %d\n",
								hsbData->stag_if,
								(hsbData->stag&0xE000)>>13,
								(hsbData->stag&0x1000)>>12,
								hsbData->stag&0xFFF);
			break;
    }
	
	switch(DIAG_UTIL_CHIP_TYPE)
	{
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:

			diag_util_printf("ip4if: %d dip: %s ",
								hsbData->ip4_if,
								diag_util_inet_ntoa(hsbData->dip));
			diag_util_printf("sip: %s tos_dscp: 0x%2.2x\n",
								diag_util_inet_ntoa(hsbData->sip),
								hsbData->tos_dscp);
			diag_util_printf("ip6if: %d ",hsbData->ip6_if);
			
			diag_util_printf("dip6: %s ",diag_util_inet_n6toa(&hsbData->dip6.ipv6_addr[0]));
			diag_util_printf("sip6: %s ",diag_util_inet_n6toa(&hsbData->sip6.ipv6_addr[0]));
			diag_util_printf("tc: 0x%2.2x\n",hsbData->tc);
			break;
#endif

		default:
			diag_util_printf("dip: %s ",
								diag_util_inet_ntoa(hsbData->dip));
			diag_util_printf("sip: %s iptype: %d tos_dscp: 0x%2.2x\n",
								diag_util_inet_ntoa(hsbData->sip),
								hsbData->ip_type,
								hsbData->tos_dscp);
			break;

	}
	
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
            diag_util_printf("l4ok l3ok gt1 gt5 gre icmp udp tcp\n");
            diag_util_printf("%-4d %-4d %-3d %-3d %-3d %-4d %-3d %-3d\n",
                                hsbData->cks_ok_l4,
                                hsbData->cks_ok_l3,
                                hsbData->ttl_gt1,
                                hsbData->ttl_gt5,
                                hsbData->gre_if,
                                hsbData->icmp_if,
                                hsbData->udp_if,
                                hsbData->tcp_if);

            diag_util_printf("ptp oam rlpp rldp llc snap pppoe session\n");
            diag_util_printf("%-3d %-3d %-4d %-4d %-3d %-4d %-5d 0x%4.4x\n",
                                hsbData->ptp_if,
                                hsbData->oampdu,
                                hsbData->rlpp_if,
                                hsbData->rldp_if,
                                hsbData->llc_other,
                                hsbData->snap_if,
                                hsbData->pppoe_if,
                                hsbData->pppoe_session);

            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
            diag_util_printf("ip6hnrg l4ok l3ok gt1 gt5 gre icmp udp tcp\n");
            diag_util_printf("%-7d %-4d %-4d %-3d %-3d %-3d %-4d %-3d %-3d\n",
                                hsbData->ip6_nh_rg,
                                hsbData->cks_ok_l4,
                                hsbData->cks_ok_l3,
                                hsbData->ttl_gt1,
                                hsbData->ttl_gt5,
                                hsbData->gre_if,
                                hsbData->icmp_if,
                                hsbData->udp_if,
                                hsbData->tcp_if);

            diag_util_printf("ptp oam rlpp rldp llc snap pppoe session\n");
            diag_util_printf("%-3d %-3d %-4d %-4d %-3d %-4d %-5d 0x%4.4x\n",
                                hsbData->ptp_if,
                                hsbData->oampdu,
                                hsbData->rlpp_if,
                                hsbData->rldp_if,
                                hsbData->llc_other,
                                hsbData->snap_if,
                                hsbData->pppoe_if,
                                hsbData->pppoe_session);

            break;
#endif
#ifdef CONFIG_SDK_RTL9601B
        case RTL9601B_CHIP_ID:
            diag_util_printf("igmp udp tcp oam llc snap pppoe\n");
            diag_util_printf("%-4d %-3d %-3d %-3d %-3d %-4d %-5d\n",
                                hsbData->igmp_if,
                                hsbData->udp_if,
                                hsbData->tcp_if,
                                hsbData->oampdu,
                                hsbData->llc_other,
                                hsbData->snap_if,
                                hsbData->pppoe_if);

            break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
			case RTL9602C_CHIP_ID:
				diag_util_printf("nhlen protohn nhrg l4ok l3ok gt1 gt5 l4type\n");
				diag_util_printf("%-5d %-7d %-4d %-4d %-4d %-3d %-3d %d\n",
									hsbData->len_of_nhs,
									hsbData->l3proto_nh,
									hsbData->ip6_nh_rg,
									hsbData->cks_ok_l4,
									hsbData->cks_ok_l3,
									hsbData->ttl_gt1,
									hsbData->ttl_gt5,
									hsbData->l4_type);
	
				diag_util_printf("ptp oam rlpp rldp llc snap pppoe session\n");
				diag_util_printf("%-3d %-3d %-4d %-4d %-3d %-4d %-5d 0x%4.4x\n",
									hsbData->ptp_if,
									hsbData->oampdu,
									hsbData->rlpp_if,
									hsbData->rldp_if,
									hsbData->llc_other,
									hsbData->snap_if,
									hsbData->pppoe_if,
									hsbData->pppoe_session);
	
				break;
#endif

    }


    diag_util_printf("userfield valid: 0x%4.4x\n",hsbData->user_valid);
    diag_util_printf("00-07: 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x\n",
                        hsbData->user_field_0,
                        hsbData->user_field_1,
                        hsbData->user_field_2,
                        hsbData->user_field_3,
                        hsbData->user_field_4,
                        hsbData->user_field_5,
                        hsbData->user_field_6,
                        hsbData->user_field_7);
    diag_util_printf("08-15: 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x 0x%4.4x\n",
                        hsbData->user_field_8,
                        hsbData->user_field_9,
                        hsbData->user_field_10,
                        hsbData->user_field_11,
                        hsbData->user_field_12,
                        hsbData->user_field_13,
                        hsbData->user_field_14,
                        hsbData->user_field_15);

    if(hsbData->cputag_if)
    {
        diag_util_printf("aware cpu-tag fields\n");
        switch(DIAG_UTIL_CHIP_TYPE)
        {
#if defined(CONFIG_SDK_APOLLO)
            case APOLLO_CHIP_ID:
                diag_util_printf("l3c l4c txpmsk efidEn efid priSel pri keep vsel dislrn\n");
                diag_util_printf("%-3d %-3d 0x%2.2x   %-6d %-4d %-6d %-3d %-4d %-4d %d\n",
                                hsbData->cputag_l3c,
                                hsbData->cputag_l4c,
                                hsbData->cputag_txpmsk,
                                hsbData->cputag_efid_en,
                                hsbData->cputag_efid,
                                hsbData->cputag_prisel,
                                hsbData->cputag_pri,
                                hsbData->cputag_keep,
                                hsbData->cputag_vsel,
                                hsbData->cputag_dislrn);

                diag_util_printf("psel extspa pppoe_act pppoe_idx l2br pon_sid dsl_vcmsk\n");
                diag_util_printf("%-4d %-6d %-9d %-9d %-4d %-7d 0x%4.4x\n",
                                hsbData->cputag_psel,
                                hsbData->cputag_extspa,
                                hsbData->cputag_pppoe_act,
                                hsbData->cputag_pppoe_idx,
                                hsbData->cputag_l2br,
                                hsbData->cputag_pon_sid,
                                hsbData->cputag_dsl_vcmsk);
                break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
            case APOLLOMP_CHIP_ID:
                diag_util_printf("l3c l4c txpmsk efidEn efid priSel pri keep vsel dislrn\n");
                diag_util_printf("%-3d %-3d 0x%2.2x   %-6d %-4d %-6d %-3d %-4d %-4d %d\n",
                                hsbData->cputag_l3c,
                                hsbData->cputag_l4c,
                                hsbData->cputag_txpmsk,
                                hsbData->cputag_efid_en,
                                hsbData->cputag_efid,
                                hsbData->cputag_prisel,
                                hsbData->cputag_pri,
                                hsbData->cputag_keep,
                                hsbData->cputag_vsel,
                                hsbData->cputag_dislrn);

                diag_util_printf("psel l34keep extspa pppoe_act pppoe_idx l2br pon_sid\n");
                diag_util_printf("%-4d %-7d %-6d %-9d %-9d %-4d %-7d\n",
                                hsbData->cputag_psel,
                                hsbData->cputag_l34keep,
                                hsbData->cputag_extspa,
                                hsbData->cputag_pppoe_act,
                                hsbData->cputag_pppoe_idx,
                                hsbData->cputag_l2br,
                                hsbData->cputag_pon_sid);
                break;
#endif
#ifdef CONFIG_SDK_RTL9601B
            case RTL9601B_CHIP_ID:
                diag_util_printf("txpmsk priSel pri keep dislrn psel pon_sid\n");
                diag_util_printf("0x%2.2x   %-6d %-3d %-4d %-6d %-4d %-7d\n",
                                hsbData->cputag_txpmsk,
                                hsbData->cputag_prisel,
                                hsbData->cputag_pri,
                                hsbData->cputag_keep,
                                hsbData->cputag_dislrn,
                                hsbData->cputag_psel,
                                hsbData->cputag_pon_sid);
                break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
			case RTL9602C_CHIP_ID:
				diag_util_printf("txpmsk priSel pri keep dislrn sb\n");
				diag_util_printf("0x%2.2x   %-6d %-3d %-4d %-6d %-2d\n",
								hsbData->cputag_txpmsk,
								hsbData->cputag_prisel,
								hsbData->cputag_pri,
								hsbData->cputag_keep,
								hsbData->cputag_dislrn,								
								hsbData->cputag_sb);

				diag_util_printf("psel l34keep extspa pppoe_act pppoe_idx pon_sid\n");
				diag_util_printf("%-4d %-7d %-6d %-9d %-9d %-7d\n",
								hsbData->cputag_psel,
								hsbData->cputag_l34keep,
								hsbData->cputag_extspa,
								hsbData->cputag_pppoe_act,
								hsbData->cputag_pppoe_idx,
								hsbData->cputag_pon_sid);
				
				break;
#endif
        }
    }


    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
			diag_util_printf("--- HSB Parser ---\n");
			diag_util_printf("uni mlt brd l2mlt ipv4mlt ipv6mlt provider wan\n");
			diag_util_printf("%-3d %-3d %-3d %-5d %-7d %-7d %-8d %-3d\n",
							hsbData->par_uni,
							hsbData->par_mlt,
							hsbData->par_brd,
							hsbData->par_l2mlt,
							hsbData->par_ipv4mlt,
							hsbData->par_ipv6mlt,
							hsbData->par_frm_provider,
							hsbData->par_frm_wan);
			diag_util_printf("tcp udp igmp mld l34 rma fld00 sw utpmsk extmsk cpusel\n");
			diag_util_printf("%-3d %-3d %-4d %-3d %-3d %-3d %-5d %-2d 0x%2.2x   0x%2.2x   %-6d\n",
							hsbData->par_tcp_if,
							hsbData->par_udp_if,
							hsbData->par_igmp_if,
							hsbData->par_mld_if,
							hsbData->par_l34pkt,
							hsbData->par_rma_type,
							hsbData->par_fld00_vld,
							hsbData->par_sw_pkt,
							hsbData->par_utp_pmsk,
							hsbData->par_ext_pmsk,
							hsbData->par_cpu_sel);

			diag_util_printf("ip6rvmc ip4rvmc prictag dslite myrldp ponidx mpcp_omci rxpktlen\n");
			diag_util_printf("%-7d %-7d %-7d %-6d %-6d %-6d %-9d %-8d\n",
							hsbData->par_ipv6_resv_mc,
							hsbData->par_ipv4_resv_mc,
							hsbData->par_pri_ctag,
							hsbData->par_dslite_match_idx,
							hsbData->par_myrldp,
							hsbData->par_ponidx,
							hsbData->par_mpcp_omci,
							hsbData->par_rx_pktlen);
			
			break;
#endif
    }
	
}

static void _diag_debug_hsa_display(rtk_hsa_t *hsaData)
{
#if defined(CONFIG_SDK_RTL9602C)
	uint32 tmp_val;
#endif

#if defined(CONFIG_SDK_APOLLO) || defined(CONFIG_SDK_APOLLOMP)

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
            diag_util_printf("Port      CPU  5    4    PON  2    1    0\n");
            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
            diag_util_printf("Port      CPU  5    PON  3    2    1    0\n");
            break;
#endif
    }



    diag_util_printf("user_pri: %-4d %-4d %-4d %-4d %-4d %-4d %-4d\n",
                                (hsaData->rng_nhsam_user_pri>>18)&7,
                                (hsaData->rng_nhsam_user_pri>>15)&7,
                                (hsaData->rng_nhsam_user_pri>>12)&7,
                                (hsaData->rng_nhsam_user_pri>>9)&7,
                                (hsaData->rng_nhsam_user_pri>>6)&7,
                                (hsaData->rng_nhsam_user_pri>>3)&7,
                                hsaData->rng_nhsam_user_pri&7);

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
            diag_util_printf("qid:      %-4d %-4d %-4d %-4d %-4d %-4d %-4d\n",
                                        (hsaData->rng_nhsab_qid>>22)&7,
                                        (hsaData->rng_nhsab_qid>>19)&7,
                                        (hsaData->rng_nhsab_qid>>16)&7,
                                        (hsaData->rng_nhsab_qid>>9)&0x7F,
                                        (hsaData->rng_nhsab_qid>>6)&7,
                                        (hsaData->rng_nhsab_qid>>3)&7,
                                        hsaData->rng_nhsab_qid&7);
            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
            diag_util_printf("qid:      %-4d %-4d %-4d %-4d %-4d %-4d %-4d\n",
                                        (hsaData->rng_nhsab_qid>>22)&7,
                                        (hsaData->rng_nhsab_qid>>19)&7,
                                        (hsaData->rng_nhsab_qid>>12)&0x7F,
                                        (hsaData->rng_nhsab_qid>>9)&7,
                                        (hsaData->rng_nhsab_qid>>6)&7,
                                        (hsaData->rng_nhsab_qid>>3)&7,
                                        hsaData->rng_nhsab_qid&7);
            break;
#endif
    }


    diag_util_printf("dmp:      %-4d %-4d %-4d %-4d %-4d %-4d %-4d\n",
                                (hsaData->rng_nhsab_dpm>>12)&3,
                                (hsaData->rng_nhsab_dpm>>10)&3,
                                (hsaData->rng_nhsab_dpm>>8)&3,
                                (hsaData->rng_nhsab_dpm>>6)&3,
                                (hsaData->rng_nhsab_dpm>>4)&3,
                                (hsaData->rng_nhsab_dpm>>2)&3,
                                hsaData->rng_nhsab_dpm&3);

    diag_util_printf("untagset: %-4d %-4d %-4d %-4d %-4d %-4d %-4d\n",
                                (hsaData->rng_nhsac_untagset>>6)&1,
                                (hsaData->rng_nhsac_untagset>>5)&1,
                                (hsaData->rng_nhsac_untagset>>4)&1,
                                (hsaData->rng_nhsac_untagset>>3)&1,
                                (hsaData->rng_nhsac_untagset>>2)&1,
                                (hsaData->rng_nhsac_untagset>>1)&1,
                                hsaData->rng_nhsac_untagset&1);

    diag_util_printf("spa ctag_act tag_if vid  cfi pri vidzero\n");
    diag_util_printf("%-3d %-8d %-6d %-4d %-3d %-3d %-7d\n",
                                hsaData->rng_nhsab_spa,
                                hsaData->rng_nhsac_ctag_act,
                                hsaData->rng_nhsac_ctag_if,
                                hsaData->rng_nhsac_vid,
                                hsaData->rng_nhsac_cfi,
                                hsaData->rng_nhsac_pri,
                                hsaData->rng_nhsac_vidzero);


    diag_util_printf("stag_type stag_if sp2s svid svidx dei spri pkt_spri vidsel frctag frctag_if\n");
    diag_util_printf("%-9d %-7d %-4d %-4d %-5d %-3d %-4d %-8d %-6d %-6d %-9d\n",
                                hsaData->rng_nhsas_stag_type,
                                hsaData->rng_nhsas_stag_if,
                                hsaData->rng_nhsas_sp2s,
                                hsaData->rng_nhsas_svid,
                                hsaData->rng_nhsas_svidx,
                                hsaData->rng_nhsas_dei,
                                hsaData->rng_nhsas_spri,
                                hsaData->rng_nhsas_pkt_spri,
                                hsaData->rng_nhsas_vidsel,
                                hsaData->rng_nhsas_frctag,
                                hsaData->rng_nhsas_frctag_if);
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
            diag_util_printf("1p_rem 1p_rem_en dscp_rem dscp_rem_en keep ptp ipv4 ipv6 1042 pppoe\n");
            diag_util_printf("%-6d %-9d %-8d %-11d %-4d %-3d %-4d %-4d %-4d %-5d\n",
                                        hsaData->rng_nhsam_1p_rem,
                                        hsaData->rng_nhsam_1p_rem_en,
                                        hsaData->rng_nhsam_dscp_rem,
                                        hsaData->rng_nhsam_dscp_rem_en,
                                        hsaData->rng_nhsaf_keep,
                                        hsaData->rng_nhsaf_ptp,
                                        hsaData->rng_nhsaf_ipv4,
                                        hsaData->rng_nhsaf_ipv6,
                                        hsaData->rng_nhsaf_rfc1042,
                                        hsaData->rng_nhsaf_pppoe_if);

            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
            diag_util_printf("1prem 1premen dscprem dscpremen cpukeep keep ptp tcp udp ipv4 ipv6 1042 pppoe\n");
            diag_util_printf("%-5d %-7d %-7d %-9d %-7d %-4d %-3d %-3d %-3d %-4d %-4d %-4d %-5d\n",
                                        hsaData->rng_nhsam_1p_rem,
                                        hsaData->rng_nhsam_1p_rem_en,
                                        hsaData->rng_nhsam_dscp_rem,
                                        hsaData->rng_nhsam_dscp_rem_en,
                                        hsaData->rng_nhsaf_cpukeep,
                                        hsaData->rng_nhsaf_keep,
                                        hsaData->rng_nhsaf_ptp,
                                        hsaData->rng_nhsaf_tcp,
                                        hsaData->rng_nhsaf_udp,
                                        hsaData->rng_nhsaf_ipv4,
                                        hsaData->rng_nhsaf_ipv6,
                                        hsaData->rng_nhsaf_rfc1042,
                                        hsaData->rng_nhsaf_pppoe_if);

            break;
#endif
    }



    diag_util_printf("endsc  bgdsc  cpupri fwdrsn pon_sid pktlen regen_crc\n");
    diag_util_printf("0x%-4.4x 0x%-4.4x %-6d %-6d %-7d %-6d %-d\n",
                                hsaData->rng_nhsab_endsc,
                                hsaData->rng_nhsab_bgdsc,
                                hsaData->rng_nhsab_cpupri,
                                hsaData->rng_nhsab_fwdrsn,
                                hsaData->rng_nhsab_pon_sid,
                                hsaData->rng_nhsab_pktlen,
                                hsaData->rng_nhsaf_regen_crc);


    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO)
        case APOLLO_CHIP_ID:
            diag_util_printf("vc_spa: %d vc_mask: 0x%4.4x ext_mask: 0x%4.4x\n",
                                        hsaData->rng_nhsab_vc_spa,
                                        hsaData->rng_nhsab_vc_mask,
                                        hsaData->rng_nhsab_ext_mask);

            diag_util_printf("l3: %d org: %d l2trans: %d l34trans: %d src_mode: %d l3chsum: 0x%4.4x l4schsum 0x%4.4x\n",
                                        hsaData->rng_nhsan_l3,
                                        hsaData->rng_nhsan_org,
                                        hsaData->rng_nhsan_l2trans,
                                        hsaData->rng_nhsan_l34trans,
                                        hsaData->rng_nhsan_src_mode,
                                        hsaData->rng_nhsan_l3chsum,
                                        hsaData->rng_nhsan_l4chsum);

            break;
#endif
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
            diag_util_printf("ext_mask l3 org ipmc l2trans l34trans src_mode l3chsum l4schsum\n");
            diag_util_printf("0x%4.4x   %-2d %-3d %-4d %-7d %-8d %-8d 0x%4.4x  0x%4.4x\n",
                                        hsaData->rng_nhsab_ext_mask,
                                        hsaData->rng_nhsan_l3,
                                        hsaData->rng_nhsan_org,
                                        hsaData->rng_nhsan_ipmc,
                                        hsaData->rng_nhsan_l2trans,
                                        hsaData->rng_nhsan_l34trans,
                                        hsaData->rng_nhsan_src_mode,
                                        hsaData->rng_nhsan_l3chsum,
                                        hsaData->rng_nhsan_l4chsum);

            break;
#endif
    }


    diag_util_printf("pppoe_idx: %d pppoe_act: %d ttl_extmask: 0x%2.2x ttl_pmask: 0x%2.2x\n",
                                hsaData->rng_nhsan_pppoe_idx,
                                hsaData->rng_nhsan_pppoe_act,
                                hsaData->rng_nhsan_ttlm1_extmask,
                                hsaData->rng_nhsan_ttlm1_pmask);

    diag_util_printf("newmac: %s smac_idx: %d ",
                        diag_util_inet_mactoa(&hsaData->rng_nhsan_newmac.octet[0]),
                        hsaData->rng_nhsan_smac_idx);
    diag_util_printf("newip: %s newport: 0x%4.4x\n",
                        diag_util_inet_ntoa(hsaData->rng_nhsan_newip),
                        hsaData->rng_nhsan_newport);

    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLOMP)
        case APOLLOMP_CHIP_ID:
            diag_util_printf("floodpkt: %d ctag_ponac: %d ponvid: %d ponpri: %d ponsvid: %d ponspir: %d\n",
                                        hsaData->rng_nhsab_floodpkt,
                                        hsaData->rng_nhsac_ctag_ponact,
                                        hsaData->rng_nhsac_ponvid,
                                        hsaData->rng_nhsac_ponpri,
                                        hsaData->rng_nhsas_ponsvid,
                                        hsaData->rng_nhsas_ponspri);

            diag_util_printf("Port        CPU  5    PON  3    2    1    0\n");
    		diag_util_printf("un-Stagset: %-4d %-4d %-4d %-4d %-4d %-4d %-4d\n",
                                (hsaData->rng_nhsas_untagset>>6)&1,
                                (hsaData->rng_nhsas_untagset>>5)&1,
                                (hsaData->rng_nhsas_untagset>>4)&1,
                                (hsaData->rng_nhsas_untagset>>3)&1,
                                (hsaData->rng_nhsas_untagset>>2)&1,
                                (hsaData->rng_nhsas_untagset>>1)&1,
                                hsaData->rng_nhsas_untagset&1);

            break;
#endif
    }
#endif

	switch(DIAG_UTIL_CHIP_TYPE)
	{	
	
#if defined(CONFIG_SDK_RTL9601B)
		case RTL9601B_CHIP_ID:
		    diag_util_printf("Port      CPU  PON  0\n");
		    diag_util_printf("user_pri: %-4d %-4d %-4d\n",
		                                (hsaData->rng_nhsam_user_pri>>6)&7,
		                                (hsaData->rng_nhsam_user_pri>>3)&7,
		                                hsaData->rng_nhsam_user_pri&7);

		    diag_util_printf("qid:      %-4d %-4d %-4d\n",
		                                (hsaData->rng_nhsab_qid>>6)&7,
		                                (hsaData->rng_nhsab_qid>>3)&7,
		                                hsaData->rng_nhsab_qid&7);


		    diag_util_printf("dmp:      %-4d %-4d %-4d\n",
		                                (hsaData->rng_nhsab_dpm>>2)&1,
		                                (hsaData->rng_nhsab_dpm>>1)&1,
		                                hsaData->rng_nhsab_dpm&1);

		    diag_util_printf("untagset: %-4d %-4d %-4d\n",
		                                (hsaData->rng_nhsac_untagset>>2)&1,
		                                (hsaData->rng_nhsac_untagset>>1)&1,
		                                hsaData->rng_nhsac_untagset&1);
		    diag_util_printf("s-untag:  %-4d %-4d %-4d\n",
		                                (hsaData->rng_nhsas_untagset>>2)&1,
		                                (hsaData->rng_nhsas_untagset>>1)&1,
		                                hsaData->rng_nhsas_untagset&1);

		    diag_util_printf("orgcvid orgcpri orgcfi ctagponact ctagact vidzero\n");
		    diag_util_printf("%-7d %-7d %-6d %-10d %-7d %-7d\n",
		                                hsaData->rng_nhsac_org_cvid,
		                                hsaData->rng_nhsac_org_cpri,
		                                hsaData->rng_nhsac_org_cfi,
		                                hsaData->rng_nhsac_ctag_ponact,
		                                hsaData->rng_nhsac_ctag_act,
		                                hsaData->rng_nhsac_vidzero);
		    
		    diag_util_printf("pritagif ctagif ponvid vid ponpri pri cactnop cacttag\n");
		    diag_util_printf("%-8d %-6d %-6d %-3d %-6d %-3d %-7d %-7d\n",
		                                hsaData->rng_nhsac_pritag_if,
		                                hsaData->rng_nhsac_ctag_if,
		                                hsaData->rng_nhsac_ponvid,
		                                hsaData->rng_nhsac_vid,
		                                hsaData->rng_nhsac_ponpri,
		                                hsaData->rng_nhsac_pri,
		                                hsaData->rng_nhsac_cact_nop,
		                                hsaData->rng_nhsac_cact_tag);

		    diag_util_printf("mdysvid mdysvidpon stagif dei stagtype frctag frctagif\n");
		    diag_util_printf("%-7d %-10d %-6d %-3d %-8d %-6d %-8d\n",
		                                hsaData->rng_nhsas_mdy_svid,
		                                hsaData->rng_nhsas_mdy_svid_pon,
		                                hsaData->rng_nhsas_stag_if,
		                                hsaData->rng_nhsas_dei,
		                                hsaData->rng_nhsas_stag_type,
		                                hsaData->rng_nhsas_frctag,
		                                hsaData->rng_nhsas_frctag_if);
		    diag_util_printf("ponsvid svid pktspri ponspri spri\n");
		    diag_util_printf("%-7d %-4d %-7d %-7d %-4d\n",
		                                hsaData->rng_nhsas_ponsvid,
		                                hsaData->rng_nhsas_svid,
		                                hsaData->rng_nhsas_pkt_spri,
		                                hsaData->rng_nhsas_ponspri,
		                                hsaData->rng_nhsas_spri);
		    
		    diag_util_printf("cputagif 1prempon 1premenpon dscprempon dscpremenpon\n");
		    diag_util_printf("%-8d %-8d %-10d %-10d %-12d\n",
		                                hsaData->rng_nhsam_cputag_if,
		                                hsaData->rng_nhsam_1p_rem_pon,
		                                hsaData->rng_nhsam_1p_rem_en_pon,
		                                hsaData->rng_nhsam_dscp_rem_pon,
		                                hsaData->rng_nhsam_dscp_rem_en_pon);
		    diag_util_printf("1prem 1premen dscprem dscpremen\n");
		    diag_util_printf("%-5d %-7d %-7d %-9d\n",
		                                hsaData->rng_nhsam_1p_rem,
		                                hsaData->rng_nhsam_1p_rem_en,
		                                hsaData->rng_nhsam_dscp_rem,
		                                hsaData->rng_nhsam_dscp_rem_en);

            diag_util_printf("regencrc cpukeep keep tcp udp ipv4 ipv6 1042 pppoe ipmc\n");
            diag_util_printf("%-8d %-7d %-4d %-3d %-3d %-4d %-4d %-4d %-5d %-4d\n",
                                        hsaData->rng_nhsaf_regen_crc,
                                        hsaData->rng_nhsaf_cpukeep,
                                        hsaData->rng_nhsaf_keep,
                                        hsaData->rng_nhsaf_tcp,
                                        hsaData->rng_nhsaf_udp,
                                        hsaData->rng_nhsaf_ipv4,
                                        hsaData->rng_nhsaf_ipv6,
                                        hsaData->rng_nhsaf_rfc1042,
                                        hsaData->rng_nhsaf_pppoe_if,
                                        hsaData->rng_nhsab_ipmc);

            diag_util_printf("endsc  bgdsc  floodpkt cpupri fwdrsn pon_sid spa pktlen omcipktlen\n");
            diag_util_printf("0x%-4.4x 0x%-4.4x %-8d %-6d %-6d %-7d %-3d %-6d %-d\n",
                                        hsaData->rng_nhsab_endsc,
                                        hsaData->rng_nhsab_bgdsc,
                                        hsaData->rng_nhsab_floodpkt,
                                        hsaData->rng_nhsab_cpupri,
                                        hsaData->rng_nhsab_fwdrsn,
                                        hsaData->rng_nhsab_pon_sid,
                                        hsaData->rng_nhsab_spa,
                                        hsaData->rng_nhsab_pktlen,
                                        hsaData->rng_nhsab_omci_pktlen);

			break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
		case RTL9602C_CHIP_ID:
			diag_util_printf("Port      CPU PON 1   0\n");
	
	
	
			diag_util_printf("qid:      %-3d %-3d %-3d %-3d\n",
										(hsaData->rng_nhsab_qid>>9)&7,
										(hsaData->rng_nhsab_qid>>6)&7,
										(hsaData->rng_nhsab_qid>>3)&7,
										hsaData->rng_nhsab_qid&7);

	
			diag_util_printf("dmp:      %-3d %-3d %-3d %-3d\n",
										(hsaData->rng_nhsab_dpm>>6)&3,
										(hsaData->rng_nhsab_dpm>>4)&3,
										(hsaData->rng_nhsab_dpm>>2)&3,
										hsaData->rng_nhsab_dpm&3);
	
			diag_util_printf("untagset: %-3d %-3d %-3d %-3d\n",
										(hsaData->rng_nhsac_untagset>>3)&1,
										(hsaData->rng_nhsac_untagset>>2)&1,
										(hsaData->rng_nhsac_untagset>>1)&1,
										hsaData->rng_nhsac_untagset&1);
			diag_util_printf("unStag:   %-3d %-3d %-3d %-3d\n",
										(hsaData->rng_nhsas_untagset>>3)&1,
										(hsaData->rng_nhsas_untagset>>2)&1,
										(hsaData->rng_nhsas_untagset>>1)&1,
										hsaData->rng_nhsas_untagset&1);
		
			diag_util_printf("spa dmaspa flood cpupri ipmc issb pktlen ponsid fwdrsn intpri extmsk\n");
			diag_util_printf("%-3d %-6d %-5d %-6d %-4d %-4d %-6d %-6d %-6d %-6d 0x%2.2x\n",
										hsaData->rng_nhsab_spa,
										hsaData->rng_nhsab_dma_spa,
										hsaData->rng_nhsab_floodpkt,
										hsaData->rng_nhsab_cpupri,
										hsaData->rng_nhsab_ipmc,
										hsaData->rng_nhsab_issb,
										hsaData->rng_nhsab_pktlen,
										hsaData->rng_nhsab_pon_sid,
										hsaData->rng_nhsab_fwdrsn,
										hsaData->rng_nhsam_intpri,
										hsaData->rng_nhsab_ext_mask);
	
	
			diag_util_printf("ctagif act pri vid remen ponact ponpri ponvid remponen tagpri tagcfi tagvid\n");
			diag_util_printf("%-6d %-3d %-3d %-3d %-5d %-6d %-6d %-6d %-8d %-6d %-6d %-6d\n",
										hsaData->rng_nhsac_ctag_if,
										hsaData->rng_nhsac_ctag_act,
										hsaData->rng_nhsac_pri,
										hsaData->rng_nhsac_vid,
										hsaData->rng_nhsac_1p_rem_en,
										hsaData->rng_nhsac_ctag_ponact,
										hsaData->rng_nhsac_ponpri,
										hsaData->rng_nhsac_ponvid,
										hsaData->rng_nhsac_1p_rem_pon_en,
										hsaData->rng_nhsac_tagpri,
										hsaData->rng_nhsac_tagcfi,
										hsaData->rng_nhsac_tagvid);
	
	
			diag_util_printf("stagif tpid pri vid ponact ponpri ponvid sp2cvid sp2cact tagpri tagdei tagvid\n");
			diag_util_printf("%-6d %-4d %-3d %-3d %-6d %-6d %-6d %-7d %-7d %-6d %-6d %-6d\n",
										hsaData->rng_nhsas_stag_if,
										hsaData->rng_nhsas_tpid_type,
										hsaData->rng_nhsas_spri,
										hsaData->rng_nhsas_svid,
										hsaData->rng_nhsas_ponact,
										hsaData->rng_nhsas_ponspri,
										hsaData->rng_nhsas_ponsvid,
										hsaData->rng_nhsas_sp2cvid,
										hsaData->rng_nhsas_sp2cact,
										hsaData->rng_nhsas_tagpri,
										hsaData->rng_nhsas_tagdei,
										hsaData->rng_nhsas_tagvid);
			diag_util_printf("dscprem dscpremen dscprempon dscpremponen\n");
			diag_util_printf("%-7d %-9d %-10d %-11d\n",
										hsaData->rng_nhsam_dscp_rem,
										hsaData->rng_nhsam_dscp_rem_en,
										hsaData->rng_nhsam_dscp_rem_pon,
										hsaData->rng_nhsam_dscp_rem_en_pon);
			diag_util_printf("crc cpukeep keep ptp tcp udp ipv4 ipv6 1042 pppoe org l3r endsc  bgdsc\n");
			diag_util_printf("%-3d %-7d %-4d %-3d %-3d %-3d %-4d %-4d %-4d %-5d %-3d %-3d 0x%-4.4x 0x%-4.4x\n",
										hsaData->rng_nhsaf_regen_crc,
										hsaData->rng_nhsaf_cpukeep,
										hsaData->rng_nhsaf_keep,
										hsaData->rng_nhsaf_ptp,
										hsaData->rng_nhsaf_tcp,
										hsaData->rng_nhsaf_udp,
										hsaData->rng_nhsaf_ipv4,
										hsaData->rng_nhsaf_ipv6,
										hsaData->rng_nhsaf_rfc1042,
										hsaData->rng_nhsaf_pppoe_if,
										hsaData->rng_nhsab_org,
										hsaData->rng_nhsab_l3r,
										hsaData->rng_nhsab_endsc,
								        hsaData->rng_nhsab_bgdsc);

			if(hsaData->rng_nhsaf_ptp)
			{
				diag_util_printf("ptpid:%d act:%d sec:0x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x nsec:0x%8.8x\n",
											hsaData->rng_nhsap_ptp_id,
											hsaData->rng_nhsap_ptp_act,
											hsaData->rng_nhsap_ptp_sec[5],
											hsaData->rng_nhsap_ptp_sec[4],
											hsaData->rng_nhsap_ptp_sec[3],
											hsaData->rng_nhsap_ptp_sec[2],
											hsaData->rng_nhsap_ptp_sec[1],
											hsaData->rng_nhsap_ptp_sec[0],
											hsaData->rng_nhsap_ptp_nsec);			
			}
			else
			{

				diag_util_printf("dsact dsidx wansa l2tr l3tr srcmod l3chsum l4chsum\n");
				diag_util_printf("%-5d %-5d %-5d %-4d %-4d %-6d 0x%4.4x  0x%4.4x\n",
											hsaData->rng_nhsan_dslite_act,
											hsaData->rng_nhsan_dslite_idx,
											hsaData->rng_nhsan_wansa,
											hsaData->rng_nhsan_l2trans,
											hsaData->rng_nhsan_l34trans,
											hsaData->rng_nhsan_src_mode,
											hsaData->rng_nhsan_l3chsum,
											hsaData->rng_nhsan_l4chsum);
		
				diag_util_printf("pppoe_idx: %d pppoe_act: %d ttl_pmask: 0x%2.2x\n",
											hsaData->rng_nhsan_pppoe_idx,
											hsaData->rng_nhsan_pppoe_act,
											hsaData->rng_nhsan_ttlm1_pmask);
			
				diag_util_printf("newmac: %s smac_idx: %d ",
									diag_util_inet_mactoa(&hsaData->rng_nhsan_newmac.octet[0]),
									hsaData->rng_nhsan_smac_idx);
				diag_util_printf("newip: %s newport: 0x%4.4x\n",
									diag_util_inet_ntoa(hsaData->rng_nhsan_newip),
									hsaData->rng_nhsan_newport);

				diag_util_printf("omci msgtype:%d contlen:%d\n",
											hsaData->rng_nhsan_omci_msg_type,
											hsaData->rng_nhsan_omci_cont_len);		

				
				reg_field_read(RTL9602C_MODEL_NAME_INFOr, RTL9602C_MODEL_CHAR_1STf,&tmp_val);
				if( 2 != tmp_val)
				{
					diag_util_printf("hostmib_en:%d hostinf:%d l3mib_en:%d l3inf:%d\n",
												hsaData->rng_nhsab_hostmibEn,
												hsaData->rng_nhsab_hostinf,
												hsaData->rng_nhsab_l34mibEn,
												hsaData->rng_nhsab_l34inf);		

				}
			
			}
			
			break;
#endif
	}

}

static void _diag_debug_hsd_display(rtk_hsa_debug_t *hsdData)
{
    switch(DIAG_UTIL_CHIP_TYPE)
    {
#if defined(CONFIG_SDK_APOLLO) || defined(CONFIG_SDK_APOLLOMP)
		case APOLLO_CHIP_ID:
		case APOLLOMP_CHIP_ID:
		    diag_util_printf("newmac: %s ",
		                        diag_util_inet_mactoa(&hsdData->hsa_debug_newmac.octet[0]));

		    diag_util_printf("l34mac: %s ",
		                        diag_util_inet_mactoa(&hsdData->hsa_debug_34smac.octet[0]));

		    diag_util_printf("newip: %s newprt: %d\n",
		                        diag_util_inet_ntoa(hsdData->hsa_debug_newip),hsdData->hsa_debug_newprt);

		    diag_util_printf("ep dsl_vc 34pppoe ttlpmsk ttlexmsk l4cksum l3cksum pppoeact\n");
		    diag_util_printf("%-2d %-6d %-7d 0x%-5.5x 0x%-6.6x %-7d %-7d %-8d\n",
		                        hsdData->hsa_debug_ep,
		                        hsdData->hsa_debug_dsl_vc,
		                        hsdData->hsa_debug_34pppoe,
		                        hsdData->hsa_debug_ttlpmsk,
		                        hsdData->hsa_debug_ttlexmsk,
		                        hsdData->hsa_debug_l4cksum,
		                        hsdData->hsa_debug_l3cksum,
		                        hsdData->hsa_debug_pppoeact);

		    diag_util_printf("src_mod l34trans l2trans org l3r sv_dei styp pktlen_ori qid\n");
		    diag_util_printf("%-7d %-8d %-7d %-3d %-3d %-6d %-4d %-10d %-3d\n",
		                        hsdData->hsa_debug_src_mod,
		                        hsdData->hsa_debug_l34trans,
		                        hsdData->hsa_debug_l2trans,
		                        hsdData->hsa_debug_org,
		                        hsdData->hsa_debug_l3r,
		                        hsdData->hsa_debug_sv_dei,
		                        hsdData->hsa_debug_styp,
		                        hsdData->hsa_debug_pktlen_ori,
		                        hsdData->hsa_debug_qid);
		    diag_util_printf("stdsc cpupri spri cori cmdy crms cins cvid cfi regencrc pppoe\n");
		    diag_util_printf("%-5d %-6d %-4d %-4d %-4d %-4d %-4d %-4d %-3d %-8d %-5d\n",
		                        hsdData->hsa_debug_stdsc,
		                        hsdData->hsa_debug_cpupri,
		                        hsdData->hsa_debug_spri,
		                        hsdData->hsa_debug_cori,
		                        hsdData->hsa_debug_cmdy,
		                        hsdData->hsa_debug_crms,
		                        hsdData->hsa_debug_cins,
		                        hsdData->hsa_debug_cvid,
		                        hsdData->hsa_debug_cfi,
		                        hsdData->hsa_debug_regencrc,
		                        hsdData->hsa_debug_pppoe);
		    diag_util_printf("rfc1042 ipv6 ipv4 ptp remdscp_pri rem1q_pri remdscp_en rem1q_en\n");
		    diag_util_printf("%-7d %-4d %-4d %-3d %-11d %-9d %-10d %-8d\n",
		                        hsdData->hsa_debug_rfc1042,
		                        hsdData->hsa_debug_ipv6,
		                        hsdData->hsa_debug_ipv4,
		                        hsdData->hsa_debug_ptp,
		                        hsdData->hsa_debug_remdscp_pri,
		                        hsdData->hsa_debug_rem1q_pri,
		                        hsdData->hsa_debug_remdscp_en,
		                        hsdData->hsa_debug_rem1q_en);
		    diag_util_printf("svid instag inctag pktlen spa dpc extmsk vcmsk ponsid trprsn\n");
		    diag_util_printf("%-4d %-6d %-6d %-6d %-3d %-3d 0x%-4.4x 0x%-3.3x %-6d %-6d\n",
		                        hsdData->hsa_debug_svid,
		                        hsdData->hsa_debug_instag,
		                        hsdData->hsa_debug_inctag,
		                        hsdData->hsa_debug_pktlen,
		                        hsdData->hsa_debug_spa,
		                        hsdData->hsa_debug_dpc,
		                        hsdData->hsa_debug_extmsk,
		                        hsdData->hsa_debug_vcmsk,
		                        hsdData->hsa_debug_ponsid,
		                        hsdData->hsa_debug_trprsn);
			break;
#endif

#if defined(CONFIG_SDK_RTL9601B)
		
		case RTL9601B_CHIP_ID:
		    diag_util_printf("pktchange tcp udp padg svdei styp pktlenori qid\n");
		    diag_util_printf("%-9d %-3d %-3d %-4d %-5d %-4d %-9d %-3d\n",
		                        hsdData->hsa_debug_pkt_change,
		                        hsdData->hsa_debug_tcp,
		                        hsdData->hsa_debug_udp,
		                        hsdData->hsa_debug_padg,
		                        hsdData->hsa_debug_sv_dei,
		                        hsdData->hsa_debug_styp,
		                        hsdData->hsa_debug_pktlen_ori,
		                        hsdData->hsa_debug_qid);

		    diag_util_printf("stdsc cpupri spri txins cvid cfi regencrc pppoe\n");
		    diag_util_printf("%-5d %-6d %-4d %-5d %-4d %-3d %-8d %-5d\n",
		                        hsdData->hsa_debug_stdsc,
		                        hsdData->hsa_debug_cpupri,
		                        hsdData->hsa_debug_spri,
		                        hsdData->hsa_debug_txins,
		                        hsdData->hsa_debug_cvid,
		                        hsdData->hsa_debug_cfi,
		                        hsdData->hsa_debug_regencrc,
		                        hsdData->hsa_debug_pppoe);


		    diag_util_printf("rfc1042 ipv6 ipv4 remdscppri ctagpri remdscpen\n");
		    diag_util_printf("%-7d %-4d %-4d %-10d %-7d %-9d\n",
		                        hsdData->hsa_debug_rfc1042,
		                        hsdData->hsa_debug_ipv6,
		                        hsdData->hsa_debug_ipv4,
		                        hsdData->hsa_debug_remdscp_pri,
		                        hsdData->hsa_debug_ctag_pri,
		                        hsdData->hsa_debug_remdscp_en);


		    diag_util_printf("svid instag pktlen spa dpc ponsid trprsn\n");
		    diag_util_printf("%-4d %-6d %-6d %-3d %-3d %-6d %-6d\n",
		                        hsdData->hsa_debug_svid,
		                        hsdData->hsa_debug_instag,
		                        hsdData->hsa_debug_pktlen,
		                        hsdData->hsa_debug_spa,
		                        hsdData->hsa_debug_dpc,
		                        hsdData->hsa_debug_ponsid,
		                        hsdData->hsa_debug_trprsn);
			break;
#endif
#if defined(CONFIG_SDK_RTL9602C)
			case RTL9602C_CHIP_ID:
				diag_util_printf("newdmac: %s\n",
									diag_util_inet_mactoa(&hsdData->hsa_debug_newdmac.octet[0]));
				diag_util_printf("newsmac: %s\n",
									diag_util_inet_mactoa(&hsdData->hsa_debug_newsmac.octet[0]));

				diag_util_printf("newip: %s newprt: %d\n",
									diag_util_inet_ntoa(hsdData->hsa_debug_newip),hsdData->hsa_debug_newprt);
				diag_util_printf("ip6 ip4 tcp udp pppoe l34pppoe pppoeact l2trans l34trans l3r\n");
				diag_util_printf("%-3d %-3d %-3d %-3d %-5d %-8d %-8d %-7d %-8d %-3d\n",
									hsdData->hsa_debug_ipv6,
									hsdData->hsa_debug_ipv4,
									hsdData->hsa_debug_tcp,
									hsdData->hsa_debug_udp,
									hsdData->hsa_debug_pppoe,
									hsdData->hsa_debug_l34pppoe,
									hsdData->hsa_debug_pppoe_act,
									hsdData->hsa_debug_l2trans,
									hsdData->hsa_debug_l34trans,
									hsdData->hsa_debug_l3r);
				diag_util_printf("org l3chsum l4chsum extmsk ttlexmsk ttlpmsk dsliteact dsliteidx\n");
				diag_util_printf("%-3d 0x%4.4x  0x%4.4x  0x%2.2x   0x%2.2x     0x%2.2x    %-9d %-9d\n",
									hsdData->hsa_debug_org,
									hsdData->hsa_debug_l3cksum,
									hsdData->hsa_debug_l4cksum,
									hsdData->hsa_debug_extmsk,
									hsdData->hsa_debug_ttlexmsk,
									hsdData->hsa_debug_ttlpmsk,
									hsdData->hsa_debug_dslite_act,
									hsdData->hsa_debug_dslite_idx);


				diag_util_printf("spa dmaspa dpc issb cpuins cpupri qid ponsid trprsn egrport\n");
				diag_util_printf("%-3d %-6d %-3d %-4d %-6d %-6d %-3d %-6d %-6d %-7d\n",
									hsdData->hsa_debug_spa,
									hsdData->hsa_debug_dmaspa,
									hsdData->hsa_debug_dpc,
									hsdData->hsa_debug_issb,
									hsdData->hsa_debug_cpuins,
									hsdData->hsa_debug_cpupri,
									hsdData->hsa_debug_qid,
									hsdData->hsa_debug_ponsid,
									hsdData->hsa_debug_trprsn,
									hsdData->hsa_debug_egr_port);
	
				diag_util_printf("lendma lenpla sins styp stag   cins ctag   dscprmen dscprmpri crc\n");
				diag_util_printf("%-6d %-6d %-4d %-4d 0x%-4.4x %-4d 0x%4.4x %-8d %-9d %-3d\n",
									hsdData->hsa_debug_pktlen_dma,
									hsdData->hsa_debug_pktlen_pla,
									hsdData->hsa_debug_sins,
									hsdData->hsa_debug_styp,
									hsdData->hsa_debug_stag,
									hsdData->hsa_debug_cins,
									hsdData->hsa_debug_ctag,
									hsdData->hsa_debug_dscp_rem_en,
									hsdData->hsa_debug_dscp_rem_pri,
									hsdData->hsa_debug_regencrc);
				
				diag_util_printf("1042 srcmod stdsc txpad ptp ptpact ptpid ptpsec       ptpnsec\n");
				diag_util_printf("%-4d %-6d 0x%3.3x %-5d %-3d %-6d %-5d %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x %-8.8x\n",
									hsdData->hsa_debug_rfc1042,
									hsdData->hsa_debug_src_mod,
									hsdData->hsa_debug_stdsc,
									hsdData->hsa_debug_txpad,
									hsdData->hsa_debug_ptp,
									hsdData->hsa_debug_ptpact,
									hsdData->hsa_debug_ptpid,
									hsdData->hsa_debug_ptpsec[5],
									hsdData->hsa_debug_ptpsec[4],
									hsdData->hsa_debug_ptpsec[3],
									hsdData->hsa_debug_ptpsec[2],
									hsdData->hsa_debug_ptpsec[1],
									hsdData->hsa_debug_ptpsec[0],
									hsdData->hsa_debug_ptpnsec);
				break;
#endif
	


    }
}


#endif /* end of __DIAG_DISPLAY_H__ */
