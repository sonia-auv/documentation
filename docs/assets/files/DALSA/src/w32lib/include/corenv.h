/******************************************************************************
CORENV.H																	(c) Coreco inc. 1997

Description:
   Environment description constants. 

*******************************************************************************/   

#ifndef _CORENV_H_
#define _CORENV_H_

#define COR_LINUX	1

// Linux needs these defined to prevent compiler warnings.
#define COR_WIN32	0
#define COR_WIN64 0
#define COR_NTKERNEL	0

#define COR_95KERNEL	0
#define COR_C60		0
#define COR_I960	0
#define COR_IOP321	0
#define COR_C165	0
#define COR_POWERPC	0
#define COR_TI_DM642	0
#define COR_ARMV4I	0

// do not define this !!!  #define _MSC_VER	0

#define COR_WIN64 0
#define COR_NIOS2_GCC 0
#define COR_CYGWIN 0
#define COR_MSP430_TI 0
#define COR_ATMEL_AT91 0

#endif // _CORENV_H_

