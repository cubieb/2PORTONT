
#include "8190n.h"
#include "wapi_wai.h"
#include "wapiCrypto.h"
#ifdef __LINUX_2_6__
#include <common/rtl_types.h>
#include "8190n_tx.h"
#else
#include "../rtl865x/rtl_types.h"
#include "8190n_headers.h"
#endif
#include "8190n_rx.h"
#define	WAPI_KM_OUI_LEN	3

extern void issue_deauth(struct rtl8190_priv *priv,	unsigned char *da, int reason);
extern int32 init_SMS4_CK_Sbox(void);
extern int switch_iram(unsigned int identity);
extern void WapiSMS4ForMNKEncrypt(uint8 *key, uint8*IV, uint8*input, uint32 inputLength, uint8 *output, uint8 *outputLength, uint32 CryptFlag);
extern int DOT11_EnQueue(unsigned long task_priv, DOT11_QUEUE *q, unsigned char *item, int itemsize);
extern int32 WapiIncreasePN(uint8 *PN, uint8 AddCount);
extern int rtl8190_start_xmit(struct sk_buff *skb, struct net_device *dev);
extern void GenerateRandomData(unsigned char * data, unsigned int len);
extern void sha256_hmac( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char *output, int hlen);
extern void KD_hmac_sha256( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char *output, int hlen);
extern void sha2( unsigned char *input, int ilen,
           unsigned char output[32], int is224 );
extern struct sk_buff *alloc_skb_from_queue(struct rtl8190_priv *priv);

extern void (*wapiInit_hook)(struct rtl8190_priv *priv);
extern void (*wapiExit_hook)(struct rtl8190_priv *priv);
extern void (*wapiStationInit_hook)(struct stat_info *pstat);
extern void (*wapiSetIE_hook)(struct rtl8190_priv	*priv);
extern void (*wapiReleaseFragementQueue_hook)(wapiStaInfo *wapiInfo);
extern int (*DOT11_Process_WAPI_Info_hook)(struct rtl8190_priv *priv, uint8 *data, int32 len);
extern int (*wapiHandleRecvPacket_hook)(struct rx_frinfo *pfrinfo, struct stat_info *pstat);
extern int	(*wapiIEInfoInstall_hook)(struct rtl8190_priv *priv, struct stat_info *pstat);
extern void (*wapiReqActiveCA_hook)(struct stat_info *pstat);
extern void	(*wapiSetBK_hook)(struct stat_info *pstat);
extern int	 (*wapiSendUnicastKeyAgrementRequeset_hook)(struct rtl8190_priv *priv, struct stat_info *pstat);
extern int (*wapiUpdateUSK_hook)(struct rtl8190_priv *priv, struct stat_info *pstat);
extern int	 (*wapiUpdateMSK_hook)(struct rtl8190_priv *priv, struct stat_info *pstat);
