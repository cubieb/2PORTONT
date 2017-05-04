/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI generic OS type define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI generic OS type define
 */

#ifndef __GOSTYPE_H__
#define __GOSTYPE_H__

#ifdef  __cplusplus
extern "C" {
#endif


#define _BIG_ENDIAN     1234

#define _BYTE_ORDER     _BIG_ENDIAN


typedef char  CHAR;
typedef char  INT8;
typedef short INT16;
typedef int   INT32;

typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
#ifndef BOOL
typedef UINT32         BOOL;
#endif
#ifndef STATUS
typedef INT32          STATUS;
#endif
typedef unsigned long  ulong_t;

typedef struct {
    UINT32 high;
    UINT32 low;
} UINT64;

typedef void *  (*FUNCPTR)(void *);
typedef void    (*VOIDFUNCPTR)(void *);
typedef UINT32  (*RW_FUNC)(UINT32, UINT32, UINT32, UINT32);

#if !defined(WAIT_FOREVER)
#define WAIT_FOREVER  (0xffffffff)
#endif

#define STD_IN     0
#define STD_OUT    1
#define STD_ERR    2


#if !defined(FALSE) || (FALSE!=0)
#define FALSE       ((BOOL)0)
#endif

#if !defined(TRUE) || (TRUE!=1)
#define TRUE        ((BOOL)1)
#endif

#define ERROR -1
#define OK    0


typedef enum
{
    GOS_OK = 0,             // OK
    GOS_FAIL,               // Common failure
    GOS_ERR_PARAM,          // Parameter illegal, general error
    GOS_ERR_NOTSUPPORT,     // Not Supported, it should supported in future
    GOS_ERR_DISABLE,        // Object is disabled
    GOS_ERR_STATE,          // unacceptable state
    GOS_ERR_EVENT,          // unacceptable event
    GOS_ERR_OVERFLOW,       // Requests are more than the max capacity
    GOS_ERR_HARDWARE,       // hardware handle failed
    GOS_ERR_FPGA_DOWNLOAD,  // FPGA download failed
    GOS_ERR_UNREACHABLE,    // Unreachable case, if so, there are some sequence error or design fail
    GOS_ERR_SOCKFAIL,       // Failed to read/write on the socket
    GOS_ERR_INVALID_CHAR,   // Receive a invalid char
    GOS_ERR_USER_CANCEL,    // User canceled
    GOS_ERR_INVALID_INPUT,  // Invalid input from user
    GOS_ERR_NOT_FOUND,      // Object not found
    GOS_ERR_SYS_CALL_FAIL,  // System call returned failure,
    GOS_ERR_USER_EXIT,      // User exit the program
    GOS_ERR_DUPLICATED,     // Object or operation duplicated
    GOS_ERR_CRC,            // CRC check failed
    GOS_ERR_NOT_INIT,       // Not initialized
    GOS_ERR_UNKNOWN = 255
}GOS_ERROR_CODE;

#define GOS_ASSERT(cond) if (!(cond)) \
                      {\
                        printf("\r\nAssertion Failed:"#cond", file %s, line %d, pid %d\r\n", __FILE__, __LINE__, getpid()); \
                        abort();\
                      }\
                      else {}

#define GOS_NELEM(array)   (sizeof(array)/sizeof(array[0]))

#ifdef  __cplusplus
}
#endif

#endif
