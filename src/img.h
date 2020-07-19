#ifndef IMG_H
#define IMG_H

#include <sega_gfs.h>
#include "kbox.h"
#include "cd.h"

// inside ram lies a lookup table, where every
// value is a pointer to the image's graphics,
// and every index is the image id.
#define IMG_CHAR_LEN (0x200)

typedef enum IMG_LIST {
	IMG_TESTTEX0,
	IMG_ARCFONT
} IMG_LIST;

extern const char *IMG_NAME_LUT[];
extern u32 IMG_CHAR_LUT[IMG_CHAR_LEN];

INLINE void img_fail()
{
	asm( // error with BAD LOAD
		"0:\t\n" "MOV %[inp1],R0\t\n" "MOV %[inp2],R1\t\n" "BRA 0b\t\n"
		:: [inp1]"r"(0x00000404),[inp2]"r"(0x0BAD10AD)
	);
	while(1);
}

extern void img_load(u32 imgid);

#endif
