#ifndef SINLUT_H
#define SINLUT_H

#include "kbox.h"

// 10-bit sinlut.
extern const s32 sinlut[0x0800] ALIGN(2);
#define SIN_SHF (16)

INLINE s32 lu_sin(u32 a)
{ return sinlut[(a>>5)&0x7FF]; }
INLINE s32 lu_cos(u32 a)
{ return lu_sin(a + 0x4000); }

#endif
