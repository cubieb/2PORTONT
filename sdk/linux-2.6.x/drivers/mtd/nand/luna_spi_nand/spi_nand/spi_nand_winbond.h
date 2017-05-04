#ifndef NAND_SPI_WINBOND_H
#define NAND_SPI_WINBOND_H

/***********************************************
  * Driver Section Definitions
  ***********************************************/
#ifndef SECTION_SPI_NAND
    #define SECTION_SPI_NAND 
#endif
#ifndef SECTION_SPI_NAND_DATA
    #define SECTION_SPI_NAND_DATA 
#endif


/***********************************************
  *  Winbond's ID definition
  ***********************************************/
#define MID_WINBOND         (0xEF)
#define DID_W25N01GV        (0xAA21)
#define DID_W25M02GVZEIG    (0xAB21) //BUF=1


/********************************************
  Export functions
  ********************************************/
spi_nand_flash_info_t *probe_winbond_spi_nand_chip(void);

#endif //#ifdef NAND_SPI_WINBOND_H

