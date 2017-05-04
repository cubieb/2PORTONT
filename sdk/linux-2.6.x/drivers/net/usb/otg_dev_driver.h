//#define TX_ONLY
//#define RX_ONLY

#define BULKBUFSIZE		0x4000
#define START_DMA_ADDR	0x0 //988	//0x0
#define END_DMA_ADDR	0x400 //988	//0x400
#define START_DATA_LEN	1	//1925	// 1
#define END_DATA_LEN	2048	//2048	//1925	// 2048
#define START_DATA		2 //0x84 // 0

/*
data error and ehci will stop if
starting address is 4N+1, len % 64 = 4~6
			      4N+2, len % 64 = 3~5
			      4N+3, len % 64 = 2~4
*/
//#define SKIP_64_REST 

/*
1. starting address is not at 1K boundary and
2. starting address ~ starting address+data length cross 1K boundary and left 4~7 bytes
ex: 
          0K------1K------2K
data:          s------e	s:start, e:end, the difference between e and 1K is 4~7
*/
//#define SKIP_CROSS_1K_4_7

enum alignment_class {
	ALIGNED_4N = 0,
	ALIGNED_4N_1 = 1,
	ALIGNED_4N_2 = 2,
	ALIGNED_4N_3 = 3,
	ALIGNED_NO = 4,
};

#define BULKOUT_CLASS ALIGNED_NO
#define BULKIN_CLASS ALIGNED_NO

