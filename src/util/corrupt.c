#if !defined(FLIP_SEED)
#define FLIP_SEED 0xdeadbeef
#endif

unsigned int flip_seed = FLIP_SEED;

void set_flip_seed(unsigned int s) {
    flip_seed = s;
}

unsigned int* get_flip_seed_addr() {
	return &flip_seed;
}

void reset_flip_seed() {
	flip_seed = FLIP_SEED;
}

