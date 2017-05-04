#include <linux/skbuff.h>
#include "8190n_cfg.h"
#include "8190n.h"

#ifndef	_RTL80211_H_
#define _RTL80211_H_

#define DEFAULT_BINTVAL 100

#ifdef NEW_MAC80211_DRV
#define WME_NUM_TID     16
#define WME_NUM_AC      4

#define ADDBA_EXCHANGE_ATTEMPTS 10

#define WME_BA_BMP_SIZE 64
#define WME_MAX_BA      WME_BA_BMP_SIZE
#define RTL_TID_MAX_BUFS        (2 * WME_MAX_BA)

#define AGGR_CLEANUP            BIT(1)
#define AGGR_ADDBA_COMPLETE     BIT(2)
#define AGGR_ADDBA_PROGRESS     BIT(3)

#define RTL_ND_2_TID(_nd, _tidno)  (&(_nd)->tid[(_tidno)])

#define IEEE80211_SEQ_SEQ_SHIFT    4
#define IEEE80211_SEQ_MAX          4096

#define INCR(_l, _sz)   do {                    \
                (_l)++;                         \
                (_l) &= ((_sz) - 1);            \
        } while (0)

#endif

enum side_band_def {
	SIDE_BAND_DONOT_CARE = 0,
	SIDE_BAND_UPPER = 1,
	SIDE_BAND_LOWER = 2,
	SIDE_BAND_MASK = 3	
};

enum rts_def {
	RTS_DISABLE = 0,
	RTS_ENABLE = 1,
	RTS_SHORT_PREAMBLE = 2
};

enum tx_rate_def {
	CCK_1M = 0,
	CCK_2M = 1,
	CCK_5M = 2,
	CCK_11M = 3,
	
	OFDM_6M = 4,
	OFDM_9M = 5,
	OFDM_12M = 6,
	OFDM_18M = 7,
	OFDM_24M = 8,	
	OFDM_36M = 9,
	OFDM_48M = 10,
	OFDM_54M = 11,

	MCS0 = 12,
	MCS1 = 13,
	MCS2 = 14,
	MCS3 = 15,
	MCS4 = 16,
	MCS5 = 17,
	MCS6 = 18,
	MCS7 = 19,
	MCS8 = 20,
	MCS9 = 21,
	MCS10 = 22,
	MCS11 = 23,
	MCS12 = 24,
	MCS13 = 25,
	MCS14 = 26,
	MCS15 = 27,
};


#define RATE_MAX 30

struct rtl_rate_table {
	int rate_cnt;
	struct ieee80211_rate rates[30];
};

static const struct rtl_rate_table rtl819x_11g_ratetbl = {
	12,
	{
		{ .bitrate = 10, .hw_value = 0, },
		{ .bitrate = 20, .hw_value = 1, },
		{ .bitrate = 55, .hw_value = 2, },
		{ .bitrate = 110, .hw_value = 3, },
		{ .bitrate = 60, .hw_value = 4, },
		{ .bitrate = 90, .hw_value = 5, },
		{ .bitrate = 120, .hw_value = 6, },
		{ .bitrate = 180, .hw_value = 7, },
		{ .bitrate = 240, .hw_value = 8, },
		{ .bitrate = 360, .hw_value = 9, },
		{ .bitrate = 480, .hw_value = 10, },
		{ .bitrate = 540, .hw_value = 11, },
	}
};

static const struct rtl_rate_table rtl819x_11gn_ratetbl = {
	39,
	{
		{ .bitrate = 10, .hw_value = 0, },
		{ .bitrate = 20, .hw_value = 1, },
		{ .bitrate = 55, .hw_value = 2, },
		{ .bitrate = 110, .hw_value = 3, },
		{ .bitrate = 60, .hw_value = 4, },
		{ .bitrate = 90, .hw_value = 5, },
		{ .bitrate = 120, .hw_value = 6, },
		{ .bitrate = 180, .hw_value = 7, },
		{ .bitrate = 240, .hw_value = 8, },
		{ .bitrate = 360, .hw_value = 9, },
		{ .bitrate = 480, .hw_value = 10, },
		{ .bitrate = 540, .hw_value = 11, },
		{ .bitrate = 65, .hw_value = 12, },
		{ .bitrate = 130, .hw_value = 13, },
		{ .bitrate = 195, .hw_value = 14, },
		{ .bitrate = 260, .hw_value = 15, },
		{ .bitrate = 390, .hw_value = 16, },
		{ .bitrate = 520, .hw_value = 17, },
		{ .bitrate = 585, .hw_value = 18, },
		{ .bitrate = 650, .hw_value = 19, },
		{ .bitrate = 130, .hw_value = 20, },
		{ .bitrate = 260, .hw_value = 21, },
		{ .bitrate = 390, .hw_value = 22, },
		{ .bitrate = 520, .hw_value = 23, },
		{ .bitrate = 780, .hw_value = 24, },
		{ .bitrate = 1040, .hw_value = 25, },
		{ .bitrate = 1170, .hw_value = 26, },
		{ .bitrate = 1300, .hw_value = 27, },
		{ .bitrate = 135, .hw_value = 28, },
		{ .bitrate = 150, .hw_value = 29, },
		{ .bitrate = 270, .hw_value = 30, },
		{ .bitrate = 540, .hw_value = 31, },
		{ .bitrate = 810, .hw_value = 32, },
		{ .bitrate = 1080, .hw_value = 33, },
		{ .bitrate = 1620, .hw_value = 34, },
		{ .bitrate = 2160, .hw_value = 35, },
		{ .bitrate = 2430, .hw_value = 36, },
		{ .bitrate = 2700, .hw_value = 37, },
		{ .bitrate = 3000, .hw_value = 38, },
	}
};


/*
#define N_BITRATES 12

static const struct ieee80211_rate rtl818x_rates[] = {
	{ .bitrate = 10, .hw_value = 0, },
	{ .bitrate = 20, .hw_value = 1, },
	{ .bitrate = 55, .hw_value = 2, },
	{ .bitrate = 110, .hw_value = 3, },
	{ .bitrate = 60, .hw_value = 4, },
	{ .bitrate = 90, .hw_value = 5, },
	{ .bitrate = 120, .hw_value = 6, },
	{ .bitrate = 180, .hw_value = 7, },
	{ .bitrate = 240, .hw_value = 8, },
	{ .bitrate = 360, .hw_value = 9, },
	{ .bitrate = 480, .hw_value = 10, },
	{ .bitrate = 540, .hw_value = 11, },
};
*/

static const struct ieee80211_channel rtl819x_channels[] = {
	{ .center_freq = 2412 },
	{ .center_freq = 2417 },
	{ .center_freq = 2422 },
	{ .center_freq = 2427 },
	{ .center_freq = 2432 },
	{ .center_freq = 2437 },
	{ .center_freq = 2442 },
	{ .center_freq = 2447 },
	{ .center_freq = 2452 },
	{ .center_freq = 2457 },
	{ .center_freq = 2462 },
	{ .center_freq = 2467 },
	{ .center_freq = 2472 },
	{ .center_freq = 2484 },
};

enum more_frag_def {
	NONE_FRAG = 0,
	IS_FRAG = 1,
	IS_FIRST_FRAG = 2,
	IS_LAST_FRAG = 4
};

enum aggre_def {
	AGGRE_NONE = 0,
	AGGRE_AMPDU = 1,
	AGGRE_AMSDU = 2
};

#ifdef NEW_MAC80211_DRV
enum tx_queue {
	MGNT_QUEUE = 0,
	BK_QUEUE = 1,
	BE_QUEUE = 2,
	VI_QUEUE = 3,
	VO_QUEUE = 4,
	HIGH_QUEUE = 5,
	BEACON_QUEUE = 6
};
#endif


struct tx_buffer {
	unsigned char *data;			/* data buffer */
	unsigned int len;				/* data len */
	unsigned int type;				/* buffer type, referred when free */
	struct sk_buff *skb;			/* skb ptr, referred when free */
};

#define MAX_BUF_NO		3

struct tx_info {
	unsigned int q_num;			/* hw queue number, enum tx_queue */
	unsigned int priority;			/* data priority, referred when q_num != MGNT_QUEUE */

	int buf_num;					/* number of tx_buffer carried */
	struct tx_buffer buf[MAX_BUF_NO];	/* tx buffer array */
		
	unsigned int is_40m_bw;		/* 0: 20M, 1: 40M */
	unsigned int side_band;		/* enum side_band_def  */
	unsigned char aid;
	unsigned int hw_crypto;		/* use hw crypto engine */
	unsigned char short_gi;
	unsigned char stbc;
	unsigned char short_preamble;
	unsigned char rts;				/* bit mask of enum rts_def */
	unsigned char rts_retry;		/* max retry limit of RTS */
	unsigned char cts2self;	
	unsigned char rts_rate;
	unsigned char rts_bw;			/* RTS bandwidth,  0: 20M, 1: 40M */	
	unsigned int need_ack;
	unsigned char	tx_rate;			/* enum tx_rate_def */
	unsigned char	fixed_rate;
	unsigned char aggre_mthd;			/* enum aggre_def  */
	int  retry;					/* max retry limit. -1: disable to limit max retry in desc */
	unsigned int more_frag;		/* more fragment flag, bit mask of enum more_frag_def */
};

#ifdef NEW_MAC80211_DRV

struct rtl_atx_ac {
        int sched;
        int qnum;
        struct list_head list;
        struct list_head tid_q;
};

struct rtl_atx_tid {
        struct list_head list;
        struct list_head buf_q;
        struct rtl_rc_node *rtl_node;
        struct rtl_atx_ac *rtl_control;
        struct rtl_buf *tx_buf[RTL_TID_MAX_BUFS];
        u16 seq_start;
        u16 seq_next;
        u16 baw_size;
        int tidno;
        int baw_head;   /* first un-acked tx buffer */
        int baw_tail;   /* next unused tx buffer slot */
        int sched;
        int paused;
        u8 state;
        int addba_exchangeattempts;
};


struct priv_stainfo {
	unsigned int tx_ra_bitmap;
	unsigned char rssi_level;
	unsigned char rssi;
	unsigned long		t_raupdate;	
	unsigned int	current_tx_rate;

	struct mac80211_shared_priv *priv;
	struct rtl_atx_tid tid[WME_NUM_TID];
	struct rtl_atx_ac ac[WME_NUM_AC];
	u16 maxampdu;
	u8 mpdudensity;

#ifdef RTL_TX_AMPDU
	struct sk_buff_head	ampdu_tx_que[8];
	int					ampdu_timer_id[8];
	int					ampdu_size[8];
#endif

};

struct priv_vifinfo {
};




struct priv_shared_hw {
	unsigned int			type;
	unsigned long			ioaddr;
	
#ifdef RTL8192SU	
	struct usb_device 			*udev;
#else
	struct pci_dev			*pdev;
#endif
	struct tasklet_struct	rx_tasklet;
	struct tasklet_struct	tx_tasklet;
	struct tasklet_struct	oneSec_tasklet;

	struct rtl8190_hw		*phw;
	struct rtl8190_tx_desc_info 	*pdesc_info;
	unsigned int			have_hw_mic;

#if defined(MERGE_FW) ||defined(DW_FW_BY_MALLOC_BUF)
	unsigned char			*fw_IMEM_buf;
	unsigned char			*fw_EMEM_buf;
	unsigned char			*fw_DMEM_buf;
#else
	unsigned char			fw_IMEM_buf[FW_IMEM_SIZE];
	unsigned char			fw_EMEM_buf[FW_EMEM_SIZE];
	unsigned char			fw_DMEM_buf[FW_DMEM_SIZE];
#endif
	unsigned char			agc_tab_buf[AGC_TAB_SIZE];
	unsigned char			mac_reg_buf[MAC_REG_SIZE];
	unsigned char			phy_reg_buf[PHY_REG_SIZE];
#ifdef MP_TEST
	unsigned char			phy_reg_mp_buf[PHY_REG_SIZE];
#endif
	unsigned char			phy_reg_pg_buf[PHY_REG_PG_SIZE];
	unsigned char			phy_reg_2to1[PHY_REG_1T2R];
	unsigned short			fw_IMEM_len;
	unsigned short			fw_EMEM_len;
	unsigned short			fw_DMEM_len;
	
#ifdef HW_QUICK_INIT
	unsigned short			hw_inited;
	unsigned short			hw_init_num;
	unsigned char			last_reinit;
#endif

	unsigned int			curr_band;				// remember the current band to save switching time
	unsigned short			fw_version;
	unsigned short			fw_src_version;
	unsigned short			fw_sub_version;
	unsigned int			CamEntryOccupied;		// how many entries in CAM?
	
	spinlock_t				lock;
	
	// for RF fine tune
	struct rf_finetune_var	rf_ft_var;

	// for Tx power control
	unsigned char			working_channel;
	unsigned char			use_default_para;
	unsigned char			legacyOFDM_pwrdiff_A;
	unsigned char			legacyOFDM_pwrdiff_B;
	signed char				channelAB_pwrdiff;
	unsigned char			min_ampdu_spacing;
	unsigned char			ra40MLowerMinus;
	unsigned int			bcnTxAGC;


	/*********************************************************
	 * from here on, data will be clear in rtl8190_init_sw() *
	 *********************************************************/

	// monitor Tx and Rx
	unsigned long			tx_packets_pre;
	unsigned long			rx_packets_pre;

	// bcm old 11n chipset iot debug, and TXOP enlarge
	unsigned long			current_tx_bytes;
	unsigned long			current_rx_bytes;

	unsigned int			CurrentChannelBW;
	unsigned char			*txcmd_buf;
	unsigned long			cmdbuf_phyaddr;
	unsigned long			InterruptMask;

	unsigned long			InterruptMaskExt;
	unsigned int			rx_rpt_ofdm;
	unsigned int			rx_rpt_cck;
	unsigned int			rx_rpt_ht;
	unsigned int			successive_bb_hang;

	struct sk_buff_head		skb_queue;

	unsigned int			has_2r_sta;
	int						has_triggered_rx_tasklet;
	int						has_triggered_tx_tasklet;

	int						offset_2nd_chan;
	int 					is_40m_bw; /* 0: 20M, 1: 40M */

	unsigned char eFuseData[128];

#ifdef RTL_TX_AMPDU
	struct reorder_ctrl_timer	ampdu_timer[64];
	unsigned short				ampdu_timer_head;
	unsigned short				ampdu_timer_tail;
#endif


#ifdef NEW_HAL_API
	int last_ht_dst_cached;
	unsigned char last_ht_dst_mac[6];	
#endif

};

enum wireless_mode {
	RTL_MODE_11B,
	RTL_MODE_11G,
	RTL_MODE_11N_HT20,
	RTL_MODE_11N_HT40PLUS,
	RTL_MODE_11N_HT40MINUS,
	RTL_MODE_MAX
};

#define SC_OP_INVALID           BIT(0)
#define SC_OP_BEACONS           BIT(1)
#define SC_OP_RXAGGR            BIT(2)
#define SC_OP_TXAGGR            BIT(3)
#define SC_OP_CHAINMASK_UPDATE  BIT(4)
#define SC_OP_FULL_RESET        BIT(5)
#define SC_OP_PREAMBLE_SHORT    BIT(6)
#define SC_OP_PROTECT_ENABLE    BIT(7)
#define SC_OP_RXFLUSH           BIT(8)
#define SC_OP_LED_ASSOCIATED    BIT(9)
#define SC_OP_RFKILL_REGISTERED BIT(10)
#define SC_OP_RFKILL_SW_BLOCKED BIT(11)
#define SC_OP_RFKILL_HW_BLOCKED BIT(12)
#define SC_OP_WAIT_FOR_BEACON   BIT(13)
#define SC_OP_LED_ON            BIT(14)
#define SC_OP_SCANNING          BIT(15)
#define SC_OP_TSF_RESET         BIT(16)


struct mac80211_shared_priv{
	int							drv_state;		// bit0 - init, bit1 - open/close
	struct priv_shared_hw		*pshare_hw;		// pointer of shared info, david
	struct extra_stats			ext_stats;
	spinlock_t					rxlock;
	unsigned int				rxfilter;
	struct proc_dir_entry	*proc_root;
	unsigned int 			beacon_interval;
	struct net_device *dev;

	// cache of EEPROM
	unsigned int			EE_Cached;
	unsigned int			EE_AutoloadFail;
	unsigned int			EE_ID;
	unsigned int			EE_Version;
	unsigned int			EE_RFTypeID;
	unsigned int			EE_AnaParm;
	unsigned int			EE_AnaParm2;
	unsigned char			EE_Mac[6];
	unsigned char			EE_TxPower_CCK[MAX_CCK_CHANNEL_NUM];
	unsigned char			EE_TxPower_OFDM[MAX_OFDM_CHANNEL_NUM];
	unsigned char			EE_CrystalCap;		// added for initial 8192

	struct wifi_mib                 *mib;

	/* below for mac80211 only */
	struct ieee80211_hw		*hw;
	struct ieee80211_vif		*vif;
	struct ieee80211_channel 	channels[14];
	struct ieee80211_rate 		rates[RTL_MODE_MAX][RATE_MAX];
//	struct rtl_tx	tx;

	u32 sc_flags;

	struct ieee80211_supported_band band;
	enum nl80211_iftype 		opmode;
	unsigned char			hwaddr[6];
	unsigned char			bssid[6];
	struct	sk_buff			*bcn_skb;
	unsigned long			bcndma_addr;
	u32						cfg_flags;
	unsigned long			raupdate_intv;

#ifdef RTL_TX_AMPDU
	struct tx_desc			*ampdu_first_desc;
#endif
};

#endif // NEW_MAC80211_DRV


#ifdef NEW_MAC80211_DRV
extern void fill_tx_desc(struct mac80211_shared_priv *priv, struct tx_info *txcfg);
#else
extern void fill_tx_desc(struct rtl8190_priv *priv, struct tx_info *txcfg);
#endif

#ifdef TX_SHORTCUT
void fill_tx_desc_shortcut(struct rtl8190_priv *priv, struct tx_insn *txcfg);
#endif

#ifdef SUPPORT_TX_AMSDU
void fill_tx_desc_amsdu(struct rtl8190_priv *priv, struct tx_insn* txcfg);
#endif

#endif // _RTL80211_H_
