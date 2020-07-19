#ifndef SINLUT_H
#define SINLUT_H

#include "kbox.h"

// 10-bit sinlut.
extern const s16 sinlut[0x0800] ALIGN(2);

// 0x8000

INLINE s32 lu_sin(u32 a)
{ return (s32)sinlut[(a>>5)&0x7FF]; }
INLINE s32 lu_cos(u32 a)
{ return lu_sin(a + 0x4000); }

#endif
