int rtk_nv_erase(int num);
int rtk_nv_write(int num, unsigned int dst, unsigned char *src, unsigned int count);
int rtk_env_get(const char *name, char *buf, unsigned int buflen);
int rtk_env_set(const char *name, const char *value);