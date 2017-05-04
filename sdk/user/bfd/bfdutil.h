#ifndef _BFDUTIL_H_
#define _BFDUTIL_H_

void bfdutil_set_iprules( int debug, unsigned char *intfname, unsigned char *localip, unsigned char *remoteip, unsigned char *echoremoteip );
void bfdutil_clear_iprules( int debug, unsigned char *intfname, unsigned char *localip, unsigned char *remoteip, unsigned char *echoremoteip );

#endif //_BFDUTIL_H_
