#include "img.h"

/*---- LUTs ---------------------------------------------------------*/
const char *IMG_NAME_LUT[] = {
	[IMG_TESTTEX0] = "TESTTEX0.BIN",
	[IMG_ARCFONT] = "ARCFONT.BIN"
};
u32 IMG_CHAR_LUT[IMG_CHAR_LEN];

/*---- functions ----------------------------------------------------*/
void img_load(u32 imgid)
{
	/* load file for the header */
	const char *fname = IMG_NAME_LUT[imgid];
	GfsHn imgfile = GFS_Open(GFS_NameToId((char*)fname));
	if(imgfile == NULL) img_fail();
	/* read the header first */
	u16 imgheader[3] ALIGN(2); {
		GFS_Fread(imgfile,1,imgheader,sizeof(u16)*3);
		GFS_Close(imgfile);
	}
	/* get header info */
	u32 width = imgheader[0];
	u32 height = imgheader[1];
	u32 clrmode = imgheader[2];
	/* write to vram */
	u8 buffer[KBSIZE(16)] ALIGN(2);
	cd_load(fname,buffer,KBSIZE(16));
	// add to char lut
	SPR_2SetChar(
		imgid,CMDPMOD_CLR(clrmode),clrmode,
		width,height,
		&buffer[6]
	);
	
	IMG_CHAR_LUT[imgid] = SPR_2CharNoToVram(imgid);
}
