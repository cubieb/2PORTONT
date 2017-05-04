#ifndef __SCFILESYSTEM__
#define __SCFILESYSTEM__

// CLASS
#define SIMCLASS		0xA0

// INSTRUTION
#define SELECT			0xA4
#define STATUS			0xF2

#define READBINARY		0xB0
#define UPDATEBINARY	0xD6

#define READRECORD		0xB2
#define UPDATERECORD	0xDC
#define SEARCHRECORD	0xA2

#define RETRIEVEDATA	0xCB
#define SETDATA			0xDB

#define GETRESPONSE		0xC0
#define TERMINALPROFILE	0x10
#define ENVELOPE		0xC2
#define FETCH			0x12
#define TERMINALRESPONSE	0x14

#define SEEK			0xA2
#define INCREASE		0x32

#define VERIFYCHV		0x20
#define CHANGECHV		0x24
#define DISABLECHV		0x26
#define ENABLECHV		0x28
#define UNBLOCKCHV		0x2C

#define INVALIDATE		0x04
#define REHABILITATE	0x44

#define	RUNGSMALORITHM	0x88
#define GETCHALLENGE	0x84

#define MANAGECHANNEL	0x70
#define MANAGESECURECH	0x73
#define TRANSACTDATA	0x75

#define SLEEP			0xFA


// File characteristics
#define FILE_CHAR_CLOCKSTOP_BIT0	(1<<0)
#define FILE_CHAR_CLOCKSTOP_BIT2	(1<<2)
#define FILE_CHAR_CLOCKSTOP_BIT3	(1<<3)
#define FILE_CHAR_CHV	(1<<7)

// CHV status
#define CHV_

// structure of file
#define EF_TRANSPARENT	0x0
#define EF_LINEAR_FIXED	0x1
#define EF_CYCLIC		0x3

// type of file
#define FILETYPE_RFU	0x0
#define FILETYPE_MF	0x1
#define FILETYPE_DF	0x2
#define FILETYPE_EF	0x4

// file access condition
#define ACC_ALW  0x00 // always
#define ACC_CHV1 0x01 // after CHV1 verification
#define ACC_CHV2 0x02 // after CHV2 verification
#define ACC_RFU  0x03
#define ACC_ADM4 0x04 // administrative codes (ALW, CHV1, CHV2 and NEV can be
                      // used as administrative codes too): they have meaning
                      // to the administrative authority only
#define ACC_ADM5 0x05
#define ACC_ADM6 0x06
#define ACC_ADM7 0x07
#define ACC_ADM8 0x08
#define ACC_ADM9 0x09
#define ACC_ADMA 0x0A
#define ACC_ADMB 0x0B
#define ACC_ADMC 0x0C
#define ACC_ADMD 0x0D
#define ACC_ADME 0x0E
#define ACC_NEV  0x0F // never

// type of number (TON)
#define TON_INTERNATIONAL_NUMBER	0x1
#define TON_NATIONAL_NUMBER	0x2
#define TON_NET_SPECIFIC_NUMBER	0x3
#define TON_SUBSCRIBER_NUMBER	0x4
#define TON_ALPHANUMERIC	0x5
#define TON_ABBREVIATED_NUMBER	0x6
#define TON_RFU	0x7

// Numbering plan identification  (NPI)
#define NPI_ISDN_TELE_NUMBERING_PLAN	0x1
#define NPI_DATA_NUMBERING_PLAN	0x3
#define NPI_TELEX_NUMBERING_PLAN	0x4
#define NPI_SERVICE_CENTRE_SPECIFIC_PLAN1	0x5
#define NPI_SERVICE_CENTRE_SPECIFIC_PLAN2	0x6
#define NPI_NATIONAL_NUMBERING_PLAN	0x8
#define NPI_PRIVATE_NUMBERING_PLAN	0x9
#define NPI_ERMES_NUMBERING_PLAN	0xa
#define NPI_RFU						0xf

// ADN
#define ADN_NUMBER_LEN	14	// GSM 11.11

#define ICCID_NUM		10
#define GSM_ALGO_NUM	0x10
#define CHV_LEN			8

typedef struct _GSMSpecificData{
	unsigned char characteristics;
	unsigned char numberofDF;
	unsigned char numberofEF;
	unsigned char numberofCHV;
	unsigned char RFU1;
	unsigned char CHV1Status;
	unsigned char UNBLOCKCHV1Status;
	unsigned char CHV2Status;
	unsigned char UNBLOCKCHV2Status;
	unsigned char RFU2;
	unsigned char RFAdiministrative[2];
	unsigned char RFU3;
}GSMSpecificData;

typedef struct _DFFileStatus{
	unsigned short RFU1;
	unsigned short totalMemory;
	unsigned short FileID;
	unsigned char typeOfFile;
	unsigned char RFU2[5];
	unsigned char lengnthOfFollowingData;
	GSMSpecificData gsmdata;
}DFFileStatus;

typedef struct _EFFileStatus{
	unsigned short RFU1;
	unsigned short FileSize;
	unsigned short FileID;
	unsigned char typeOfFile;
	unsigned char cycleStruturePermissionBit7; // bit7 =1, INCREASE command is allowed
	unsigned char AccessConditions[3];
	unsigned char fileStatus;
	unsigned char lengnthOfFollowingData;
	unsigned char strutureOfEF;
	unsigned char lengthOfRecord; // for cyclic and linear fixed EF denotes the length of a record
}EFFileStatus;

typedef unsigned int uint32_t;

typedef enum file_type {
    FILE_TYPE_TRANSPARENT = 0,
    FILE_TYPE_LINEAR_FIXED = 1,
    FILE_TYPE_CYCLIC = 3,
    FILE_TYPE_INVALID = 0xff
}SIMFileType;

enum file_access {
    FILE_ACCESS_ALWAYS = 0,
    FILE_ACCESS_CHV1 = 1,
    FILE_ACCESS_CHV2 = 2,
    FILE_ACCESS_RESERVED = 3,
    FILE_ACCESS_ADM = 4,
    FILE_ACCESS_NEVER = 15,
};

enum file_op {
    FILE_OP_READ = 20,
    FILE_OP_UPDATE = 16,
    FILE_OP_INCREASE = 12,
    FILE_OP_REHABILITATE = 4,
    FILE_OP_INVALIDATE = 0,
};

typedef struct
{
    const char *fileid;
    const char *parent;
    const char *name;
    uint32_t access;
    enum file_type type;
} SimFileInfo;

static SimFileInfo const knownFiles[] =
{
    {"3F00",        0,          "MF",          0,         FILE_TYPE_TRANSPARENT},
    {"7F20",        "3F00",     "DFgsm",       0,         FILE_TYPE_TRANSPARENT},
    {"7F10",        "3F00",     "DFtelecom",   0,         FILE_TYPE_TRANSPARENT},

    // Order according to sections in 31.102
    {"6F05",        "7F20",     "EFli",        0x01ff44,  FILE_TYPE_TRANSPARENT},//Language Preference
    {"6F07",        "7F20",     "EFimsi",      0x14ff44,  FILE_TYPE_TRANSPARENT},//IMSI
    {"6F60",        "7F20",     "EFplmnwact",  0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F31",        "7F20",     "EFhpplmn",    0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F37",        "7F20",     "EFacmmax",    0x12ff44,  FILE_TYPE_TRANSPARENT},
    {"6F38",        "7F20",     "EFust",       0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F39",        "7F20",     "EFacm",       0x111f44,  FILE_TYPE_CYCLIC},
    {"6F3E",        "7F20",     "EFgid1",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F3F",        "7F20",     "EFgid2",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F46",        "7F20",     "EFspn",       0x04ff44,  FILE_TYPE_TRANSPARENT},
    {"6F41",        "7F20",     "EFpuct",      0x12ff44,  FILE_TYPE_TRANSPARENT},
    {"6F45",        "7F20",     "EFcbmi",      0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F78",        "7F20",     "EFacc",       0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F7B",        "7F20",     "EFfplmn",     0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F7E",        "7F20",     "EFloci",      0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6FAD",        "7F20",     "EFad",        0x04ff44,  FILE_TYPE_TRANSPARENT},
    {"6F48",        "7F20",     "EFcbmid",     0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FB7",        "7F20",     "EFecc",       0x04ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F50",        "7F20",     "EFcbmir",     0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F73",        "7F20",     "EFpsloci",    0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F3B",        "7F10",     "EFfdn",       0x12ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F3C",        "7F10",     "EFsms",       0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F40",        "7F10",     "EFmsisdn",    0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F42",        "7F10",     "EFsmsp",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F43",        "7F10",     "EFsmss",      0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F49",        "7F10",     "EFsdn",       0x14ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F4B",        "7F10",     "EFext2",      0x12ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F4C",        "7F10",     "EFext3",      0x14ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F47",        "7F10",     "EFsmsr",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F80",        "7F20",     "EFici",       0x111f44,  FILE_TYPE_CYCLIC},
    {"6F81",        "7F20",     "EFoci",       0x111f44,  FILE_TYPE_CYCLIC},
    {"6F82",        "7F20",     "EFict",       0x121f44,  FILE_TYPE_CYCLIC},
    {"6F83",        "7F20",     "EFoct",       0x121f44,  FILE_TYPE_CYCLIC},
    {"6F4E",        "7F10",     "EFext5",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F4F",        "7F10",     "EFccp2",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FB5",        "7F20",     "EFemlpp",     0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FB6",        "7F20",     "EFaaem",      0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F4D",        "7F10",     "EFbdn",       0x12ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F55",        "7F10",     "EFext4",      0x12ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F58",        "7F10",     "EFcmi",       0x14ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F56",        "7F20",     "EFest",       0x12ff44,  FILE_TYPE_TRANSPARENT},
    {"6F2C",        "7F20",     "EFdck",       0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F32",        "7F20",     "EFcnl",       0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F61",        "7F20",     "EFoplmnwact", 0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F62",        "7F20",     "EFhplmnwact", 0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FC5",        "7F20",     "EFpnn",       0x04ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FC6",        "7F20",     "EFopl",       0x04ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FC7",        "7F20",     "EFmbdn",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FC8",        "7F20",     "EFext6",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FC9",        "7F20",     "EFmbi",       0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FCA",        "7F20",     "EFmwis",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FCB",        "7F20",     "EFcfis",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FCC",        "7F20",     "EFext7",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FCD",        "7F20",     "EFspdi",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FCE",        "7F20",     "EFmmsn",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FCF",        "7F20",     "EFext8",      0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FD0",        "7F20",     "EFmmsicp",    0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FD1",        "7F20",     "EFmmsup",     0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6FD2",        "7F20",     "EFmmsucp",    0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6FB1",        "7F20",     "EFvgcs",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FB2",        "7F20",     "EFvgcss",     0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6FB3",        "7F20",     "EFvbs",       0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FB4",        "7F20",     "EFvbss",      0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6FD9",        "7F20",     "EFehplmn",    0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FDB",        "7F20",     "EFehplmnpi",  0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6FDE",        "7F20",     "EFspni",      0x04ff44,  FILE_TYPE_TRANSPARENT},
    {"6FDF",        "7F20",     "EFpnni",      0x04ff44,  FILE_TYPE_LINEAR_FIXED},

    {"2FE2",        "3F00",     "EFiccid",     0x0fff44,  FILE_TYPE_TRANSPARENT},
    {"2F05",        "3F00",     "EFpl",        0x01ff44,  FILE_TYPE_TRANSPARENT},

    {"5F50",        "7F10",     "DFgraphics",  0,         FILE_TYPE_TRANSPARENT},
    {"4F20",        "5F50",     "EFimg",       0x14ff44,  FILE_TYPE_LINEAR_FIXED},
    {"4F01",        "5F50",     "EFimg1",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"4F02",        "5F50",     "EFimg2",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"4F03",        "5F50",     "EFimg3",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"4F04",        "5F50",     "EFimg4",      0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"4F05",        "5F50",     "EFimg5",      0x14ff44,  FILE_TYPE_TRANSPARENT},

    // CPHS 4.2
    {"6F13",        "7F20",     "EFcphs_cff",  0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F14",        "7F20",     "EFcphs_spn",  0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F15",        "7F20",     "EFcphs_csp",  0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F16",        "7F20",     "EFcphs_info", 0x14ff44,  FILE_TYPE_TRANSPARENT},
    {"6F17",        "7F20",     "EFcphs_mbdn", 0x11ff44,  FILE_TYPE_LINEAR_FIXED},
    {"6F18",        "7F20",     "EFcphs_spns", 0x11ff44,  FILE_TYPE_TRANSPARENT},
    {"6F11",        "7F20",     "EFcphs_mwis", 0x11ff44,  FILE_TYPE_TRANSPARENT},

    // TS 102.221
    {"2F00",        "3F00",     "EFdir",       0x04ff44,  FILE_TYPE_LINEAR_FIXED},
    {"2F06",        "3F00",     "EFarr",       0x04ff44,  FILE_TYPE_LINEAR_FIXED},
	
    {0,             0,          0,             0,         FILE_TYPE_TRANSPARENT}
};



unsigned short getEF_FileSize(void);
unsigned char getEF_LenOfRecord(void);

int selectFS(unsigned short fileID);
int getStatus(unsigned char len);
int getResponse(void);

int readRecord(unsigned char recNo, unsigned char mode, unsigned char len);
int updateRecord(unsigned char recNo, unsigned char mode, unsigned char len, unsigned char *data);

int readBinary(unsigned char offset_H, unsigned char offset_L, unsigned char len);
int updateBinary(unsigned char offset_H, unsigned char offset_L, unsigned char len, unsigned char *data);
int fs_init(void);
int fs_deinit(void);
int getFileLen(void);

#endif
