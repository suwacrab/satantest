/* libs */
#include "kbox.h"
#include "sinlut.h"
#include "vblank.h"
#include "img.h"
#include "cd.h"
#include "mat4.h"

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
#include <stdio.h>
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
suwa_mem *suwako = (suwa_mem*)WRAM_LO;

void draw_txt();
void draw_rotquad();
void draw_3dquad();

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
			/* line drawin */
			draw_rotquad();
			/* 3d drawin */
			draw_3dquad();
			/* text drawin */
			draw_txt();
		}
		SPR_2CloseCommand();
		
		suwako->time++;
		SCL_DisplayFrame();
	}
}

void draw_txt()
{
	u32 fontaddr = IMG_CHAR_LUT[IMG_ARCFONT]<<3;
	char txt[0x0100] ALIGN(2);
	sprintf(txt,"ARCFONT TEST\n\nnow with rotation\n\ntime: %08X",suwako->time);
	VDP1_CMD base_cmd = {
		.cmdpmod = CMDPMOD_CLR(VDP1_CLRMODE_PAL256),
		.cmdsize = VDP1_CMDSIZE(8,8)
	};
	u32 len = (u32)strlen(txt);
	u32 x = 0;
	u32 y = 0;
	for(u32 i=0; i<len; i++)
	{
		char c = txt[i];
		if(c == '\n')
		{ x=0; y++; }
		else {
			VDP1_CMD cmd = base_cmd;
			cmd.cmdxa = x*8;
			cmd.cmdya = y*8;
			cmd.cmdsrca = (fontaddr + (c<<6))>>3;
			SPR_2Cmd(0,(void*)&cmd);
			x++;
		}
	}
}

void draw_rotquad()
{
	XyInt pos[4];
	s32 len = 32; //mulf32(32,lu_cos(suwako->time<<4));
	u32 ang_offset = suwako->time<<4;
	for(u32 i=0;i<4;i++) {
		u32 ang = ang_offset + (0x4000*i);
		pos[i].x = mulf32(64+len,lu_cos(ang));
		pos[i].y = mulf32(64+len,lu_sin(ang));
		pos[i].x+=(WIDTH>>1);
		pos[i].y+=(HEIGHT>>1);
	}
	
	SPR_2DistSpr(0,0,CMDPMOD_MESH*0,RGB16(31,0,0),IMG_TESTTEX0,
		pos,NO_GOUR
	);
}

void draw_3dquad()
{
	vec3_f32 quad_vrt[4*4] = {
		// front
		{s32tof32(-1),s32tof32(-1),s32tof32(1)},
		{s32tof32(1),s32tof32(-1),s32tof32(1)},
		{s32tof32(1),s32tof32(1),s32tof32(1)},
		{s32tof32(-1),s32tof32(1),s32tof32(1)},
		// right (NO GOOD)
		{s32tof32(1),s32tof32(-1),s32tof32(1)}, // top left
		{s32tof32(1),s32tof32(-1),s32tof32(-1)}, // top right
		{s32tof32(1),s32tof32(1),s32tof32(-1)}, // botom right
		{s32tof32(1),s32tof32(1),s32tof32(1)}, // bottom left
		// back
		{s32tof32(1),s32tof32(-1),s32tof32(-1)}, // top left
		{s32tof32(-1),s32tof32(-1),s32tof32(-1)}, // top right
		{s32tof32(-1),s32tof32(1),s32tof32(-1)}, // botom right
		{s32tof32(1),s32tof32(1),s32tof32(-1)}, // bottom left
		// left ( NO GOOD )
		{s32tof32(-1),s32tof32(-1),s32tof32(-1)}, // top left
		{s32tof32(-1),s32tof32(-1),s32tof32(1)}, // top right
		{s32tof32(-1),s32tof32(1),s32tof32(1)}, // botom right
		{s32tof32(-1),s32tof32(1),s32tof32(-1)}, // bottom left
	};
	// Z-buffering
	VDP1_CMD cmdbuf[0x0200];
	u16 zbuf[0x0200];
	for(u32 q=0; q<4; q++)
	{

	}
	// rotate the quad
	mat4 rotmatrix = mat4_ang(suwako->time<<2,suwako->time<<4,0);
	for(u32 q=0; q<4; q++)
	{
		// world space
		vec3_f32 quad_rot[4];
		for(u32 v=0; v<4; v++)
		{
			quad_rot[v] = mat4_mulV(&rotmatrix,&quad_vrt[v+(q*4)]);
			quad_rot[v].z += 0x20000;
		}
		// to screen space
		XyInt quad_scrn[4];
		for(u32 v=0; v<4; v++)
		{
			f32 zscale = s32tof32(0x20)/quad_rot[v].z;
			quad_scrn[v].x = (WIDTH/2)  + (mulf32(quad_rot[v].x,zscale));
			quad_scrn[v].y = (HEIGHT/2) + (mulf32(quad_rot[v].y,zscale));
		}
		
		// draw
		VDP1_CMD sprcmd = {
			.cmdctrl = comm_distspr,
			.cmdpmod = CMDPMOD_CLR(VDP1_CLRMODE_RGB),
			.cmdsize = VDP1_CMDSIZE(32,32),
			.cmdxa = quad_scrn[0].x, .cmdya=quad_scrn[0].y,
			.cmdxb = quad_scrn[1].x, .cmdyb=quad_scrn[1].y,
			.cmdxc = quad_scrn[2].x, .cmdyc=quad_scrn[2].y,
			.cmdxd = quad_scrn[3].x, .cmdyd=quad_scrn[3].y,
			.cmdsrca = IMG_CHAR_LUT[IMG_TESTTEX0]
		};
		SPR_2Cmd(0,(void*)&sprcmd);	
	}
}

