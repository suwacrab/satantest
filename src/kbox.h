/********************************************************************
 *                   KAPPA BOX for SEGA SATURN
 *           a header useful for sega saturn developers 
 *                    BY suwacrab 2020-21-06
 *                   REVISION DATE 2020-23-06
 ********************************************************************/
/*                             TODO
 * >           make function to load data from the CD
 * >           add VDP1 interface
*********************************************************************/
/*                             LOG
 * >           2020-21-06: development begin: DMA
 *             and type definitions!
 * >           2020-24-06: CD Block start... god
 * >           2020-25-06: VDP1 FINALLY THANK FUCK
*********************************************************************/
#ifndef KBOX_H
#define KBOX_H


/*----- type defines ------------------------------------------------*/
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#define KBSIZE(n) (0x400*(n))
#define MBSIZE(n) (KBSIZE(0x400)*n)

#define INLINE static inline
#define PACKED   __attribute__((packed))
#define ALIGN(n) __attribute__((aligned(n)))

/*---- color defines ------------------------------------------------*/
typedef u16 CLR16; // 16-bit color in format 1BBBBBGGGGGRRRRR
typedef struct CLR24 { u8 r,g,b; } PACKED CLR24; // 24-bit color
INLINE CLR16 RGB16(u32 r,u32 g,u32 b)
{ return r | (g<<5) | (b<<10) | 0x8000; }

/*----- RAM defines -------------------------------------------------*/
#define WRAM_LO  ((volatile u8*)(0x20200000))
#define VDP1_MEM ((volatile u8*)(0x25C00000))
#define VDP2_MEM ((volatile u8*)(0x25E00000))
#define SCU_MEM  ((volatile u8*)(0x25FE0000))
#define WRAM_HI  ((volatile u8*)(0x26000000))
#define DMA_MEM  ((volatile u8*)(0xFFFFFF80))

/*----- SH2 defines -------------------------------------------------*/
// > anything specific to the SH2's defined here. DMA, registers, etc.
// >>>> CPU
// get the SH2's status register
INLINE u32 sh2_getSR()
{
  u32 sr; asm("STC SR,%[out]":[out]"=r"(sr));
  return sr;
}
// set the SH2's status register
INLINE void sh2_setSR(u32 SR)
{ asm("LDC %[inp],SR"::[inp]"r"(SR)); }

// get the SH2's interrupt mask
INLINE u32 sh2_getintmask()
{ return (sh2_getSR()>>4)&0xF; }
// set the SH2's interrupt mask
INLINE void sh2_setintmask(u32 i)
{
  u32 intmask = i&0xF;
  u32 SR = sh2_getSR() & 0xFFFFFF0F; // clear previous intmask.
  SR |= (intmask<<4);
  sh2_setSR(SR);
}

// >>>> DMA
typedef struct CH_DMA {
  u32 src;                         /* $00 (SAR, source address)      */
  u32 dst;                         /* $04 (DAR, destination address) */
  u32 cnt;                         /* $08 (TCR, transfer count)      */
  u32 ctrl;                        /* $0C (CHCR, control, i guess)   */
} PACKED CH_DMA;

typedef enum dma_sizes {
  DMA_SIZE_BYTE,                                  /* $01 byte.       */
  DMA_SIZE_WORD,                                  /* $02 bytes.      */
  DMA_SIZE_LONG,                                  /* $04 bytes.      */
  DMA_SIZE_128B                                   /* $10 bytes!      */
} dma_sizes;

// DMA channels, operation register, control registers
#define DMA_REG ((volatile CH_DMA*)DMA_MEM)
#define DMA_REG_OR ((volatile u32*)(0xFFFFFFB0))
#define DMA_REG_CR ((volatile u8*)(0xFFFFFE71))
// DMA destination is fixed, dest is incremented, dest is decremented
// > used for changing the direction that the destination
// > address goes during a transfer.
#define DMA_DST_FIX (0<<14)
#define DMA_DST_INC (1<<14)
#define DMA_DST_DEC (2<<14)
// DMA source fixed, src increments, src decrements
// > same as the above, except with the source address.
#define DMA_SRC_FIX (0<<12)
#define DMA_SRC_INC (1<<12)
#define DMA_SRC_DEC (2<<12)
// DMA size
// > controls the size of each unit transferred.
// > $0 is 1 byte, $1 is 2 bytes, $2 is 4 bytes, $3 is 16 bytes. 
#define DMA_SIZE(n) (((n)&3)<<10)

// DMA auto-request, interrupt enable, transfer-end, enable
#define DMA_AR (1<<9)
#define DMA_IE (1<<2)
#define DMA_TE (1<<1)
#define DMA_ON (1)

// DMA master enable,NMIF,address error, priority
#define DMA_DME (1)
#define DMA_NMIF (1<<1)
#define DMA_AE (1<<2)
#define DMA_PR (1<<3)

// stop all DMA transfers
INLINE void sh2_dma_stop()
{ *DMA_REG_OR &= (DMA_AE | DMA_PR | DMA_NMIF); }
// initialize DMA channels
INLINE void sh2_dma_init()
{
  sh2_dma_stop();
  DMA_REG[0].ctrl = 0;
  DMA_REG[1].ctrl = 0;
}
// check if DMA channel `ch` is active
INLINE u32 sh2_dma_active(u32 ch)
{ return (DMA_REG[ch].ctrl&DMA_TE); }
// blasts from `src` to `dst`, in `cnt` amount of transfers, using `mode`.
INLINE void sh2_dma_blast(u32 ch, const void *src,const void *dst,u32 cnt,u32 mode)
{
  // "why are you using assembly?" it's easier to debug in an emu, trust me.
  // alternatively, you may comment this part out and just use
  // the C section (ha) below.
  asm(
    // move src,dst,cnt
    "MOV.L %[src],@(0,%[ch])\t\n"
    "MOV.L %[dst],@(4,%[ch])\t\n"
    "MOV.L %[cnt],@(8,%[ch])\t\n"
    // REG_CR[ch] = 0
    "MOV #0,R0\t\n"
    "MOV.B R0,@%[cr]\t\n"
    // move ctrl to DMA_REG[ch].ctrl
    "MOV.L @(12,%[ch]),R0\t\n" // read from ctrl before writing.
    "MOV.L %[mode],@(12,%[ch])\t\n"
    :: [ch]"r"(((u32)&DMA_REG[ch])),[src]"r"(src),
    [dst]"r"(dst),[cnt]"r"(cnt),
    [mode]"r"(mode),[cr]"r"(((u32)&DMA_REG_CR[ch]))
  ); // */
  
  /* DMA_REG[ch].src = (u32)src;
  DMA_REG[ch].dst = (u32)dst;
  DMA_REG[ch].cnt = cnt;
  DMA_REG_CR[ch] = 0; // external request
  DMA_REG[ch].ctrl = (DMA_REG[ch].ctrl&0) | mode; // */
	while(sh2_dma_active(ch));
}

INLINE void sh2_dma_cpy(u32 ch,const void *src,const void *dst,u32 cnt)
{ sh2_dma_blast(ch,src,dst,cnt>>1,DMA_SRC_INC|DMA_DST_INC|DMA_ON|DMA_SIZE(1)); }

/* DMA example: copy $10 bytes from WRAM_LO[$000000] to WRAM_LO[$000100]
  * sh2_dma_init();
  * DMA_REG_OR = DMA_DME | DMA_PR;
  *
  * sh2_dma_blast(0,
  *   &WRAM_LO[0],&WRAM_LO[0x100],0x10,
  *   DMA_SRC_INC|DMA_DST_INC|DMA_AR|DMA_ON
  * );
  * // DMA_SRC_INC is so the source address increases
  * // DMA_DST_INC is so the destination address increases
  * // DMA_AR is so it the transfer occurs immediately...?
  ** idk what its actually for but if u dont put it in it doesn't copy
*/

/*---- VDP1 defines -------------------------------------------------*/
// > definitions for the VDP1.

typedef struct VDP1_IO {
  u16 TVMR;           /* TV mode selection                           */
  u16 FBCR;           /* framebuffer control                         */
  u16 PTMR;           /* plot trigger                                */
  u16 EWDR;           /* erase/write color                           */
  u16 EWLR;           /* erase/write upper-left pos                  */
  u16 EWRR;           /* erase/write lower-right pos                 */
  u16 ENDR;           /* force draw end                              */
  u16 EDSR;           /* transfer end status                         */
  u16 LOPR;           /* returns last command table processed        */
  u16 COPR;           /* returns current command table processed     */
  u16 MODR;           /* mode status                                 */
} PACKED vdp_io;

#define VDP1_VRAM     ((volatile u8*)(VDP1_MEM + 0x000000))
#define VDP1_FB       ((volatile u8*)(VDP1_MEM + 0x080000))
#define VDP1_REG_IO   ((volatile VDP1_IO*)(VDP1_MEM + 0x100000))

// >>>> commands
typedef enum vdp1_comm {
	// textured draw commands
	comm_normspr = 0x0,
	comm_scalspr = 0x1,
	comm_distspr = 0x2,
	// non-textured draw commands
	comm_polygon = 0x4,
	comm_polline = 0x5,
	comm_line    = 0x6,
	// register set commmands
	comm_usrclip = 0x8,
	comm_sysclip = 0x9,
	comm_cordset = 0xA
} vdp1_comm;

typedef enum VDP1_CLRMODE {
	VDP1_CLRMODE_PAL16BNK,
	VDP1_CLRMODE_PAL16LUT,
	VDP1_CLRMODE_PAL64,
	VDP1_CLRMODE_PAL128,
	VDP1_CLRMODE_PAL256,
	VDP1_CLRMODE_RGB
} VDP1_CLRMODE;

#define CMDCTRL_COMM(n) ((n)&15)
#define CMDCTRL_DIR(n)  (((n)&3)<<4)
#define CMDCTRL_ZP(n)   (((n)&15)<<8)
#define CMDCTRL_JP(n)   (((n)&7)<<12)
#define CMDCTRL_END     (1<<15)

#define CMDPMOD_CCB(n) ((n)&7)
#define CMDPMOD_CLR(n) (((n)&7)<<3)
#define CMDPMOD_SPD (1<<6)
#define CMDPMOD_ECD (1<<7)
#define CMDPMOD_MESH (1<<8)
#define CMDPMOD_CMOD (1<<9)
#define CMDPMOD_CLIP (1<<10)
#define CMDPMOD_PCLP (1<<11)
#define CMDPMOD_HSS (1<<12)
#define CMDPMOD_MON (1<<15)

#define VDP1_CMDSIZE(w,h) ( (((w)>>3)<<8) | ((h)&0xFF) )

typedef struct VDP1_CMD {
	u16  cmdctrl;
	u16  cmdlink;
	u16  cmdpmod;
	u16  cmdcolr;
	u16  cmdsrca;
	u16  cmdsize;
	s16  cmdxa,cmdya;
	s16  cmdxb,cmdyb;
	s16  cmdxc,cmdyc;
	s16  cmdxd,cmdyd;
	u16  cmdgrda;
	u16  dummy;
} PACKED VDP1_CMD;

INLINE void vdp1_cmd_cpy(const void *src,u32 index)
{ sh2_dma_cpy(0,src,&((VDP1_CMD*)VDP1_VRAM)[index],sizeof(VDP1_CMD)); }

/*---- VPD2 defines -------------------------------------------------*/

typedef u8 VDP2_BANK[0x20000];
#define VDP2_VRAM ((volatile VDP2_BANK*)VDP2_MEM);
#define VDP2_CRAM ((volatile u8*)(VDP2_MEM + 0x100000))
#define VDP2_REG_IO   ((volatile u8*)(VDP2_MEM + 0x180000))

#define VDP2_SPRPRIORITY ((volatile u16*)(VDP2_REG_IO + 0x0000F0))
#define VDP_PRIORITY(a,b) ( ((a)&7) | (((b)&7)<<8) )

/*---- CD defines ---------------------------------------------------*/
// > NOTE: i have no clue on what the hell half of these registers do
// > or what they're even for, just hope for the best.
typedef struct CD_CR {
  u16 CR1,dummy1;
  u16 CR2,dummy2;
  u16 CR3,dummy3;
  u16 CR4,dummy4;
} PACKED CD_CR;

typedef struct CD_CMD {
  u16 CR1,CR2,CR3,CR4;
} PACKED CD_CMD;

typedef enum CD_HIRQ {
  CD_HIRQ_CMOK = 0x0001, // (1<<0 ) 
  CD_HIRQ_DRDY = 0x0002, // (1<<1 )
  CD_HIRQ_CSCT = 0x0004, // (1<<2 )
  CD_HIRQ_BFUL = 0x0008, // (1<<3 )
  CD_HIRQ_PEND = 0x0010, // (1<<4 )
  CD_HIRQ_DCHG = 0x0020, // (1<<5 )
  CD_HIRQ_ESEL = 0x0040, // (1<<6 )
  CD_HIRQ_EHST = 0x0080, // (1<<7 )
  CD_HIRQ_ECPY = 0x0100, // (1<<8 )
  CD_HIRQ_EFLS = 0x0200, // (1<<9 )
  CD_HIRQ_SCDQ = 0x0400 // (1<<10)
} CD_HIRQ;

typedef enum CD_STATUS {
  CD_STAT_WAIT    = 0x80,
  CD_STAT_REJECT  = 0xFF
} CD_STATUS;

#define CD_REG_BASE    (0x25890000)
#define CD_REG_HIRQREQ ((volatile u16*)(CD_REG_BASE + 0x0008))
#define CD_REG_HIRQMSK ((volatile u16*)(CD_REG_BASE + 0x000C))
#define CD_REG_CR      ((volatile CD_CR*)(CD_REG_BASE + 0x0018))
#define CD_REG_MPEGRGB ((volatile u16*)(CD_REG_BASE + 0x0028))

// write a command to the CD command registers
INLINE void kcd_writecmd(CD_CMD *cmd)
{
  CD_REG_CR->CR1 = cmd->CR1;
  CD_REG_CR->CR2 = cmd->CR2;
  CD_REG_CR->CR3 = cmd->CR3;
  CD_REG_CR->CR4 = cmd->CR4;
}
// read the CD command registers
INLINE void kcd_readCR(CD_CMD *cmd)
{
  cmd->CR1 = CD_REG_CR->CR1; 
  cmd->CR2 = CD_REG_CR->CR2;
  cmd->CR3 = CD_REG_CR->CR3;
  cmd->CR4 = CD_REG_CR->CR4;
}

INLINE u32 kcd_execcmd(u16 hirqmask,CD_CMD *cmd,CD_CMD *cmdread)
{
  u32 old_intmask;
  u32 hirq_buf;
  u16 cdstatus;
  u32 cmdtime;
  // mask all interrupts, we don't want interruptions.
  old_intmask = sh2_getintmask();
  sh2_setintmask(0xF);
  hirq_buf = *CD_REG_HIRQREQ;
  // if CMOK isn't set, this means we can't issue a command,
  // so we'll exit if that's the case.
  if(!(hirq_buf & CD_HIRQ_CMOK)) return 0;
  // clear CMOK and other flags
  *CD_REG_HIRQREQ = ~(hirqmask | CD_HIRQ_CMOK);
  // now we write the command!
  kcd_writecmd(cmd);
  // wait until command's finished...
  for(cmdtime = 0; cmdtime < 0x240000; cmdtime++)
  {
    hirq_buf = *CD_REG_HIRQREQ;
    if(hirq_buf & CD_HIRQ_CMOK) break;
  }
  if(!(hirq_buf & CD_HIRQ_CMOK)) return 0;
  // return the command's data
  kcd_readCR(cmdread);
  cdstatus = cmdread->CR1>>8;
  // was the command fine?
  if(cdstatus == CD_STAT_REJECT) return 0;
  if(cdstatus & CD_STAT_WAIT) return 0;
  // return interrupts to normal.
  sh2_setintmask(old_intmask);
  
  return 1; // all good!
}


INLINE u32 kcd_abort()
{
  CD_CMD cmdW = { .CR1 = 0x7500 };
  CD_CMD cmdR;
  return kcd_execcmd(CD_HIRQ_EFLS,&cmdW,&cmdR);
}
INLINE u32 kcd_init(u32 standby)
{
  CD_CMD cmdW = { .CR1 = 0x0400,.CR2 = standby,.CR4 = 0x040F };
  CD_CMD cmdR;
  return kcd_execcmd(0,&cmdW,&cmdR);
}

#endif
