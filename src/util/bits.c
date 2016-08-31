#include <stdio.h>

#include "../picos_types.h"

void print_bits(byte *ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        for (short j = 7; j >= 0; j--)
        {
            printf("%d", (ptr[i] >> j) & 0x1U);
            if (j == 4)
                printf(".");
        }
        if (i < size - 1)
            printf(",");
    }
}