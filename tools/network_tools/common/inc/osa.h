

#ifndef _OSA_H_
#define _OSA_H_

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define OSA_DEBUG_MODE // enable OSA_printf, OSA_assert
#define OSA_DEBUG_FILE // enable printf's during OSA_fileXxxx
#define OSA_PRF_ENABLE // enable profiling APIs

#define OSA_SOK      0  ///< Status : OK
#define OSA_EFAIL   -1  ///< Status : Generic error

#ifndef _TI_STD_TYPES
#define _TI_STD_TYPES

#ifndef TRUE

typedef int Bool;                 ///< Boolean type

#define TRUE		((Bool) 1)        ///< Boolean value : TRUE
#define FALSE		((Bool) 0)        ///< Boolean value : FALSE

#endif

/* unsigned quantities */
typedef unsigned long long Uint64;      ///< Unsigned 64-bit integer
typedef unsigned int Uint32;            ///< Unsigned 32-bit integer
typedef unsigned short Uint16;          ///< Unsigned 16-bit integer
typedef unsigned char Uint8;            ///< Unsigned  8-bit integer

typedef unsigned long long UInt64;      ///< Unsigned 64-bit integer
typedef unsigned int UInt32;            ///< Unsigned 32-bit integer
typedef unsigned short UInt16;          ///< Unsigned 16-bit integer
typedef unsigned char UInt8;            ///< Unsigned  8-bit integer

/* signed quantities */
typedef long long Int64;               ///< Signed 64-bit integer
typedef int Int32;                     ///< Signed 32-bit integer
typedef short Int16;                   ///< Signed 16-bit integer
typedef char Int8;                     ///< Signed  8-bit integer

#endif /* _TI_STD_TYPES */

#ifndef KB
#define KB ((Uint32)1024)
#endif

#ifndef MB
#define MB (KB*KB)
#endif

#define OSA_TIMEOUT_NONE        ((Uint32) 0)  // no timeout
#define OSA_TIMEOUT_FOREVER     ((Uint32)-1)  // wait forever

#define OSA_memAlloc(size)      (void*)malloc((size))
#define OSA_memFree(ptr)        free(ptr)

#define OSA_align(value, align)   ((( (value) + ( (align) - 1 ) ) / (align) ) * (align) )

#define OSA_floor(value, align)   (( (value) / (align) ) * (align) )
#define OSA_ceil(value, align)    OSA_align(value, align)

#include <osa_debug.h>

Uint32 OSA_getCurTimeInMsec();
void   OSA_waitMsecs(Uint32 msecs);

int xstrtoi(char *hex);

#endif /* _OSA_H_ */



