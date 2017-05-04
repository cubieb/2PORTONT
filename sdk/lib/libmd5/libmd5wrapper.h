#ifndef LIBMD5WRAPPER_H
#define LIBMD5WRAPPER_H

#include <libmd5.h>

//struct wrapper
#define MD5Context	LibMD5Context
#define MD5_CTX		LibMD5_CTX
#define EVP_MD_CTX	LibMD5_CTX

//function wrapper
#define MD5Init		LibMD5Init
#define MD5Update	LibMD5Update
#define MD5Final	LibMD5Final

//function wrapper
#define MD5_Init	LibMD5Init
#define MD5_Update	LibMD5Update
#define MD5_Final	LibMD5Final

#endif /* !LIBMD5WRAPPER_H */
