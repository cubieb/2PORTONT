#include <linux/skbuff.h>
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

#define RATE_MAX 30

struct rtl_rate_table {
	int rate_cnt;
	struct ieee80211_rate rates[40];
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

#ifdef BUFFER_TX_AMPDU
	struct sk_buff_head	ampdu_tx_que[8];
	int					ampdu_timer_id[8];
	int					ampdu_size[8];
#endif

};

struct priv_vifinfo {
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


#endif // NEW_MAC80211_DRV


#ifdef TX_SHORTCUT
void fill_tx_desc_shortcut(struct rtl8190_priv *priv, struct tx_insn *txcfg);
#endif

#ifdef SUPPORT_TX_AMSDU
void fill_tx_desc_amsdu(struct rtl8190_priv *priv, struct tx_insn* txcfg);
#endif

#endif // _RTL80211_H_
