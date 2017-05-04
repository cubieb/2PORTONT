#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>

#define debug printf
#define INIT_PERM (S_IRUSR|S_IWUSR)

typedef struct {
	unsigned int	key;		/* magic key */

#define BOOT_IMAGE             0xB0010001
#define CONFIG_IMAGE           0xCF010002
#define APPLICATION_IMAGE      0xA0000003
#define BOOTPTABLE             0xB0AB0004


	unsigned int	address;	/* image loading DRAM address */
	unsigned int	length;		/* image length */
	unsigned int	entry;		/* starting point of program */
	unsigned short	chksum;		/* chksum of */
	
	unsigned char	type;
#define KEEPHEADER    0x01   /* set save header to flash */
#define FLASHIMAGE    0x02   /* flash image */
#define COMPRESSHEADER    0x04       /* compress header */
#define MULTIHEADER       0x08       /* multiple image header */
#define IMAGEMATCH        0x10       /* match image name before upgrade */
	
	
	unsigned char	   date[25];  /* sting format include 24 + null */
	unsigned char	   version[16];
        unsigned int  *flashp;  /* pointer to flash address */

} IMGHDR;

//ql_xu ---signature header
typedef struct {
	unsigned int sigLen;	//signature len
	unsigned char sigStr[20];	//signature content
	unsigned short chksum;	//chksum of imghdr and img
}SIGHDR;

 static struct option arg_options[] = {
                {"input",    required_argument,      0, 'i'},
                {"output",  required_argument,            0, 'o'},
                //ql add
                {"output2",  required_argument,     0, 's'},
                {"head",  no_argument,            0, 'h'},
                {"key",    required_argument,      0, 'k'},
                {"flash",    required_argument,      0, 'f'},
                {"address",    required_argument,      0, 'a'},
                {"entry",    required_argument,      0, 'e'},
                
                {0, 0, 0, 0}
};


#define ntohs
#define uint16 unsigned short
unsigned short 
ipchksum(unsigned char *ptr, int count, unsigned short resid)
{
	register unsigned int sum = resid;
       if ( count==0) 
       	return(sum);
        
	while(count > 1) {
		//sum += ntohs(*ptr);	
		sum += (( ptr[0] << 8) | ptr[1] );
		//sum += (*ptr);	
		if ( sum>>31)
			sum = (sum&0xffff) + ((sum>>16)&0xffff);
		//ptr++;
		ptr += 2;
		count -= 2;
	}

	if (count > 0) 
		sum += (*((unsigned char*)ptr) << 8) & 0xff00;

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	if (sum == 0xffff) 
		sum = 0;
	return (unsigned short)sum;
}


static uint16 fchksum(int fd) {
	unsigned char buf[128];
	int byteRead;
	uint16 resid = 0; 
	int i = 0;


	//memset(buf, 0, sizeof (buf));
	lseek(fd, 0, SEEK_SET);
	while (( byteRead = read(fd, buf, sizeof (buf) ) ) > 0) {
		resid = ipchksum(buf, byteRead, resid);
		//debug("[%4d] %d - %04x\n", ++i, byteRead, resid);
		//memset(buf, 0, sizeof (buf));
	}

	//debug("last code = %d(errno %d)\n", byteRead, errno);

	return ~resid;
}

static void show_usage(void)
{
        printf(
"Usage: genHeader [OPTIONS]\n\n"
"  -i, --input=file         Input file\n"
"  -o, --output=file        Output file\n"
//ql add
"  -s, --Output2=file       Output file2\n"
"  -h, --head               Keep image header\n"
"  -k, --key=hex            Image Key 0xA0000003(app)\n"
"  -f, --flash=address      Address to write to (FLASH)\n"
"  -a, --address=address    Address to decompress to (RAM)\n"
"  -e, --entry=address      Address to start\n"
"\n Example for Linux \n"
"  genhead -i vm.img -o vm.hdr -s vm_sig.hdr -k 0xa0000003 -f 0xbfc10000  -a 0x80000000 -e 0x80000000\n"
        );
        exit(0);
}

//ql -- signature str
char signature[]="ZXDSL531BIIZ22";

int main(int argc, char *argv[]) {
	IMGHDR imgHdr;
	char *input=0;
	char *output=0;
    //ql add output 2 for signatured img
    char *output2=0;
	int len,c ;
	int fdInput, fdOutput;
	struct stat statbuf;
    int i;
    //ql add
    int fdOutput2;
	SIGHDR sigHdr;
	uint16 resid = 0;
	
	memset((void *)&imgHdr, 0, sizeof(imgHdr));
	//ql add
	memset((void *)&sigHdr, 0, sizeof(sigHdr));
	/* get options */
	while (1) {
		int option_index = 0;
		unsigned int val;
		c = getopt_long(argc, argv, "i:o:s:hk:f:a:e:", arg_options, &option_index);		/* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
		if (c == -1) break;
		
		switch (c) {
		case 'i':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);			
			if (input) free(input);
			input = malloc(len + 2);			
			strncpy(input, optarg, len);			
			debug ("input file %s\n", input);
			break;
		case 'o':			
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);			
			if (output) free(output);
			output = malloc(len + 2);			
			strncpy(output, optarg, len);
			debug ("output file %s\n", output);
			break;
        case 's'://ql add
            len = strlen(optarg) > 255 ? 255 : strlen(optarg);
            if (output2) free(output2);
            output2 = malloc(len+2);
            strncpy(output2, optarg, len);
            debug("output file %s\n", output);
            break;
		case 'h':
			imgHdr.type |= KEEPHEADER;			
			debug ("type =  %x\n", imgHdr.type);
			break;
		case 'k':
			if (sscanf(optarg, "%x", &val) != 1) {
				printf("Incorrect key: %s\n", optarg);
				exit (1);
			}			
			imgHdr.key = htonl(val);
			debug ("key =  %x\n", ntohl(imgHdr.key));
			break;
		case 'f':
			if (sscanf(optarg, "%x", &val) != 1) {
				printf("Incorrect key: %s\n", optarg);
				exit (1);
			}			
			imgHdr.flashp = htonl(val);
			debug ("key =  %x\n", ntohl(imgHdr.key));
			break;

		case 'a':
			if (sscanf(optarg, "%x", &val) != 1) {
				printf("Incorrect key: %s\n", optarg);
				exit (1);
			}			
			imgHdr.address= htonl(val);
			debug ("address =  %x\n", ntohl(imgHdr.address));
			break;
		case 'e':
			if (sscanf(optarg, "%x", &val) != 1) {
				printf("Incorrect key: %s\n", optarg);
				exit (1);
			}			
			imgHdr.entry= htonl(val);
			debug ("entry =  %x\n", ntohl(imgHdr.entry));
			break;
		default:
			show_usage();
		}
	}

	// check if req. arg is present
	if (!input || !output) {
		printf("Must specify input and output\n");
		exit(1);
	}

	if (!imgHdr.key) {
		printf("Must specify Key\n");
		exit(1);
	}

	if (!imgHdr.flashp) {
		printf("Must specify flash address\n");
		exit(1);
	}

	// start the work.
	if ((fdInput = open(input, O_RDONLY)) <= 0) {
		printf("Canot open %s\n", input);
		exit(1);
	}
	lseek(input, 0, SEEK_SET);
	
	if ((fdOutput = open(output, O_RDWR|O_TRUNC|O_CREAT, INIT_PERM)) <= 0) {
		printf("Canot open %s\n", output);
		exit(1);
	}
    //ql add
    if ( output2 ) {
        if ((fdOutput2 = open(output2, O_RDWR|O_TRUNC|O_CREAT, INIT_PERM)) <= 0) {
            printf("Canot open %s\n", output2);
            exit(1);
        }
    }

	fstat(fdInput, &statbuf);
	printf("size = %d\n", statbuf.st_size);
	imgHdr.length = htonl(statbuf.st_size);
	
	imgHdr.chksum = fchksum(fdInput);
	printf("checksum = %04x\n", imgHdr.chksum);
	imgHdr.chksum = htons(imgHdr.chksum);

    //ql_xu add:
    sigHdr.sigLen = strlen(signature);	//set len
    printf("sig size = %d\n", sigHdr.sigLen);
    for (i=0; i<sigHdr.sigLen; i++)
        sigHdr.sigStr[i] = signature[i]+10;
    sigHdr.sigStr[i] = '\0';			//set content
    sigHdr.sigLen = htonl(sigHdr.sigLen);
    //get chksum
    resid = ipchksum(&imgHdr, sizeof(imgHdr), resid);
	sigHdr.chksum = ~resid;
    printf("sig chksum= %04x\n", sigHdr.chksum);
	sigHdr.chksum = htons(sigHdr.chksum);
	
    write(fdOutput2, &sigHdr, sizeof(sigHdr));
    write(fdOutput2, &imgHdr, sizeof(imgHdr));
	write(fdOutput, &imgHdr, sizeof(imgHdr));

	close(fdInput);
	close(fdOutput);
	if (output2)
        close(fdOutput2);
}
