#ifndef __RTK_RG_RTSP_H__
#define __RTK_RG_RTSP_H__

/* port block types */ 
typedef enum { 
    PORT_SINGLE,  /* client_port=x */ 
    PORT_RANGE,   /* client_port=x-y */ 
    PORT_DISCON   /* client_port=x/y (rtspbis) */ 
} rtk_rg_rtsp_portblock_t; 

typedef struct rtk_rg_rtsp_s
{
	unsigned int	cseqOff; 
	unsigned int	cseqLen; 
	unsigned int	transOff; 
	unsigned int	transLen; 
	uint16 lowPort;
	uint16 highPort;
	rtk_rg_rtsp_portblock_t portType;
}rtk_rg_rtsp_t;

rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_rtsp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);


#endif

