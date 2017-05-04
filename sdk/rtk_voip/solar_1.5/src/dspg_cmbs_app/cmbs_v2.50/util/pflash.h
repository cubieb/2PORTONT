/*
   pflash.h

*/

#if   !defined( _PFLASH_H )
#define  _PFLASH_H

#define  PFLASH_BASE          0x40000     // sector 4-7 for testing
#define  PFLASH_PAGE_SIZE     0x200
#define  PFLASH_PAGE_MASK     (PFLASH_PAGE_SIZE - 1)

typedef struct _PFLASH_TIMING
{
   u32   u32_AhbPllDiv;
   u16   u16_FcraClock;
   u16   u16_ProgTimer;
   u32   u32_EraseTimer;
}  PFLASH_TIMING, * PPFLASH_TIMING;

typedef const PFLASH_TIMING   * PCPFLASH_TIMING;

#if   defined( __cplusplus )
extern "C"
{
#endif

void              pflErase( u32 u32_Sector, u32 u32_Count );
void              pflWrite( u32 * pu32_Destination, u32 * pu32_Source, u32 u32_Count );
void              pflUnprotect( u32 u32_Sector, u32 u32_Count );
void              pflProtect( u32 u32_Sector, u32 u32_Count );

#if   defined( __cplusplus )
}
#endif

#endif // !defined( _PFLASH_H )

//*/
