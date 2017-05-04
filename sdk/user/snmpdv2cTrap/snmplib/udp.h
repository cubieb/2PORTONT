#ifndef		_UDP_H_
#define		_UDP_H_

#include	"ctypes.h"
#include	"smp.h"

SmpStatusType	udpSend (SmpSocketType udp, CBytePtrType bp, CIntfType n);
SmpSocketType	udpNew (int so, u_long host, u_short port);
SmpSocketType	udpFree (SmpSocketType udp);

// Added by Mason Yu for modify snmp agent
SmpStatusType	pvcSend (SmpSocketType udp, CBytePtrType bp, CIntfType n);
SmpStatusType	eocSend (SmpSocketType udp, CBytePtrType bp, CIntfType n);

#endif		/*	_UDP_H_	*/
