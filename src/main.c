/* libs */
#include "kbox.h"
#include "sinlut.h"
#include "vblank.h"
#include "img.h"
#include "cd.h"

#include <machine.h>
#define _SPR2_
#include <sega_spr.h>
#include <sega_scl.h> 
#include <sega_mth.h>
#include <sega_cdc.h>
#include <sega_sys.h>
#include <sega_gfs.h>
#include <sega_int.h>
#include <sega_xpt.h>
#include <sega_gfs.h>

#include <sega_dma.h>
#include <string.h>

/* sbl defines */
#define WIDTH (320)
#define HEIGHT (224)

#define MAX_FILE (100)
#define READSECT (50)

#define COMMAND_MAX    (0x400)
#define GOUR_TBL_MAX   (0x200)
#define LOOKUP_TBL_MAX (0x200)
#define CHAR_MAX       (0x200)
#define DRAW_PRTY_MAX  (0x100)

// NOTE!!!!
// READ!!!!
// to compile with makefile, use `make OBJFMT=coff`

// vars
typedef struct suwa_mem
{
	u32 time;
	u16 width,height,clrmode;
	u16 buffer[0x2000];
} PACKED suwa_mem;
static suwa_mem *suwako = (suwa_mem*)WRAM_LO;

// static
SPR_2DefineWork(
	spr_work2d,COMMAND_MAX,GOUR_TBL_MAX,LOOKUP_TBL_MAX,CHAR_MAX,DRAW_PRTY_MAX
);

int main(void)
{
  /*---- system init ------------------------------------------------*/
  // >> CD
	cd_init();
	// >> SH2
	sh2_dma_init();
	*DMA_REG_OR = DMA_DME | DMA_PR;
	// >> VDP
  SCL_Vdp2Init();
	SCL_SetDisplayMode(SCL_NON_INTER,SCL_224LINE,SCL_NORMAL_A);
	SCL_SetPriority(
		SCL_SP0|SCL_SP1|SCL_SP2|SCL_SP3|SCL_SP4|SCL_SP5|SCL_SP6|SCL_SP7,7
	);
	SCL_SetSpriteMode(SCL_TYPE1,SCL_MIX,SCL_SP_WINDOW);
	SPR_2Initial(&spr_work2d);
  SetVblank();
	
	SCL_SetFrameInterval(0);
	SCL_SetColRamMode(SCL_CRM15_2048);
	SCL_SetVramConfig(
		&(SclVramConfig){.vramModeB=ON,.vramB0=0,.vramB1=1}
	);
	/*---- vdp1 test --------------------------------------------------*/
	// test image
	{
		img_load(IMG_TESTTEX0);
		img_load(IMG_ARCFONT);
	}
	for(u32 i=0; i<0x10; i++) ((CLR16*)VDP2_CRAM)[i] = RGB16(31,0,0);
	((CLR16*)VDP2_CRAM)[0xD] = RGB16(198>>3,113>>3,178>>3);
	((CLR16*)VDP2_CRAM)[0xE] = RGB16(242>>3,0,250>>3);
	((CLR16*)VDP2_CRAM)[0xF] = RGB16(31,31,31);
	suwako->time++;
	
	for(;;)
	{
		SPR_2OpenCommand(SPR_2DRAW_PRTY_OFF);
		{
			/* clipping */
			SPR_2SysClip(0, &(XyInt){319,223});
			XyInt usrclip[2] = {{0,0},{100,200}};
			SPR_2UserClip(0,usrclip);
			/* text drawin */
			u32 fontaddr = IMG_CHAR_LUT[IMG_ARCFONT]<<3;
			const char txt[] = "THIS IS A SAMPLE FONT!\nIT TOOK WAY TOO LONG FOR ME TO\nFIGURE OUT HOW TO DISPLAY\nTHIS TEXT.\n\n\nalso, i can load images\nfrom the CD now.";
			VDP1_CMD base_cmd = {
				.cmdpmod = CMDPMOD_CLR(VDP1_CLRMODE_PAL256),
				.cmdsize = VDP1_CMDSIZE(8,8)
			};
			u32 chr_len = (u32)strlen(txt);
			u32 x,y;
			x = y = 0;
			for(u32 i=0; i<chr_len; i++)
			{
				char c = txt[i];
				if(c == '\n')
				{ x=0; y++; }
				else {
					VDP1_CMD cmd = base_cmd;
					cmd.cmdxa = x*8;
					cmd.cmdya = y*8;
					cmd.cmdsrca = (fontaddr + (c<<6))>>3;
					SPR_2Cmd(0,&cmd);
					x++;
				}
			}
			/* line drawin */
			XyInt pos[4];
			u32 len = 64;
			u32 ang_offset = suwako->time<<4;
			for(u32 i=0;i<4;i++) {
				u32 ang = ang_offset + (0x4000*i);
				pos[i].x = (u32)(len*lu_cos(ang))>>16;
				pos[i].y = (u32)(len*lu_sin(ang))>>16;
				pos[i].x+=(WIDTH>>1);pos[i].y+=(HEIGHT>>1);
			}
			
			SPR_2DistSpr(0,0,CMDPMOD_MESH*1,RGB16(31,0,0),IMG_TESTTEX0,
				pos,NO_GOUR
			);
		}
		SPR_2CloseCommand();
		
		suwako->time++;
		SCL_DisplayFrame();
	}
}
