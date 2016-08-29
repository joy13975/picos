#ifndef _PICOS_PRIVATE_H_
#define _PICOS_PRIVATE_H_

#include <stdbool.h>
#include <time.h>

#if     defined(USE_MPI)
#include    <mpi.h>
#define         IF_MPI(stm) stm
#define         IFN_MPI(stm)
#else   /*USE_MPI*/
#define         IF_MPI(stm)
#define         IFN_MPI(stm) stm
#endif  /*USE_MPI*/

#include "picos_types.h"




#define LIKE_STM(stms)          do{stms}while(0)

#define log(...) \
    LIKE_STM(PRINT_PICOS_TAG(); printf(__VA_ARGS__);)

#define dbg(...) \
    LIKE_STM(if(picos.verbosity > 0) log(__VA_ARGS__); )

#define elog(...) \
    LIKE_STM(PRINT_PICOS_TAG(); fprintf(stderr, __VA_ARGS__); )

#define die(fmt, ...) \
    LIKE_STM(elog("*****[Fatal error]*****\n"); elog(fmt, ##__VA_ARGS__); elog("\nFailure at: %s:%d\n", __FILE__, __LINE__); free_resources(); exit(1); )

#define check_init(func_name) \
    if (!picos.initialised) die("Must call picos_init() before %s()\n", func_name)



void get_time_string(char **buffer_ptr, const char* fmt);

inline void PRINT_PICOS_TAG()
{
    char *buffer;
    get_time_string(&buffer, "%Y-%m-%d %H:%M:%S");
    printf("[picos %s] ", buffer);
    free(buffer);
}

void signal_handler(int sig);

double get_unix_ms();

ulong ez_strtoul(const char *restrict str);

void format_ddump_filename(char **dst, const char *prefix, long cold_rcvr_pid);

void ddump_flush();

void free_resources();




#endif //_PICOS_PRIVATE_H_