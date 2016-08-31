#ifndef _PICOS_TYPES_H
#define _PICOS_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t                     byte;
typedef unsigned long               ulong;
typedef unsigned long long          ullong;
typedef uint32_t                    crc32;

typedef struct picos_region
{
    int                     id;
    size_t                  size;
    bool                    is_ptr;
    byte                    **ptr_to_origin;
    byte                    *image;
    struct picos_region     *next;
} picos_region;

typedef struct picos_page
{
    int                     id;
    byte                    *start;
    byte                    *end;
    int                     flags;
    struct picos_page       *next;
} picos_page;

#endif /*_PICOS_TYPES_H*/