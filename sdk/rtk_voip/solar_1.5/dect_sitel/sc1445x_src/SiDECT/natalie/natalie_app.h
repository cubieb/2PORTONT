#ifndef NATALIE_APP_H
#define NATALIE_APP_H

/*Missing definitions*/
#define FALSE 0
#define TRUE 1
/*Function prototypes*/
void dect_nvs_init(void);
/*Interprocess communication*/
void rsx_SendMail(uint32 iTaskId, uint32 iLength, uint8 *bDataPtr);
void rsx_SendMail(uint32 iTaskId, uint32 iLength, uint8 *bDataPtr);
void rsx_SendMailP0(uint32 iTaskId, PrimitiveType Primitive);
void rsx_SendMailP1(uint32 iTaskId, PrimitiveType Primitive, uint8 bParm1);
void rsx_SendMailP2(uint32 iTaskId, PrimitiveType Primitive, uint8 bParm1, uint8 bParm2);
void rsx_SendMailP3(uint32 iTaskId, PrimitiveType Primitive, uint8 bParm1, uint8 bParm2, uint8 bParm3);


void natalie_main(void);


#endif// NATALIE_APP_H
