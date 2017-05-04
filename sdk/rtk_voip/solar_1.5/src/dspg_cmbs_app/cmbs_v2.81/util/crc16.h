/*
   crc16.h
*/

#if   !defined( _CRC16_H )
#define  _CRC16_H

#define CRC_START 0xffff
#define CRC_GOOD  0xf0b8

#if   defined( __cplusplus )
extern "C"
{
#endif

u16               crc16( u8 * pu8_Buffer, u16 u16_Length, u16 u16_CrcIn );

#if   defined( __cplusplus )
}
#endif

#endif   // !defined( _CRC16_H )

//*/
