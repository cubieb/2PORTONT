#ifndef __CWMP_MAIN_TR104_H__
#define __CWMP_MAIN_TR104_H__

extern int gVoipReq;

/*open the connection from solar to cwmpclient*/
void cwmp_solarOpen( void );

/*close the connection from solar to cwmpclient*/
void cwmp_solarClose(void);

/* send the request to solar */
void cwmpSendRequestToSolar(void);


void cwmpDoRefresh(void);

/* send the Event to solar */
void cwmpStartVoipDiag(void);

#endif /* __CWMP_MAIN_TR104_H__ */

