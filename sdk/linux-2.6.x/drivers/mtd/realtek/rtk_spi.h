#ifndef RTK_SPI_H_
#define RTK_SPI_H_

#define SFCR_DIV(x)	  ((x&7) << 29)
#define SFCR_RBO_BE	  (1 << 28)
#define SFCR_WBO_BE	  (1 << 27)
#define SFCR_TCS(x)   (((x)&0x1f) << 22)

#define CSNONE		  3
#define CS0			  1
#define CS1			  2

#define SFCSR_CS(x)	  ((x) << 30)
#define SFCSR_LEN(x)  (((x)&3) << 28)
#define SFCSR_RDY     (1<<27)


#endif
