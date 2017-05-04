#ifndef __SCT1__
#define __SCT1__

/* I-block */

/* R-block */

// bit 4:1
enum {
	R_BLOCK_ErrorFree = 0,
	R_BLOCK_EDCError =	1,
	R_BLOCK_OtherError = 2,
	R_BLOCK_RFU
};

/* S-block */
// bit 5:1
enum {
	S_BLOCK_RSYNCH_REQ = 0,
	S_BLOCK_IFSC_REQ = 1,
	S_BLOCK_ABORT_REQ = 2,
	S_BLOCK_BWT_EXT	= 3,
	S_BLOCK_VPPError = 4,
	S_BLOCK_RFU
};
#endif