#ifndef _PICOSU_H_
#define _PICOSU_H_

#include "picos_types.h"



/*Checksum functions*/

crc32 checksum_new(void const *restrict pData, size_t size);

void checksum_cascade(crc32 *restrict crc, void const *restrict pData, size_t size);



/*Bit flip functions*/

void set_flip_seed(unsigned int s);

unsigned int* get_flip_seed_addr();

void reset_flip_seed();

inline ulong jrand(const ulong rmax)
{
    return rand_r(get_flip_seed_addr()) % rmax;
}

inline flip_bit(void *dataPtr, const size_t size) {
    const size_t byteId = jrand(size);
    const size_t nshifts = jrand(8);

    // printf("Flipping byte ID: %d, bit mask: %d\n", byteId, nshifts);
    // printf("Byte before: %x\n", ((BasePtr) dataPtr)[byteId]);
    ((byte *) dataPtr)[byteId] ^= (0x1U << (nshifts));
    // printf("Byte after: %x\n", ((BasePtr) dataPtr)[byteId]);
}

#endif /*_PICOSU_H_*/