#ifndef __SI_DECT_API_H__
#define __SI_DECT_API_H__

// -------------------------------------------------------- 
// DECT API - SIP to RTX 
// -------------------------------------------------------- 

typedef struct {
	const char *ifname;		/// interface name to get MAC as RFPI. default: "br0"
	const char *fwname;		/// firmware filename for upgrade 
} dect_api_init_args_t;

/**
 * @ingroup VOIP_DECT_SITEL
 * @brief Initial DECT api, and start TX/RX thread for SPI bus 
 * @param ifname IF name to get MAC, and use to decide RFPI 
 */
extern void dect_api_S2R_init_with_args( const dect_api_init_args_t * args );
#define dect_api_S2R_init()		dect_api_S2R_init_with_args( NULL )

typedef struct {
	unsigned long VersionHex;		/// Firmware version. 13010000 = 1.13 ; In FWU program, it will be 0. 
	unsigned char AccessCode[ 2 ];	/// Access code. It is a BCD string, so '1234' will be 0x12 and 0x34. 
} dect_api_info_t;

/**
 * @ingroup VOIP_DECT_SITEL
 * @brief Get runtime information  
 * @param info Information structure @ref dect_api_info_t
 */
extern void dect_api_S2R_get_info( dect_api_info_t *info );

/**
 * @ingroup VOIP_DECT_SITEL
 * @brief Set filename for firmware upgrade in runtime.  
 * @param file filename 
 */
extern void dect_api_S2R_set_fwu( const char *file );


// ------------------- Call functions ----------------------
/**
 * @ingroup VOIP_DECT_SITEL
 * @brief Check handset is ready or not 
 * @param chid Channel ID 
 * @note Use this function to check handset before call_setup(), 
 *       but this is software checking.
 */
extern int dect_api_S2R_check_handset_busy( unsigned long chid );

/**
 * @ingroup VOIP_DECT_SITEL
 * @brief Inform dect for an incoming call 
 * @param wideband Use wideband codec in PCM 
 * @param chid Channel ID
 * @param dialednum Dialed number 
 * @param callerid Caller ID
 */
extern int dect_api_S2R_call_setup( int wideband, unsigned long chid, const char *dialednum, const char *callerid );

/**
 * @ingroup VOIP_DECT_SITEL
 * @brief Check handset is ringing or not 
 * @param chid Channel ID 
 * @note If handset is still rining and peer releases the call, 
 *       we should call call_release() to stop ringing. 
 */
extern int dect_api_S2R_check_handset_ringing( unsigned long chid );

/**
 * @ingroup VOIP_DECT_SITEL
 * @brief Inform dect to release a call 
 * @param chid Channel ID
 * @note If 
 */
extern void dect_api_S2R_call_release( unsigned long chid );


// -------------------------------------------------------- 
// DECT API - RTX to SIP 
// -------------------------------------------------------- 

/**
 * @ingroup VOIP_DECT_SITEL
 * @brief send pressed key to SIP 
 * @param accountid It can be seen as channel ID 
 * @param port PCM port. CVM allocate it dynamically, so it may not same with accountid. 
 * @param input Input digits or on/off hook. 
 */
extern void dect_api_R2S_pressed_key( int accountid, int port, char input );


#endif // __SI_DECT_API_H__ 

