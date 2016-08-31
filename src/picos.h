#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#ifndef _PICOS_H_
#define _PICOS_H_

#include <stdbool.h>

#include "picos_types.h"

void picos_init();

picos_page *picos_get_xpages();

void picos_register_primitive(void *ptr_to_prim, size_t size);

void picos_register_ptr(void **ptr_to_ptr_to_data, size_t size);

void picos_enable_disk_dump(const char* prefix, int every_n_chkpts);

void picos_checkpoint_now();

void picos_warm_recover();

void picos_cold_recover(const char* prefix, long from_pid);

void picos_finalise();

/*support functions*/

/*checksum.c*/

crc32 checksum_new(void const *restrict pData, size_t size);

void checksum_cascade(crc32 *restrict crc, void const *restrict pData, size_t size);

/*corrupt.c*/

void set_flip_seed(unsigned int s);

unsigned int* get_flip_seed_addr();

void reset_flip_seed();

ulong rrand(const ulong rmax);

void flip_bit(void *dataPtr, const size_t size);

/*bits.c*/

void print_bits(byte *ptr, size_t size);

/*vma-iter.c*/

/* Bit mask for the FLAGS parameter of a vma_iterate callback function.  */
#define PAGE_HAS_R(f) ((f >> 0) & 0x1U)
#define PAGE_HAS_W(f) ((f >> 1) & 0x1U)
#define PAGE_HAS_X(f) ((f >> 2) & 0x1U)

typedef int (*vma_iterate_callback_fn) (void *data,
                                        uintptr_t start, uintptr_t end,
                                        unsigned int flags);

/* Iterate over the virtual memory areas of the current process.
   If such iteration is supported, the callback is called once for every
   virtual memory area, in ascending order, with the following arguments:
     - DATA is the same argument as passed to vma_iterate.
     - START is the address of the first byte in the area, page-aligned.
     - END is the address of the last byte in the area plus 1, page-aligned.
       Note that it may be 0 for the last area in the address space.
     - FLAGS is a combination of the VMA_* bits.
   If the callback returns 0, the iteration continues.  If it returns 1,
   the iteration terminates prematurely.
   This function may open file descriptors, but does not call malloc().  */
extern void vma_iterate (vma_iterate_callback_fn callback, void *data);

/* The macro VMA_ITERATE_SUPPORTED indicates that vma_iterate is supported on
   this platform.
   Note that even when this macro is defined, vma_iterate() may still fail to
   find any virtual memory area, for example if /proc is not mounted.  */
#if defined __linux__ || defined __FreeBSD__ || defined __NetBSD__ || defined __sgi || defined __osf__ || (defined __APPLE__ && defined __MACH__) || (defined _WIN32 || defined __WIN32__) || defined __CYGWIN__ || defined __BEOS__ || defined __HAIKU__ || HAVE_MQUERY
# define VMA_ITERATE_SUPPORTED 1
#endif



#endif //_PICOS_H_

