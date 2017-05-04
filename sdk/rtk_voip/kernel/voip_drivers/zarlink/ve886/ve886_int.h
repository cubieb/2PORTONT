#ifndef __VE886_INT_H__
#define __VE886_INT_H__

#include "zarlinkCommon.h"

void Ve886RegisterHandler(RTKDevObj *pDevObj);


int Ve886CreateDevObj(
	/* Realtek */
	RTKDevType 				dev_type, 
	int						ch_id,
	RTKDevObj 				*pDevObj, 
	RTKLineObj 				LineObj[],

	/* Zarlink */
	VpDeviceIdType 			devId, 
	VpDeviceType  			vpDevType, 
	Vp886DeviceObjectType	*pVpDevObj, 	/* use 'void *' to support 886/890 */
	VpDevCtxType  			*pVpDevCtx, 
	Vp886LineObjectType  	pVpLineObj[],
	VpLineCtxType 			pVpLineCtx[]);

BOOL Ve886CreateLineObj(
	int ch_id, 
	int channelId, 	/* line# within a slic. usually 0 or 1 */
	RTKLineType	line_type,
	RTKLineObj *pLine,
	int law,
	unsigned int slot);

int Ve886Init(int pcm_mode);
int Ve886GetRev(RTKDevObj *pDev);

#endif /* __VE886_INT_H__ */


