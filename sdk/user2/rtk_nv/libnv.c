#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <linux/types.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

//#define NV_DBG_ON
#if defined(NV_DBG_ON)
#define NV_DBG(args...) printf(args)
#else
#define NV_DBG(args...)
#endif

#define u32 __u32
#define u16 __u16
#define u8  __u8

#define NV_ENV_MTD "env"
#define NV_ENV2_MTD "env2"
extern u32 crc32 (u32 crc, const u8 *buf, unsigned int len);

static int uboot_env_size   = 0;//To be filled at env_init()
static int env_valid = 0;

#define HaveRedundEnv (1)
#define CONFIG_ENV_SIZE uboot_env_size //ENVSIZE(dev_current)
#define ENV_SIZE      getenvsize()

#define FLIO_R (0)
#define FLIO_W (1)

#define ACTIVE_FLAG	1
#define OBSOLETE_FLAG	0
struct env_image_redundant {
	u32	crc;	/* CRC32 over data bytes    */
	unsigned char	flags;	/* active or obsolete */
	char		data[];
};	
	
struct environment {
	void	*image;
	u32	*crc;
	u8	*flags;
	char	*data;
};

static struct environment environment;	

static inline ulong getenvsize (void)
{
	ulong rc = CONFIG_ENV_SIZE - sizeof (long);

	if (HaveRedundEnv)
		rc -= sizeof (char);
	return rc;
}

/* The function is ported from BoA */	
static int mtd_get_partition_fd(const char *name) 
{
	FILE *fp_mtd;
	char buf[128], flashdev[32];
	int index = -1, fd = -1;

	fp_mtd = fopen("/proc/mtd", "r");
	if (fp_mtd) {
		while (fgets(buf, sizeof(buf), fp_mtd)) {
			if (strstr(buf, name)) {
				sscanf(buf, "mtd%u", &index);
				break;
			}
		}
		fclose(fp_mtd);

		if (index == -1) {
			fprintf(stderr, "can't find partition %s in /proc/mtd\n", name);
			goto fail;
		}
	} else {
		fprintf(stderr, "open /proc/mtd fail!\n");
		goto fail;
	}

	sprintf(flashdev, "/dev/mtd%d", index);
	//printf("%s: %s\n", name, flashdev);
	fd = open(flashdev, O_RDWR);

fail:
	return fd;

}

static int mtd_erase_partition(int fd, mtd_info_t *mtd_info) 
{
	erase_info_t ei;
	ei.length = mtd_info->erasesize;
  
	for(ei.start = 0; ei.start < mtd_info->size; ei.start += mtd_info->erasesize) {
		ioctl(fd, MEMUNLOCK, &ei);
		ioctl(fd, MEMERASE, &ei);
	}
}

/* Write buf to the partition */
static int mtd_write_partition(int fd, mtd_info_t *mtd_info, const void *buf) 
{
	ssize_t wlen;	
	int rc = 0;

	wlen = write(fd, buf, mtd_info->size);
	if(wlen != mtd_info->size) {
		fprintf(stderr, "Tried writing %ld bytes but only %ld is written\n", mtd_info->size, wlen);
		rc = -2;
	}

	return rc;
}

static int mtd_write(int fd, mtd_info_t *mtd_info, const void *buf, ssize_t sz, ssize_t offset)  
{	ssize_t wlen;	
	int rc = 0;

	if(lseek(fd, offset, SEEK_SET) != offset) {
		perror("lseek failed in mtd_write()");
		rc = -1;
		goto ERR;
	}
	wlen = write(fd, buf, sz);
	if(wlen != sz) {
		fprintf(stderr, "Tried writing %ld bytes but only %ld is written\n", sz, wlen);
		rc = -2;
		goto ERR;
	}

ERR:
	return rc;
}

static int mtd_get_info(int fd, mtd_info_t *mtd_info) 
{
	ioctl(fd, MEMGETINFO, mtd_info);
 
	return 0;
}

static int flash_io (int isWrite, char *mtd_name)
{	
	int rc = -1;
	size_t retlen;
	int i;
	int env_mtd_fd;
	mtd_info_t mtd_info;

	env_mtd_fd = mtd_get_partition_fd(mtd_name);

	if(env_mtd_fd < 0) {
		fprintf(stderr, "Error: cannot find partition named as %s\n", NV_ENV_MTD);
		return -EIO;
	}

	mtd_get_info(env_mtd_fd, &mtd_info);
 
	if (isWrite) {
		rc = mtd_erase_partition(env_mtd_fd, &mtd_info);
		if(rc != 0) {
			fprintf(stderr, "ERROR: failed to erase partition\n");
			return -EIO;
		}
#if defined(CONFIG_MTD_NAND_RTK) || defined(CONFIG_MTD_SPI_NAND_RTK)
		rc = mtd_write(env_mtd_fd, &mtd_info, environment.image, uboot_env_size, 0);
#else
		rc = mtd_write_partition(env_mtd_fd, &mtd_info, environment.image);
#endif /* CONFIG_MTD_NAND_RTK */
		if(rc != 0) {
			fprintf(stderr, "ERROR: failed to write partition\n");
			return -EIO;
		}
	} else {
		rc = lseek(env_mtd_fd, 0, SEEK_SET);
		rc = read(env_mtd_fd, environment.image, CONFIG_ENV_SIZE);
			
		if(rc != CONFIG_ENV_SIZE) {
			perror("Failed to read mtd partition");
			fprintf(stderr, "WARNING: tried reading %ld bytes but only got %ld bytes.\n", CONFIG_ENV_SIZE, rc);
		}
		rc = 0;
	}

	close(env_mtd_fd);
	return rc;
}

static int saveenv(void)
{
	char	flag = OBSOLETE_FLAG;
	int	ret = 0;
	char *env_old, *env_new;

	int env_mtd_fd;
	mtd_info_t mtd_info;

	/*
	 * Update CRC
	 */
	*environment.crc = crc32 (0, (uint8_t *) environment.data, ENV_SIZE);
#if defined(CONFIG_MTD_NAND_RTK) || defined(CONFIG_MTD_SPI_NAND_RTK)
	NV_DBG("old flags=%u\n", *environment.flags);
	(*environment.flags)++;
	NV_DBG("new flags=%u\n", *environment.flags);
#else
	*environment.flags = ACTIVE_FLAG;
#endif /* CONFIG_MTD_NAND_RTK */

	if (env_valid == 1) {
		env_old = NV_ENV_MTD; 
		env_new = NV_ENV2_MTD; 
	} else {
		env_old = NV_ENV2_MTD; 
		env_new = NV_ENV_MTD; 
	}

	NV_DBG("Writing to flash...");
	ret = flash_io(FLIO_W, env_new);
	if (ret)
		goto done;

#if !defined(CONFIG_MTD_NAND_RTK) && !defined(CONFIG_MTD_SPI_NAND_RTK)
	NV_DBG("Obsoleting old env...");
	env_mtd_fd = mtd_get_partition_fd(env_old);
	if(env_mtd_fd < 0) {
		fprintf(stderr, "Error: cannot found partition named as %s\n", NV_ENV_MTD);
		return -EIO;
	}
	mtd_get_info(env_mtd_fd, &mtd_info);

	ret = mtd_write(env_mtd_fd, &mtd_info, &flag, sizeof(flag), offsetof(struct env_image_redundant, flags));
	if (ret)
		goto done;
#endif /* ! CONFIG_MTD_NAND_RTK */

	NV_DBG("done\n");

	env_valid = env_valid == 2 ? 1 : 2;

	NV_DBG("Valid environment: %d\n", (int)env_valid);

done:
	return ret;
}


static int env_relocate_spec(void)
{
	int ret;
	int crc1_ok = 0, crc2_ok = 0;
	struct env_image_redundant *tmp_env1 = NULL;
	struct env_image_redundant *tmp_env2 = NULL;
	struct env_image_redundant *ep;

	int env_mtd_fd;
	mtd_info_t mtd_info;

#if defined(CONFIG_MTD_NAND_RTK) || defined(CONFIG_MTD_SPI_NAND_RTK)
	/* On NAND platform, partition size is larger than the actual uboot env size */
	/* The following definition should be the same as the setting in U-Boot (CONFIG_ENV_SIZE@include/configs/luna.h)*/
#define NAND_UBOOT_ENV_SIZE (16*1024) 
	uboot_env_size = NAND_UBOOT_ENV_SIZE;
	NV_DBG("uboot_env_size=%u\n", uboot_env_size);
#else /* ! CONFIG_MTD_NAND_RTK */

	/* For SPI NOR, env mtd partition size is the same as U-Boot env size */
	env_mtd_fd = mtd_get_partition_fd(NV_ENV2_MTD);
	if(env_mtd_fd < 0) {
		fprintf(stderr, "Error: cannot find partition named as %s\n", NV_ENV2_MTD);
		return -EIO;
	}
	mtd_get_info(env_mtd_fd, &mtd_info);
	close(env_mtd_fd);
	uboot_env_size = mtd_info.size;
#endif /* CONFIG_MTD_NAND_RTK */

	tmp_env1 = (struct env_image_redundant *)malloc(CONFIG_ENV_SIZE);
	tmp_env2 = (struct env_image_redundant *)malloc(CONFIG_ENV_SIZE);
	if (!tmp_env1) {
		fprintf(stderr, "malloc() failed");
		goto out;
	}
	if (!tmp_env2) {
		fprintf(stderr, "malloc() failed");
		goto err_read;
	}

	environment.image = tmp_env1;
	ret = flash_io(FLIO_R, NV_ENV_MTD);
	if (ret) {
		fprintf(stderr, "flashio(FLIO_R, NV_ENV_MTD) failed");
		goto err_read;
	}
	if (crc32(0, (uint8_t *)tmp_env1->data, ENV_SIZE) == tmp_env1->crc)
		crc1_ok = 1;

	environment.image = tmp_env2;
	ret = flash_io(FLIO_R, NV_ENV2_MTD);
	if (!ret) {
		if (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc)
			crc2_ok = 1;
	} else {
		printf("WARNING: flashio(FLIO_R, NV_ENV2_MTD) failed");
	}

	if (!crc1_ok && !crc2_ok) {
		printf ("Warning: Bad CRC, please reboot to U-Boot to run saveenv to fill the default value\n");
		goto err_read;
	} else if (crc1_ok && !crc2_ok) {
		env_valid = 1;
	} else if (!crc1_ok && crc2_ok) {
		env_valid = 2;
#if defined(CONFIG_MTD_NAND_RTK) || defined(CONFIG_MTD_SPI_NAND_RTK)
	} else {
		/* both ok - check serial */
		NV_DBG("env1_flags=%u,env2_flags=%u\n", tmp_env1->flags, tmp_env2->flags);
		if ((tmp_env1->flags == 255) && (tmp_env2->flags == 0)) {
			env_valid = 2;
		} else if ((tmp_env2->flags == 255) && (tmp_env1->flags == 0)) {
			env_valid = 1;
		} else if (tmp_env1->flags > tmp_env2->flags) {
			env_valid = 1;
		} else if (tmp_env2->flags > tmp_env1->flags) {
			env_valid = 2;
		} else {/* flags are equal - almost imposiblle */
			env_valid = 1;
		}
		NV_DBG("env_valid=%u\n", env_valid);
	}
#else /* ! CONFIG_MTD_NAND_RTK */
	} else if (tmp_env1->flags == ACTIVE_FLAG &&
		   tmp_env2->flags == OBSOLETE_FLAG) {
		env_valid = 1;
		NV_DBG("env1 active;env2 obsolete\n");
	} else if (tmp_env1->flags == OBSOLETE_FLAG &&
		   tmp_env2->flags == ACTIVE_FLAG) {
		env_valid = 2;
		NV_DBG("env1 obsolete;env2 active\n");
	} else if (tmp_env1->flags == tmp_env2->flags) {
		env_valid = 2;
		NV_DBG("1=2\n");
	} else if (tmp_env1->flags == 0xFF) {
		env_valid = 2;
		NV_DBG("1=0xFF\n");
	} else {
		/*
		 * this differs from code in env_flash.c, but I think a sane
		 * default path is desirable.
		 */
		env_valid = 2;
	}
#endif /* CONFIG_MTD_NAND_RTK */

	if (env_valid == 1) {
		free(tmp_env2);
		ep = tmp_env1;
	} else {
		free(tmp_env1);
		ep = tmp_env2;
	}
	environment.image = ep;
	environment.crc		= &ep->crc;
	environment.flags	= &ep->flags;
	environment.data	= ep->data;

	return 0;
err_read:
	free(tmp_env1);
	free(tmp_env2);
out:
	return -EIO;
}

/*
 * s1 is either a simple 'name', or a 'name=value' pair.
 * s2 is a 'name=value' pair.
 * If the names match, return the value of s2, else NULL.
 */

static char *envmatch (const char * s1, char * s2)
{

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return s2;
	if (*s1 == '\0' && *(s2 - 1) == '=')
		return s2;
	return NULL;
}

/* get name=value, if name=NULL, return all tuples (if buf is big enough) */
int rtk_env_get(const char *name, char *buf, u32 buflen) {
	char *env, *nxt;
	char *val = NULL;
	int inc;
	
	if (env_relocate_spec ())
		return -EPERM;
		
	if (0 == name) {
		printf("Valid environment: %d\n", (int)env_valid);
		for (env = environment.data; *env; env = nxt + 1) {
			for (nxt = env; *nxt; ++nxt) {
				if (nxt >= &environment.data[ENV_SIZE]) {
					printf ("## Error: "
						"environment not terminated\n");
					free(environment.image);
					return -EPERM;
				}
			}

			inc = snprintf(buf, buflen, "%s\n", env);
			if (inc >= buflen) { /* truncated */
				free(environment.image);
				return -ENOMEM;
			}
			buf += inc;
			buflen -= inc;			
		}
		free(environment.image);
		return 0;
	}
	
	for (env = environment.data; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				printf ("## Error: "
					"environment not terminated\n");
				free(environment.image);
				return -EPERM;
			}
		}
		val = envmatch (name, env);
		if (val) {
			inc = snprintf(buf, buflen, "%s=%s", name, val);
			if (inc >= buflen) {
				free(environment.image);
				return -ENOMEM;
			}
			break;
		}
	}
	free(environment.image);
	return 0;
}

/*set name=value in u-boot, value=NULL then delete this key */
int rtk_env_set(const char *name, const char *value) {
	char *env, *nxt;
	char *oldval = NULL;
	int len;
	
	if (env_relocate_spec ())
		return -1;
	/*
	 * search if variable with this name already exists
	 */
	for (nxt = env = environment.data; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				printf ("## Error: "
					"environment not terminated\n");				
				free(environment.image);
				return -1;
			}
		}
		if ((oldval = envmatch (name, env)) != NULL)
			break;
	}
	
	/*
	 * Delete any existing definition
	 */
	if (oldval) {
		/*
		 * Ethernet Address and serial# can be set only once
		 */		
		if ((strcmp (name, "ethaddr") == 0) ||
			(strcmp (name, "serial#") == 0)) {
			printf ("Can't overwrite \"%s\"\n", name);			
			free(environment.image);
			return -1;
		}

		if (*++nxt == '\0') {
			*env = '\0';
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}
	
	/* Delete only ? */
	if (!value)
		goto WRITE_FLASH;

	/*
	 * Append new definition at the end
	 */
	for (env = environment.data; *env || *(env + 1); ++env);
	if (env > environment.data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > CONFIG_ENV_SIZE - (env-environment)
	 */
	len = strlen (name) + 2;
	len += strlen(value) + 1;
	/* add '=' for first arg, ' ' for all others */	
	if (len > (&environment.data[ENV_SIZE] - env)) {
		printf (
			"Error: environment overflow, \"%s\" deleted\n",
			name);
		free(environment.image);
		return -1;
	}
	while ((*env = *name++) != '\0')
		env++;
	*env = '=';
	while ((*++env = *value++) != '\0');
	*++env = '\0';

  WRITE_FLASH:
	/* write environment back to flash */
	if (saveenv()) {
		fprintf (stderr, "Error: can't write fw_env to flash\n");
		free(environment.image);
		return -1;
	}

	free(environment.image);
	return 0;
}



