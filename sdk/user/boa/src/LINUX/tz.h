#ifndef TZ_H
#define TZ_H
typedef enum {
	FOR_WEB = 0,
	FOR_CLI
} tz_chk_t;
extern const size_t nr_tz;
const char *get_tz_utc_offset(unsigned int index);
const char *get_tz_string(unsigned int index, unsigned char dst_enabled);
inline const char *get_tz_location(unsigned int index, tz_chk_t tz_chk);
#endif
