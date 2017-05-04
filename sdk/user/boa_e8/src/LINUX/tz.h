#ifndef TZ_H
#define TZ_H
extern const size_t nr_tz;
const char *get_tz_utc_offset(unsigned int index);
const char *get_tz_string(unsigned int index, unsigned char dst_enabled);
inline const char *get_tz_location(unsigned int index);
#endif
