#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xml_config.h"
#include "xml_config_gramma.h"

static inline ssize_t
get_file_length(int fd) {
	struct stat fdstat;
	fstat(fd, &fdstat);
	ssize_t flen=fdstat.st_size;
	return flen;
}
extern char current_filename[MAX_NAME_STRING];
extern unsigned int linenum;
int
config_parser(const char *filename) {
	int fd, count = 0;
	ssize_t sz = 0;
	if (filename != NULL) {
		if ((fd = open (filename, O_RDONLY)) < 0) {
			return -1;
		}
		strcpy (current_filename, filename);
		sz = get_file_length (fd);
		g_config = (char*) malloc (sz + 3);
		if (read (fd, g_config, sz) != sz) {
			printf ("read file failed.\n");
			close (fd);
			return -1;
		}
		close (fd);
		g_config[sz] = '\0';
	} else {
		fd = fileno (stdin);
		sz = 2048;
		g_config = (char*) malloc (sz + 3);
		if ((count = read (fd, g_config, sz)) <= 0) {
			printf ("read stdin failed.\n");
			return -1;
		}
		g_config[count] = '\0';
	}

	stoken_init ();
	Index_cnt_init ();

	if (g_program () != CFG_OK) {
		fprintf (stderr, "Error occurs in %s:%d\n",
			current_filename, linenum);
		return CFG_SYNTAX_ERROR;
	}
	return 0;
}


#define ascii_hex(a) ((((a)>='0')&&((a)<='9'))?((a)-'0'):(((a)>='a')&&((a)<='f'))?((a)-'a'+10):(((a)>='A')&&((a)<='F'))?((a)-'A'+10):0)
#if 0
XML_OPR_t string_to_hex(unsigned char *dst, const char *src) {
    while (*src!='\0') {
        unsigned char b0=*(src++);
        if (*src=='\0') return XML_FAIL;
        unsigned char b1=*(src++);
        unsigned char d0=ascii_hex(b0);
        unsigned char d1=ascii_hex(b1);
        *(dst++) = (d0<<4)+d1;
// 		XML_DBG("%02x ", dst[-1]);
        if (*src==',') ++src;
    }
    return XML_OK;
}
#else
XML_OPR_t string_to_hex (void *dst, const char *src, ARRAY_TYPE type) {
	unsigned char hex_num = 0, sepa = 0;
	unsigned char check_base[] = {2, 8};
	unsigned int chk_len = 0;
	unsigned int i, d;
	unsigned int *tmp_int_dst;
	unsigned char *tmp_char_dst;
	char *pch;

	if (type == BYTE_ARRAY)
		tmp_char_dst = (unsigned char *)dst;
	else
		tmp_int_dst = (unsigned int *)dst;

	if ((pch = strchr (src, ',')) != NULL || (pch = strchr (src, ' ')) != NULL) sepa = 1;

	pch = strtok ((char *)src, ", ");
	while (pch != NULL)
	{
		chk_len = strlen (pch);
		hex_num = 0;
		do {
			pch += hex_num;

			if (chk_len <= check_base[type]) {
				hex_num = chk_len;
			} else {
				// Truncate pch for negative and positive value.
				if (sepa) pch += (chk_len - check_base[type]);
				hex_num = check_base[type];
			}

			d = 0;
			// Parsing string to hex.
			for (i = 0; i < hex_num; i++) {
				d <<= 4;
				d |= ascii_hex (pch[i]);
			}

			// Fill the mib data.
			(type == BYTE_ARRAY) ? (*(tmp_char_dst++) = d) : (*(tmp_int_dst++) = d);

		} while (pch[i] != '\0');

		pch = strtok (NULL, ", ");
	}
	return XML_OK;
}
#endif

char *str_replace (const char *source, xmlent_t *ent, int ent_nr)
{
	int src_TL = 0, rep_TL = 0, ma_sz = 0;
	int gap = 0;
	char *former;
	char *location;
	char *result;
	int i = 0, rep_en = 0;

	rep_TL = src_TL = strlen (source);
	// Search the whole pre-defined strings.
	for (i = 0; i < ent_nr; i++) {
		former = (char *)source;
		location = NULL;

		if (NULL != (location = strstr (former, (ent + i)->find_ent))) {
			rep_en = 1;
			while (NULL != location) {
				rep_TL += strlen ((ent + i)->rep_str) - strlen ((ent + i)->find_ent);
				former = location + strlen ((ent + i)->find_ent);
				location = strstr (former, (ent + i)->find_ent);
			}
		}
	}

	if (!rep_en) return (char *)source;

	(src_TL > rep_TL) ? (ma_sz = src_TL) : (ma_sz = rep_TL);
	result = (char*) calloc (ma_sz, sizeof (char));

	if (result != NULL) {
		strcpy (result, source);

		// Search the string and replace it.
		for (i = 0; i < ent_nr; i++) {
			gap = 0;

			while (NULL != (location = strstr ((result + gap), (ent + i)->find_ent))) {
				gap += location - (result + gap);

				// Shift unrep string.
				char *ur = location + strlen ((ent + i)->find_ent);
				int urep_L = strlen (ur);
				int sh = strlen ((ent + i)->rep_str) - strlen ((ent + i)->find_ent);

				if (sh > 0) {
					memmove (ur + sh, ur, urep_L);
				} else  if (sh < 0) {
					ur[urep_L] = '\0';
					memmove (location + strlen ((ent + i)->rep_str), ur, urep_L + 1);
				}

				// Insert rep string.
				strncpy (location, (ent + i)->rep_str, strlen ((ent + i)->rep_str));

				// Move the gap to start of unrep string.
				gap += strlen ((ent + i)->rep_str);
			}
		}
		return result;
	}
	fprintf (stderr, "calloc() fail! The xmlconfig parsing may be wrong!\n");
	return result;
}

#ifdef CONFIG_MIB_STRING_ENCRYPT
int str_encrypt (const char *de_str, char *en_str)
{
	int i, j;
	int k = 0;

	while (isprint (de_str[k])) k++;
	if (k != strlen (de_str)) {
		fprintf (stderr,
			"[ERR] %s(): Unprintable character detected!! The entry value written will be NULL.\n",
			__func__);
		return -1;
	}

	if (en_str == NULL) {
		fprintf (stderr,
			"[ERR] %s(): calloc() failed!! The entry value written will be NULL.\n",
			__func__);
		return -1;
	}


	// String reverse for test.
	// Replace testing code with the real encrypt function.
	for (i = 0, j = strlen (de_str) - 1; j >= 0; --j)
		en_str[i++] = de_str[j];


	while (isprint (en_str[k])) k++;
	if (k != strlen (en_str)) {
		fprintf (stderr,
			"[ERR] %s(): Unprintable character detected!! The entry value written will be NULL.\n",
			__func__);
		return -1;
	}

	return 0;
}

int str_decrypt (const char *en_str, char *de_str)
{
	int i, j;
	int k = 0;

	while (isprint (en_str[k])) k++;
	if (k != strlen (en_str)) {
		fprintf (stderr,
			"[ERR] %s(): Unprintable character detected!! The entry value written will be NULL.\n",
			__func__);
		return -1;
	}

	if (de_str == NULL) {
		fprintf (stderr,
			"[ERR] %s(): calloc() failed!! The entry value written will be NULL.\n",
			__func__);
		return -1;
	}


	// String reverse for test.
	// Replace testing code with the real decrypt function.
	for (i = 0, j = strlen (en_str) - 1; j >= 0; --j)
		de_str[i++] = en_str[j];


	k = 0;
	while (isprint (de_str[k])) k++;
	if (k != strlen (de_str)) {
		fprintf (stderr,
			"[ERR] %s(): Unprintable character detected!! The entry value written will be NULL.\n",
			__func__);
		return -1;
	}

	return 0;
}
#endif

#if 0
int
config_parser_init(void) {
    gramma_init();
    return 0;
}

extern dir_t root_dir;


int
config_entry_paramter(const char *dir, const char *ent, const char *value){
	return 0;
}
#endif
