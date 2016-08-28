#ifndef _PICOS_TYPES_H
#define _PICOS_TYPES_H

#include <stdint.h>

typedef uint8_t                     byte;
typedef unsigned long               ulong;
typedef unsigned long long          ullong;

typedef struct Region
{
    int                 id;
    size_t              size;
    byte                **ptr_to_origin;
    byte                *image;
    struct Region       *next;
} Region;

struct {
    bool            initialised;
    int             verbosity;

    int             curr_region_id;
    int             chkpt_counter;
    ullong          tot_size;
    Region          *region_list_head;
    Region          *region_list_end;

    bool            do_ddump;
    int             ddump_interval;

    int             ddump_fd;
    byte            *ddump_file_ptr, *ddump_curr_ptr;
    ullong          ddump_file_bytes;
    char            *ddump_filename;
} picos = {0};

#endif /*_PICOS_TYPES_H*/