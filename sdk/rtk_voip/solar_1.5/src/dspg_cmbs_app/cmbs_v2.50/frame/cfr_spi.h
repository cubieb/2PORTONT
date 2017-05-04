#if   !defined( CFR_SPI_H )
#define  CFR_SPI_H

#if defined( __cplusplus )
extern "C"
{
#endif
void  *cfr_SpiRxThread( void * pVoid );
int cfr_SpiInitialze( void );
#if defined( __cplusplus )
}
#endif

#endif   // CFR_SPI_H
//*/
