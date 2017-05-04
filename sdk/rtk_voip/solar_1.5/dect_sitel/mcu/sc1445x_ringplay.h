
#ifndef FORMATS_H
#define FORMATS_H		1

#include <endian.h>
#include <byteswap.h>

/* Definitions for .VOC files */
 
typedef struct voc_header {
	unsigned char magic[20];	/* must be MAGIC_STRING */
	unsigned short headerlen;	/* Headerlength, should be 0x1A */
	unsigned short version;	/* VOC-file version */
	unsigned short coded_ver;	/* 0x1233-version */
} VocHeader;
 
 
/* Definitions for Microsoft WAVE format */

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)		(v)
#define LE_INT(v)		(v)
#define BE_SHORT(v)		bswap_16(v)
#define BE_INT(v)		bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)		bswap_16(v)
#define LE_INT(v)		bswap_32(v)
#define BE_SHORT(v)		(v)
#define BE_INT(v)		(v)
#else
#error "Wrong endian"
#endif

#define WAV_RIFF		COMPOSE_ID('R','I','F','F')
#define WAV_WAVE		COMPOSE_ID('W','A','V','E')
#define WAV_FMT			COMPOSE_ID('f','m','t',' ')
#define WAV_DATA		COMPOSE_ID('d','a','t','a')
#define WAV_PCM_CODE		1

/* it's in chunks like .voc and AMIGA iff, but my source say there
   are in only in this combination, so I combined them in one header;
   it works on all WAVE-file I have
 */
typedef struct {
	unsigned int magic;		/* 'RIFF' */
	unsigned int length;		/* filelen */
	unsigned int type;		/* 'WAVE' */
} WaveHeader;

typedef struct {
	unsigned short format;		/* should be 1 for PCM-code */
	unsigned short modus;		/* 1 Mono, 2 Stereo */
	unsigned int sample_fq;	/* frequence of sample */
	unsigned int byte_p_sec;
	unsigned short byte_p_spl;	/* samplesize; 1 or 2 bytes */
	unsigned short bit_p_spl;	/* 8, 12 or 16 bit */
} WaveFmtBody;

typedef struct {
	unsigned int type;		/* 'data' */
	unsigned int length;		/* samplecount */
} WaveChunkHeader;

/* Definitions for Sparc .au header */
typedef struct au_header {
	unsigned int magic;		/* '.snd' */
	unsigned int hdr_size;		/* size of header (min 24) */
	unsigned int data_size;	/* size of data */
	unsigned int encoding;		/* see to AU_FMT_XXXX */
	unsigned int sample_rate;	/* sample rate */
	unsigned int channels;		/* number of channels (voices) */
} AuHeader;

#define AU_MAGIC		COMPOSE_ID('.','s','n','d')

#define AU_FMT_ULAW		1
#define AU_FMT_LIN8		2
#define AU_FMT_LIN16		3


int ua_playring(char *ringtone);
int ua_stopringplay(void);
#endif				/* FORMATS */
