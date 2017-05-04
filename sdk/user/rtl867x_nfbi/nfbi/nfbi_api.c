#include <stdio.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include "nfbi_api.h"
#include "boot_param.h"

int nfbi_fd;

void real_sleep(unsigned int sec)
{
    unsigned int s;
    
	s = sec;
	while ( (s=sleep(s)) )
	    ;
}

int _atoi(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else {
		while (*s != '\0') {			
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				printf("error hex format [%x]!\n", *s);
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

int nfbi_set_hcd_pid(int pid)
{
    int param;

    param = pid;
	return ioctl(nfbi_fd, NFBI_IOCTL_HCD_PID, &param);
}

int nfbi_get_event_msg(struct evt_msg *evt)
{
	return ioctl(nfbi_fd, NFBI_IOCTL_GET_EVENT, evt);
}

int hwreset(void)
{
    int param;
    
    param = 2; //hardware reset
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 0;
	else
	    return -1;
}

int eqreset(void)
{
    int param;
    
    param = 5; //event queue reset
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 0;
	else
	    return -1;
}

int nfbi_register_read(int reg, int *pval)
{
    int param, ret;
    
    param = reg << 16; //put register address to high word
    ret = ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param);
	if (0 == ret)
        *pval = param&0xffff;
    return ret;
}

int nfbi_register_mask_read(int reg, int mask, int *pval)
{
    int param, ret;
    
    param = reg << 16; //put register address to high word
    ret = ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param);
	if (0 == ret)
        *pval = param&(mask&0xffff);
    return ret;
}

int nfbi_register_write(int reg, int val)
{
    int param;
    
    //put register address to high word and the value to low word
    param = (reg << 16) | (val & 0xffff);
	return ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param);
}

int nfbi_register_mask_write(int reg, int mask, int val)
{
    int param, ret;

    // read register first for the 1st kind of command
    param = reg << 16; //put register address to high word
    ret = ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param);
	if (0 != ret)
	    return ret;
	
	mask &= 0xffff;
	val = ((param&0xffff)&(~mask)) | (val&mask);
    //put register address to high word and the value to low word
    param = (reg << 16) | (val & 0xffff);
	return ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param);
}

int nfbi_mem32_write(int addr, int val)
{
    struct nfbi_mem32_param param;

    param.addr = addr;
    param.val = val;
	return ioctl(nfbi_fd, NFBI_IOCTL_MEM32_WRITE, &param);
}

int nfbi_mem32_read(int addr, int *pval)
{
    int ret;
    struct nfbi_mem32_param param;

    param.addr = addr;
	ret = ioctl(nfbi_fd, NFBI_IOCTL_MEM32_READ, &param);
	if (0 == ret)
	    *pval = param.val;
	return ret;
}

int nfbi_bulk_mem_write(int addr, int len, char *buf)
{
    struct nfbi_bulk_mem_param param;

    // 0 < len <= NFBI_MAX_BULK_MEM_SIZE is a must
    if ((len <=0) || (len > NFBI_MAX_BULK_MEM_SIZE))
        return -1;
    param.addr = addr;
    param.len = len;
    memcpy(param.buf, buf, len);
	return ioctl(nfbi_fd, NFBI_IOCTL_BULK_MEM_WRITE, &param);
}

int nfbi_bulk_mem_read(int addr, int len, char *buf)
{
    int ret;
    struct nfbi_bulk_mem_param param;

    // 0 < len <= NFBI_MAX_BULK_MEM_SIZE is a must
    if ((len <=0) || (len > NFBI_MAX_BULK_MEM_SIZE))
        return -1;
    param.addr = addr;
    param.len = len;
	ret = ioctl(nfbi_fd, NFBI_IOCTL_BULK_MEM_READ, &param);
	if (0 == ret) {
	    memcpy(buf, param.buf, len);
	}
	return ret;
}


/* before sending anything to ram, it's necessary to call do_dram_settings() to configure DRAM first */
//int send_file_to_ram(int fd, unsigned int addr, int verify)
int send_file_to_ram(char *filename, unsigned int ram_addr, int verify)
{
    struct stat ffstat;
	//int val, h_val, l_val, ret;
	int flen=0;
	int rc, i, j;
	int fd;
	char buf[NFBI_MAX_BULK_MEM_SIZE], buf2[NFBI_MAX_BULK_MEM_SIZE];
	int num;
	unsigned int addr;

    fd = open(filename, O_RDONLY);
	if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
	}
	
	//get file length	
	fstat(fd, &ffstat);
	flen = ffstat.st_size;
	//printf("File Length=%d bytes\n", flen);
    addr = ram_addr;
    num = flen/NFBI_MAX_BULK_MEM_SIZE;
    for (i=0; i<num; i++) {
        rc = read(fd, buf, NFBI_MAX_BULK_MEM_SIZE);
  	    if (rc != NFBI_MAX_BULK_MEM_SIZE) {
  	        printf("Reading error\n");
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_write(addr, NFBI_MAX_BULK_MEM_SIZE, buf);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
        addr += NFBI_MAX_BULK_MEM_SIZE;
        if((i&0x1f)==0) {printf(">");fflush(stdout);}
    }
    num = flen%NFBI_MAX_BULK_MEM_SIZE;
    if (num > 0) {
        rc = read(fd, buf, num);
  	    if (rc != num) {
  	        printf("Reading error, %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_write(addr, num, buf);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
		{printf(">");fflush(stdout);}
    }
    printf("\n");
    close(fd);
    
    if (!verify)
        return 0;

    real_sleep(1);
    //sets the file position indicator to the beginning of the file
    //fseek(fd, 0, SEEK_SET);

    fd = open(filename, O_RDONLY);
	if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
	}
    addr = ram_addr;
    num = flen/NFBI_MAX_BULK_MEM_SIZE;
    for (i=0; i<num; i++) {
        rc = read(fd, buf, NFBI_MAX_BULK_MEM_SIZE);
  	    if (rc != NFBI_MAX_BULK_MEM_SIZE) {
  	        printf("Reading error\n");
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_read(addr, NFBI_MAX_BULK_MEM_SIZE, buf2);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
        
        for (j=0; j<NFBI_MAX_BULK_MEM_SIZE; j++) {
            if (buf[j] != buf2[j]) {
                printf("Error! at addr=%x value=%x Expect val=%x\n", 
                        addr+j, buf2[j], buf[j]);
                close(fd);
	            return -1;
            }
        }
        /*
        if (memcmp(buf, buf2, NFBI_MAX_BULK_MEM_SIZE) != 0) {
            printf("Error! at addr=%x\n", addr);
        }
        */
        addr += NFBI_MAX_BULK_MEM_SIZE;

        if((i&0x1f)==0) {printf("<");fflush(stdout);}
    }
    num = flen%NFBI_MAX_BULK_MEM_SIZE;
    if (num > 0) {
        rc = read(fd, buf, num);
  	    if (rc != num) {
  	        printf("Reading error, %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_read(addr, num, buf2);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
        for (j=0; j<num; j++) {
            if (buf[j] != buf2[j]) {
                printf("Error! at addr=%x value=%x Expect val=%x\n", 
                        addr+j, buf2[j], buf[j]);
                close(fd);
	            return -1;
            }
        }
		/*
		if (memcmp(buf, buf2, NFBI_MAX_BULK_MEM_SIZE) != 0) {
			printf("Error! at addr=%x\n", addr);
		}
		*/
		{printf("<");fflush(stdout);}
    }
    printf("\n");
   	close(fd);
			
	return 0;
}

int send_bp_to_ram(unsigned int ram_addr, int verify)
{
	int ret;
	char *p, *p_verify;
	int size;

	bootconf_t bc, bc_verify;
	p=(char*)&bc;
	p_verify=(char*)&bc_verify;
	size=sizeof(bootconf_t);
	bootconf_get(&bc);
	
	//printf( "set boot parameter for slave\n" );
	ret = nfbi_bulk_mem_write(ram_addr, size, p);
	{printf(">\n");fflush(stdout);}
	if (ret != 0) {
		printf("writing bootparam error: %s %d\n", __FUNCTION__, __LINE__);
		return -201;
	}

	if(verify)
	{
		ret = nfbi_bulk_mem_read(ram_addr, size, p_verify);
		{printf("<\n");fflush(stdout);}
		if (ret != 0) {
			printf("read bootparam error: %s %d\n", __FUNCTION__, __LINE__);
			return -202;
		}

		if( memcmp(p, p_verify, size) )
		{
			printf("memcmp bootparam error: %s %d\n", __FUNCTION__, __LINE__);
			return -203;			
		}
	}
	
	return 0;
}

int bootcode_download(int verify, char *filename)
{
    int ret, val, ret2, count;
    
    ret2 = 0;
    count = 0;
begin:
    if (ret2)
        printf("count=%d ret2=%d\n", count, ret2);
    if (count < BOOTCODE_DOWNLOAD_RETRY_MAX)
        count++;
    else
        return ret2;

    hwreset();
    real_sleep(1);
    
    // check if PHYID2 is equal to 0xcb61
    ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
    if ((ret != 0) || 
		((val != NFBI_REG_PHYID2_DEFAULT)&&
		 (val != NFBI_REG_PHYID2_DEFAULT2)&&
		 (val != NFBI_REG_PHYID2_DEFAULT3)&&
		 (val != NFBI_REG_PHYID2_DEFAULT4))) 
	{
        ret2 = -101;
        goto begin;
    }

    // check if StartRunBootCode bit is equal to 0
    if (0 != nfbi_register_read(NFBI_REG_CMD, &val)) {
        ret2 = -102;
        goto begin;
    }
    if (val&BM_START_RUN_BOOTCODE) {
        ret2 = -103;
        goto begin;
    }
    
    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0) {
        ret2 = -105;
        goto begin;
    }
    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0) {
        ret2 = -106;
        goto begin;
    }

    //upload the bootcode to 8197B DRAM
	if (0 != send_file_to_ram(filename, NFBI_BOOTADDR, verify)) {
		printf("send_file_to_ram fail\n");
        ret2 = -108;
        //for debugging
        //send_foreverloop_to_ram(0x00008000);
        //nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
        goto begin;
	}


	if (0 != send_bp_to_ram(NFBI_BOOTPARAM_START, verify)) {
		printf("send_bp_to_ram fail\n");
        ret2 = -120;
        goto begin;
	}

	
    //dump_misc("StartRunBootCode000");
	//run the bootcode, write 1 to StartRunBootCode bit
    ret = nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
    if (ret != 0) {
        ret2 = -109;
        goto begin;
    }


#if 1
{
	int sleep_time;
#ifdef CONFIG_RTL8685
	sleep_time=10;
#else
	sleep_time=3;
#endif

	while(sleep_time>0)
	{
		//wait for bootcode to be ready
		real_sleep(1);
		sleep_time--;
		//dump_misc("StartRunBootCode111");

		//check if bit 5 is equal to 1
		if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
			ret2 = -110;
			goto begin;
		}
		printf("%s(%d):ISR=%x\n",__func__,__LINE__,val);
		if (!(val&IP_BOOTCODE_READY)) 
		{
			if(sleep_time==0)
			{
				dump_misc("info");
				ret2 = -111;
				
				do {
					int imr,syssr,syscr;
					nfbi_register_read(NFBI_REG_IMR, &imr);
					nfbi_register_read(NFBI_REG_SYSSR, &syssr);
					nfbi_register_read(NFBI_REG_SYSCR, &syscr);
					printf("%s(%d):IMR=%x SYSSR=%x SYSCR=%x\n",__func__,__LINE__,imr,syssr,syscr);
				} while (0);
				
				goto begin;
			}//else continue;
		}else{
			//printf( "got bootcode ready bit\n" );
			break;
		}
	}
}
#else
	//wait for bootcode to be ready
	real_sleep(3);
    //dump_misc("StartRunBootCode111");


    //check if bit 5 is equal to 1
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -110;
        goto begin;
    }

    if (!(val&IP_BOOTCODE_READY)) {
        dump_misc("info");
        ret2 = -111;
        goto begin;
    }
	//printf( "got bootcode ready bit\n" );
#endif
    
    //Write 1 to clear the IP_BOOTCODE_READY bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_BOOTCODE_READY);
    if (ret != 0) {
        ret2 = -112;
        goto begin;
    }
	//printf( "clear bootcode ready bit\n" );


    return 0;
}


int test_send_file_to_ram(char *filename, unsigned int ram_addr, int verify)
{
    struct stat ffstat;
	//int val, h_val, l_val, ret;
	int flen=0;
	int rc, i, j;
	int fd;
	char buf[NFBI_MAX_BULK_MEM_SIZE], buf2[NFBI_MAX_BULK_MEM_SIZE];
	int num;
	unsigned int addr;

    fd = open(filename, O_RDONLY);
	if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
	}
	
	//get file length	
	fstat(fd, &ffstat);
	flen = ffstat.st_size;
	//printf("File Length=%d bytes\n", flen);
    addr = ram_addr;

if(verify) 
{
	printf("use mem32 way\n");
	while(1)
	{
		unsigned int val_1, val_2;
		int rep;

        rc = read(fd, &val_1, 4);
		if(rc<=0) break;

		rep=5;
readagain:
	    if (0 != nfbi_mem32_write(addr, val_1))
	    {
            printf("error: %s %d  write\n", __FUNCTION__, __LINE__);
		    return -1;
	    }

	#if 1
		if (0 != nfbi_mem32_read(addr, &val_2))
	    {
            printf("error: %s %d read\n", __FUNCTION__, __LINE__);
		    return -1;
	    }

		//if(val_1!=val_2)
		//if( (val_1>>1)!=val_2)
		if( ((val_1&0xfffefffe)>>1)!=val_2 )//skip bit16 & bit 0
		{
          	printf("compare error addr=%08x, value=(%08x,%08x,%08x), rep=%d\n", 
						addr, val_1, (val_1>>1), val_2, rep);
			if(rep)
			{
				rep--;
				goto readagain;
			}else{
            	//printf("compare error addr=%08x, value=(%08x,%08x,%08x), rep=%d\n", 
				//		addr, val_1, (val_1>>1), val_2, rep);
			}
			
		    //return -1;
		}
	#endif
	
		addr=addr+4;;
	}
}else{
	printf("use mem bulk way\n");
    num = flen/NFBI_MAX_BULK_MEM_SIZE;
    for (i=0; i<num; i++) {
        rc = read(fd, buf, NFBI_MAX_BULK_MEM_SIZE);
  	    if (rc != NFBI_MAX_BULK_MEM_SIZE) {
  	        printf("Reading error\n");
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_write(addr, NFBI_MAX_BULK_MEM_SIZE, buf);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
        addr += NFBI_MAX_BULK_MEM_SIZE;
        if((i&0x1f)==0) {printf(">");fflush(stdout);}
    }
    num = flen%NFBI_MAX_BULK_MEM_SIZE;
    if (num > 0) {
        rc = read(fd, buf, num);
  	    if (rc != num) {
  	        printf("Reading error, %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_write(addr, num, buf);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
		{printf(">");fflush(stdout);}
    }
}

    printf("\n");
    close(fd);
    			
	return 0;
}

int bootcode_download_test(int verify, char *filename)
{
    int ret, val, ret2, count;
    
    ret2 = 0;
    count = 0;
begin:
    if (ret2)
        printf("count=%d ret2=%d\n", count, ret2);
    if (count < BOOTCODE_DOWNLOAD_RETRY_MAX)
        count++;
    else
        return ret2;

#ifdef _F_DL_
    hwreset();
#endif
    real_sleep(1);
printf( "1\n" );
    
#ifdef _F_DL_
    // check if PHYID2 is equal to 0xcb61
    ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
    if ((ret != 0) || 
		((val != NFBI_REG_PHYID2_DEFAULT)&&
		 (val != NFBI_REG_PHYID2_DEFAULT2)&&
		 (val != NFBI_REG_PHYID2_DEFAULT3)&&
		 (val != NFBI_REG_PHYID2_DEFAULT4))) 
	{
        ret2 = -101;
        goto begin;
    }

    // check if StartRunBootCode bit is equal to 0
    if (0 != nfbi_register_read(NFBI_REG_CMD, &val)) {
        ret2 = -102;
        goto begin;
    }
    if (val&BM_START_RUN_BOOTCODE) {
        ret2 = -103;
        goto begin;
    }
#endif

    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0) {
        ret2 = -105;
        goto begin;
    }
       
    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0) {
        ret2 = -106;
        goto begin;
    }
printf( "2\n" );

    
    //upload the bootcode to 8197B DRAM
	if (0 != test_send_file_to_ram(filename, NFBI_BOOTADDR, verify)) {
		printf("send_file_to_ram fail\n");
        ret2 = -108;
        //for debugging
        //send_foreverloop_to_ram(0x00008000);
        //nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
        goto begin;
	}

printf( "3\n" );

#ifdef _F_DL_
	if (0 != send_bp_to_ram(NFBI_BOOTPARAM_START, verify)) {
		printf("send_bp_to_ram fail\n");
        ret2 = -120;
        goto begin;
	}
#endif

printf( "4\n" );
	
    //dump_misc("StartRunBootCode000");
	//run the bootcode, write 1 to StartRunBootCode bit
#ifdef _F_DL_
    ret = nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
#else
    ret = nfbi_register_write(NFBI_REG_CMD, 0x1);
#endif
    if (ret != 0) {
        ret2 = -109;
        goto begin;
    }
printf( "5\n" );

#ifdef _F_DL_
#if 1
{
	int sleep_time;
	sleep_time=3;
	while(sleep_time>0)
	{
		//wait for bootcode to be ready
		real_sleep(1);
		sleep_time--;
		//dump_misc("StartRunBootCode111");

		//check if bit 5 is equal to 1
		if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
			ret2 = -110;
			goto begin;
		}

		if (!(val&IP_BOOTCODE_READY)) 
		{
			if(sleep_time==0)
			{
				dump_misc("info");
				ret2 = -111;
				goto begin;
			}//else continue;
		}else{
			//printf( "got bootcode ready bit\n" );
			break;
		}
	}
}
#else
	//wait for bootcode to be ready
	real_sleep(3);
    //dump_misc("StartRunBootCode111");


    //check if bit 5 is equal to 1
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -110;
        goto begin;
    }

    if (!(val&IP_BOOTCODE_READY)) {
        dump_misc("info");
        ret2 = -111;
        goto begin;
    }
	//printf( "got bootcode ready bit\n" );
#endif
#endif

    //Write 1 to clear the IP_BOOTCODE_READY bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_BOOTCODE_READY);
    if (ret != 0) {
        ret2 = -112;
        goto begin;
    }
	//printf( "clear bootcode ready bit\n" );


    return 0;
}

int firmware_download_w_boot(int verify, char *fw_filename, char *boot_filename)
{
    int ret, val, timeout;

	bootparam_set_filename(fw_filename);

    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0)
        return -1;

	//wait for bootcode to calculate the checksum of firmware
	//real_sleep(5);

    //check if ChecksumDoneIP is equal to 1
    //if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
    //    return -2;
    printf( "<image start time=%d>\n", (int)time(NULL) );
    timeout = 20; //10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
            printf("fail to read ISR\n");
            timeout--;
            continue;
        }
        if (val&IP_CHECKSUM_DONE)
            break;
        timeout--;
    }
    printf( "<image end time=%d>\n", (int)time(NULL) );
    printf("ISR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        return -2;
    }
    if (!(val&IP_CHECKSUM_DONE)) {
        return -3;
    }
    //real_sleep(1);
    //Write 1 to clear the CheckSumDoneIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_DONE);
    if (ret != 0)
        return -4;
    if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val))
        return -5;
    printf("SYSSR=0x%04x\n", val);
    if (!(val&BM_CHECKSUM_DONE))
        return -6;

    //real_sleep(1);
    //check if ChecksumOKIP is equal to 1
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
        return -7;
    printf("ISR=0x%04x\n", val);
    if (!(val&IP_CHECKSUM_OK))
        return -8;
    
    //Write 1 to clear the ChecksumOKIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_OK);
    if (ret != 0)
        return -9;
    if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val))
        return -10;
    printf("SYSSR=0x%04x\n", val);
    if (!(val&BM_CHECKSUM_OK))
        return -11;

#ifndef NFBI_BOOT_ONLY    
	//wait for firmware running to be ready
	//real_sleep(3);

    //check if AllSoftwareReadyIP is equal to 1
    //if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
    //    return -12;
    
    timeout = 10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
            printf("fail to read ISR\n");
            timeout--;
            continue;
        }
        if (val&IP_ALLSOFTWARE_READY)
            break;
        timeout--;
    }
    printf("ISR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        return -12;
    }
    if (!(val&IP_ALLSOFTWARE_READY))
        return -13;

    //Write 1 to clear the AllSoftwareReadyIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_ALLSOFTWARE_READY);
    if (ret != 0)
        return -14;
#endif /*NFBI_BOOT_ONLY*/

    return 0;
}



int firmware_downloadtest(int verify, char *fw_filename, char *boot_filename)
{
    int ret;

	printf("set verify=%d\n", verify);
	
    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0)
        return -1;

#if 1
{
	//test
	unsigned int itime=0;
	unsigned int j=0;

	while(1)
	{
		printf( "load %s to 0x%08x (%d)\n", fw_filename, j, itime);
		if (0 != send_file_to_ram(fw_filename, j, verify)) 
		{
			printf("send_file_to_ram kernel fail itime=%u, j=0x%x\n", itime, j);
			return -20;
		}
		j=j+0x1000;
		#if 0
		if(j>=0x600000)
		{
			j=0;
			itime++;
		}
		#else
		if(j>=0x600000) break;
		#endif
	}
}
#endif

	return 0;
}

int firmware_download_w_boot_nfbi(int verify, char *fw_filename, char *boot_filename)
{
    int ret, val, timeout;

	printf("set verify=%d\n", verify);
	
    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0)
        return -1;

#if 0
{
	//test
	unsigned int itime=0;
	unsigned int j=0;

	while(1)
	{
		printf( "load %s to 0x%08x\n", fw_filename, j);
		if (0 != send_file_to_ram(fw_filename, j, verify)) 
		{
			printf("send_file_to_ram kernel fail itime=%u, j=0x%x\n", itime, j);
		}
		j=j+0x1000;
		if(j>=0x600000) j=0;
		//if(j>=0x600000) break;
	}
}
#endif

    //upload the kernel to 867x-slave's RAM
    printf( "<image start time=%d>\n", (int)time(NULL) );
#if 1
	if (0 != send_file_to_ram(fw_filename, NFBI_KERNADDR, verify)) {
		printf("send_file_to_ram kernel fail\n");
       return -20;
	}
#else
	{ 	//test
	    int param;
		printf( "<by kernel directly>\n");
	    param = 4; //dump private data
		ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param);
	}
#endif	
    printf( "<image end time=%d>\n", (int)time(NULL) );

	//signal to slave kernelisready
	printf("set CR_KERNELCODE_READY\n");
    if (0 != nfbi_register_read(NFBI_REG_SYSCR, &val)) {
        printf("fail to read SYSCR\n");
		return -21;
    }
	nfbi_register_write(NFBI_REG_SYSCR, val&~CR_KERNELCODE_READY);
	
	printf( " ori syscr=0x%08x\n", val );
	val=val | CR_KERNELCODE_READY;
	printf( " new syscr=0x%08x, set 0x%04x\n", val, (unsigned short)CR_KERNELCODE_READY );
    if (0 != nfbi_register_write(NFBI_REG_SYSCR, val)) {
        printf("fail to write SYSCR\n");
		return -22;
    }


    //check if ChecksumDoneIP is equal to 1
    //if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
    //    return -2;
    timeout = 5; //10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
            printf("fail to read ISR\n");
            timeout--;
            continue;
        }
        if (val&IP_CHECKSUM_DONE)
            break;
        timeout--;
    }
    printf("ISR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        return -2;
    }
    if (!(val&IP_CHECKSUM_DONE)) {
        return -3;
    }
    //real_sleep(1);
    //Write 1 to clear the CheckSumDoneIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_DONE);
    if (ret != 0)
        return -4;
    if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val))
        return -5;
    printf("SYSSR=0x%04x\n", val);
    if (!(val&BM_CHECKSUM_DONE))
        return -6;

    //real_sleep(1);
    //check if ChecksumOKIP is equal to 1
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
        return -7;
    printf("ISR=0x%04x\n", val);
    if (!(val&IP_CHECKSUM_OK))
        return -8;
    
    //Write 1 to clear the ChecksumOKIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_OK);
    if (ret != 0)
        return -9;
    if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val))
        return -10;
    printf("SYSSR=0x%04x\n", val);
    if (!(val&BM_CHECKSUM_OK))
        return -11;

#ifndef NFBI_BOOT_ONLY
	//wait for firmware running to be ready
	//real_sleep(3);

    //check if AllSoftwareReadyIP is equal to 1
    //if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
    //    return -12;
    
    timeout = 10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
            printf("fail to read ISR\n");
            timeout--;
            continue;
        }
        if (val&IP_ALLSOFTWARE_READY)
            break;
        timeout--;
    }
    printf("ISR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        return -12;
    }
    if (!(val&IP_ALLSOFTWARE_READY))
        return -13;

    //Write 1 to clear the AllSoftwareReadyIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_ALLSOFTWARE_READY);
    if (ret != 0)
        return -14;
#endif /*NFBI_BOOT_ONLY*/

    return 0;
}


void dump_misc(char *msg)
{
#if 0
    printf("%s\n", msg);
    printf("----------------------------\n");
    //printf("hw strap/clk manage/GISR/timer 0/memory/memory\n");
    printf("GISR/timer 0/memory/memory\n");
	//system("nfbi memread 0x18000108"); //hw strap	
	//system("nfbi memread 0x18000010"); //clk manage	
	system("nfbi memread 0x18003004"); //GISR
	system("nfbi memread 0x18003100"); //timer 0
    system("nfbi memread 0x00008000"); //memory
    system("nfbi memread 0x007f0000"); //memory
    printf("----------------------------\n");
#endif
}

