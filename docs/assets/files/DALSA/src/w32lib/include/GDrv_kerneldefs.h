/******************************************************************************
GDrv_kerneldefs.h			 									(c) Coreco inc. 2004

Description:
   Linux kernel definitions.

Platform:
	-Linux kernel level.

History:
   1.00 August 23, 2004, parhug

$Log: GDrv_kerneldefs.h $
Revision 1.4  2007/06/14 16:09:13  parhug
config.h not required in kernel 2.6
Revision 1.3  2006/01/10 13:35:09  PARHUG
Update for 2.6 kernel.
Revision 1.2  2004/09/14 14:49:04  BOUERI
- Added definition for Linux RedHat distribution compatibility.
Revision 1.1  2004/08/31 14:40:51  parhug
Initial revision

*******************************************************************************/

#ifndef _GDRV_KERNELDEFS_H_
#define _GDRV_KERNELDEFS_H_

#include <linux/version.h>

#ifdef __KERNEL__           
// Linux kernel development includes.
#if LINUX_VERSION_CODE < 0x020600
	#include <linux/config.h>
#endif

#if defined (CONFIG_SMP) && !defined (__SMP__)
#define __SMP__
#endif

#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif

#if defined(MODVERSIONS) && (LINUX_VERSION_CODE < 0x020600)
#include <linux/modversions.h>
#endif

#if defined(MODULE)
#include <linux/module.h>
#endif

#include <linux/kernel.h>   /* printk() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
//????#include <linux/pci.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
#include <asm/switch_to.h>     /* cli(), *_flags */
#else
#include <asm/system.h>     /* cli(), *_flags */
#endif
#include <asm/uaccess.h>    /* VERIFY_READ, VERIFY_WRITE */

#include <linux/string.h>
#if LINUX_VERSION_CODE < 0x02040f	/* 2.4.15 */
#  include <linux/malloc.h>			/* kmalloc */
#else
#  include <linux/slab.h>				/* kmalloc */
#endif
#include <linux/vmalloc.h>          /* vmalloc, vfree, etc              */

#include <linux/list.h>             /* circular linked list             */
#include <linux/stddef.h>           /* NULL, offsetof                   */

#include <linux/wait.h>             /* wait queues                      */
#if LINUX_VERSION_CODE < 0x020600
   #include <linux/tqueue.h>           /* struct tq_struct                 */
   #include <linux/wrapper.h>          /* mem_map_reserve                  */
#endif
#include <linux/poll.h>             /* poll_wait                        */
#include <linux/delay.h>            /* mdelay, udelay                   */

#include <linux/interrupt.h>        /* mark_bh, init_bh, remove_bh      */
#include <linux/timer.h>

#include <asm/io.h>                 /* ioremap, virt_to_phys            */
#include <asm/page.h>               /* PAGE_OFFSET                      */
#include <asm/pgtable.h>            /* pte bit definitions              */

#include <linux/spinlock.h>
#if LINUX_VERSION_CODE < 0x02061A
	#include <asm/semaphore.h>
#else
	#include <linux/semaphore.h>
#endif
#include <linux/highmem.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#include <linux/seq_file.h>
#endif

#ifndef pte_offset
#define pte_offset pte_offset_kernel
#endif

#ifndef VM_RESERVED
#define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif

/* Debug printing */
#ifdef DEBUG_PRINT
#define DBGPRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#define KdPrint(_x_) printk _x_
#else
#define DBGPRINT(fmt, args...)
#define KdPrint(_x_) 
#endif


#endif
#endif	// _GDRV_KERNELDEFS_H_

