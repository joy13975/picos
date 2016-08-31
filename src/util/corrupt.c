#include <stdlib.h>

#if !defined(FLIP_SEED)
#define FLIP_SEED 0xdeadbeef
#endif

#include "../picos.h"

unsigned int flip_seed = FLIP_SEED;

ulong rrand(const ulong rmax)
{
    return rand_r(get_flip_seed_addr()) % rmax;
}

void flip_bit(void *dataPtr, const size_t size) {
    const size_t byteId = rrand(size);
    const size_t nshifts = rrand(8);

    // printf("Flipping byte ID: %d, bit mask: %d\n", byteId, nshifts);
    // printf("Byte before: %x\n", ((BasePtr) dataPtr)[byteId]);
    ((byte *) dataPtr)[byteId] ^= (0x1U << (nshifts));
    // printf("Byte after: %x\n", ((BasePtr) dataPtr)[byteId]);
}

void set_flip_seed(unsigned int s) {
    flip_seed = s;
}

unsigned int* get_flip_seed_addr() {
	return &flip_seed;
}

void reset_flip_seed() {
	flip_seed = FLIP_SEED;
}

