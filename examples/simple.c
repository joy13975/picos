#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#if defined(USE_MPI)
#include <mpi.h>
#define IF_MPI(stm) stm
#else
#define IF_MPI(stm)
#endif

#include "picos.h"

#define log(...) \
    do {printf(TAG, my_rank); printf(__VA_ARGS__); } while(0)

const char *TAG = "[Simple Example - R%d] ";

int main(int argc, char *argv[])
{
    int my_rank = 0, tot_procs = 1;
    IF_MPI(
    {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &tot_procs);
    }
    );

    log("PID: %ld, total %d ranks\n", (long) getpid(), tot_procs);
    log("Picos example initialised\n");


    //example variables: a is protected while b is not
    int a = 1, b = 1;
    const int lim = 5;
    int i = 0, j = 0;

    picos_init();
    // picos_register_text();
    // while(true){};
    picos_register_primitive(&a, sizeof(int));
    picos_register_primitive(&i, sizeof(int));
    picos_enable_disk_dump("simple-chkpt", 1);

    log("Will do %d iterations\n", lim);

    for (i = 0; i < lim; i++)
    {
        //fail half way
        log("Before itr %d: a=%d, b=%d\n", i, a, b);

        IF_MPI(
        {
            fflush(stdout);
            MPI_Barrier(MPI_COMM_WORLD);
        });

        a *= (1 + i);
        b *= (1 + i);

        crc32 oldcrc_a = checksum_new(&a, sizeof(a)), oldcrc_b = checksum_new(&b, sizeof(b));
        bool fail = (j == lim - (lim / 2));
        if (fail)
        {
            log("Before corrupt: a=%d, b=%d\n", a, b);

            //bit flip injection with picos utility
            flip_bit(&b, sizeof(b));

            log("After corrupt: a=%d, b=%d\n", a, b);
        }

        if (checksum_new(&a, sizeof(a)) != oldcrc_a ||
            checksum_new(&b, sizeof(b)) != oldcrc_b)
        {
            log("Faul detected - requesting recovery\n");

            picos_warm_recover();

            log("Recovery completed\n");
        }
        else
        {
            picos_checkpoint_now();

            //recover from checkpoint file given a PID (master PID in case of MPI)
            // picos_cold_recover("simple-chkpt", 65502);
        }
        log("After itr %d: a=%d, b=%d\n", i, a, b);
        j++;

        IF_MPI(
        {
            fflush(stdout);
            MPI_Barrier(MPI_COMM_WORLD);
        });

    }

    log("Protected(a)=%d, Unprotected(b)=%d\n", a, b);

    picos_finalise();

    IF_MPI(
    {
        MPI_Finalize();
    }
    );

    return 0;
}
