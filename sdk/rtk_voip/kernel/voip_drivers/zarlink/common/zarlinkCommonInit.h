
#ifndef __ZARLINKCOMMONINIT_H__
#define __ZARLINKCOMMONINIT_H__

#include "zarlinkCommon.h"

BOOL zarlinkInitDevice( RTKDevObj *pDev );
int zarlinkCaculateDevObj(RTKDevType dev_type);
int zarlinkRegDevForEvHandle(RTKDevObj * pDev);
int rtkGetNewChID(void);
RTKLineObj * rtkGetLine(int chid);
RTKDevObj * rtkGetDev(int chid);
void * rtkDumpReg( RTKDevObj *pDev );
void * rtkDumpObj( RTKDevObj *pDev );
void * rtkRxGain( RTKDevObj *pDev, int gain );
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LINETEST)
void * rtkPortDetect( RTKLineObj *pLine , int tID, void *data);
#endif
void * rtkStartMeter( RTKLineObj *pLine, uint16 hz);
int rtkGetCoeff( RTKLineObj *pLine, void *data);
int rtkSetCoeff( RTKLineObj *pLine, void *data);
int rtkDoCalibration( RTKLineObj *pLine, void *data);

#endif /* __ZARLINKCOMMONINIT_H__ */


