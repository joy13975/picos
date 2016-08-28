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

#include "libpicos.h"

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
    int *a_ptr = &a, *i_ptr = &i;

    picos_init();
    picos_register((void**) &(a_ptr), sizeof(int));
    picos_register((void**) &(i_ptr), sizeof(int));
    picos_enable_disk_dump("simple-chkpt", 1);

    log("Will do %d iterations\n", lim);

    for (i = 0; i < lim; i++)
    {
        //fail half way
        bool fail = (j == lim - (lim / 2));
        log("Before itr %d: a=%d, b=%d\n", i, a, b);

        IF_MPI(
        {
            fflush(stdout);
            MPI_Barrier(MPI_COMM_WORLD);
        });

        a *= (1 + i);
        b *= (1 + i);

        if (fail)
        {
            log("Before corrupt: a=%d, b=%d\n", a, b);
            b = (a ^= 0x1U);
            log("After corrupt: a=%d, b=%d\n", a, b);

            picos_warm_recover();
            log("Recovered - now redo iteration\n");
        }
        else
        {
            picos_checkpoint_now();

            // picos_cold_recover("simple-chkpt", 66635);
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